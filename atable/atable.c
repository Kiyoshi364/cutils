/* atable (ASCII Table)
 *
 * Prints all unicode characters (utf-8 encoded)
 * that fit in 1 byte.
 *
 * Copyright (C) 2023 Daniel K Hashimoto
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef unsigned char byte;

#define UTF8BUFLEN 5
#define UTF8BUFLENSTR "5"
void toutf8(const byte c, char buf[static UTF8BUFLEN]) {
    static const char NPA[0x20][UTF8BUFLEN] = {
        [0x00] = "NUL ", [0x01] = "SOH ",
        [0x02] = "STX ", [0x03] = "ETX ",
        [0x04] = "EOT ", [0x05] = "ENQ ",
        [0x06] = "ACK ", [0x07] = "BEL ",
        [0x08] = "BS  ", [0x09] = "HT  ",
        [0x0A] = "LF  ", [0x0B] = "VT  ",
        [0x0C] = "FF  ", [0x0D] = "CR  ",
        [0x0E] = "SO  ", [0x0F] = "SI  ",
        [0x10] = "DLE ", [0x11] = "DC1 ",
        [0x12] = "DC2 ", [0x13] = "DC3 ",
        [0x14] = "DC4 ", [0x15] = "NAK ",
        [0x16] = "SYN ", [0x17] = "ETB ",
        [0x18] = "CAN ", [0x19] = "EM  ",
        [0x1A] = "SUB ", [0x1B] = "ESC ",
        [0x1C] = "FS  ", [0x1D] = "GS  ",
        [0x1E] = "RS  ", [0x1F] = "US  ",
    };
    static const char ECC[0x21][UTF8BUFLEN] = {
        [0x00] = "DEL ",
        [0x01] = "PAD ", [0x02] = "HOP ",
        [0x03] = "BPH ", [0x04] = "NBH ",
        [0x05] = "IND ", [0x06] = "NEL ",
        [0x07] = "SSA ", [0x08] = "ESA ",
        [0x09] = "HTS ", [0x0A] = "HTJ ",
        [0x0B] = "VTS ", [0x0C] = "PLD ",
        [0x0D] = "PLU ", [0x0E] = "RI  ",
        [0x0F] = "SS2 ", [0x10] = "SS3 ",
        [0x11] = "DCS ", [0x12] = "PU1 ",
        [0x13] = "PU2 ", [0x14] = "STS ",
        [0x15] = "CCH ", [0x16] = "MW  ",
        [0x17] = "SPA ", [0x18] = "EPA ",
        [0x19] = "SOS ", [0x1A] = "SGCI",
        [0x1B] = "SCI ", [0x1C] = "CSI ",
        [0x1D] = "ST  ", [0x1E] = "OSC ",
        [0x1F] = "PM  ", [0x20] = "APC ",
    };
    for ( byte i = 0; i < UTF8BUFLEN; i += 1 ) {
        buf[i] = '*';
    }
    if ( c < 0x20 ) {
        // Note: Non-Printable ASCII (Control Codes)
        for ( byte i = 0; i < UTF8BUFLEN; i += 1 ) {
            buf[i] = NPA[c][i];
        }
    } else if ( c < 0x7F ) {
        // Note: ASCII
        buf[0] = '\'';
        buf[1] = c;
        buf[2] = '\'';
        for ( byte i = 3; i < UTF8BUFLEN - 1; i += 1 ) {
            buf[i] = ' ';
        }
        buf[UTF8BUFLEN - 1] = '\0';
    } else if ( c < 0xA0 ) {
        // Note: DEL (0x7F), Extended Control Codes
        for ( byte i = 0; i < UTF8BUFLEN; i += 1 ) {
            buf[i] = ECC[c - 0x7F][i];
        }
    } else {
        // Note: Extended ASCII
        buf[0] = '\'';
        buf[1] = 0xC0 | (c >> 6);
        buf[2] = 0x80 | (c & 0x3F);
        buf[3] = '\'';
        for ( byte i = 4; i < UTF8BUFLEN; i += 1 ) {
            buf[i] = ' ';
        }
    }
}

void print(const byte c, const char str[static 1]) {
    static const char fmtstr[] =
        " (%03hhu) %02hhX - %."UTF8BUFLENSTR"s %s";
    char utf8[UTF8BUFLEN] = {0};
    toutf8(c, utf8);
    printf(fmtstr, c, c, utf8, str);
}

const char sep[] = "+";
const char mid[] = "|";
const char end[] = "|\n|";

void printdashline(const bool cont) {
    static const char dashes[] = "-----------------";
    for ( byte i = 0; i < 3; i += 1 ) {
        printf(dashes);
        printf(sep);
    }
    printf(dashes);
    if (cont) {
        printf(end);
    }
}

void printline(const byte ci) {
    assert((ci & 0x03) == 0);
    for ( byte i = 0; i < 3; i += 1 ) {
        const byte c = ci | i;
        print(c, mid);
    }
    print(ci | 0x03, end);
}

#define START 0x00
#define FINISH 0xFF
int main(void) {
    byte start = START >> 2;
    byte finish = (FINISH >> 2) + 1;

    static const byte sep_inds[] = {
        0x20, 0x30, 0x40, 0x60,
        0x80, 0xA0,
    };
    static const byte sep_len = sizeof(sep_inds);
    byte sepi = 0;
    while ( sep_inds[sepi] <= ((start + 1) << 2) ) {
        sepi += 1;
    };

    { // Print table
        printf("|");
        printdashline(true);
        printline(start << 2);

        for ( byte b = start + 1; b < finish; b += 1 ) {
            const byte ci = b << 2;
            if ( sepi < sep_len && ci == sep_inds[sepi] ) {
                printdashline(true);
                sepi += 1;
            }
            assert( sepi == sep_len || ci < sep_inds[sepi] );

            printline(ci);
        }
        printdashline(false);
        printf("|\n");
    }
}
