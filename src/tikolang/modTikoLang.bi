'    tiko editor - support for the tiko programming language (https://github.com/PaulSquires/tiko-lang)
'    Licensed under the GNU GPLv3 or later.
'
' The editor works with two compilers: FreeBASIC (fbc) for .bas/.bi/.inc files and the
' tiko compiler for .tiko/.ti/.tko files. Which one builds is decided by the main file
' (the project's main file, or the active document when no project is open).
'
' The tiko language uses .tiko for its source files, the same extension as the editor's
' project files. A .tiko file is a project only when it starts with the "' PROJECT FILE"
' line the editor writes (TikoLang_IsProjectFile); every other .tiko file is source code.

#pragma once

#define TIKOLANG_QUICKRUN   wstr("_tiko_quickrun.tiko")

declare function TikoLang_IsTikoExtension( byref wszFilename as wstring ) as boolean
declare function TikoLang_IsProjectFile( byref wszFilename as wstring ) as boolean
declare function TikoLang_IsSourceFile( byref wszFilename as wstring ) as boolean
declare function TikoLang_IsTikoBuild() as boolean
declare function TikoLang_FindCompiler() as DWSTRING
declare function TikoLang_Compile( byval wID as long, byval idxBuild as long ) as boolean
declare sub TikoLang_SetupLexer( byval hSci as any ptr, byref wszFilename as wstring )
declare function TikoLang_ShowHelp( byref wszWord as wstring ) as boolean
declare function TikoLang_UsesTiko( byref wszFilename as wstring ) as boolean
declare sub TikoLang_LoadSettings()
declare sub TikoLang_SaveSettings()

' "Compilers" page of Environment Options (frmOptionsTikoLang.inc)
dim shared as HWND HWND_FRMOPTIONSTIKOLANG
declare function frmOptionsTikoLang_Show( byval hWndParent as HWND ) as LRESULT
declare sub frmOptionsTikoLang_Save()
