# Tiko Editor for Linux

Native Linux (GTK3 + Scintilla) port of the [Tiko editor](https://github.com/PaulSquires/tiko-editor)
for FreeBASIC by Paul Squires. Licensed under the GNU GPLv3 or later (see LICENSE).

Build: `make` (downloads and builds Scintilla/Lexilla on first run), then `./tiko`.

Tiko can also build itself: open `tiko-linux.tiko` in tiko and compile (run `make` once
first so Scintilla, Lexilla and `obj/LexTiko.o` exist).

See [PORTING.md](PORTING.md) for the architecture, upstream changes and current status.
