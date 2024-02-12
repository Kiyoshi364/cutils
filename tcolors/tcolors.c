#include <stdio.h>
#include <stdbool.h>

typedef unsigned char byte;

void tcolors(void) {
    {
        // Terminal Colors
        printf(" ======  ======  ====== Terminal Colors  ======  ======  ======\n");
        for (byte i = 0; i < 8; i += 1) {
            printf(
                "\x1b[4%1$1hhu;37m4%1$1hhu  \x1b[30m  4%1$1hhu",
                i
            );
        }
        printf("\x1b[0m\n");
        for (byte i = 0; i < 8; i += 1) {
            printf(
                "\x1b[10%1$1hhu;37m10%1$1hhu \x1b[30m 10%1$1hhu",
                i
            );
        }
    }
    {
        // 8-bit Colors
        printf("\x1b[0m\n ======  ======  ======   8-bit  Colors  ======  ======  ======\n");
        byte i = 0;
        for (; i < 0x10; i += 1) {
            printf(
                "\x1b[48;5;%1$1hhu;37m%1$-4hhu\x1b[30m%1$4hhu",
                i
            );
            if (((i+1) & 0x7) == 0) {
                printf("\x1b[49m\n");
            }
        }
        for (; i < 0xE8; i += 1) {
            printf(
                "\x1b[48;5;%1$1hhu;37m%1$-4hhu\x1b[30m%1$4hhu",
                i
            );
            if (((i+1-0x10) % 6) == 0) {
                printf("\x1b[49m\n");
            }
        }
        for (; i != 0; i += 1) {
            printf(
                "\x1b[48;5;%1$1hhu;37m%1$-4hhu\x1b[30m%1$4hhu",
                i
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
