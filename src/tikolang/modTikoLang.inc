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

extern "C"
    declare function realpath( byval path as const zstring ptr, byval resolved as zstring ptr ) as zstring ptr
end extern

dim shared as string gTikoLangKeywords
dim shared as boolean gTikoLangKeywordsLoaded

' folder of the editor executable, with a trailing "/"
private function TikoLang_ExeDir() as string
    dim as DWSTRING w = AfxGetExePathName
    dim as string s = w.utf8
    if right(s, 1) <> "/" then s &= "/"
    return s
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
    dim as string sPath = WideToUtf8(@wszFilename)
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

' The document that a compile would build: the project's main file or the active document
private function TikoLang_MainDocument() as clsDocument ptr
    if gApp.IsProjectActive then return gApp.GetMainDocumentPtr()
    return gTTabCtl.GetActiveDocumentPtr()
end function

function TikoLang_IsTikoBuild() as boolean
    dim pDoc as clsDocument ptr = TikoLang_MainDocument()
    if pDoc = 0 then return false
    return TikoLang_IsSourceFile(pDoc->DiskFilename)
end function

' The tiko compiler: $TIKO_COMPILER, else "tiko" on the PATH (skipping this editor, which is
' also called tiko), else the usual install folders.
function TikoLang_FindCompiler() as DWSTRING
    dim as string sEnv = environ("TIKO_COMPILER")
    if (len(sEnv) > 0) andalso (AfxFileExists(DWSTRING(sEnv)) <> 0) then return DWSTRING(sEnv)

    dim as string sSelf = TikoLang_ExeDir() & "tiko"
    dim as zstring ptr pSelf = realpath( sSelf, NULL )
    dim as string sSelfReal = iif(pSelf, *pSelf, sSelf)
    if pSelf then free(pSelf)

    dim as string sPath = environ("PATH")
    dim as long nCount = AfxStrParseCount(DWSTRING(sPath), ":")
    for i as long = 1 to nCount
        dim as string sDir = AfxStrParse(DWSTRING(sPath), i, ":").utf8
        if len(sDir) = 0 then continue for
        dim as string sCand = sDir & "/tiko"
        if AfxFileExists(DWSTRING(sCand)) = 0 then continue for
        dim as zstring ptr pReal = realpath( sCand, NULL )
        dim as string sReal = iif(pReal, *pReal, sCand)
        if pReal then free(pReal)
        if sReal = sSelfReal then continue for
        ' a tiko-lang toolchain keeps its runtime beside the compiler
        if AfxFileExists(DWSTRING(sDir & "/../lib")) orelse AfxFileExists(DWSTRING(sDir & "/../pkg")) then
            return DWSTRING(sCand)
        end if
    next

    dim as string sHome = environ("HOME")
    dim as string sTry(2) = { sHome & "/.local/tiko-lang/bin/tiko", sHome & "/tiko-lang/bin/tiko", "/opt/tiko-lang/bin/tiko" }
    for i as long = 0 to 2
        if AfxFileExists(DWSTRING(sTry(i))) then return DWSTRING(sTry(i))
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
    RedirConsoleToFile( wszCompiler, "--version", sText )
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
private function TikoLang_ShowResults( _
            byref sDiagFile as string, _
            byref sLog as string, _
            byval wID as long, _
            byval bShowSuccess as boolean _
            ) as boolean

    dim as HWND hLV = HWND_FRMOUTPUT_LVRESULTS
    frmListView_DeleteAllItems( hLV )

    dim as long nErrors, nWarnings, nNotes, nExitCode = -1
    dim as boolean bEndRecord
    dim as long f = freefile
    if open(sDiagFile for input as #f) = 0 then
        do until eof(f)
            dim as string sLine
            line input #f, sLine
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
        loop
        close #f
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
        gCompile.wszOutputMsg = gCompile.wszOutputMsg & _
            L(228,"Successful Compile") & " (" & L(193,"Errors") & " " & nErrors & " " & _
                                                 L(192,"Warnings") & " " & nWarnings & ")" & vbcrlf & vbcrlf & _
            L(194,"Primary Source:") & " " & gCompile.MainFilename & vbCrLf & _
            L(195,"Target Compilation:") & " " & gCompile.OutputFilename & " (" & _
                 rtrim(AfxStrFormatKBSize(nFileSize)) & ", " & nFileSize & " " & L(199,"bytes") & ")" & vbcrlf & _
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

' Compile (and run / debug) a tiko program. Called by code_Compile when the main file is
' tiko source. Returns what code_Compile returns.
function TikoLang_Compile( byval wID as long, byval idxBuild as long ) as boolean
    dim as DWSTRING wDQ = wchr(34)

    dim as DWSTRING wszCompiler = TikoLang_FindCompiler()
    if len(wszCompiler) = 0 then
        SetCompileStatusBarMessage( "", 0 )
        MessageBox( HWND_FRMMAIN, _
            "The tiko compiler was not found." & vbcrlf & vbcrlf & _
            "Put the tiko-lang/bin folder on the PATH, or set the TIKO_COMPILER" & vbcrlf & _
            "environment variable to the compiler (for example ~/tiko-lang/bin/tiko).", _
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
        gCompile.OutputFilename = gCompile.MainFolder & "lib" & gCompile.MainName & DLL_EXTENSION
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
            if (pDoc->ProjectFileType = FILETYPE_MODULE) andalso TikoLang_IsTikoExtension(pDoc->DiskFilename) then
                wszInputs = wszInputs & " " & wDQ & pDoc->DiskFilename & wDQ
            end if
            pDoc = pDoc->pDocNext
        loop
    end if

    dim as string sDiagFile = *g_get_tmp_dir() & "/tiko-editor-diag-" & str(getpid()) & ".txt"
    kill sDiagFile
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
    ctp.wszCmdLine = wszParams
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
        if left(sOut, 1) <> "/" then sOut = WideToUtf8(@gCompile.MainFolder) & sOut
        gCompile.OutputFilename = DWSTRING(sOut, CP_UTF8)
    end if

    if wID = IDM_QUICKRUN then AfxDeleteFile( gCompile.MainFilename )
    gApp.IsCompiling = false

    dim as boolean bFailed = TikoLang_ShowResults( sDiagFile, ctp.sConsoleText, wID, true )
    kill sDiagFile
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
    dim as boolean bTiko = TikoLang_IsSourceFile(wszFilename)
    dim as string sProp = "lexer.tiko.blockstrings", sValue = iif(bTiko, "1", "0")
    SciMsg( hSci, SCI_SETPROPERTY, cast(WPARAM, strptr(sProp)), cast(LPARAM, strptr(sValue)) )
    if bTiko = false then exit sub

    if gTikoLangKeywordsLoaded = false then
        gTikoLangKeywordsLoaded = true
        dim as string sFile = TikoLang_ExeDir() & "settings/keywords/tiko_keywords.txt"
        dim as long f = freefile
        if open(sFile for input as #f) = 0 then
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
end sub

' F1 in a tiko file: the keyword's page of the tiko wiki (settings/help/tiko_wiki_index.txt)
function TikoLang_ShowHelp( byref wszWord as wstring ) as boolean
    dim as string sWord = lcase(trim(WideToUtf8(@wszWord)))
    dim as string sPage
    if len(sWord) then
        dim as string sFile = TikoLang_ExeDir() & "settings/help/tiko_wiki_index.txt"
        dim as long f = freefile
        if open(sFile for input as #f) = 0 then
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
