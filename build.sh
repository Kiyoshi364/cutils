#!/bin/sh

COMPILER_WARNS="-Warith-conversion -Wcast-align=strict -Wcast-qual -Wshadow -Wstrict-prototypes -Wundef -Wall -Wextra -Wstrict-overflow=5 -Wswitch-enum"
COMPILER_PEDANTIC="-Wattributes -Wimplicit-int -Wimplicit-function-declaration -Wincompatible-pointer-types -Wint-conversion -Wmain -Wpointer-arith -Wpointer-sign -Wincompatible-pointer-types -Wwrite-strings"
# COMPILER_PEDANTIC="-Wpedantic"
COMPILER_FLAGS="-std=c99 ${COMPILER_PEDANTIC} ${COMPILER_WARNS} -Werror"

set -xe

OUTDIR=${OUTDIR:-./outbin}
prog="$1"
shift

mkdir -p "${OUTDIR}"
gcc -o "${OUTDIR}/${prog}" ${COMPILER_FLAGS} "${prog}/${prog}.c" "$@"
