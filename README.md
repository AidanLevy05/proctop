# proctop

`proctop` is a small terminal process monitor for Linux. It reads system and
process data from `/proc` and shows CPU usage, memory usage, and a live process
table in the terminal.

## Build

```sh
make
```

This builds the binary at `src/proctop`.

## Run

```sh
./src/proctop
```

If you install the binary somewhere in your `PATH`, you can run:

```sh
proctop
```

## Usage

```sh
./src/proctop [filter]
```

Installed command usage:

```sh
proctop [filter]
```

If `filter` is provided, `proctop` only shows processes whose `COMMAND` value
contains that substring.

Examples:

```sh
./src/proctop
./src/proctop chrome
proctop ssh
```

## Controls

- `q` quit
- `Esc` quit
- `r` refresh immediately
- `m` sort by memory
- `p` sort by pid
- `c` sort by cpu

The screen also refreshes automatically once per second.

## Notes

- Process information is read from `/proc`.
- The terminal UI uses `termbox2`.

## Third Party Libraries

This project uses termbox2 for terminal UI rendering.

termbox2
Copyright (c) 2012-2023 termbox2 contributors
https://github.com/termbox/termbox2

Licensed under the MIT License.
