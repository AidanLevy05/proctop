# proctop

`proctop` is a small Linux terminal process monitor built around `/proc` and
`termbox2`. It shows live system stats, a process table, selection details, and
interactive controls without leaving the terminal.

## Features

- live CPU, memory, load average, uptime, and per-core CPU stats
- NVIDIA GPU usage and VRAM stats through `nvidia-smi` when available
- process list sorting by memory, pid, or cpu
- optional startup filter: `proctop chrome`
- interactive search mode with `/`
- process tree mode with `t`
- selected-process details with state, thread count, and CPU history
- optional logging with `--log file`
- terminate and force-kill shortcuts with confirmation

## Screenshot

No screenshot file is in the repository yet.

Add a screenshot here when one is available:

```md
![proctop screenshot](path/to/screenshot.png)
```

## Build

```sh
make
```

This builds the binary at `src/proctop`.

## Install

```sh
make install
make install-man
```

Default install prefix is `/usr/local`. You can override it:

```sh
make install PREFIX=$HOME/.local
make install-man PREFIX=$HOME/.local
```

Remove installed files with:

```sh
make uninstall
make uninstall-man
```

## Run

```sh
./src/proctop
./src/proctop [filter]
./src/proctop --log proctop.log [filter]
proctop [filter]
```

If a filter is provided, only processes whose displayed command contains that
substring are shown.

Examples:

```sh
./src/proctop
./src/proctop ssh
./src/proctop --log proctop.log chrome
proctop --help
proctop --version
```

## Controls

- `q` quit
- `Esc` quit, or cancel search mode
- `r` refresh immediately
- `m` sort by memory
- `p` sort by pid
- `c` sort by cpu
- `t` toggle process tree mode
- `/` start in-UI search/filter mode
- `Up` move selection up
- `Down` move selection down
- `K` start SIGTERM confirmation for the selected process
- `X` start SIGKILL confirmation for the selected process
- `Y` confirm an active terminate/kill prompt
- `N` cancel an active terminate/kill prompt

## Notes

- System and process information is read from `/proc`.
- GPU stats are read with `nvidia-smi` through `popen()`. If `nvidia-smi` is
  missing or no NVIDIA GPU is available, the header falls back to `GPU: N/A`.
- Tree mode adds simple indentation before the command name and overrides the
  normal sort order while it is enabled.
- Search mode applies the same command substring filter used by the startup
  argument.
- Logging writes the currently visible process snapshot during the refresh loop.

## Man Page

A basic man page is included as `proctop.1`.

## Third Party Libraries

This project uses termbox2 for terminal UI rendering.

termbox2
Copyright (c) 2012-2023 termbox2 contributors
https://github.com/termbox/termbox2

Licensed under the MIT License.
