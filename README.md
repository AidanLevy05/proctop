# proctop

`proctop` is a small Linux terminal process monitor. It reads system and
process information from `/proc` and shows CPU, memory, load average, per-core
CPU usage, and a live process table.

## Build

```sh
make
```

This builds the binary at `src/proctop`.

## Run

```sh
./src/proctop
```

If you install the binary in your `PATH`:

```sh
proctop
```

## Usage

```sh
./src/proctop [--log file] [filter]
proctop [--log file] [filter]
```

If `filter` is provided, only processes whose `COMMAND` value contains that
substring are shown.

Examples:

```sh
./src/proctop
./src/proctop chrome
./src/proctop --log proctop.log
./src/proctop --log proctop.log ssh
proctop --help
proctop --version
```

## Controls

- `q` quit
- `Esc` quit
- `r` refresh immediately
- `m` sort by memory
- `p` sort by pid
- `c` sort by cpu
- `Up` move selection up
- `Down` move selection down

The screen refreshes once per second.

## Logging

Use `--log file` to append visible process snapshots to a text file during live
updates.

## Help And Version

- `proctop --help`
- `proctop --version`

## Man Page

A basic man page is included as `proctop.1`.

## Notes

- System and process data is read from `/proc`.
- The terminal UI uses `termbox2`.

## Third Party Libraries

This project uses termbox2 for terminal UI rendering.

termbox2
Copyright (c) 2012-2023 termbox2 contributors
https://github.com/termbox/termbox2

Licensed under the MIT License.
