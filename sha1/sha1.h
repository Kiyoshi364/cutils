#ifndef _HASHI_SHA1_H_
#define _HASHI_SHA1_H_

#include <stdint.h>

#define SHA1_CHUNK_LEN 64
#define SHA1_CHUNK_INDEX_MASK (SHA1_CHUNK_LEN - 1)
#define SHA1_CHUNK_INDEX(x) ((x) & SHA1_CHUNK_INDEX_MASK)

#define SHA1_DIGEST_LEN 20

#define SHA1_S_LEN 5

typedef struct {
    uint8_t buf[SHA1_CHUNK_LEN];
    uint32_t s[SHA1_S_LEN];
    uint64_t size;
} Sha1State;

void sha1_hash(const uint8_t data[], uint64_t size, uint8_t out[SHA1_DIGEST_LEN]);

void sha1_init(Sha1State *s);

void sha1_update(Sha1State *s, const uint8_t data[], uint64_t size);
void sha1_finish(Sha1State *s, uint8_t out[SHA1_DIGEST_LEN]);

#endif /* _HASHI_SHA1_H_ */

#ifdef HASHI_SHA1_EXE

#ifndef _HASHI_SHA1_EXE_
#define _HASHI_SHA1_EXE_
#define HASHI_SHA1_IMPLEMENTATION

#include <stdio.h>

#include <assert.h>

void run(Sha1State *s, FILE *fin, const char *filename, uint8_t *buf, uint64_t buf_len, uint8_t out[SHA1_DIGEST_LEN]) {
    sha1_init(s);

    while (!feof(fin)) {
        size_t n = fread(buf, sizeof(buf[0]), buf_len, fin);
        sha1_update(s, buf, n);
    }

    sha1_finish(s, out);

    for (uint8_t i = 0; i < SHA1_DIGEST_LEN; i += 1) {
        printf("%02hhx", out[i]);
    }
    printf("  %s\n", filename);
}

int main(const int argc, const char *argv[]) {
    Sha1State s;
    uint8_t buf[SHA1_CHUNK_LEN];
    uint8_t out[SHA1_DIGEST_LEN];

    if(1 < argc) {
        for (int i = 1; i < argc; i += 1) {
            const char *filename = argv[i];
            const uint8_t is_stdin = filename[0] == '-' && filename[1] == '\0';
            FILE *fin = stdin;
            if (!is_stdin) {
                fin = fopen(filename, "rb");
            }
            assert(fin);

            run(&s, fin, filename, buf, sizeof(buf)/sizeof(buf[0]), out);

            if (!is_stdin) {
                fclose(fin);
            }
        }
    } else {
        run(&s, stdin, "-", buf, sizeof(buf)/sizeof(buf[0]), out);
    }
    return 0;
}
#endif /* _HASHI_SHA1_EXE_ */
#endif /* HASHI_SHA1_EXE */

#ifdef HASHI_SHA1_IMPLEMENTATION
#ifndef _HASHI_SHA1_IMPL_
#define _HASHI_SHA1_IMPL_

/* Resources:
 * - https://en.wikipedia.org/wiki/SHA-1
 * - zig std 0.11.0: lib/std/crypto/sha1.zig
 */

#include <assert.h>

void sha1_hash(const uint8_t data[], uint64_t size, uint8_t out[SHA1_DIGEST_LEN]) {
    Sha1State s;
    sha1_init(&s);
    sha1_update(&s, data, size);
    sha1_finish(&s, out);
}

void sha1_init(Sha1State *s) {
    s->s[4] = 0x67452301; /* a */
    s->s[3] = 0xEFCDAB89; /* b */
    s->s[2] = 0x98BADCFE; /* c */
    s->s[1] = 0x10325476; /* d */
    s->s[0] = 0xC3D2E1F0; /* e */
    s->size = 0;
}

static inline
void sha1__round(uint32_t s[SHA1_S_LEN], const uint8_t chunk[SHA1_CHUNK_LEN]) {
    #define SHA1_ROUND_INDEX(i) ((i) & ((SHA1_CHUNK_LEN/4) - 1))
    #define SHA1_ROTL_32(x, i) ((((x) & ((1U << (32-(i))) - 1)) << (i)) | (((x) >> (32-(i))) & ((1U << (i)) - 1)))

    uint32_t ns[SHA1_S_LEN];
    uint32_t w[SHA1_CHUNK_LEN/4];

    for (uint8_t i = 0; i < SHA1_S_LEN; i += 1) {
        ns[i] = s[i];
    }

    for (uint8_t i = 0; i < SHA1_CHUNK_LEN/4; i += 1) {
        w[i] = 0;
        for (uint8_t j = 0; j < 4; j += 1) {
            w[i] |= chunk[(i*4) + j] << (8*(4-1-j));
        }
    }

    {
        #define SHA1_ROUND(len, k, in_f) do { \
            for (; i < (len); i += 1) { \
                const uint8_t e = (i+0) % SHA1_S_LEN; \
                const uint8_t d = (i+1) % SHA1_S_LEN; \
                const uint8_t c = (i+2) % SHA1_S_LEN; \
                const uint8_t b = (i+3) % SHA1_S_LEN; \
                const uint8_t a = (i+4) % SHA1_S_LEN; \
                \
                const uint32_t ra = SHA1_ROTL_32(ns[a], 5); \
                const uint32_t f = (in_f); \
                \
                w[SHA1_ROUND_INDEX(i)] = SHA1_ROTL_32(w[SHA1_ROUND_INDEX(i-3)] ^ w[SHA1_ROUND_INDEX(i-8)] ^ w[SHA1_ROUND_INDEX(i-14)] ^ w[SHA1_ROUND_INDEX(i-16)], 1); \
                ns[b] = SHA1_ROTL_32((ns[b]), 30); \
                ns[e] = ra + f + ns[e] + (k) + w[SHA1_ROUND_INDEX(i)]; \
            } \
        } while(0)

        uint8_t i = 0;
        for (; i < 16; i += 1) {
            const uint8_t e = (i+0) % SHA1_S_LEN;
            const uint8_t d = (i+1) % SHA1_S_LEN;
            const uint8_t c = (i+2) % SHA1_S_LEN;
            const uint8_t b = (i+3) % SHA1_S_LEN;
            const uint8_t a = (i+4) % SHA1_S_LEN;

            const uint32_t ra = SHA1_ROTL_32(ns[a], 5);
            const uint32_t f = ((ns[b] & ns[c]) | ((~ns[b]) & ns[d]));
            const uint32_t k = 0x5A827999;

            ns[b] = SHA1_ROTL_32(ns[b], 30);
            ns[e] = ra + f + ns[e] + k + w[i];
        }
        SHA1_ROUND(20, 0x5A827999, ((ns[b] & ns[c]) | ((~ns[b]) & ns[d])));
        SHA1_ROUND(40, 0x6ED9EBA1, (ns[b] ^ ns[c] ^ ns[d]));
        SHA1_ROUND(60, 0x8F1BBCDC, ((ns[b] & ns[c]) | (ns[b] & ns[d]) | (ns[c] & ns[d])));
        SHA1_ROUND(80, 0xCA62C1D6, (ns[b] ^ ns[c] ^ ns[d]));

        assert(i == 80);
        #undef SHA1_ROUND
    }

    for (uint8_t i = 0; i < SHA1_S_LEN; i += 1) {
        s[i] += ns[i];
    }
    #undef SHA1_ROTL_32
    #undef SHA1_ROUND_INDEX
}

void sha1_update(Sha1State *s, const uint8_t data[], uint64_t size) {
    uint64_t i = 0;

    if (0 < SHA1_CHUNK_INDEX(s->size) && SHA1_CHUNK_LEN <= SHA1_CHUNK_INDEX(s->size) + size) {
        for (; SHA1_CHUNK_INDEX(s->size) + i < SHA1_CHUNK_LEN; i += 1) {
            s->buf[SHA1_CHUNK_INDEX(s->size) + i] = data[i];
        }
        sha1__round(s->s, s->buf);
    }

    for (; i + SHA1_CHUNK_LEN <= size; i += SHA1_CHUNK_LEN) {
        sha1__round(s->s, data + i);
    }

    for (uint8_t j = 0; i + j < size; j += 1) {
        s->buf[j] = data[i + j];
    }

    s->size += size;
}

void sha1_finish(Sha1State *s, uint8_t out[SHA1_DIGEST_LEN]) {
    assert(SHA1_CHUNK_INDEX(s->size) < SHA1_CHUNK_LEN);
    s->buf[SHA1_CHUNK_INDEX(s->size)] = 0x80;

    for (uint8_t i = SHA1_CHUNK_INDEX(s->size) + 1; i < SHA1_CHUNK_LEN; i += 1) {
        s->buf[i] = 0;
    }

    if (SHA1_CHUNK_LEN - 9 < SHA1_CHUNK_INDEX(s->size)) {
        sha1__round(s->s, s->buf);
        for (uint8_t i = 0; i < SHA1_CHUNK_INDEX(s->size) + 1; i += 1) {
            s->buf[i] = 0;
        }
    }

    {
        uint64_t size = (s->size) << 3;
        for (uint8_t i = 0; i < 8; i += 1) {
            s->buf[SHA1_CHUNK_LEN - 1 - i] = (size >> (8*i)) & 0xFF;
        }
    }

    sha1__round(s->s, s->buf);

    for (uint8_t i = 0; i < SHA1_S_LEN; i += 1) {
        const uint8_t idx = 4*(SHA1_S_LEN-1-i);
        for (uint8_t j = 0; j < 4; j += 1) {
            out[idx + j] = (s->s[i] >> (8*(4-1-j))) & 0xFF;
        }
    }
}

#endif /* _HASHI_SHA1_IMPL_ */
#endif /* HASHI_SHA1_IMPLEMENTATION */

/* From https://en.wikipedia.org/wiki/SHA-1

Note 1: All variables are unsigned 32-bit quantities and wrap modulo 2^32 when calculating, except for
        ml, the message length, which is a 64-bit quantity, and
        hh, the message digest, which is a 160-bit quantity.
Note 2: All constants in this pseudo code are in big endian.
        Within each word, the most significant byte is stored in the leftmost byte position

Initialize variables:

h0 = 0x67452301
h1 = 0xEFCDAB89
h2 = 0x98BADCFE
h3 = 0x10325476
h4 = 0xC3D2E1F0

ml = message length in bits (always a multiple of the number of bits in a character).

Pre-processing:
append the bit '1' to the message e.g. by adding 0x80 if message length is a multiple of 8 bits.
append 0 ≤ k < 512 bits '0', such that the resulting message length in bits
   is congruent to −64 ≡ 448 (mod 512)
append ml, the original message length in bits, as a 64-bit big-endian integer. 
   Thus, the total length is a multiple of 512 bits.

Process the message in successive 512-bit chunks:
break message into 512-bit chunks
for each chunk
    break chunk into sixteen 32-bit big-endian words w[i], 0 ≤ i ≤ 15

    Message schedule: extend the sixteen 32-bit words into eighty 32-bit words:
    for i from 16 to 79
        Note 3: SHA-0 differs by not having this leftrotate.
        w[i] = (w[i-3] xor w[i-8] xor w[i-14] xor w[i-16]) leftrotate 1

    Initialize hash value for this chunk:
    a = h0
    b = h1
    c = h2
    d = h3
    e = h4

    Main loop:
    for i from 0 to 79
        if 0 ≤ i ≤ 19 then
            f = (b and c) or ((not b) and d)
            k = 0x5A827999
        else if 20 ≤ i ≤ 39
            f = b xor c xor d
            k = 0x6ED9EBA1
        else if 40 ≤ i ≤ 59
            f = (b and c) or (b and d) or (c and d) 
            k = 0x8F1BBCDC
        else if 60 ≤ i ≤ 79
            f = b xor c xor d
            k = 0xCA62C1D6

        temp = (a leftrotate 5) + f + e + k + w[i]
        e = d
        d = c
        c = b leftrotate 30
        b = a
        a = temp

    Add this chunk's hash to result so far:
    h0 = h0 + a
    h1 = h1 + b 
    h2 = h2 + c
    h3 = h3 + d
    h4 = h4 + e

Produce the final hash value (big-endian) as a 160-bit number:
hh = (h0 leftshift 128) or (h1 leftshift 96) or (h2 leftshift 64) or (h3 leftshift 32) or h4
*/
