'    tiko editor - tiko-lsp client (Language Server Protocol, https://github.com/PaulSquires/tiko-lang)
'    Licensed under the GNU GPLv3 or later.
'
' tiko-lsp (beside the tiko compiler) is started when the first file built with tiko is
' open. It runs as a child process talking JSON-RPC over its stdin/stdout; its output is
' read by a GLib watch on the main loop, so nothing blocks the editor.
'
'   * documents: a 300 ms timer opens/closes the editor's tiko documents with the server;
'     edits (SCN_MODIFIED -> TikoLsp_DocModified) are sent as full text after a pause
'   * errors as you type: publishDiagnostics -> squiggly underlines (indicators 20/21),
'     the message shows when the mouse rests on one (SCN_DWELLSTART)
'   * hover: the mouse resting on a name shows its declaration (textDocument/hover)
'   * F12: textDocument/definition, else the editor's own Goto Definition

#pragma once

#define TKLSP_INDIC_ERROR     20
#define TKLSP_INDIC_WARNING   21
#define TKLSP_CHANGE_DELAY    0.4      ' seconds after the last edit before the text is sent

#ifndef SCI_COUNTCODEUNITS
    #define SCI_COUNTCODEUNITS              2715
    #define SCI_POSITIONRELATIVECODEUNITS   2716
#endif
#ifndef SCI_SETMOUSEDWELLTIME
    #define SCI_SETMOUSEDWELLTIME           2264
#endif

#include once "tkplat.bi"
#include once "tkproc.bi"

#include once "tkjson.bi"

' ----------------------------------------------------------------------------------------
' file:// URIs
' ----------------------------------------------------------------------------------------
private function TikoLsp_PathToUri( byref sPath as string ) as string
    ' Windows: C:\dir\file -> file:///C:/dir/file
    dim as string sIn2 = sPath
    for i as long = 0 to len(sIn2) - 1
        if sIn2[i] = asc("\") then sIn2[i] = asc("/")
    next
    dim as string sOut = "file://"
    if mid(sIn2, 2, 1) = ":" then sOut &= "/"
    for i as long = 1 to len(sIn2)
        dim as long c = sIn2[i - 1]
        select case c
            case asc("A") to asc("Z"), asc("a") to asc("z"), asc("0") to asc("9"), _
                 asc("-"), asc("."), asc("_"), asc("~"), asc("/"), asc(":")
                sOut &= chr(c)
            case else
                sOut &= "%" & hex(c, 2)
        end select
    next
    return sOut
end function

private function TikoLsp_UriToPath( byref sUri as string ) as string
    dim as string s = sUri
    if left(s, 7) = "file://" then s = mid(s, 8)
    dim as string sOut
    dim as long i = 1
    do while i <= len(s)
        if (mid(s, i, 1) = "%") andalso (i + 2 <= len(s)) then
            sOut &= chr(valint("&h" & mid(s, i + 1, 2)))
            i += 3
        else
            sOut &= mid(s, i, 1)
            i += 1
        end if
    loop
#ifndef __FB_LINUX__
    ' /C:/dir/file -> C:\dir\file
    if (left(sOut, 1) = "/") andalso (mid(sOut, 3, 1) = ":") then sOut = mid(sOut, 2)
    for k as long = 0 to len(sOut) - 1
        if sOut[k] = asc("/") then sOut[k] = asc("\")
    next
#endif
    return sOut
end function

' Two URIs of the same file compare equal through this (servers spell drive letters and
' escapes differently on Windows)
private function TikoLsp_UriKey( byref sUri as string ) as string
#ifdef __FB_LINUX__
    return TikoLsp_UriToPath(sUri)
#else
    return lcase(TikoLsp_UriToPath(sUri))
#endif
end function

' ----------------------------------------------------------------------------------------
' State
' ----------------------------------------------------------------------------------------
type TKLSP_DIAG
    uri      as string      ' document
    line1    as long        ' LSP range: zero based lines, UTF-16 columns
    char1    as long
    line2    as long
    char2    as long
    severity as long
    message  as string
end type

type TKLSP_DOC
    pDoc      as clsDocument ptr
    uri       as string
    version   as long
    dirty     as boolean
    changedAt as double
    seen      as boolean
end type

type TKLSP_STATE
    timerOn     as boolean
    ticks       as long
    proc        as TKPROC        ' the tiko-lsp process
    running     as boolean
    initialized as boolean
    failures    as long
    nextId      as long
    initId      as long
    buffer      as string
    pending     as string        ' messages queued until the server answered initialize
    ' outstanding requests
    defId       as long
    defDoc      as clsDocument ptr
    hoverId     as long
    hoverHwnd   as HWND
    hoverPos    as long
    tipShown    as boolean
    tipHwnd     as HWND
end type

dim shared gTkLsp as TKLSP_STATE
dim shared gTkLspDocs() as TKLSP_DOC
dim shared gTkLspDiags() as TKLSP_DIAG     ' diagnostics of all documents (by uri)

' Removes the diagnostics of one document
private sub TikoLsp_ClearDiags( byref sUri as string )
    dim as long n = 0
    for k as long = 0 to ubound(gTkLspDiags)
        if gTkLspDiags(k).uri <> sUri then
            if n <> k then gTkLspDiags(n) = gTkLspDiags(k)
            n += 1
        end if
    next
    if n = 0 then erase gTkLspDiags else redim preserve gTkLspDiags(n - 1)
end sub
dim shared as boolean gTikoLspBypassDefinition

declare sub TikoLsp_Stop()

' ----------------------------------------------------------------------------------------
' Transport
' ----------------------------------------------------------------------------------------
private sub TikoLsp_WriteRaw( byref sMsg as string )
    if gTkLsp.running = false then exit sub
    if TkProc_Write( gTkLsp.proc, "Content-Length: " & len(sMsg) & chr(13, 10, 13, 10) & sMsg ) = false then TikoLsp_Stop()
end sub

private sub TikoLsp_Send( byref sMsg as string )
    ' everything but initialize waits for the server's answer to initialize
    if gTkLsp.initialized then TikoLsp_WriteRaw( sMsg ) else gTkLsp.pending &= sMsg & chr(0)
end sub

private function TikoLsp_Request( byref sMethod as string, byref sParams as string ) as long
    gTkLsp.nextId += 1
    TikoLsp_Send( "{""jsonrpc"":""2.0"",""id"":" & gTkLsp.nextId & ",""method"":""" & sMethod & """,""params"":" & sParams & "}" )
    return gTkLsp.nextId
end function

private sub TikoLsp_Notify( byref sMethod as string, byref sParams as string )
    TikoLsp_Send( "{""jsonrpc"":""2.0"",""method"":""" & sMethod & """,""params"":" & sParams & "}" )
end sub

' ----------------------------------------------------------------------------------------
' Documents
' ----------------------------------------------------------------------------------------
private function TikoLsp_FindDoc( byval pDoc as clsDocument ptr ) as long
    for i as long = 0 to ubound(gTkLspDocs)
        if gTkLspDocs(i).pDoc = pDoc then return i
    next
    return -1
end function

private function TikoLsp_FindUri( byref sUri as string ) as long
    for i as long = 0 to ubound(gTkLspDocs)
        if (gTkLspDocs(i).pDoc <> 0) andalso (TikoLsp_UriKey(gTkLspDocs(i).uri) = TikoLsp_UriKey(sUri)) then return i
    next
    return -1
end function

' the document text as UTF-8 (ANSI documents are Windows-1252)
private function TikoLsp_DocText( byval pDoc as clsDocument ptr ) as string
    dim as string s = pDoc->GetText()
    if right(s, 1) = chr(0) then s = left(s, len(s) - 1)
    if SciExec( pDoc->hWindow(0), SCI_GETCODEPAGE, 0, 0 ) <> SC_CP_UTF8 then
        dim as DWSTRING w = DWSTRING(s, 1252)
        s = w.utf8
    end if
    return s
end function

private function TikoLsp_DocParams( byval i as long, byval bWithText as boolean ) as string
    dim as string s = "{""textDocument"":{""uri"":" & TkJ_Esc(gTkLspDocs(i).uri)
    if bWithText then
        s &= ",""languageId"":""tiko"",""version"":" & gTkLspDocs(i).version & _
             ",""text"":" & TkJ_Esc(TikoLsp_DocText(gTkLspDocs(i).pDoc))
    end if
    return s & "}}"
end function

private sub TikoLsp_SendChange( byval i as long )
    gTkLspDocs(i).version += 1
    gTkLspDocs(i).dirty = false
    TikoLsp_Notify( "textDocument/didChange", _
        "{""textDocument"":{""uri"":" & TkJ_Esc(gTkLspDocs(i).uri) & ",""version"":" & gTkLspDocs(i).version & "}," & _
        """contentChanges"":[{""text"":" & TkJ_Esc(TikoLsp_DocText(gTkLspDocs(i).pDoc)) & "}]}" )
end sub

' byte position <-> LSP position (zero based line, UTF-16 column)
private function TikoLsp_PosToLsp( byval hEdit as HWND, byval nPos as long ) as string
    dim as long nLine = SciExec( hEdit, SCI_LINEFROMPOSITION, nPos, 0 )
    dim as long nStart = SciExec( hEdit, SCI_POSITIONFROMLINE, nLine, 0 )
    dim as long nChar = SciExec( hEdit, SCI_COUNTCODEUNITS, nStart, nPos )
    return "{""line"":" & nLine & ",""character"":" & nChar & "}"
end function

private function TikoLsp_LspToPos( byval hEdit as HWND, byval nLine as long, byval nChar as long ) as long
    dim as long nLines = SciExec( hEdit, SCI_GETLINECOUNT, 0, 0 )
    if nLine >= nLines then return SciExec( hEdit, SCI_GETLENGTH, 0, 0 )
    dim as long nStart = SciExec( hEdit, SCI_POSITIONFROMLINE, nLine, 0 )
    dim as long nEnd = SciExec( hEdit, SCI_GETLINEENDPOSITION, nLine, 0 )
    dim as long nPos = SciExec( hEdit, SCI_POSITIONRELATIVECODEUNITS, nStart, nChar )
    if (nPos < nStart) orelse (nPos > nEnd) then nPos = nEnd
    return nPos
end function

' Draws the stored diagnostics of a document as indicators
private sub TikoLsp_PaintDiagnostics( byval i as long )
    dim as clsDocument ptr pDoc = gTkLspDocs(i).pDoc
    if pDoc = 0 then exit sub
    dim as HWND hEdit = pDoc->hWindow(0)
    if hEdit = 0 then exit sub
    dim as long nLen = SciExec( hEdit, SCI_GETLENGTH, 0, 0 )
    for k as long = TKLSP_INDIC_ERROR to TKLSP_INDIC_WARNING
        SciExec( hEdit, SCI_SETINDICATORCURRENT, k, 0 )
        SciExec( hEdit, SCI_INDICATORCLEARRANGE, 0, nLen )
    next
    for d as long = 0 to ubound(gTkLspDiags)
        if gTkLspDiags(d).uri <> gTkLspDocs(i).uri then continue for
        with gTkLspDiags(d)
            dim as long p1 = TikoLsp_LspToPos( hEdit, .line1, .char1 )
            dim as long p2 = TikoLsp_LspToPos( hEdit, .line2, .char2 )
            if p2 <= p1 then p2 = p1 + 1
            if p2 > nLen then p2 = nLen
            if p1 >= p2 then p1 = p2 - 1
            if p1 < 0 then continue for
            SciExec( hEdit, SCI_SETINDICATORCURRENT, iif(.severity = 1, TKLSP_INDIC_ERROR, TKLSP_INDIC_WARNING), 0 )
            SciExec( hEdit, SCI_INDICATORFILLRANGE, p1, p2 - p1 )
        end with
    next
end sub

' ----------------------------------------------------------------------------------------
' Messages from the server
' ----------------------------------------------------------------------------------------
private sub TikoLsp_OnDiagnostics( byval nParams as long )
    dim as long i = TikoLsp_FindUri( TkJ_Str(TkJ_Get(nParams, "uri")) )
    if i < 0 then exit sub
    dim as long nArr = TkJ_Get(nParams, "diagnostics")
    dim as long nCount = 0
    dim as long c = iif(nArr >= 0, gTkj(nArr).firstChild, -1)
    do while c >= 0 : nCount += 1 : c = gTkj(c).nextSib : loop
    TikoLsp_ClearDiags( gTkLspDocs(i).uri )
    if nCount > 0 then
        dim as long nBase = ubound(gTkLspDiags) + 1
        redim preserve gTkLspDiags(nBase + nCount - 1)
        c = gTkj(nArr).firstChild
        for d as long = nBase to nBase + nCount - 1
            dim as long nRange = TkJ_Get(c, "range")
            dim as long nS = TkJ_Get(nRange, "start"), nE = TkJ_Get(nRange, "end")
            with gTkLspDiags(d)
                .uri = gTkLspDocs(i).uri
                .line1 = TkJ_Num(TkJ_Get(nS, "line")) : .char1 = TkJ_Num(TkJ_Get(nS, "character"))
                .line2 = TkJ_Num(TkJ_Get(nE, "line")) : .char2 = TkJ_Num(TkJ_Get(nE, "character"))
                .severity = TkJ_Num(TkJ_Get(c, "severity"))
                if .severity = 0 then .severity = 1
                .message = TkJ_Str(TkJ_Get(c, "message"))
            end with
            c = gTkj(c).nextSib
        next
    end if
    TikoLsp_PaintDiagnostics( i )
end sub

' Markdown from hover -> plain calltip text
private function TikoLsp_PlainText( byref sMd as string ) as string
    dim as string sOut, sLine
    dim as long p = 1
    do while p <= len(sMd) + 1
        dim as long q = instr(p, sMd, chr(10))
        if q = 0 then q = len(sMd) + 1
        sLine = mid(sMd, p, q - p)
        p = q + 1
        if left(sLine, 3) = "```" then continue do
        if (left(sLine, 1) = "*") andalso (right(sLine, 1) = "*") andalso (len(sLine) > 2) then sLine = mid(sLine, 2, len(sLine) - 2)
        if len(trim(sLine)) = 0 then continue do
        if len(sOut) then sOut &= chr(10)
        sOut &= sLine
    loop
    return rtrim(sOut, any chr(10, 32))
end function

private sub TikoLsp_ShowTip( byval hEdit as HWND, byval nPos as long, byref sText as string )
    if len(sText) = 0 then exit sub
    dim as string s = sText
    if SciExec( hEdit, SCI_GETCODEPAGE, 0, 0 ) <> SC_CP_UTF8 then
        ' ANSI document: the calltip uses the same code page
        dim as DWSTRING w = DWSTRING(s, CP_UTF8)
        s = ""
        for k as long = 0 to len(w) - 1
            dim as ulong ch = w[k]
            s &= iif(ch < 256, chr(ch), "?")
        next
    end if
    SciExec( hEdit, SCI_CALLTIPSHOW, nPos, cast(LPARAM, strptr(s)) )
    gTkLsp.tipShown = true
    gTkLsp.tipHwnd = hEdit
end sub

private sub TikoLsp_OnHover( byval nResult as long )
    dim as HWND hEdit = gTkLsp.hoverHwnd
    gTkLsp.hoverId = 0
    if (hEdit = 0) orelse (nResult < 0) orelse (gTkj(nResult).kind <> TKJK_OBJ) then exit sub
    dim as long nContents = TkJ_Get(nResult, "contents")
    dim as string sText
    select case gTkj(nContents).kind
        case TKJK_OBJ: sText = TkJ_Str(TkJ_Get(nContents, "value"))
        case TKJK_STR: sText = TkJ_Str(nContents)
        case TKJK_ARR
            dim as long c = gTkj(nContents).firstChild
            do while c >= 0
                sText &= iif(len(sText), chr(10), "") & iif(gTkj(c).kind = TKJK_STR, TkJ_Str(c), TkJ_Str(TkJ_Get(c, "value")))
                c = gTkj(c).nextSib
            loop
    end select
    TikoLsp_ShowTip( hEdit, gTkLsp.hoverPos, TikoLsp_PlainText(sText) )
end sub

private sub TikoLsp_OnDefinition( byval nResult as long )
    dim as clsDocument ptr pDoc = gTkLsp.defDoc
    gTkLsp.defId = 0
    gTkLsp.defDoc = 0
    ' Location | Location[] | LocationLink[] | null
    dim as long nLoc = nResult
    if (nLoc >= 0) andalso (gTkj(nLoc).kind = TKJK_ARR) then nLoc = TkJ_Item(nLoc, 0)
    dim as string sUri
    dim as long nRange = -1
    if (nLoc >= 0) andalso (gTkj(nLoc).kind = TKJK_OBJ) then
        sUri = TkJ_Str(TkJ_Get(nLoc, "uri"))
        nRange = TkJ_Get(nLoc, "range")
        if len(sUri) = 0 then
            sUri = TkJ_Str(TkJ_Get(nLoc, "targetUri"))
            nRange = TkJ_Get(nLoc, "targetSelectionRange")
        end if
    end if
    if (len(sUri) = 0) orelse (nRange < 0) then
        ' not found by the server: the editor's own search
        if pDoc then
            gTikoLspBypassDefinition = true
            OnCommand_SearchGotoDefinition( pDoc )
            gTikoLspBypassDefinition = false
        end if
        exit sub
    end if
    dim as long nStart = TkJ_Get(nRange, "start")
    dim as long nLine = TkJ_Num(TkJ_Get(nStart, "line")), nChar = TkJ_Num(TkJ_Get(nStart, "character"))
    if pDoc then OnCommand_SearchSaveLastPosition( pDoc )
    dim as DWSTRING wszFile = DWSTRING(TikoLsp_UriToPath(sUri), CP_UTF8)
    dim as clsDocument ptr pTarget = OpenSelectedDocument( wszFile, "", nLine )
    if pTarget = 0 then pTarget = gTTabCtl.GetActiveDocumentPtr()
    if pTarget andalso pTarget->hWndActiveScintilla then
        dim as HWND hEdit = pTarget->hWndActiveScintilla
        SciExec( hEdit, SCI_GOTOPOS, TikoLsp_LspToPos(hEdit, nLine, nChar), 0 )
    end if
end sub

private sub TikoLsp_Dispatch( byref sJson as string )
    dim as long nRoot = TkJ_Parse(sJson)
    if (nRoot < 0) orelse (gTkj(nRoot).kind <> TKJK_OBJ) then exit sub
    dim as long nId = TkJ_Get(nRoot, "id")
    dim as long nMethod = TkJ_Get(nRoot, "method")
    if nMethod >= 0 then
        dim as string sMethod = TkJ_Str(nMethod)
        if nId >= 0 then
            ' a request from the server (configuration, progress, ...): answer with null
            dim as string sId = iif(gTkj(nId).kind = TKJK_STR, TkJ_Esc(TkJ_Str(nId)), TkJ_Str(nId))
            TikoLsp_WriteRaw( "{""jsonrpc"":""2.0"",""id"":" & sId & ",""result"":null}" )
            exit sub
        end if
        if sMethod = "textDocument/publishDiagnostics" then TikoLsp_OnDiagnostics( TkJ_Get(nRoot, "params") )
        exit sub
    end if
    if nId < 0 then exit sub
    dim as long id = TkJ_Num(nId)
    dim as long nResult = TkJ_Get(nRoot, "result")
    if id = gTkLsp.initId then
        gTkLsp.initialized = true
        TikoLsp_WriteRaw( "{""jsonrpc"":""2.0"",""method"":""initialized"",""params"":{}}" )
        ' send what waited for the answer
        dim as string sQueue = gTkLsp.pending
        gTkLsp.pending = ""
        dim as long p = 1
        do while p <= len(sQueue)
            dim as long q = instr(p, sQueue, chr(0))
            if q = 0 then exit do
            TikoLsp_WriteRaw( mid(sQueue, p, q - p) )
            p = q + 1
        loop
    elseif id = gTkLsp.defId then
        TikoLsp_OnDefinition( nResult )
    elseif id = gTkLsp.hoverId then
        TikoLsp_OnHover( nResult )
    end if
end sub

' Reads what the server wrote and dispatches the complete messages (called from the timer)
private sub TikoLsp_Pump()
    if gTkLsp.running = false then exit sub
    do
        dim as long n = TkProc_Read( gTkLsp.proc, gTkLsp.buffer )
        if n < 0 then
            ' the server ended
            TikoLsp_Stop()
            gTkLsp.failures += 1
            exit sub
        end if
        if n = 0 then exit do
    loop
    do
        dim as long h = instr(gTkLsp.buffer, chr(13, 10, 13, 10))
        if h = 0 then exit do
        dim as string sHead = lcase(left(gTkLsp.buffer, h - 1))
        dim as long p = instr(sHead, "content-length:")
        if p = 0 then gTkLsp.buffer = mid(gTkLsp.buffer, h + 4) : continue do
        dim as long nLen = valint(mid(sHead, p + 15))
        if len(gTkLsp.buffer) < h + 3 + nLen then exit do
        dim as string sBody = mid(gTkLsp.buffer, h + 4, nLen)
        gTkLsp.buffer = mid(gTkLsp.buffer, h + 4 + nLen)
        TikoLsp_Dispatch( sBody )
        if gTkLsp.running = false then exit do
    loop
end sub

' ----------------------------------------------------------------------------------------
' Server process
' ----------------------------------------------------------------------------------------
private function TikoLsp_ServerPath() as string
    ' beside the compiler (Linux: bin/tiko-lsp, Windows: tiko-lsp.exe)
    dim as DWSTRING wszCompiler = TikoLang_FindCompiler()
    if len(wszCompiler) then
        dim as string sLsp = TkDirOf(wszCompiler.utf8) & "tiko-lsp" & TK_EXE
        if TkFileExists(sLsp) then return sLsp
    end if
    ' on the PATH
    dim as string sPath = environ("PATH")
    dim as long p = 1
    do while p <= len(sPath)
        dim as long q = instr(p, sPath, TK_PATHLIST_SEP)
        if q = 0 then q = len(sPath) + 1
        dim as string sDir = rtrim(trim(mid(sPath, p, q - p), chr(34)), any "/\")
        p = q + 1
        if (len(sDir) > 0) andalso TkFileExists(sDir & TK_SEP & "tiko-lsp" & TK_EXE) then return sDir & TK_SEP & "tiko-lsp" & TK_EXE
    loop
    return ""
end function

private function TikoLsp_Start( byref sRootDir as string ) as boolean
    if gTkLsp.running then return true
    if gTkLsp.failures >= 3 then return false        ' it keeps crashing: give up for this session
    dim as string sServer = TikoLsp_ServerPath()
    if len(sServer) = 0 then gTkLsp.failures = 3 : return false
    if TkProc_Start( gTkLsp.proc, sServer, "--stdio" ) = false then
        gTkLsp.failures += 1
        return false
    end if
    gTkLsp.running = true
    gTkLsp.initialized = false
    gTkLsp.buffer = ""
    gTkLsp.pending = ""

    gTkLsp.nextId += 1
    gTkLsp.initId = gTkLsp.nextId
    dim as string sRoot = rtrim(sRootDir, any "/\")
    TikoLsp_WriteRaw( "{""jsonrpc"":""2.0"",""id"":" & gTkLsp.initId & ",""method"":""initialize"",""params"":{" & _
        """processId"":" & TkPid() & ",""clientInfo"":{""name"":""tiko-editor""}," & _
        """rootUri"":" & TkJ_Esc(TikoLsp_PathToUri(sRoot)) & "," & _
        """workspaceFolders"":[{""uri"":" & TkJ_Esc(TikoLsp_PathToUri(sRoot)) & ",""name"":" & TkJ_Esc(TkNameOf(sRoot)) & "}]," & _
        """capabilities"":{""general"":{""positionEncodings"":[""utf-16""]},""textDocument"":{" & _
            """synchronization"":{""didSave"":false},""publishDiagnostics"":{}," & _
            """hover"":{""contentFormat"":[""markdown"",""plaintext""]},""definition"":{}}}}}" )
    return true
end function

sub TikoLsp_Stop()
    TkProc_Stop( gTkLsp.proc )
    gTkLsp.running = false
    gTkLsp.initialized = false
    gTkLsp.defId = 0 : gTkLsp.hoverId = 0
    ' documents are opened again when a new server starts; clear their underlines
    for i as long = 0 to ubound(gTkLspDocs)
        if gTkLspDocs(i).pDoc then
            TikoLsp_ClearDiags( gTkLspDocs(i).uri )
            if IsWindow(gTkLspDocs(i).pDoc->hWindow(0)) then TikoLsp_PaintDiagnostics( i )
        end if
        gTkLspDocs(i).pDoc = 0
    next
end sub

' ----------------------------------------------------------------------------------------
' Timer: open, change and close documents
' ----------------------------------------------------------------------------------------
private sub TikoLsp_Tick()
    TikoLang_LoadSettings()
    for i as long = 0 to ubound(gTkLspDocs)
        gTkLspDocs(i).seen = false
    next

    if gTikoLang.LanguageServer then
        dim pDoc as clsDocument ptr = gApp.pDocList
        do until pDoc = 0
            dim as boolean bTikoDoc = false
            if (pDoc->hWindow(0) <> 0) andalso (pDoc->IsNewFlag = false) then bTikoDoc = TikoLang_UsesTiko(pDoc->DiskFilename)
            if bTikoDoc then
                dim as string sPath = WideToUtf8(@pDoc->DiskFilename)
                if TkIsAbsolute(sPath) then
                    dim as string sUri = TikoLsp_PathToUri(sPath)
                    dim as long i = TikoLsp_FindDoc(pDoc)
                    if (i >= 0) andalso (gTkLspDocs(i).uri <> sUri) then
                        ' renamed (Save As): close the old name
                        TikoLsp_Notify( "textDocument/didClose", TikoLsp_DocParams(i, false) )
                        gTkLspDocs(i).pDoc = 0
                        i = -1
                    end if
                    if i < 0 then
                        ' new document: start the server if needed and open it
                        dim as string sRoot = TikoLang_FindPackage(pDoc->DiskFilename)
                        if len(sRoot) = 0 then sRoot = TkDirOf(sPath)
                        if TikoLsp_Start(sRoot) then
                            for k as long = 0 to ubound(gTkLspDocs)
                                if gTkLspDocs(k).pDoc = 0 then i = k : exit for
                            next
                            if i < 0 then
                                i = ubound(gTkLspDocs) + 1
                                redim preserve gTkLspDocs(i)
                            end if
                            gTkLspDocs(i).pDoc = pDoc
                            gTkLspDocs(i).uri = sUri
                            gTkLspDocs(i).version = 1
                            gTkLspDocs(i).dirty = false
                            TikoLsp_ClearDiags( sUri )
                            TikoLsp_Notify( "textDocument/didOpen", TikoLsp_DocParams(i, true) )
                        end if
                    elseif gTkLspDocs(i).dirty andalso (timer - gTkLspDocs(i).changedAt >= TKLSP_CHANGE_DELAY) then
                        TikoLsp_SendChange( i )
                    end if
                    if i >= 0 then gTkLspDocs(i).seen = true
                end if
            end if
            pDoc = pDoc->pDocNext
        loop
    end if

    ' closed documents (or the language server was switched off)
    dim as boolean bAny = false
    for i as long = 0 to ubound(gTkLspDocs)
        if gTkLspDocs(i).pDoc then
            if gTkLspDocs(i).seen then
                bAny = true
            else
                TikoLsp_Notify( "textDocument/didClose", TikoLsp_DocParams(i, false) )
                gTkLspDocs(i).pDoc = 0
                TikoLsp_ClearDiags( gTkLspDocs(i).uri )
            end if
        end if
    next
    ' no tiko document left: stop the server
    if (bAny = false) andalso gTkLsp.running then TikoLsp_Stop()
end sub

' The timer (every 30 ms): messages from the server; every 10th time the documents
private sub TikoLsp_OnTimer()
    TikoLsp_Pump()
    gTkLsp.ticks += 1
    if gTkLsp.ticks >= 10 then
        gTkLsp.ticks = 0
        TikoLsp_Tick()
    end if
end sub

#ifdef __FB_LINUX__
private function TikoLsp_TimerProc cdecl ( byval user as gpointer ) as gboolean
    TikoLsp_OnTimer()
    return true
end function
#else
private sub TikoLsp_TimerProc( byval hwnd as HWND, byval uMsg as UINT, byval idEvent as UINT_PTR, byval dwTime as DWORD )
    TikoLsp_OnTimer()
end sub
#endif

' Called for every editor window of a tiko file (TikoLang_SetupLexer)
sub TikoLsp_SetupWindow( byval pSci as any ptr )
    if gTkLsp.timerOn = false then
        gTkLsp.timerOn = true
#ifdef __FB_LINUX__
        g_timeout_add( 30, cast(GSourceFunc, @TikoLsp_TimerProc), NULL )
#else
        SetTimer( NULL, 0, 30, cast(TIMERPROC, @TikoLsp_TimerProc) )
#endif
    end if
    SciMsg( pSci, SCI_INDICSETSTYLE, TKLSP_INDIC_ERROR, INDIC_SQUIGGLE )
    SciMsg( pSci, SCI_INDICSETFORE, TKLSP_INDIC_ERROR, BGR(240, 60, 60) )
    SciMsg( pSci, SCI_INDICSETUNDER, TKLSP_INDIC_ERROR, 1 )
    SciMsg( pSci, SCI_INDICSETSTYLE, TKLSP_INDIC_WARNING, INDIC_SQUIGGLE )
    SciMsg( pSci, SCI_INDICSETFORE, TKLSP_INDIC_WARNING, BGR(230, 160, 40) )
    SciMsg( pSci, SCI_INDICSETUNDER, TKLSP_INDIC_WARNING, 1 )
    SciMsg( pSci, SCI_SETMOUSEDWELLTIME, 600, 0 )
end sub

' SCN_MODIFIED (text inserted or deleted) for any document
sub TikoLsp_DocModified( byval pDoc as clsDocument ptr )
    dim as long i = TikoLsp_FindDoc(pDoc)
    if i < 0 then exit sub
    gTkLspDocs(i).dirty = true
    gTkLspDocs(i).changedAt = timer
end sub

' SCN_DWELLSTART / SCN_DWELLEND: diagnostic message or hover information
sub TikoLsp_OnDwell( byval hEdit as HWND, byval nPos as long, byval bStart as boolean )
    if bStart = false then
        if gTkLsp.tipShown andalso (gTkLsp.tipHwnd = hEdit) then SciExec( hEdit, SCI_CALLTIPCANCEL, 0, 0 )
        gTkLsp.tipShown = false
        gTkLsp.hoverHwnd = 0
        gTkLsp.hoverId = 0
        exit sub
    end if
    if (nPos < 0) orelse (gTkLsp.initialized = false) then exit sub
    ' an autocomplete list or a codetip of the editor is showing: leave it alone
    if (SciExec( hEdit, SCI_AUTOCACTIVE, 0, 0 ) <> 0) orelse (SciExec( hEdit, SCI_CALLTIPACTIVE, 0, 0 ) <> 0) then exit sub
    dim as clsDocument ptr pDoc = gApp.GetDocumentPtrByWindow( hEdit )
    dim as long i = TikoLsp_FindDoc(pDoc)
    if i < 0 then exit sub

    ' an underlined problem: its message
    if (SciExec( hEdit, SCI_INDICATORVALUEAT, TKLSP_INDIC_ERROR, nPos ) <> 0) orelse _
       (SciExec( hEdit, SCI_INDICATORVALUEAT, TKLSP_INDIC_WARNING, nPos ) <> 0) then
        dim as string sText
        for d as long = 0 to ubound(gTkLspDiags)
            if gTkLspDiags(d).uri <> gTkLspDocs(i).uri then continue for
            with gTkLspDiags(d)
                dim as long p1 = TikoLsp_LspToPos( hEdit, .line1, .char1 )
                dim as long p2 = TikoLsp_LspToPos( hEdit, .line2, .char2 )
                if p2 <= p1 then p2 = p1 + 1
                if (nPos >= p1) andalso (nPos <= p2) then
                    sText &= iif(len(sText), chr(10), "") & iif(.severity = 1, "error: ", "warning: ") & .message
                end if
            end with
        next
        if len(sText) then TikoLsp_ShowTip( hEdit, nPos, sText ) : exit sub
    end if

    ' a name: what it is (textDocument/hover)
    if gTkLspDocs(i).dirty then TikoLsp_SendChange( i )
    gTkLsp.hoverHwnd = hEdit
    gTkLsp.hoverPos = nPos
    gTkLsp.hoverId = TikoLsp_Request( "textDocument/hover", _
        "{""textDocument"":{""uri"":" & TkJ_Esc(gTkLspDocs(i).uri) & "},""position"":" & TikoLsp_PosToLsp(hEdit, nPos) & "}" )
end sub

' F12 in a tiko file: ask the server. Returns false when the editor's own search should run.
function TikoLsp_GotoDefinition( byval pDoc as clsDocument ptr ) as boolean
    if gTikoLspBypassDefinition then return false
    if (pDoc = 0) orelse (gTkLsp.initialized = false) then return false
    dim as long i = TikoLsp_FindDoc(pDoc)
    if i < 0 then return false
    dim as HWND hEdit = pDoc->hWndActiveScintilla
    if hEdit = 0 then return false
    if gTkLspDocs(i).dirty then TikoLsp_SendChange( i )
    dim as long nPos = SciExec( hEdit, SCI_GETCURRENTPOS, 0, 0 )
    gTkLsp.defDoc = pDoc
    gTkLsp.defId = TikoLsp_Request( "textDocument/definition", _
        "{""textDocument"":{""uri"":" & TkJ_Esc(gTkLspDocs(i).uri) & "},""position"":" & TikoLsp_PosToLsp(hEdit, nPos) & "}" )
    return true
end function

' A document is being destroyed: forget it before its pointer is reused
sub TikoLsp_DocClosing( byval pDoc as clsDocument ptr )
    dim as long i = TikoLsp_FindDoc(pDoc)
    if i < 0 then exit sub
    TikoLsp_Notify( "textDocument/didClose", TikoLsp_DocParams(i, false) )
    gTkLspDocs(i).pDoc = 0
    TikoLsp_ClearDiags( gTkLspDocs(i).uri )
    if gTkLsp.defDoc = pDoc then gTkLsp.defDoc = 0
end sub
