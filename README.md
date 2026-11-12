# Tiko Editor for Linux

Native Linux (GTK3 + Scintilla) port of the [Tiko editor](https://github.com/PaulSquires/tiko-editor)
for FreeBASIC by Paul Squires. Licensed under the GNU GPLv3 or later (see LICENSE).

Build: `make` (downloads and builds Scintilla/Lexilla on first run), then `./tiko`.

Install for your user: `./install.sh` adds a "Tiko Editor" menu entry and the `tiko-editor`
command (`~/.local/bin`), both pointing at this folder (nothing is copied; run it again after
moving the folder, `./install.sh --uninstall` removes them). Then `tiko-editor [files...]`.

Tiko can also build itself: open `tiko-linux.tiko` in tiko and compile (run `make` once
first so Scintilla, Lexilla and `obj/LexTiko.o` exist).

The editor also builds programs in the [tiko programming language](https://github.com/PaulSquires/tiko-lang)
(`.tiko`, `.ti`, `.tko` files) with the `tiko` compiler, next to FreeBASIC with `fbc`.
The compiler needs clang (LLVM 18+). Tiko files are checked as you type with `tiko-lsp`
(errors, hover, F12), and programs can be debugged with gdb or lldb (`lldb-dap`), chosen
in Environment Options > Compilers.

An AI assistant panel (View > View AI Assistant, Ctrl+Shift+A) chats with Claude (Anthropic API)
or OpenAI about the current file, explains compile errors and inserts suggested code. It uses
`ANTHROPIC_API_KEY` / `OPENAI_API_KEY` or a key entered in Environment Options > AI Assistant.

The tiko language support and the AI assistant also build for Windows from the same sources
(`make win`, using the upstream Windows toolchain through Wine).

See [PORTING.md](PORTING.md) for the architecture, upstream changes and current status.
