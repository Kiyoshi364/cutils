# C Utils

Some single-file executables and libraries.
Each executable has its own folder.

## How to run executables

```console
$ ./build.sh <name> [-o <outputbin>]
$ ./outbin/<name>
```

or

```console
$ ./build.sh <name> -o <outputbin>
$ ./<outputbin>
```

# Utils list

## atable (ASCII table)

Prints all unicode characters (utf-8 encoded)
that fit in 1 byte.

## aucompress (Ancient UNIX Compress)

Small and simple implementation of `compress`.
Works only with `stdin` and `stdout`.
Use flag `-d` to decompress.

## becho (binary echo)

Reads stdin and echos decoded utf-8 characters.

## sha1 (sha1sum)

A simple implementation of `sha1sum`
(no support for flags).

The file `sha1/sha1.h` may be used as a library.
To get the implementation of the functions,
define `HASHI_SHA1_IMPLEMENTATION` before including this file.

Also has a simple implementation of `sha1sum` (no support for flags).
For the implementation, define `HASHI_SHA1_EXE` before including this file.

## tcolors (terminal colors)

Shows ANSI terminal colors.
Pallets 0-7 (dull and vivid) and 8 bit mode.
(Note: Missing 24-bit colors)
