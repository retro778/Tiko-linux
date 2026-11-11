# Tiko Editor for Linux

Native Linux (GTK3 + Scintilla) port of the [Tiko editor](https://github.com/PaulSquires/tiko-editor)
for FreeBASIC by Paul Squires. Licensed under the GNU GPLv3 or later (see LICENSE).

Build: `make` (downloads and builds Scintilla/Lexilla on first run), then `./tiko`.

Tiko can also build itself: open `tiko-linux.tiko` in tiko and compile (run `make` once
first so Scintilla, Lexilla and `obj/LexTiko.o` exist).

The editor also builds programs in the [tiko programming language](https://github.com/PaulSquires/tiko-lang)
(`.tiko`, `.ti`, `.tko` files) with the `tiko` compiler, next to FreeBASIC with `fbc`.
The compiler needs clang (LLVM 18+). Tiko files are checked as you type with `tiko-lsp`
(errors, hover, F12), and programs can be debugged with gdb or lldb (`lldb-dap`), chosen
in Environment Options > Compilers.

See [PORTING.md](PORTING.md) for the architecture, upstream changes and current status.
