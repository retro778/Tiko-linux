# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Native Linux (GTK3) port of the Windows FreeBASIC IDE [Tiko](https://github.com/PaulSquires/tiko-editor),
extended (Linux only) to also build programs in the [tiko programming language](https://github.com/PaulSquires/tiko-lang).
The goal is exact compatibility with the Windows version (look, layout, features, shortcuts, file
formats) while keeping `src/` as close to upstream as possible so upstream changes can be merged
file by file. `PORTING.md` is the living status document (architecture table, upstream changes,
tiko language support, verified features, known issues) — keep it updated when behaviour changes.

## Build and run

```
make            # first run downloads+builds Scintilla 5.5.3 / Lexilla 5.4.1 into deps/, then builds ./tiko and bin/tiko-lldb-mi
make clean      # removes ./tiko, bin/tiko-lldb-mi and obj/LexTiko.o (not deps/)
./tiko [file.bas | file.tiko | project.tiko]
./install.sh         # per-user menu entry + ~/.local/bin/tiko-editor link to this folder (--uninstall)
LNX_TRACE=1 ./tiko   # prints top-level window SetWindowPos/ShowWindow and child allocations
make win             # Windows build (tiko.exe, bin/tiko-lldb-mi.exe) through Wine into tiko-lang-win64/tiko-editor-main
```

- Compiler: FreeBASIC `fbc` (Linux, 64-bit). The whole app is one translation unit: `src/tiko.bas`
  `#include`s every `.bi/.inc`. Link line is in the `Makefile` (static Scintilla/Lexilla + LexTiko.o,
  `-l gmodule-2.0 -l pangoft2-1.0 -l fontconfig -l X11`, libstdc++ via `#inclib`).
- `bin/tiko-lldb-mi` is a second, separate program (`src/tikolang/tiko-lldb-mi.bas`, built by `make`).
- tiko can build itself: open `tiko-linux.tiko` in tiko and compile (needs one `make` first).
- Debug build for crashes: `fbc -g -exx src/tiko.bas -x tiko_dbg -a obj/LexTiko.o -a deps/scintilla/bin/scintilla.a -a deps/lexilla/bin/liblexilla.a -l gmodule-2.0 -l pangoft2-1.0 -l fontconfig -l X11`
  then run under `gdb`. `-exx` reports the source line of null-pointer accesses.
- Runtime files live next to the executable: `settings/` (themes, languages, keywords, help) and
  `bin/` (SegoeFluentIcons.ttf icon font, Selawik UI fonts, tiko.png, tiko-lldb-mi).
  `settings/settings.ini`, `settings/tikolang.ini`, `settings/ai.ini` (may hold API keys, mode 600),
  `default.session`, `keybindings.ini` are per-user files written at runtime (gitignored).
- External tools: `fbc`; for the tiko language the tiko-lang toolchain (`~/.local/tiko-lang/bin/tiko`,
  `tiko-lsp`) plus clang; `gdb` and/or `lldb-dap-19` for debugging; `kchmviewer` for F1; libcurl
  (linked through `curl.bi`'s `#inclib`) for the AI assistant.

- The same `src/` builds for Windows with the upstream toolchain (`make win`, about 25 s). Code in
  `src/tikolang` and `src/ai` must compile on both: use `tkplat.bi` / `tkproc.bi` for paths and child
  processes, keep sources ASCII (Windows fbc reads them as ANSI), no `#ifdef` inside a continued
  statement. Test the Windows build under Wine on the virtual X server (`wine tiko.exe 'Z:\path\file'`,
  start-up takes ~20 s); Windows tiko needs `C:\Program Files\LLVM` in the Wine prefix.

## Testing

There is no unit test suite. `tests/demo*.bas` are standalone programs exercising the platform layer
(build like the debug command above with the test file instead of `src/tiko.bas`; `demo1 auto <png>`
runs a self check and saves a screenshot).

Real verification is done by running `./tiko`, driving it with `xdotool` and capturing with `import`.
Always do this on a virtual X server — the user works on the same desktop, and stray keys have
already landed in their apps:

```
Xvfb :99 -screen 0 1600x1000x24 -nolisten tcp &
DISPLAY=:99 xfwm4 --compositor=off --sm-client-disable &
export DISPLAY=:99   # then ./tiko, xdotool, import -window root / import -window <id>
```

- Guard every key batch: `T=$(xdotool search --onlyvisible --name '^Tiko' | head -1); [ -z "$T" ] && exit 1`,
  then `xdotool windowactivate --sync $T`. Give test instances a `timeout` longer than the test.
- GTK popups (combo lists, calltips) render black on Xvfb: select with keys, and capture a calltip
  with `import -window <id>` (find it with `xwininfo -root -tree`).
- Use anchored kill patterns (`pkill -f '^\./tiko'`); unanchored ones match the controlling shell (exit 144).
  Killing tiko leaves its "Tiko Debug Console" terminal open; close it separately.
- Keys: Shift+F7 Environment Options, F5 build+run, F6 debug, F9 breakpoint, F12/Shift+F12 definition/back.

## Architecture

**Upstream code (`src/*.bas|bi|inc`)** is Paul Squires' Windows source copied verbatim: Win32 window
procedures (`frmX_WndProc`), AfxNova `CWindow`, custom GDI painting via `clsDoubleBuffer`, a
`GetMessage` loop with pre-filters in `frmMain_Show`/`modMsgPump.inc`. Linux-specific changes are
only allowed as small `#ifdef __FB_LINUX__` blocks (feature hooks: `#ifdef TIKOLANG_SUPPORT` for the tiko
language, `#ifdef AI_ASSISTANT` for the AI panel; all listed in PORTING.md); prefer fixing things in the
platform layer instead. New features live in their own folders (`src/tikolang/`, `src/ai/`) and only hook
into upstream files. Files are CRLF; edit
them preserving CRLF (Python with `newline=''`). Platform macros (`EXE_EXTENSION`, `DLL_EXTENSION`,
`TOOLCHAIN_FBC64`, `TOOLCHAIN_INCFOLDER`, `TIKOLANG_SUPPORT`, `AI_ASSISTANT`) are in `tiko.bas`.

**Platform layer (`src/lnx/`, included via `lnx/lnx.bi`)** implements exactly the Win32/AfxNova
surface tiko uses on GTK3 ("hybrid" design):
- `HWND` = `LNXWINDOW ptr` (`winapi.bi`). Top level windows are `GtkWindow` + `GtkLayout` client;
  custom child windows are `GtkLayout`s whose GTK `draw` is turned into `WM_PAINT` (`BeginPaint`
  returns an HDC wrapping the cairo context, `gdi.inc` implements GDI on Cairo/Pango).
- `window.inc`: a real Win32-style message queue. GTK input becomes queued `MSG`s retrieved by
  `GetMessage`. Keyboard events for native widgets (Scintilla, entries) are queued too and the
  original GDK event is re-injected only when the message reaches the end of the window-procedure
  chain (`LnxControlProc`), so accelerators, subclass procs and pre-filters see keys first, like
  Windows. `TranslateMessage` defers printable text to `WM_CHAR`.
- Child geometry is applied immediately in `SetWindowPos`/`ShowWindow` (`LnxAllocateChild`), because
  GTK drops relayout requests made during size-allocate (tiko repositions children from `WM_SIZE`).
- `controls.inc`: BUTTON/CHECKBOX/TEXTBOX/COMBOBOX/… are GTK widgets answering LB_/CB_/BM_/EM_/LVM_/TVM_
  messages; LISTBOX is drawn by the layer (owner draw via `WM_DRAWITEM`, `LBS_NODATA`); STATIC
  controls are painted through `WM_PAINT` so subclass procedures can owner-draw them; Scintilla
  `sci-notify` → `WM_NOTIFY` with `hwndFrom` rewritten to the HWND.
- `cwindow.inc`: `CWindow`, message crackers, accelerators, `IsDialogMessage`, popup menus (GtkMenu,
  needs a trigger event), MessageBox, tooltips. `comdlg.inc` emulates the COM file dialogs
  (IFileOpenDialog/IShellItem) so upstream dialog code is unchanged. `ipc.inc`: cross-process
  FindWindow/WM_COPYDATA over a unix socket (single instance).
- `process.inc` emulates file/pipe/process HANDLEs (compiler output, gdb), ShellExecute, terminals,
  F1 help and the debug console (`LnxCreateDebugConsole` → `set inferior-tty`). Like Windows'
  `CreateProcess`, the first word of the command line is argv[0] and is not passed to the program
  (upstream relies on this: fbc gets `-m "main.bas"` and sees only the file).
- `DWSTRING` (`dwstring.bi`) is UTF-32 on Linux; `string` ↔ DWSTRING is UTF-8, with invalid UTF-8
  decoded as Windows-1252 ("ANSI" files). `CTextStream` writes UTF-16LE+BOM+CRLF like Windows.
  All file APIs accept `\` separators (`NativePath`); relative paths written to project/session files
  use `\` (`PortablePath`) so files stay interchangeable with Windows Tiko.
- `winconst.bi` is generated from FreeBASIC's Windows headers: constants must be `const` (not
  `#define`) because `HANDLE_MSG` token-pastes message names.

**tiko language (`src/tikolang/`)**: `code_Compile` hands off to `TikoLang_Compile` when the main file
is built with tiko — decided by extension plus the user's choice on the Environment Options
"Compilers" page (`frmOptionsTikoLang.inc`, `settings/tikolang.ini`). Single files build with
`tiko -v --diagnostics-file`, `tiko.toml` packages with `tiko build --message-format json`;
diagnostics are fed into the upstream Compiler Results list. `.tiko` is both the editor's project
extension and the language's source extension: a `.tiko` file is a project only if it starts with
`' PROJECT FILE` (`TikoLang_IsProjectFile`).
- `modTikoLsp.inc`: tiko-lsp client (JSON-RPC over pipes read by a GLib watch, diagnostics as
  Scintilla indicators 20/21, hover on `SCN_DWELLSTART`, F12). JSON reader: `tkjson.bi`.
- `tiko-lldb-mi.bas`: answers the gdb/MI subset `frmDebug.inc` uses by driving lldb-dap, so the
  upstream debugger window works with lldb. `frmDebug`'s reader (`gdb_receive`) keeps only the first
  message of a burst, so the adapter spaces its answers 150 ms apart and writes nothing unparsed.
  Test it without the GUI by piping MI commands (`break "file":N`, `-exec-run`, ...) to it.

**AI assistant (`src/ai/`, `#ifdef AI_ASSISTANT`)**: chat panel docked right (`frmAiChat.inc`, laid out by a
hook in `frmMain_PositionWindows`), libcurl streaming client for the Anthropic Messages API / OpenAI
Chat Completions in a worker thread that only buffers bytes (`modAiClient.inc`; parsing happens on the UI
thread because `tkjson.bi` uses one global node pool), options page `frmOptionsAi.inc`, settings
`settings/ai.ini` (gitignored, mode 600). Test it against a local mock server by setting
`AnthropicBaseUrl` / `OpenAIBaseUrl` in `ai.ini` (back up and restore the user's `ai.ini`). Real API
calls cost the user money: ask before making one. A mock over plain HTTP does not exercise TLS.

**Lexer**: `src/lexer/LexTiko.cxx` (derived from Lexilla LexBasic, registered as "tiko" like the
unpublished Windows lexer) is created via `TikoCreateLexer()`. Property `lexer.tiko.blockstrings`
enables tiko `"""` block strings (styled `SCE_B_DATE`).

## FreeBASIC pitfalls hit in this port

Identifiers are case-insensitive and share one namespace with keywords and C/runtime functions: a
parameter `hwnd` hides the `HWND` type, `sIn` is `sin`, `pOpen`/`pClose` are `popen`/`pclose`, an enum
member `TKJ_NUM` collides with a function `TkJ_Num`; `format`, `cls`, `inp`, `access`, `resume` are
keywords. String literals have no escapes unless prefixed with `!` (`"\"` is one backslash). A
trailing `_` in a name is a line continuation; `strptr("")` is NULL (use `LnxZ()` for GTK calls);
methods named `Open/Close/Write` hide the file keywords inside the class; multi-line `#macro`s can't
be used in single-line `if`; an array field can't be `redim`med through an indexed element
(`redim a(i).b(n)`); mixing boolean and integer comparisons in `andalso`/`orelse` warns (fix it). `ThreadCreate` gives a thread a 16 KB
stack unless a size is passed (OpenSSL's TLS handshake overflowed it); pass C `long` arguments to
varargs C functions such as `curl_easy_setopt` as `cast(clong, n)`, not `clng`.
Some GLib functions are renamed in FB headers (`g_source_remove_`) or missing (declare them in
`extern "C"`); Scintilla structs use C `long` = 64-bit positions on Linux (`modScintilla.bi` has
Linux variants).

## Git and releases

`upstream/` (reference clone of PaulSquires/tiko-editor, contains the AfxNova sources under
`upstream/toolchains/*/inc/AfxNova` used as API reference) and `deps/` are not tracked.
Remote: private GitHub repo `retro778/Tiko-linux` (renamed from kito-lnx; use `gh ... -R retro778/Tiko-linux`).
Releases are tags `v1.3.2-linux.N` with a tarball `tiko-<tag>-x86_64.tar.gz` (+ `.sha256`) of
`tiko-linux/`: stripped `tiko` and `bin/tiko-lldb-mi` plus `git ls-files bin settings LICENSE README.md PORTING.md install.sh`
(copy with `-z`/`xargs -0`: `settings/languages/español.lang`), never the per-user settings files.
