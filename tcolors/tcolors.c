/* tcolors (terminal colors)
 *
 * Shows ANSI terminal colors.
 * Pallets 0-7 (dull and vivid) and 8 bit mode.
 * (Note: Missing 24-bit colors)
 *
 * Copyright (C) 2024, 2026 Daniel K Hashimoto
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
#include <stdbool.h>

typedef unsigned char byte;

void tcolors(void) {
    {
        // Terminal Colors
        printf(" ======  ======  ======   4-bit  Colors  ======  ======  ======\n");
        for (byte i = 0; i < 8; i += 1) {
            printf(
                "\x1b[4%hhu;37m4%hhu  \x1b[30m  4%hhu",
                i, i, i
            );
        }
        printf("\x1b[0m\n");
        for (byte i = 0; i < 8; i += 1) {
            printf(
                "\x1b[10%hhu;37m10%hhu \x1b[30m 10%hhu",
                i, i, i
            );
        }
    }
    {
        // 8-bit Colors
        printf("\x1b[0m\n ======  ======  ======   8-bit  Colors  ======  ======  ======\n");
        byte i = 0;
        for (; i < 0x10; i += 1) {
            printf(
                "\x1b[48;5;%1hhu;37m%-4hhu\x1b[30m%4hhu",
                i, i, i
            );
            if (((i+1) & 0x7) == 0) {
                printf("\x1b[49m\n");
            }
        }
        for (; i < 0xE8; i += 1) {
            printf(
                "\x1b[48;5;%hhu;37m%-4hhu\x1b[30m%4hhu",
                i, i, i
            );
            if (((i+1-0x10) % 6) == 0) {
                printf("\x1b[49m\n");
            }
        }
        for (; i != 0; i += 1) {
            printf(
                "\x1b[48;5;%hhu;37m%-4hhu\x1b[30m%4hhu",
                i, i, i
            );
            if (((i+1) & 0x7) == 0) {
                printf("\x1b[49m\n");
            }
        }
    }
    printf("\x1b[0m\n");
}

int main(void) {
    tcolors();
    return 0;
}
