# Tiko Editor – Linux port

Native Linux (GTK3 + Scintilla/Lexilla) port of [tiko-editor](https://github.com/PaulSquires/tiko-editor).

Goals: same file formats, same look & layout, same features & shortcuts, and source kept
identical to upstream wherever possible so upstream changes can be merged file by file.

## Build & run

```
make            # builds deps (scintilla/lexilla) if needed, then ./tiko
./tiko [file.bas | project.tiko]
```

Requires: fbc (Linux), libgtk-3-dev, g++ (for Scintilla/Lexilla). `bin/` must contain
`SegoeFluentIcons.ttf` (icons) and `tiko.png`; `settings/` sits next to the executable.

## Layout

```
kito-lnx/
  upstream/        reference clone of PaulSquires/tiko-editor (do not edit)
  deps/            scintilla 5.5.3 + lexilla 5.4.1 sources (GTK3 build, linked statically)
  src/             port: upstream/src copied verbatim + small #ifdef __FB_LINUX__ blocks
  src/lnx/         platform layer replacing windows.bi + AfxNova
  settings/ bin/   runtime files
  tests/           layer demos (demo1.bas, demo2.bas)
```

## Architecture (hybrid)

Upstream forms are Win32 window procedures. `src/lnx` implements the subset of Win32/AfxNova
that tiko uses on top of GTK3, so the forms compile unchanged:

| File | Provides |
|---|---|
| `wintypes.bi`, `winconst.bi`, `winapi.bi` | Win32 types, constants (values from FB's Windows headers), MSG/NMHDR/DRAWITEMSTRUCT… |
| `dwstring.bi`, `AfxStr.inc`, `afxfile.bi`, `textstream.bi` | DWSTRING (UTF‑32), AfxNova string/file/path helpers, CTextStream/CFileStream |
| `gdi.inc` | HDC/HFONT/HPEN/HBRUSH/HBITMAP on Cairo + Pango |
| `window.inc` | HWND objects, Win32 message queue + `GetMessage` loop, GTK events → WM_*, timers, focus, capture, cursors, painting |
| `controls.inc` | BUTTON/CHECKBOX/RADIO/GROUPBOX/LABEL/TEXTBOX/COMBOBOX (GTK widgets), LISTBOX (custom drawn, owner draw), LISTVIEW/TREEVIEW (GtkTreeView), RICHEDIT (GtkTextView), Scintilla notifications |
| `cwindow.inc` | CWindow, message crackers, accelerators, IsDialogMessage, popup menus (GtkMenu), MessageBox, tooltips, Afx window helpers |
| `comdlg.inc` | IFileOpenDialog/IFileSaveDialog/IShellItem emulation (GtkFileChooser), folder picker |
| `process.inc` | file/pipe/process HANDLEs (CreateProcess, pipes, ReadFile…), ShellExecute, run in terminal |
| `winmisc.inc`, `winextra.inc`, `afxextra.inc` | Toolhelp (/proc), INI files, fonts, FindFirstFile, CFindFile, code pages, layered windows, RTF loader (help), CWinHttpRequest (curl) … |

Key behaviours:
* Keyboard events are queued as WM_KEYDOWN/WM_CHAR and, for native widgets, the original GDK
  event is re-injected only when the message reaches the end of the window procedure chain
  (so accelerators, menu pre-filters and subclass procedures work exactly like on Windows).
* Painting: GTK "draw" → WM_PAINT; BeginPaint returns an HDC wrapping the cairo context.
* SetWindowPos on child windows allocates the new geometry immediately (GTK drops relayout
  requests made during a size-allocate, e.g. when tiko repositions children in WM_SIZE).
* GetWindowPlacement returns the outer window rectangle (frame included) like Win32.
* WS_POPUP: untitled/no-activate → GTK popup; layered → managed undecorated (no focus);
  titled (Find/Replace) → undecorated focusable window.

## Upstream changes (all inside `#ifdef __FB_LINUX__`)

* `tiko.bas` – include `lnx/lnx.bi`, `LnxInit()`, static Scintilla/Lexilla, platform macros
  (`EXE_EXTENSION`, `DLL_EXTENSION`, `TOOLCHAIN_FBC32/64`, `TOOLCHAIN_INCFOLDER`).
* `modCWindow.inc` – CPanelWindow built on CWindow.
* `frmHelpViewer.bi` – no AfxRichEdit include.
* `modRoutines.inc` – `Utf8ToAnsi` uses Windows‑1252 explicitly; `.exe` → `EXE_EXTENSION`.
* `modCompile.inc` – output names, include folder, no `.rc` resources.
* `modCompileErrors.inc` – `RunEXE` runs console programs in a terminal.
* `clsConfig.inc`, `frmOptions.inc`, `frmOptionsCompiler.inc` – toolchain `bin/fbc`, system fbc fallback.
* `frmDebug.inc` – gdb from `bin/gdb/gdb` or PATH.

## tiko language support (Linux build only, `#ifdef TIKOLANG_SUPPORT`)

The editor builds FreeBASIC (`fbc`) and [tiko-lang](https://github.com/PaulSquires/tiko-lang)
programs. The compiler is chosen by the main file (project main file, else the active
document) and the user's choice in **Environment Options > Compilers**
(`src/tikolang/frmOptionsTikoLang.inc`, saved in `settings/tikolang.ini`): `.bas/.bi/.inc`
with fbc (default) or tiko, `.tiko/.ti/.tko` with tiko (default) or fbc, and the tiko
compiler's path. The editor follows the compiler: files built with tiko get the tiko
keywords and F1 opens the tiko wiki. fbc builds a non-`.bas` main file with `-b`. All of it lives in
`src/tikolang/modTikoLang.inc`; the hooks in upstream files are small `#ifdef TIKOLANG_SUPPORT`
blocks (`TIKOLANG_SUPPORT` is defined in `tiko.bas` on Linux).

* **.tiko is two things.** The editor's project files and the language's source files share
  the extension. A `.tiko` file is a project only when it starts with the `' PROJECT FILE`
  line the editor writes (`TikoLang_IsProjectFile`); command line, drag & drop and session
  restore check this (`frmMain.inc`, `clsConfig.inc`). File > Open always opens a document.
* **Compiler:** the path from the options page, else `$TIKO_COMPILER`, else `tiko` on the PATH (not the editor itself, also named
  tiko: the candidate must have `../lib` or `../pkg`), else `~/.local/tiko-lang/bin/tiko`,
  `~/tiko-lang/bin/tiko`, `/opt/tiko-lang/bin/tiko`. It needs clang (LLVM 18+).
* **Build** (`TikoLang_Compile`, hooked at the top of `code_Compile`):
  `tiko -v --color never --diagnostics-file <tmp> <options> main.tiko [modules]`.
  Build configurations are shared with FreeBASIC; fbc-only options the tiko compiler
  refuses (`-gen`, `-export`, `-nostrip`, `-b`, ...) are dropped and `-O 2` becomes `-O2`.
  Debug builds remove `-O*` and add `-g -exx`. Project modules are passed in the same run
  (the program is named after the first input). Errors/warnings come from the diagnostics
  file (`kind id path line col endline endcol message`, `end errors warnings exitcode`) into
  the Compiler Results list; clang/linker failures show the last log line. Quick Run uses
  `_tiko_quickrun.tiko`. `-lib` needs `llvm-ar` (tiko error TKO0111 otherwise).
* **Debugging:** gdb reads tiko's DWARF; the existing gdb/MI debugger works unchanged
  (breakpoints, stepping, locals; `ustring` shows as text with its length).
* **Editor:** the FreeBASIC lexer with `settings/keywords/tiko_keywords.txt` added to the
  keywords (`int32`, `ustring`, `nullptr`, `defer`, `#import`, ...) and `"""` block strings
  (`LexTiko.cxx` property `lexer.tiko.blockstrings`, styled as `SCE_B_DATE` = string colours).
  File dialogs list `*.tiko;*.ti;*.tko` beside the FreeBASIC patterns (`comdlg.inc`).
* **F1** in a tiko file opens the keyword's page of the tiko wiki
  (`settings/help/tiko_wiki_index.txt`, generated from the wiki's Keyword-Index page),
  else the Keyword Index.
* **tiko.toml packages:** a file built with tiko that has a `tiko.toml` in its folder or
  above (and no editor project open), or `tiko.toml` itself, is built as the package:
  `tiko build --message-format json --profile debug|release` in the package root (debug
  for debugging and for build configurations with `-g`; Rebuild All adds `--no-cache`).
  The compiler's diagnostic lines (`path(line,col): kind TKOnnnn: message`) go to Compiler
  Results; the program run or debugged is the `artifact` of the target whose `main` is the
  current file, else the first executable. Quick Run still builds the single file.
* **Command lines:** like Windows' `CreateProcess`, the layer takes the first word of the
  command line as argv[0], so the tiko command lines start with the compiler path.
* Not done: the tiko-lsp language server, lldb-dap.

## File format compatibility

* Unicode files (config, `.tiko`, sessions, `.lang`) are UTF‑16LE + BOM + CRLF (verified byte identical).
* Paths read from files: `\` accepted everywhere (all file APIs normalize to `/`).
* "ANSI" source files are Windows‑1252 (as on Windows).

## Status

Verified working (driven with xdotool, checked by screenshots): main window, menubar and
popup menus, context menus, top tabs, explorer, function list, bookmarks panel, status bar,
editor (Scintilla, FreeBASIC lexer, themes), Find/Replace, Find in Files, Go to Line,
Command Line, Environment Options (all pages), Build Configurations, Keyboard Shortcuts,
User Tools, New Project / Project Options, project save and reopen (.tiko round trip),
Add Files (file dialog), Tiko help viewer (RTF), About, compile (F5 / Compile menu) with
error list, log and jump to error line, debugger (gdb: breakpoints, variables, continue),
F1 FreeBASIC help, settings saved on exit.

Linux specific behaviour:
* F1: CHM viewer when installed (kchmviewer with keyword lookup, or xchm), otherwise the
  online manual page https://www.freebasic.net/wiki/KeyPg<word>.
* Console programs run in a terminal window; `-s gui` programs start directly.
* The default User Tools (asciichart32.exe, GUIDgen32.exe) are Windows programs.
* Desktop shortcuts may take keys before tiko sees them (XFCE: Ctrl+F1..F12 switch
  workspaces, so Ctrl+F5 "Compile" needs the menu or a different binding).

Also implemented: single instance (unix socket IPC, `src/lnx/ipc.inc`), drag & drop of
files, debug console window for the program being debugged (gdb `inferior-tty`), the
"tiko" lexer (`src/lexer/LexTiko.cxx`, derived from Lexilla's FreeBASIC lexer: all number
forms incl. `&h`/`&b`/`&o` styled as numbers), F1 opens the topic in kchmviewer.

Known issues / next steps:
1. Native Wayland is not supported; tiko runs through XWayland (GDK_BACKEND=x11) because
   Win32 style popups need absolute positioning.
2. gdb prints a harmless "Failed to set controlling terminal" line in the debug console.
3. F6 while debugging restarts the session (same as upstream; its tooltip says Continue).
