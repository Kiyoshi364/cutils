/* becho (binary echo)
 *
 * Reads stdin and echos decoded utf-8 characters.
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

typedef unsigned char byte;

int main(void) {
    int chr;
    unsigned int codepoint = 0;
    unsigned int last_codepoint = 0;
    byte utf8_bytes_left = 0;
    while ((chr = getchar()) != EOF) {
        const byte c = (unsigned char) chr;
        printf("%3hhu 0x%02hhX 0b_%04hhb_%04hhb\n",
            c, c, (c >> 4), (c & 0xF)
        );
        if ( utf8_bytes_left > 0 ) {
            if ( (c & 0x80) == 0x80 && (c & 0x40) == 0x00 ) {
                codepoint = (codepoint << 6) | (c & 0x7F);
                utf8_bytes_left -= 1;
                if ( utf8_bytes_left == 0 ) {
                    if (codepoint > 0xFFFF) {
                        printf("=> U+%04X_%04X\n",
                            (codepoint >> 16), (codepoint & 0xFFFF)
                        );
                    } else if ( 0xD800 <= codepoint && codepoint < 0xDC00 ) {
                        printf("=> U+%04X (HIGH)\n", codepoint);
                    } else if ( 0xDC00 <= codepoint && codepoint < 0xE000 ) {
                        unsigned long long int bigcode =
                            0x10000 + (
                            (((unsigned long long int) (last_codepoint & 0x03FF)) << 10)
                            | (codepoint & 0x03FF)
                        );
                        printf("=> U+%04X (LOW): U+%04llX_%04llX\n",
                            codepoint, (bigcode >> 16), (bigcode & 0xFFFF)
                        );
                    } else {
                        printf("=> U+%04X\n", codepoint);
                    }
                }
            } else {
                printf("=> Utf8 ERROR: Starting bits are weird\n");
                last_codepoint = codepoint;
                codepoint = 0;
                utf8_bytes_left = 0;
            }
        } else {
            for (byte i = 0x80; i > 0; i >>= 1) {
                if ( (c & i) == i ) {
                    utf8_bytes_left += 1;
                } else {
                    break;
                }
            }
            // if ( c == 0xC0 || c == 0xC1 || c > 0xF4 ) {
            //     printf("=> Utf8 ERROR: starting character is invalid\n");
            //     utf8_bytes_left = 0;
            // }
            if ( (c & 0xFE) == 0xFE ) {
                printf("=> Utf8 ERROR: starting character is 0xFF or 0xFE\n");
                utf8_bytes_left = 0;
            } else if ( utf8_bytes_left > 1 ) {
                utf8_bytes_left -= 1;
                byte mask = (1 << (7 - utf8_bytes_left)) - 1;
                last_codepoint = codepoint;
                codepoint = c & mask;
            } else if (utf8_bytes_left == 1) {
                // Note: in the middle/end of a sequence
                utf8_bytes_left = 0;
            } else if (utf8_bytes_left == 0) {
                // Note: ASCII
            } else {
                printf("=> Utf8 ERROR: Starting bits are weird (first byte)\n");
            }
        }
    }
    return 0;
}
