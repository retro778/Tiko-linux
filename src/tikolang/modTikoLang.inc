'    tiko editor - support for the tiko programming language (https://github.com/PaulSquires/tiko-lang)
'    Licensed under the GNU GPLv3 or later.
'
' See modTikoLang.bi. Building a tiko program:
'   tiko -v --color never --diagnostics-file <tmp> <build options> main.tiko [modules...]
' The build configuration options are shared with FreeBASIC; the fbc-only ones the tiko
' compiler rejects (-gen, -export, ...) are left out (TikoLang_FilterOptions). Errors and
' warnings are read from the diagnostics file (one tab separated record per diagnostic)
' and shown in the Compiler Results list like fbc's. The executable is named after the
' main file; its exact path is taken from the link command that -v prints.

#pragma once

#include once "tkplat.bi"

dim shared as string gTikoLangKeywords
dim shared as boolean gTikoLangKeywordsLoaded

' User settings (settings/tikolang.ini, written by the "Compilers" options page)
type TIKOLANG_SETTINGS
    Loaded        as boolean
    BasWithTiko   as boolean      ' compile .bas/.bi/.inc (and other files) with tiko instead of fbc
    TikoWithFbc   as boolean      ' compile .tiko/.ti/.tko with fbc instead of tiko
    CompilerPath  as string       ' tiko compiler; empty = find it (TikoLang_FindCompiler)
    LanguageServer as boolean     ' check tiko code as you type with tiko-lsp (modTikoLsp.inc)
    DebugWithLldb  as boolean     ' debug with lldb (bin/tiko-lldb-mi + lldb-dap) instead of gdb
end type
dim shared gTikoLang as TIKOLANG_SETTINGS

' folder of the editor executable, with a trailing separator
private function TikoLang_ExeDir() as string
    return TkExeDir()
end function

function TikoLang_IsTikoExtension( byref wszFilename as wstring ) as boolean
    select case lcase(AfxStrPathname("EXTN", wszFilename))
        case ".tiko", ".ti", ".tko": return true
    end select
    return false
end function

' A project file is UTF-16 (BOM) text whose first line is "' PROJECT FILE"
function TikoLang_IsProjectFile( byref wszFilename as wstring ) as boolean
    if lcase(AfxStrPathname("EXTN", wszFilename)) <> APPEXTENSION then return false
    dim as string sPath = TkFs(WideToUtf8(@wszFilename))
    dim as long f = freefile
    if open(sPath for binary access read as #f) <> 0 then return false
    dim as string sHead = space(64)
    get #f, , sHead
    close #f
    ' UTF-16LE with BOM (what the editor writes)
    if left(sHead, 2) = chr(&hFF, &hFE) then
        dim as string s
        for i as long = 3 to len(sHead) - 1 step 2
            if asc(sHead, i + 1) <> 0 then exit for
            s &= chr(asc(sHead, i))
        next
        return left(s, 14) = "' PROJECT FILE"
    end if
    ' tolerate a UTF-8 copy of a project file
    if left(sHead, 3) = chr(&hEF, &hBB, &hBF) then sHead = mid(sHead, 4)
    return left(sHead, 14) = "' PROJECT FILE"
end function

function TikoLang_IsSourceFile( byref wszFilename as wstring ) as boolean
    if TikoLang_IsTikoExtension(wszFilename) = false then return false
    if lcase(AfxStrPathname("EXTN", wszFilename)) = APPEXTENSION then
        ' a .tiko file that does not exist yet (Save As) is source code too
        if AfxFileExists(wszFilename) andalso TikoLang_IsProjectFile(wszFilename) then return false
    end if
    return true
end function

private function TikoLang_SettingsFile() as string
    return TikoLang_ExeDir() & "settings/tikolang.ini"
end function

sub TikoLang_LoadSettings()
    if gTikoLang.Loaded then exit sub
    gTikoLang.Loaded = true
    gTikoLang.LanguageServer = true
    dim as long f = freefile
    if open(TkFs(TikoLang_SettingsFile()) for input as #f) <> 0 then exit sub
    do until eof(f)
        dim as string sLine
        line input #f, sLine
        sLine = trim(sLine, any !" \t\r")
        dim as long p = instr(sLine, "=")
        if (p = 0) orelse (left(sLine, 1) = "'") orelse (left(sLine, 1) = "[") then continue do
        dim as string sKey = lcase(trim(left(sLine, p - 1))), sValue = trim(mid(sLine, p + 1))
        select case sKey
            case "bascompiler":  gTikoLang.BasWithTiko = (lcase(sValue) = "tiko")
            case "tikocompiler": gTikoLang.TikoWithFbc = (lcase(sValue) = "fbc")
            case "tikocompilerpath": gTikoLang.CompilerPath = sValue
            case "languageserver": gTikoLang.LanguageServer = (sValue <> "0")
            case "debugger": gTikoLang.DebugWithLldb = (lcase(sValue) = "lldb")
        end select
    loop
    close #f
end sub

sub TikoLang_SaveSettings()
    dim as long f = freefile
    if open(TkFs(TikoLang_SettingsFile()) for output as #f) <> 0 then exit sub
    print #f, "' Which compiler builds which files (Options > Environment Options > Compilers)"
    print #f, "BasCompiler=" & iif(gTikoLang.BasWithTiko, "tiko", "fbc")
    print #f, "TikoCompiler=" & iif(gTikoLang.TikoWithFbc, "fbc", "tiko")
    print #f, "TikoCompilerPath=" & gTikoLang.CompilerPath
    print #f, "LanguageServer=" & iif(gTikoLang.LanguageServer, "1", "0")
    print #f, "Debugger=" & iif(gTikoLang.DebugWithLldb, "lldb", "gdb")
    close #f
end sub

' true when the file is built (and edited) as tiko: by its extension and the user's choice
function TikoLang_UsesTiko( byref wszFilename as wstring ) as boolean
    TikoLang_LoadSettings()
    if TikoLang_IsSourceFile(wszFilename) then return (gTikoLang.TikoWithFbc = false)
    if lcase(AfxStrPathname("EXTN", wszFilename)) = ".rc" then return false
    return gTikoLang.BasWithTiko
end function

' The debugger window uses bin/tiko-lldb-mi (lldb) instead of gdb
function TikoLang_UseLldb() as boolean
    TikoLang_LoadSettings()
    if gTikoLang.DebugWithLldb = false then return false
    return TkFileExists(TikoLang_ExeDir() & "bin" & TK_SEP & "tiko-lldb-mi" & TK_EXE)
end function

' The program that stands in for gdb when debugging with lldb
function TikoLang_LldbAdapter() as DWSTRING
    return DWSTRING(TikoLang_ExeDir() & "bin" & TK_SEP & "tiko-lldb-mi" & TK_EXE, CP_UTF8)
end function

' File dialog filters: the tiko file types beside the FreeBASIC ones
sub TikoLang_FileTypes( byval pTypes as COMDLG_FILTERSPEC ptr, byval nCount as long )
    static as wstring * 64 wszSource = "*.bas;*.bi;*.inc;*.rc;*.tiko;*.ti;*.tko"
    static as wstring * 64 wszCode = "*.bas;*.inc;*.tiko;*.tko"
    static as wstring * 64 wszHeader = "*.bi;*.ti"
    for i as long = 0 to nCount - 1
        if pTypes[i].pszSpec = 0 then continue for
        select case *pTypes[i].pszSpec
            case "*.bas;*.bi;*.inc;*.rc": pTypes[i].pszSpec = @wszSource
            case "*.bas;*.inc":           pTypes[i].pszSpec = @wszCode
            case "*.bi":                  pTypes[i].pszSpec = @wszHeader
        end select
    next
end sub

' The document that a compile would build: the project's main file or the active document
private function TikoLang_MainDocument() as clsDocument ptr
    if gApp.IsProjectActive then return gApp.GetMainDocumentPtr()
    return gTTabCtl.GetActiveDocumentPtr()
end function

function TikoLang_IsTikoBuild() as boolean
    dim pDoc as clsDocument ptr = TikoLang_MainDocument()
    if pDoc = 0 then return false
    ' tiko.toml itself builds its package
    if lcase(AfxStrPathname("NAMEX", pDoc->DiskFilename)) = "tiko.toml" then return true
    return TikoLang_UsesTiko(pDoc->DiskFilename)
end function

' The tiko compiler: $TIKO_COMPILER, else "tiko" on the PATH (skipping this editor, which is
' also called tiko), else the usual install folders.
function TikoLang_FindCompiler() as DWSTRING
    TikoLang_LoadSettings()
    if len(gTikoLang.CompilerPath) then
        ' the path chosen in the options (a missing file is reported by the compile)
        dim as string sChosen = gTikoLang.CompilerPath
        if left(sChosen, 2) = "~/" then sChosen = TkHomeDir() & mid(sChosen, 2)
        return DWSTRING(sChosen, CP_UTF8)
    end if
    dim as string sEnv = environ("TIKO_COMPILER")
    if (len(sEnv) > 0) andalso TkFileExists(sEnv) then return DWSTRING(sEnv, CP_UTF8)

    ' On the PATH: the compiler is "tiko" too, so skip this editor and take only a tiko that
    ' has its toolchain beside it (Linux: <root>/bin/tiko, Windows: <root>\tiko.exe)
    dim as string sSelfReal = TkRealPath(TkExeDir() & "tiko" & TK_EXE)
    dim as string sPath = environ("PATH")
    dim as long p = 1
    do while p <= len(sPath)
        dim as long q = instr(p, sPath, TK_PATHLIST_SEP)
        if q = 0 then q = len(sPath) + 1
        dim as string sDir = trim(mid(sPath, p, q - p), chr(34))
        p = q + 1
        if len(sDir) = 0 then continue do
        sDir = rtrim(sDir, any "/\")
        dim as string sCand = sDir & TK_SEP & "tiko" & TK_EXE
        if TkFileExists(sCand) = false then continue do
        if TkRealPath(sCand) = sSelfReal then continue do
#ifdef __FB_LINUX__
        if TkFileExists(sDir & "/../lib") orelse TkFileExists(sDir & "/../pkg") then return DWSTRING(sCand, CP_UTF8)
#else
        if TkFileExists(sDir & "\lib") andalso TkFileExists(sDir & "\pkg") then return DWSTRING(sCand, CP_UTF8)
#endif
    loop

    ' the usual install folders
    dim as string sHome = TkHomeDir()
#ifdef __FB_LINUX__
    dim as string sTry(2) = { sHome & "/.local/tiko-lang/bin/tiko", sHome & "/tiko-lang/bin/tiko", "/opt/tiko-lang/bin/tiko" }
#else
    dim as string sTry(2) = { TkExeDir() & "..\tiko-lang\tiko.exe", sHome & "\tiko-lang\tiko.exe", sHome & "\Downloads\tiko-lang\tiko.exe" }
#endif
    for i as long = 0 to 2
        if TkFileExists(sTry(i)) then return DWSTRING(TkFullPath(sTry(i)), CP_UTF8)
    next
    return DWSTRING()
end function

' Build configurations are shared with FreeBASIC. Leave out the fbc options that the tiko
' compiler refuses (TKO0100 unknown option); the ones listed take an argument.
private function TikoLang_FilterOptions( byref wszOptions as DWSTRING ) as DWSTRING
    dim as string sSrc = " " & AfxStrShrink(wszOptions).utf8 & " ", sOut
    dim as string sWithArg = " -a -arch -asm -b -forcelang -gen -include -lang -map -prefix -print -sysroot -title -vec -wa -wc -r -rr "
    dim as string sAlone = " -export -nodeflibs -noerrline -nolib -noobjinfo -nostrip -strip -static -profile -profgen -showincludes -c -rr -r -pp -nocmdline "
    dim as long nCount = AfxStrParseCount(DWSTRING(sSrc), " ")
    dim as long i = 1
    do while i <= nCount
        dim as string sTok = AfxStrParse(DWSTRING(sSrc), i, " ").utf8
        i += 1
        if len(sTok) = 0 then continue do
        dim as string sKey = " " & lcase(sTok) & " "
        if instr(sWithArg, sKey) then
            i += 1          ' skip the option's argument
            continue do
        end if
        if instr(sAlone, sKey) then continue do
        ' fbc's "-O 2" is "-O2" for tiko
        if (sTok = "-O") andalso (i <= nCount) then
            sTok = "-O" & AfxStrParse(DWSTRING(sSrc), i, " ").utf8
            i += 1
        end if
        sOut &= sTok & " "
    loop
    return DWSTRING(rtrim(sOut))
end function

' Removes optimisation options for debug builds
private function TikoLang_RemoveOptimize( byref wszOptions as DWSTRING ) as DWSTRING
    dim as string sSrc = " " & AfxStrShrink(wszOptions).utf8 & " ", sOut
    dim as long nCount = AfxStrParseCount(DWSTRING(sSrc), " ")
    for i as long = 1 to nCount
        dim as string sTok = AfxStrParse(DWSTRING(sSrc), i, " ").utf8
        if len(sTok) = 0 then continue for
        if left(sTok, 2) = "-O" then
            if sTok = "-O" then i += 1
            continue for
        end if
        sOut &= sTok & " "
    next
    return DWSTRING(rtrim(sOut))
end function

private function TikoLang_Version( byref wszCompiler as DWSTRING ) as string
    static as string sCachedFor, sCached
    if sCachedFor = wszCompiler.utf8 then return sCached
    dim as string sText
    RedirConsoleToFile( wszCompiler, chr(34) & wszCompiler & chr(34) & " --version", sText )
    sCachedFor = wszCompiler.utf8
    sCached = trim(AfxStrParse(DWSTRING(sText), 1, vbCrLf).utf8)
    if len(sCached) = 0 then sCached = "tiko"
    return sCached
end function

' The last  -o "file"  of the link command printed by -v
private function TikoLang_OutputFromLog( byref sLog as string ) as string
    dim as long p = instrrev(sLog, " -o " & chr(34))
    if p = 0 then return ""
    dim as long q = instr(p + 5, sLog, chr(34))
    if q = 0 then return ""
    return mid(sLog, p + 5, q - p - 5)
end function

' Reads the --diagnostics-file into the Compiler Results list. Returns true on errors.
private function TikoLang_ReadTextFile( byref sFile as string ) as string
    dim as long f = freefile
    if open(TkFs(sFile) for binary access read as #f) <> 0 then return ""
    dim as string sText = space(lof(f))
    get #f, , sText
    close #f
    return sText
end function

' Records in the --diagnostics-file format, one per line:
'   kind<TAB>id<TAB>path<TAB>line<TAB>col<TAB>endline<TAB>endcol<TAB>message
'   end<TAB>errors<TAB>warnings<TAB>exitcode
private function TikoLang_ShowResults( _
            byref sDiagText as string, _
            byref sLog as string, _
            byval wID as long, _
            byval bShowSuccess as boolean _
            ) as boolean

    dim as HWND hLV = HWND_FRMOUTPUT_LVRESULTS
    frmListView_DeleteAllItems( hLV )

    dim as long nErrors, nWarnings, nNotes, nExitCode = -1
    dim as boolean bEndRecord
    dim as long nRecords = AfxStrParseCount(DWSTRING(sDiagText), chr(10))
    if len(sDiagText) then
        for nRec as long = 1 to nRecords
            dim as string sLine = AfxStrParse(DWSTRING(sDiagText), nRec, chr(10)).utf8
            if right(sLine, 1) = chr(13) then sLine = left(sLine, len(sLine) - 1)
            dim as DWSTRING wLine = DWSTRING(sLine, CP_UTF8)
            dim as string sKind = AfxStrParse(wLine, 1, chr(9)).utf8
            select case sKind
                case "error", "warning", "note"
                    ' kind id path line col endline endcol message
                    dim as DWSTRING wId   = AfxStrParse(wLine, 2, chr(9))
                    dim as DWSTRING wFile = AfxStrParse(wLine, 3, chr(9))
                    dim as DWSTRING wNum  = AfxStrParse(wLine, 4, chr(9))
                    dim as DWSTRING wMsg  = AfxStrParse(wLine, 8, chr(9))
                    dim as long nField = AfxStrParseCount(wLine, chr(9))
                    for k as long = 9 to nField     ' a tab inside the message
                        wMsg &= " " & AfxStrParse(wLine, k, chr(9))
                    next
                    if val(wNum.utf8) <= 0 then wNum = ""
                    frmListView_AddString( hLV, "" & chr(9) & wNum & chr(9) & wFile & chr(9) & _
                        DWSTRING(sKind) & " " & wId & ": " & wMsg )
                    select case sKind
                        case "error":   nErrors += 1
                        case "warning": nWarnings += 1
                        case else:      nNotes += 1
                    end select
                case "end"
                    bEndRecord = true
                    nExitCode = valint(AfxStrParse(wLine, 4, chr(9)).utf8)
            end select
        next
    end if

    ' The compiler did not finish, or clang / the linker failed: show the last lines of the log
    dim as boolean bFailedRun = (bEndRecord = false)
    if (nErrors = 0) andalso (nExitCode > 0) then bFailedRun = true
    if bFailedRun then
        dim as string sTail
        dim as long nLines = AfxStrParseCount(DWSTRING(sLog), vbCrLf)
        for k as long = nLines to 1 step -1
            sTail = trim(AfxStrParse(DWSTRING(sLog), k, vbCrLf).utf8)
            if len(sTail) then exit for
        next
        if len(sTail) = 0 then sTail = "the tiko compiler did not run"
        frmListView_AddString( hLV, "" & chr(9) & "" & chr(9) & "" & chr(9) & _
            DWSTRING("compiling FAILED: " & sTail & " (refer to log file)", CP_UTF8) )
        nErrors += 1
    end if

    ' No error but no program either
    if (nErrors = 0) andalso (AfxGetFileSize(gCompile.OutputFilename) = 0) then
        frmListView_AddString( hLV, "" & chr(9) & "" & chr(9) & "" & chr(9) & _
            "compiling FAILED: no output file (refer to log file)" )
        nErrors += 1
    end if

    dim as DWSTRING wszMsg
    if (nErrors > 0) orelse (nWarnings > 0) orelse (nNotes > 0) then
        wszMsg = L(193, "Errors") & " " & nErrors & "  " & L(192, "Warnings") & " " & nWarnings & _
                 "  [" & AfxLocalDateStr("yyyy-MM-dd") & "  " & AfxLocalTimeStr("hh:mm:ss") & "]"
        if nErrors > 0 then
            gCompile.wszOutputMsg = L(229,"Failed Compile") & " (" & wszMsg & ")" & vbcrlf
            SetCompileStatusBarMessage( wszMsg, ghIconBad )
            MessageBeep(MB_ICONWARNING)
        end if
        frmListView_SelectItem( hLV, 0 )
        select case gOutputTabsCurSel
            case 1, 2, 3, 4
                gOutputTabsCurSel = 0
                frmOutput_PositionWindows
        end select
        if nErrors > 0 then frmOutput_RestorePanel()
        SetDocumentErrorPosition( hLV, wID )
    end if

    if (nErrors = 0) andalso bShowSuccess then
        dim as long nFileSize = AfxGetFileSize(gCompile.OutputFilename)
        wszMsg = L(193, "Errors") & " " & nErrors & "  " & L(192, "Warnings") & " " & nWarnings & _
                 "  [ " & AfxStrPathname("NAMEX", gCompile.OutputFilename) & "  " & _
                 rtrim(AfxStrFormatKBSize(nFileSize)) & " (" & wstr(nFileSize) & " " & L(199,"bytes") & ") ]"
        SetCompileStatusBarMessage( wszMsg, ghIconGood )
        if gConfig.DisableCompileBeep = false then MessageBeep(MB_OK)
        dim as DWSTRING wszCounts = L(193,"Errors") & " " & wstr(nErrors) & " " & L(192,"Warnings") & " " & wstr(nWarnings)
        gCompile.wszOutputMsg = gCompile.wszOutputMsg & _
            L(228,"Successful Compile") & " (" & wszCounts & ")" & vbcrlf & vbcrlf & _
            L(194,"Primary Source:") & " " & gCompile.MainFilename & vbCrLf & _
            L(195,"Target Compilation:") & " " & gCompile.OutputFilename & " (" & _
                 rtrim(AfxStrFormatKBSize(nFileSize)) & ", " & wstr(nFileSize) & " " & L(199,"bytes") & ")" & vbcrlf & _
            L(196,"Compile Time:") & " " & format(gCompile.EndTime - gCompile.StartTime, "###0.0") & _
                 " " & L(198,"seconds") & vbcrlf
    end if

    SetLogFileTextbox()

    if nErrors > 0 then
        gCompile.RunAfterCompile = false
        return true
    end if
    return false
end function

' ----------------------------------------------------------------------------------------
' tiko.toml packages: a file built with tiko that has a tiko.toml in its folder or above
' (and no editor project is open) is built as that package with 'tiko build', like tkEdit
' and the VS Code extension do. The build reports one JSON event per line; the compiler's
' diagnostics are text lines between them:  path(line,col): kind TKOnnnn: message
' ----------------------------------------------------------------------------------------

' Folder (with trailing separator) of the tiko.toml at or above the file, or ""
function TikoLang_FindPackage( byref wszFilename as wstring ) as string
    if len(wszFilename) = 0 then return ""
    dim as string sDir = TkDirOf(WideToUtf8(@wszFilename))
    for i as long = 1 to 64
        if len(sDir) = 0 then exit for
        if TkFileExists(sDir & "tiko.toml") then return sDir
        if len(sDir) <= 3 then exit for          ' "/" or "C:\"
        sDir = TkDirOf(left(sDir, len(sDir) - 1))
    next
    return ""
end function

' The string value of "key":"..." in one JSON line
private function TikoLang_JsonStr( byref sJson as string, byref sKey as string ) as string
    dim as string sFind = chr(34) & sKey & chr(34) & ":" & chr(34)
    dim as long p = instr(sJson, sFind)
    if p = 0 then return ""
    p += len(sFind)
    dim as string sOut
    do while p <= len(sJson)
        dim as string c = mid(sJson, p, 1)
        if c = chr(34) then exit do
        if c = "\" then
            p += 1
            c = mid(sJson, p, 1)
            if c = "n" then c = chr(10) else if c = "t" then c = chr(9)
        end if
        sOut &= c
        p += 1
    loop
    return sOut
end function

' The number value of "key":n in one JSON line
private function TikoLang_JsonNum( byref sJson as string, byref sKey as string ) as long
    dim as string sFind = chr(34) & sKey & chr(34) & ":"
    dim as long p = instr(sJson, sFind)
    if p = 0 then return 0
    return valint(mid(sJson, p + len(sFind)))
end function

' A compiler diagnostic line "path(line,col): kind TKOnnnn: message" as a diagnostics record
private function TikoLang_DiagLineToRecord( byref sLine as string, byref sRoot as string ) as string
    dim as long nParenEnd = instr(sLine, "): ")
    if nParenEnd = 0 then return ""
    dim as long nParenStart = instrrev(sLine, "(", nParenEnd)
    if nParenStart <= 1 then return ""
    dim as string sPos = mid(sLine, nParenStart + 1, nParenEnd - nParenStart - 1)
    dim as long pComma = instr(sPos, ",")
    if pComma = 0 then return ""
    dim as string sRest = mid(sLine, nParenEnd + 3)
    dim as long pSp = instr(sRest, " ")
    if pSp = 0 then return ""
    dim as string sKind = left(sRest, pSp - 1)
    if (sKind <> "error") andalso (sKind <> "warning") andalso (sKind <> "note") then return ""
    dim as long pColon = instr(pSp + 1, sRest, ": ")
    if pColon = 0 then return ""
    dim as string sId = mid(sRest, pSp + 1, pColon - pSp - 1)
    dim as string sFile = left(sLine, nParenStart - 1)
    if TkIsAbsolute(sFile) = false then sFile = sRoot & sFile
    return sKind & chr(9) & sId & chr(9) & sFile & chr(9) & left(sPos, pComma - 1) & chr(9) & _
           mid(sPos, pComma + 1) & chr(9) & chr(9) & chr(9) & mid(sRest, pColon + 2)
end function

private function TikoLang_BuildPackage( _
            byval wID as long, _
            byval idxBuild as long, _
            byval pDocMain as clsDocument ptr, _
            byref sRoot as string, _
            byref wszCompiler as DWSTRING _
            ) as boolean
    dim as DWSTRING wDQ = wchr(34)

    for y as long = lbound(gConfig.Tools) to ubound(gConfig.Tools)
        if gConfig.Tools(y).Action = USERTOOL_ACTION_PRECOMPILE then frmUserTools_ExecuteUserTool(y)
    next

    gCompile.MainFilename = pDocMain->DiskFilename
    gCompile.MainName     = AfxStrPathname("NAME", pDocMain->DiskFilename)
    gCompile.MainFolder   = DWSTRING(sRoot, CP_UTF8)
    gCompile.ResourceFile = ""
    gCompile.LinkModules  = ""

    ' Profile: debug for debugging and for build configurations with -g, else release
    dim as string sOptions = " " & lcase(gConfig.Builds(idxBuild).wszOptions.utf8) & " "
    dim as string sProfile = iif(instr(sOptions, " -g "), "debug", "release")
    if wID = IDM_DEBUG_STARTDEBUGGING then sProfile = "debug"
    gCompile.CompileFlags = "--profile " & sProfile
    if instr(sOptions, " -s gui ") then gCompile.CompileFlags = gCompile.CompileFlags & " -s gui"

    dim as DWSTRING wszParams = "build --message-format json --profile " & sProfile
    if wID = IDM_REBUILDALL then wszParams = wszParams & " --no-cache"

    gApp.IsCompiling = true
    dim as HCURSOR hCurSave = GetCursor()
    SetCursor( LoadCursor(0, IDC_WAIT) )
    SetCompileStatusBarMessage( "#" & L(203, "Compiling") & "... " & DWSTRING(sRoot & "tiko.toml", CP_UTF8), 0 )

    chdir TkFs(sRoot)
    gCompile.StartTime = timer
    dim as CompileThreadParams ctp
    ctp.wszExe = wszCompiler
    ctp.wszCmdLine = wDQ & wszCompiler & wDQ & " " & wszParams
    gCompile.bCompileThreadComplete = false
    dim as any ptr pThread = ThreadCreate( @compile_thread, @ctp )
    do until gCompile.bCompileThreadComplete = true
        AfxDoEvents()
    loop
    if pThread then ThreadWait( pThread )
    gCompile.EndTime = timer
    gCompile.SystemTime = AfxSystemSystemTime
    SetCursor( hCurSave )
    ResetScintillaCursors()
    gApp.IsCompiling = false

    gCompile.wszFullCommandLine = "cd " & DWSTRING(sRoot, CP_UTF8) & vbCrLf & wszCompiler & " " & wszParams
    gCompile.wszFullLogFile = DWSTRING(ctp.sConsoleText, CP_UTF8)
    gCompile.wszOutputMsg = DWSTRING(TikoLang_Version(wszCompiler)) & vbCrLf & _
                            "Package: " & DWSTRING(sRoot & "tiko.toml", CP_UTF8) & "  (profile " & sProfile & ")" & vbCrLf

    ' Read the events and diagnostics
    dim as string sMainRel = WideToUtf8(@gCompile.MainFilename)
    if left(sMainRel, len(sRoot)) = sRoot then sMainRel = mid(sMainRel, len(sRoot) + 1)
    dim as string sRecords, sTargetRoots, sFirstExe, sMainExe, sAnyArtifact
    dim as long nErrors, nWarnings, nExit
    dim as boolean bEnded
    dim as string sLog = ctp.sConsoleText
    dim as long p = 1
    do while p <= len(sLog)
        dim as long q = instr(p, sLog, chr(10))
        if q = 0 then q = len(sLog) + 1
        dim as string sLine = mid(sLog, p, q - p)
        p = q + 1
        if right(sLine, 1) = chr(13) then sLine = left(sLine, len(sLine) - 1)
        if left(sLine, 1) = "{" then
            select case TikoLang_JsonStr(sLine, "event")
                case "build-start"
                    sTargetRoots = sLine        ' the "roots" of the target being built
                case "artifact"
                    dim as string sPath = TikoLang_JsonStr(sLine, "path")
                    if TkIsAbsolute(sPath) = false then sPath = sRoot & sPath
                    if len(sAnyArtifact) = 0 then sAnyArtifact = sPath
                    if TikoLang_JsonStr(sLine, "kind") = "exe" then
                        if len(sFirstExe) = 0 then sFirstExe = sPath
                        if instr(sTargetRoots, chr(34) & sMainRel & chr(34)) then sMainExe = sPath
                    end if
                case "build-end"
                    bEnded = true
                    nErrors += TikoLang_JsonNum(sLine, "errors")
                    nWarnings += TikoLang_JsonNum(sLine, "warnings")
                    if TikoLang_JsonNum(sLine, "exit") <> 0 then nExit = TikoLang_JsonNum(sLine, "exit")
            end select
        else
            dim as string sRec = TikoLang_DiagLineToRecord(sLine, sRoot)
            if len(sRec) then sRecords &= sRec & chr(10)
        end if
    loop
    if bEnded then sRecords &= "end" & chr(9) & nErrors & chr(9) & nWarnings & chr(9) & nExit & chr(10)

    dim as string sOutput = iif(len(sMainExe), sMainExe, iif(len(sFirstExe), sFirstExe, sAnyArtifact))
    gCompile.OutputFilename = DWSTRING(sOutput, CP_UTF8)
    if len(sOutput) = 0 then gCompile.OutputFilename = DWSTRING(sRoot & "out/" & sProfile, CP_UTF8)

    if TikoLang_ShowResults( sRecords, sLog, wID, true ) then return true

    for y as long = lbound(gConfig.Tools) to ubound(gConfig.Tools)
        if gConfig.Tools(y).Action = USERTOOL_ACTION_POSTCOMPILE then frmUserTools_ExecuteUserTool(y)
    next
    ' only an executable can be run or debugged
    if len(sMainExe) = 0 andalso len(sFirstExe) = 0 then return true
    if wID = IDM_DEBUG_STARTDEBUGGING then
        frmDebug_Show( HWND_FRMMAIN, gCompile.OutputFilename, gApp.wszCommandLine )
    end if
    if gCompile.RunAfterCompile then
        RunEXE( pDocMain, gCompile.OutputFilename, gApp.wszCommandLine )
    end if
    return true
end function

' Compile (and run / debug) a tiko program. Called by code_Compile when the main file is
' tiko source. Returns what code_Compile returns.
function TikoLang_Compile( byval wID as long, byval idxBuild as long ) as boolean
    dim as DWSTRING wDQ = wchr(34)

    dim as DWSTRING wszCompiler = TikoLang_FindCompiler()
    if (len(wszCompiler) > 0) andalso (AfxFileExists(wszCompiler) = 0) then
        SetCompileStatusBarMessage( "", 0 )
        MessageBox( HWND_FRMMAIN, L(202,"Invalid defined compiler path.") & vbcrlf & "Path: " & wszCompiler & _
            vbcrlf & vbcrlf & "(Environment Options > Compilers)", L(201,"Error"), _
            MB_OK or MB_ICONINFORMATION or MB_DEFBUTTON1 )
        return false
    end if
    if len(wszCompiler) = 0 then
        SetCompileStatusBarMessage( "", 0 )
        MessageBox( HWND_FRMMAIN, _
            "The tiko compiler was not found." & vbcrlf & vbcrlf & _
            "Choose it in Environment Options > Compilers, put the tiko-lang/bin" & vbcrlf & _
            "folder on the PATH, or set the TIKO_COMPILER environment variable.", _
            L(201,"Error"), MB_OK or MB_ICONINFORMATION or MB_DEFBUTTON1 )
        return false
    end if
    gCompile.CompilerPath = wszCompiler

    gCompile.CompileID = wID
    select case wID
        case IDM_BUILDEXECUTE, IDM_QUICKRUN: gCompile.RunAfterCompile = true
        case else:                           gCompile.RunAfterCompile = false
    end select

    dim pDocMain as clsDocument ptr = TikoLang_MainDocument()
    if pDocMain = 0 then
        SetCompileStatusBarMessage( "", 0 )
        return false
    end if

    ' A file of a tiko.toml package (no editor project open): build the package
    if (gApp.IsProjectActive = false) andalso (wID <> IDM_QUICKRUN) then
        dim as string sRoot = TikoLang_FindPackage(pDocMain->DiskFilename)
        if len(sRoot) then return TikoLang_BuildPackage( wID, idxBuild, pDocMain, sRoot, wszCompiler )
    end if

    for y as long = lbound(gConfig.Tools) to ubound(gConfig.Tools)
        if gConfig.Tools(y).Action = USERTOOL_ACTION_PRECOMPILE then frmUserTools_ExecuteUserTool(y)
    next

    if wID = IDM_QUICKRUN then
        if AfxFileExists(pDocMain->DiskFilename) then
            gCompile.MainFolder = AfxStrPathname("PATH", pDocMain->DiskFilename)
        else
            gCompile.MainFolder = AfxGetExePathName
        end if
        gCompile.MainFilename = gCompile.MainFolder & TIKOLANG_QUICKRUN
        gCompile.MainName     = AfxStrPathname("NAME", gCompile.MainFilename)
        pDocMain->SaveTempFile( gCompile.MainFilename )
    else
        gCompile.MainFilename = pDocMain->DiskFilename
        gCompile.MainName     = AfxStrPathname("NAME", pDocMain->DiskFilename)
        gCompile.MainFolder   = AfxStrPathname("PATH", pDocMain->DiskFilename)
    end if
    gCompile.ResourceFile = ""
    gCompile.LinkModules = ""

    ' Options: build configuration + project or default switches + include paths
    dim as DWSTRING wszFlags = gConfig.Builds(idxBuild).wszOptions
    if gApp.IsProjectActive then
        wszFlags = wszFlags & " " & gApp.ProjectOther64
    else
        wszFlags = wszFlags & " " & gConfig.CompilerSwitches
    end if
    dim as long numpaths = AfxStrParseCount(gConfig.CompilerIncludes, ";")
    for i as long = 1 to numpaths
        dim as DWSTRING includePath = rtrim(AfxStrParse(gConfig.CompilerIncludes, i, ";"))
        if len(includePath) then wszFlags = wszFlags & " -i " & wDQ & includePath & wDQ
    next

    ' '#CONSOLE / '#GUI directives in the main file
    redim directives(any) as COMPILE_DIRECTIVES
    pDocMain->CompileDirectives(directives())
    for i as long = lbound(directives) to ubound(directives)
        select case directives(i).DirectiveFlag
            case IDM_CONSOLE
                wszFlags = AfxStrReplace(wszFlags, "-s gui", "") & " -s console"
            case IDM_GUI
                wszFlags = AfxStrReplace(wszFlags, "-s console", "") & " -s gui"
        end select
    next

    ' Debugging: no optimisation, debug information and the runtime checks
    if wID = IDM_DEBUG_STARTDEBUGGING then
        wszFlags = TikoLang_RemoveOptimize(wszFlags) & " -g -exx"
    end if
    wszFlags = TikoLang_FilterOptions(wszFlags)
    gCompile.CompileFlags = wszFlags

    ' The program is named after the main (first) source file
    dim as string sFlagsU = " " & ucase(wszFlags.utf8) & " "
    if instr(sFlagsU, " -DLL ") orelse instr(sFlagsU, " -DYLIB ") then
#ifdef __FB_LINUX__
        gCompile.OutputFilename = gCompile.MainFolder & "lib" & gCompile.MainName & DLL_EXTENSION
#else
        gCompile.OutputFilename = gCompile.MainFolder & gCompile.MainName & DLL_EXTENSION
#endif
    elseif instr(sFlagsU, " -LIB ") then
        gCompile.OutputFilename = gCompile.MainFolder & "lib" & gCompile.MainName & ".a"
    else
        gCompile.OutputFilename = gCompile.MainFolder & gCompile.MainName & EXE_EXTENSION
    end if
    if wID = IDM_QUICKRUN then gApp.wszQuickRunFilename = gCompile.OutputFilename

    if IsProcessRunning(@gCompile.OutputFilename) then
        SetCompileStatusBarMessage( "", 0 )
        MessageBox( HWND_FRMMAIN, L(200,"Program running") & "...", L(201,"Error"), _
                    MB_OK or MB_ICONINFORMATION or MB_DEFBUTTON1 )
        return false
    end if

    ' main file first, then the project's modules: tiko compiles them in one run
    dim as DWSTRING wszInputs = wDQ & gCompile.MainFilename & wDQ
    if gApp.IsProjectActive then
        dim pDoc as clsDocument ptr = gApp.pDocList
        do until pDoc = 0
            if pDoc->ProjectFileType = FILETYPE_MODULE then
                wszInputs = wszInputs & " " & wDQ & pDoc->DiskFilename & wDQ
            end if
            pDoc = pDoc->pDocNext
        loop
    end if

    dim as string sDiagFile = TkTempDir() & "tiko-editor-diag-" & str(TkPid()) & ".txt"
    kill TkFs(sDiagFile)
    if AfxFileExists(gCompile.OutputFilename) then AfxDeleteFile(gCompile.OutputFilename)

    dim as DWSTRING wszParams = "-v --color never --diagnostics-file " & wDQ & DWSTRING(sDiagFile) & wDQ & _
                                " " & wszFlags & " " & wszInputs

    gApp.IsCompiling = true
    dim as HCURSOR hCurSave = GetCursor()
    SetCursor( LoadCursor(0, IDC_WAIT) )
    SetCompileStatusBarMessage( "#" & L(203, "Compiling") & "... " & AfxStrPathname("NAMEX", gCompile.MainFilename), 0 )

    chdir gCompile.MainFolder
    gCompile.StartTime = timer

    dim as CompileThreadParams ctp
    ctp.wszExe = wszCompiler
    ' the first word is the program name (argv[0]), as with Windows' CreateProcess
    ctp.wszCmdLine = wDQ & wszCompiler & wDQ & " " & wszParams
    ctp.sConsoleText = ""
    gCompile.bCompileThreadComplete = false
    dim as any ptr pThread = ThreadCreate( @compile_thread, @ctp )
    do until gCompile.bCompileThreadComplete = true
        AfxDoEvents()
    loop
    if pThread then ThreadWait( pThread )

    gCompile.EndTime = timer
    gCompile.SystemTime = AfxSystemSystemTime
    SetCursor( hCurSave )
    ResetScintillaCursors()

    gCompile.wszFullCommandLine = wszCompiler & " " & wszParams
    gCompile.wszFullLogFile = DWSTRING(ctp.sConsoleText, CP_UTF8)
    gCompile.wszOutputMsg = DWSTRING(TikoLang_Version(wszCompiler)) & vbCrLf

    dim as string sOut = TikoLang_OutputFromLog(ctp.sConsoleText)
    if len(sOut) then
        if TkIsAbsolute(sOut) = false then sOut = WideToUtf8(@gCompile.MainFolder) & sOut
        gCompile.OutputFilename = DWSTRING(sOut, CP_UTF8)
    end if

    if wID = IDM_QUICKRUN then AfxDeleteFile( gCompile.MainFilename )
    gApp.IsCompiling = false

    dim as boolean bFailed = TikoLang_ShowResults( TikoLang_ReadTextFile(sDiagFile), ctp.sConsoleText, wID, true )
    kill TkFs(sDiagFile)
    if bFailed then return true

    for y as long = lbound(gConfig.Tools) to ubound(gConfig.Tools)
        if gConfig.Tools(y).Action = USERTOOL_ACTION_POSTCOMPILE then frmUserTools_ExecuteUserTool(y)
    next

    if wID = IDM_DEBUG_STARTDEBUGGING then
        frmDebug_Show( HWND_FRMMAIN, gCompile.OutputFilename, _
                       iif(gApp.IsProjectActive, gApp.ProjectCommandLine, gApp.wszCommandLine) )
    end if
    if gCompile.RunAfterCompile then
        RunEXE( pDocMain, gCompile.OutputFilename, _
                iif(gApp.IsProjectActive, gApp.ProjectCommandLine, gApp.wszCommandLine) )
    end if
    return true
end function

' Called for every editor window when its language properties are applied: tiko files
' get the tiko keywords (on top of FreeBASIC's) and """ block strings.
sub TikoLang_SetupLexer( byval hSci as any ptr, byref wszFilename as wstring )
    ' the tiko language: tiko files, and FreeBASIC files that the user builds with tiko
    dim as boolean bTiko = TikoLang_IsSourceFile(wszFilename)
    if (len(wszFilename) > 0) andalso TikoLang_UsesTiko(wszFilename) then bTiko = true
    dim as string sProp = "lexer.tiko.blockstrings", sValue = iif(bTiko, "1", "0")
    SciMsg( hSci, SCI_SETPROPERTY, cast(WPARAM, strptr(sProp)), cast(LPARAM, strptr(sValue)) )
    if bTiko = false then exit sub

    if gTikoLangKeywordsLoaded = false then
        gTikoLangKeywordsLoaded = true
        dim as string sFile = TikoLang_ExeDir() & "settings/keywords/tiko_keywords.txt"
        dim as long f = freefile
        if open(TkFs(sFile) for input as #f) = 0 then
            do until eof(f)
                dim as string sLine
                line input #f, sLine
                gTikoLangKeywords &= " " & lcase(trim(sLine, any !" \t\r"))
            loop
            close #f
        end if
    end if
    dim as string sKeywords = gConfig.FBKeywords & " " & gTikoLangKeywords
    SciMsg( hSci, SCI_SETKEYWORDS, 0, cast(LPARAM, strptr(sKeywords)) )

    ' block strings look like strings
    SciMsg( hSci, SCI_STYLESETFORE, SCE_B_DATE, SciMsg(hSci, SCI_STYLEGETFORE, SCE_B_STRING, 0) )
    SciMsg( hSci, SCI_STYLESETBACK, SCE_B_DATE, SciMsg(hSci, SCI_STYLEGETBACK, SCE_B_STRING, 0) )
    SciMsg( hSci, SCI_STYLESETBOLD, SCE_B_DATE, SciMsg(hSci, SCI_STYLEGETBOLD, SCE_B_STRING, 0) )
    SciMsg( hSci, SCI_STYLESETITALIC, SCE_B_DATE, SciMsg(hSci, SCI_STYLEGETITALIC, SCE_B_STRING, 0) )
    SciMsg( hSci, SCI_COLOURISE, 0, -1 )
    TikoLsp_SetupWindow( hSci )     ' errors as you type, hover, F12
end sub

' F1 in a tiko file: the keyword's page of the tiko wiki (settings/help/tiko_wiki_index.txt)
function TikoLang_ShowHelp( byref wszWord as wstring ) as boolean
    dim as string sWord = lcase(trim(WideToUtf8(@wszWord)))
    dim as string sPage
    if len(sWord) then
        dim as string sFile = TikoLang_ExeDir() & "settings/help/tiko_wiki_index.txt"
        dim as long f = freefile
        if open(TkFs(sFile) for input as #f) = 0 then
            do until eof(f)
                dim as string sLine
                line input #f, sLine
                dim as long t = instr(sLine, chr(9))
                if t andalso (left(sLine, t - 1) = sWord) then
                    sPage = trim(mid(sLine, t + 1), any !" \r")
                    exit do
                end if
            loop
            close #f
        end if
    end if
    if len(sPage) = 0 then sPage = "Keyword-Index"
    dim as DWSTRING url = "https://github.com/PaulSquires/tiko-lang/wiki/" & sPage
    ShellExecute( NULL, "open", url.vptr, NULL, NULL, SW_SHOWNORMAL )
    return true
end function

#include once "frmOptionsTikoLang.inc"
#include once "modTikoLsp.inc"
