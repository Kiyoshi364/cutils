# C Utils

## How to run

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
Works with `stdin` and `stdout`.
Use flag `-d` to decompress.

## becho (binary echo)

Reads stdin and echos decoded utf-8 characters.

## tcolors (terminal colors)

Shows ANSI terminal colors.
Pallets 0-7 (dull and vivid) and 8 bit mode.
(Note: Missing 24-bit colors)
