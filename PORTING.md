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
