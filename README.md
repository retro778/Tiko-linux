# Tiko Editor for Linux and Windows

The [Tiko editor](https://github.com/PaulSquires/tiko-editor) for FreeBASIC by Paul Squires, with:

- a **native Linux port** (GTK3 + Scintilla) that matches the Windows version's look, features,
  shortcuts and file formats;
- support for the **[tiko programming language](https://github.com/PaulSquires/tiko-lang)**
  next to FreeBASIC;
- an **AI assistant** panel (Claude or OpenAI).

The tiko language support and the AI assistant are available on both Linux and Windows; both
builds come from the same sources. Licensed under the GNU GPLv3 or later (see LICENSE).

## Downloads

Releases are on the [releases page](https://github.com/retro778/Tiko-linux/releases):

| Platform | Release | File |
|---|---|---|
| Linux (x86_64, GTK 3, glibc 2.38+) | latest `v1.3.2-linux.N` | `tiko-1.3.2-linux.N-x86_64.tar.gz` |
| Windows (64-bit) | latest `v1.3.2-win.N` | `tiko-1.3.2-win.N-x86_64.zip` |

**Linux:** extract, then `./tiko` in the `tiko-linux` folder. `./install.sh` adds a "Tiko Editor"
menu entry and the `tiko-editor` command (`~/.local/bin`), both pointing at that folder (nothing
is copied; run it again after moving the folder, `./install.sh --uninstall` removes them).
Needs FreeBASIC (`fbc`); optional: `gdb` / `lldb`, `kchmviewer` for the offline manual.

**Windows:** unzip to a path without spaces and run `tiko-windows\tiko.exe`. FreeBASIC works out
of the box (the toolchain is bundled). See `README-tiko-lang.md` in the zip. The Windows build
has been tested under Wine, not yet on a real Windows machine.

## The tiko language

`.tiko`, `.ti` and `.tko` files build with the `tiko` compiler (it needs clang, LLVM 18+):
errors and warnings in Compiler Results, build and run, `tiko.toml` packages (`tiko build`),
tiko keywords and `"""` block strings highlighted, F1 on the tiko wiki. Tiko files are checked as
you type with `tiko-lsp` (errors, hover, F12). **Environment Options > Compilers** chooses fbc or
tiko for each kind of file, the compiler's path, the language server and the debugger.

- Linux: put the tiko-lang toolchain on the PATH or in `~/.local/tiko-lang`. Debugging works with
  gdb or lldb (`lldb-dap`).
- Windows: put the toolchain next to the editor folder (`tiko-lang\tiko.exe`), in
  `%USERPROFILE%\tiko-lang` or on the PATH, and install LLVM. Debugging works with the bundled
  gdb; lldb is included but untested.

A `.tiko` file is an editor project only when it starts with `' PROJECT FILE`; any other `.tiko`
file opens as tiko source code.

## AI assistant

A chat panel docked on the right (View > View AI Assistant, Ctrl+Shift+A) talks to Claude
(Anthropic API) or OpenAI about the current file, explains compile errors and inserts suggested
code. It uses `ANTHROPIC_API_KEY` / `OPENAI_API_KEY`, or a key entered in Environment Options >
AI Assistant. On Linux the keys are also read from `~/.profile`.

## Building from source

**Linux:** `make` (downloads and builds Scintilla/Lexilla on first run), then `./tiko`. Needs
`fbc`, `libgtk-3-dev`, `g++`, libcurl. Tiko can also build itself: open `tiko-linux.tiko` in
tiko and compile (run `make` once first).

**Windows** (cross-built on Linux through Wine, with the toolchain from an unpacked upstream
tiko-editor in `tiko-lang-win64/tiko-editor-main`):

```
make win          # tiko.exe and bin/tiko-lldb-mi.exe
make win-lexer    # bin/Lexilla64.dll (needs g++-mingw-w64-x86-64)
```

On Windows itself: `_compile.bat` in the release folder.

See [PORTING.md](PORTING.md) for the architecture, the changes to upstream and the current status.
