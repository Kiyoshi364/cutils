#include <stdio.h>
#include <assert.h>

#define ARRLEN(x) (sizeof(x)/sizeof((x)[0]))

typedef unsigned char byte;
typedef unsigned short halfword;

typedef struct {
    halfword head;
    byte follow;
} SymbolDef;

typedef enum {
    OUTPUT_BITS,
    OUTPUT_DEBUG,
} OutputMode;

struct GlobalState {
    byte output_mode; /* OutputMode */
    byte outbuf;
    byte outbuf_size;
} globals;

static byte magic[3] = { 0x1F, 0x9D, 0x90 };
static byte readbuf[0x100];

// TODO: separate into an array of pointers
// TODO: use exponential allocation
SymbolDef symbol_table[0xFF * 0x100];
byte symbol_table_meta_size = 1;
byte symbol_table_last_size = 1;

static
void inner_write(FILE *fout, const byte *buf, halfword len) {
    while (0 < len) {
        const halfword n = fwrite(buf, sizeof(*buf), len, fout);
        len -= n;
    }
}

static
halfword inner_read(FILE *fin, byte *buf, halfword len) {
    return fread(buf, sizeof(*buf), len, fin);
}

halfword table_len(void) {
    return symbol_table_meta_size*0x100 + symbol_table_last_size;
}

SymbolDef table_get_symbol(halfword s) {
    const halfword idx = s - 0x100;
    return symbol_table[idx];
}

halfword table_curr_symbol(void) {
    const halfword idx = (symbol_table_meta_size-1)*0x100 + symbol_table_last_size;
    return symbol_table[idx].head;
}

void table_write_curr_symbol(halfword head) {
    const halfword idx = (symbol_table_meta_size-1)*0x100 + symbol_table_last_size;
    assert(symbol_table[idx].head < head);
    symbol_table[idx].head = head;
}

halfword table_find_or_add_symbol(halfword head, byte follow) {
    assert(head < table_len());
    // TODO: start latter, based on head
    // i = head >> 8;
    // j = head & 0xFF;
    // ?
    halfword i = 1;
    halfword j = 1;
    for (; i < symbol_table_meta_size; i += 1, j = 0) {
        for (; j < 0x100; j += 1) {
            const halfword idx = (i-1)*0x100 + j;
            const SymbolDef entry = symbol_table[idx];
            if (entry.head == head && entry.follow == follow) {
                return idx + 0x100;
            }
        }
    }

    for (; j < symbol_table_last_size; j += 1) {
        const halfword idx = (i-1)*0x100 + j;
        const SymbolDef entry = symbol_table[idx];
        if (entry.head == head && entry.follow == follow) {
            return idx + 0x100;
        }
    }

    const halfword idx = (symbol_table_meta_size-1)*0x100 + symbol_table_last_size;
    symbol_table[idx] = (SymbolDef){
        .head = head,
        .follow = follow,
    };
    symbol_table_last_size += 1;
    if (symbol_table_last_size == 0) {
        symbol_table_meta_size += 1;
    }
    return idx + 0x100;
}

byte msb(byte n) {
    byte ans = 0;
    while (1 < n) {
        n >>= 1;
        ans += 1;
    }
    return ans;
}

halfword table_numbits(void) {
    return 9 + msb(symbol_table_meta_size);
}

void output_symbol_debug(FILE *fout, byte num_bits, halfword s) {
    if (s < 0x100) {
        if (0x19 < s && s < 0x7F) {
            fprintf(fout, "%hhu<0x%02X|%c>\n", num_bits, s, s);
        } else {
            fprintf(fout, "%hhu<0x%02X>\n", num_bits, s);
        }
    } else if (s == 0x100) {
        fprintf(fout, "%hhu[Escape]\n", num_bits);
    } else {
        fprintf(fout, "%hhu[0x%04X]\n", num_bits, s);
    }
}

void output_symbol(FILE *fout, halfword s) {
    const halfword num_bits = table_numbits();
    switch ((OutputMode) globals.output_mode) {
        case OUTPUT_BITS: {
            byte bits_left = num_bits;
            byte buf = globals.outbuf;
            byte size = globals.outbuf_size;
            while (0 < bits_left) {
                buf = ((s << size) | buf) & 0xFF;
                s >>= (8 - size);
                if (bits_left + size < 8) {
                    size += bits_left;
                    bits_left = 0;
                } else {
                    inner_write(fout, &buf, 1);
                    bits_left = bits_left - (8 - size);
                    buf = 0;
                    size = 0;
                }
            }
            globals.outbuf = buf;
            globals.outbuf_size = size;
        } break;
        case OUTPUT_DEBUG: {
            output_symbol_debug(fout, num_bits, s);
        } break;
    }
}

void output_curr_symbol(FILE *fout) {
    const halfword curr_symbol = table_curr_symbol();
    output_symbol(fout, curr_symbol);
}

void output_header(FILE *fout) {
    switch ((OutputMode) globals.output_mode) {
        case OUTPUT_BITS: {
            inner_write(fout, magic, ARRLEN(magic));
        } break;
        case OUTPUT_DEBUG: {
            fprintf(fout, "MAGIC\n");
        } break;
    }
}

byte read_header(FILE *fin) {
    byte i = 0;
    byte ret = 1;
    while (i < ARRLEN(magic)) {
        byte n = inner_read(fin, readbuf + i, ARRLEN(magic) - i);
        for (; 0 < n; n -= 1) {
            if (readbuf[i] != magic[i]) {
                fprintf(stderr, "read_header: byte %d, expected 0x%02X found 0x%02X\n", i, magic[i], readbuf[i]);
                ret = 0;
            }
            i += 1;
        }
    }
    return ret;
}

void output_flush(FILE *fout) {
    switch ((OutputMode) globals.output_mode) {
        case OUTPUT_BITS: {
            const byte buf = globals.outbuf;
            inner_write(fout, &buf, 1);
            globals.outbuf = 0;
            globals.outbuf_size = 0;
        } break;
        case OUTPUT_DEBUG: {
            fprintf(fout, "EOF\n");
        } break;
    }
}

halfword lzw_step(byte b, byte *first_round) {
    const halfword curr_symbol = table_curr_symbol();
    if (*first_round) {
        *first_round = 0;
        table_write_curr_symbol(b);
        return b;
    } else {
        const halfword len = table_len();
        const halfword new_symbol = table_find_or_add_symbol(curr_symbol, b);
        if (new_symbol < len) {
            table_write_curr_symbol(new_symbol);
        } else {
            table_write_curr_symbol(b);
        }
        return new_symbol;
    }
}

// TODO: Emmit Escape to reset symbol_table
int compress(FILE *fin, FILE *fout) {
    byte first_round = 1;
    output_header(fout);
    while (!feof(fin)) {
        halfword n = inner_read(fin, readbuf, ARRLEN(readbuf));
        for (halfword i = 0; i < n; i += 1) {
            const byte b = readbuf[i];
            const halfword len = table_len();
            const halfword curr_symbol = table_curr_symbol();
            const halfword new_symbol = lzw_step(b, &first_round);
            assert(curr_symbol < len);
            assert(new_symbol <= len);
            if (new_symbol < len) {
                // Empty
            } else {
                output_symbol(fout, curr_symbol);
            }
        }
    }
    output_curr_symbol(fout);
    output_flush(fout);
    return 0;
}

void decompress_symbol_rec(FILE *fout, halfword s, byte *inout_first_round) {
    assert(s != 0x100);
    byte b = (byte) s;
    if (0x100 < s) {
        const SymbolDef def = table_get_symbol(s);
        decompress_symbol_rec(fout, def.head, inout_first_round);
        b = def.follow;
    }
    inner_write(fout, &b, 1);
    lzw_step(b, inout_first_round);
}

void decompress_symbol(FILE *fout, halfword s, byte *inout_first_round) {
    if (s == 0x100) {
        symbol_table_meta_size = 1;
        symbol_table_last_size = 1;
        return;
    }

    byte b = (byte) s;
    if (0x100 < s) {
        const SymbolDef def = table_get_symbol(s);
        decompress_symbol_rec(fout, def.head, inout_first_round);
        b = def.follow;
    }
    inner_write(fout, &b, 1);
    lzw_step(b, inout_first_round);
}

int decompress(FILE *fin, FILE *fout) {
    halfword buf = 0;
    halfword buf_size = 0;
    byte first_round = 1;
    assert(read_header(fin));
    while (!feof(fin)) {
        halfword n = inner_read(fin, readbuf, ARRLEN(readbuf));
        for (halfword i = 0; i < n; i += 1) {
            const byte by = readbuf[i];
            buf |= (by << buf_size);
            buf_size += 8;
            const halfword num_bits = table_numbits();
            if (buf_size < num_bits) {
                continue;
            } else {
                halfword s = buf & ((1 << num_bits) - 1);
                buf >>= num_bits;
                buf_size -= num_bits;
                decompress_symbol(fout, s, &first_round);
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    FILE *fin = stdin;
    FILE *fout = stdout;

    globals = (struct GlobalState){
        .output_mode = OUTPUT_BITS,
        .outbuf = 0,
        .outbuf_size = 0,
    };

    if (1 < argc && argv[1][0] == '-' && argv[1][1] == 'd' && argv[1][2] == '\0') {
        return decompress(fin, fout);
    } else {
        return compress(fin, fout);
    }
}
