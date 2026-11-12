'    tiko editor - AI assistant chat panel (docked on the right of the main window)
'    Licensed under the GNU GPLv3 or later.
'
'   +---------------------------------------+
'   | AI ASSISTANT  model    New Errors  x  |  header (painted; actions are clickable)
'   |---------------------------------------|
'   | You                                   |  transcript: read-only Scintilla, styled by
'   | ...                                   |  hand; code blocks are followed by clickable
'   | Claude                                |  "Insert at cursor / Replace selection / Copy"
'   | ```freebasic ... ```                  |  hotspots (SCN_HOTSPOTCLICK)
'   |   Insert at cursor  Replace sel  Copy |
'   |---------------------------------------|
'   | [input, Enter sends, Shift+Enter = new line]
'   | [x] Include current file      [Send]  |
'   +---------------------------------------+
' The left edge is a splitter (drag to resize). frmMain_PositionWindows gives the panel its
' room through frmAiChat_Layout.

#pragma once

#define IDC_AICHAT_TRANSCRIPT   1201
#define IDC_AICHAT_INPUT        1202
#define IDC_AICHAT_SEND         1203
#define IDC_AICHAT_INCLUDE      1204
#define AICHAT_TIMER            1

#define AICHAT_HEADER_H   30
#define AICHAT_BOTTOM_H   34
#define AICHAT_INPUT_H    86
#define AICHAT_SPLITTER_W 5

' transcript styles
#define AIS_TEXT      0
#define AIS_USERHDR   1
#define AIS_BOTHDR    2
#define AIS_CODE      3
#define AIS_INFO      4
#define AIS_ACTION    5
#define AIS_ERROR     6
#define AIS_HEADING   7
#define AIS_USERTEXT  8

#ifndef SCN_HOTSPOTCLICK
    #define SCN_HOTSPOTCLICK 2019
#endif
#ifndef SCI_SCROLLTOEND
    #define SCI_SCROLLTOEND 2629
#endif
#ifndef SCI_COPYTEXT
    #define SCI_COPYTEXT 2420
#endif

type AICHAT_MSG
    role    as string       ' "user" | "assistant"
    content as string       ' UTF-8, exactly as sent / received
end type

type AICHAT_ACTION
    posStart as long        ' range of the clickable text in the transcript
    posEnd   as long
    kind     as long        ' 1 insert, 2 replace selection, 3 copy
    block    as long        ' index in gAiCodeBlocks
end type

type AICHAT_STATE
    hTranscript  as HWND
    hInput       as HWND
    hSend        as HWND
    hInclude     as HWND
    transfer     as AI_TRANSFER ptr
    lineBuf      as string     ' incomplete SSE line
    rawAll       as string     ' whole response body (for error messages)
    reply        as string     ' assistant text received so far
    replyStart   as long       ' transcript position where the reply text starts
    stopReason   as string
    servedModel  as string
    errorMsg     as string
    fallbackNote as string
    pendingUser  as string     ' question text, put back in the input when the request fails
    dragging     as boolean
    rcNew        as RECT       ' header actions
    rcErrors     as RECT
    rcClose      as RECT
end type

dim shared gAiChat as AICHAT_STATE
dim shared gAiMsgs() as AICHAT_MSG
dim shared gAiCodeBlocks() as string
dim shared gAiActions() as AICHAT_ACTION

declare sub frmAiChat_Send( byref sQuestion as string, byval bIncludeFile as boolean, byref sShown as string = "" )

' ----------------------------------------------------------------------------------------
' Text helpers
' ----------------------------------------------------------------------------------------
private function AiChat_Scale( byval n as long ) as long
    dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMMAIN)
    if pWindow then return pWindow->ScaleX(n)
    return n
end function

' UTF-8 text of a Scintilla control
private function AiChat_SciText( byval hSci as HWND ) as string
    dim as long n = SciExec( hSci, SCI_GETLENGTH, 0, 0 )
    if n <= 0 then return ""
    dim as string s = space(n + 1)
    SciExec( hSci, SCI_GETTEXT, n + 1, cast(LPARAM, strptr(s)) )
    return left(s, n)
end function

' A document's text (or part of it) as UTF-8: "ANSI" documents are Windows-1252
private function AiChat_DocUtf8( byval hEdit as HWND, byref s as string ) as string
    if SciExec( hEdit, SCI_GETCODEPAGE, 0, 0 ) = SC_CP_UTF8 then return s
    dim as DWSTRING w = DWSTRING(s, 1252)
    return w.utf8
end function

' UTF-8 text for a document in its code page and line endings
private function AiChat_ForDoc( byval hEdit as HWND, byref sUtf8 as string ) as string
    dim as string s = AfxStrReplace(DWSTRING(sUtf8, CP_UTF8), chr(13, 10), chr(10)).utf8
    select case SciExec( hEdit, SCI_GETEOLMODE, 0, 0 )
        case SC_EOL_CRLF: s = AfxStrReplace(DWSTRING(s, CP_UTF8), chr(10), chr(13, 10)).utf8
        case SC_EOL_CR:   s = AfxStrReplace(DWSTRING(s, CP_UTF8), chr(10), chr(13)).utf8
    end select
    if SciExec( hEdit, SCI_GETCODEPAGE, 0, 0 ) <> SC_CP_UTF8 then
        dim as DWSTRING w = DWSTRING(s, CP_UTF8)
        s = ""
        for k as long = 0 to len(w) - 1
            dim as ulong ch = w[k]
            s &= iif(ch < 256, chr(ch), "?")
        next
    end if
    return s
end function

' ----------------------------------------------------------------------------------------
' Transcript
' ----------------------------------------------------------------------------------------
private sub AiChat_ScrollToEnd()
    SciExec( gAiChat.hTranscript, SCI_GOTOPOS, SciExec(gAiChat.hTranscript, SCI_GETLENGTH, 0, 0), 0 )
    SciExec( gAiChat.hTranscript, SCI_SCROLLCARET, 0, 0 )
end sub

' Appends UTF-8 text with one style; returns where it starts
private function AiChat_Append( byref s as string, byval nStyle as long ) as long
    dim as HWND h = gAiChat.hTranscript
    dim as long nPos = SciExec( h, SCI_GETLENGTH, 0, 0 )
    if len(s) = 0 then return nPos
    SciExec( h, SCI_SETREADONLY, 0, 0 )
    SciExec( h, SCI_APPENDTEXT, len(s), cast(LPARAM, strptr(s)) )
    SciExec( h, SCI_STARTSTYLING, nPos, 0 )
    SciExec( h, SCI_SETSTYLING, len(s), nStyle )
    SciExec( h, SCI_SETREADONLY, 1, 0 )
    AiChat_ScrollToEnd()
    return nPos
end function

private sub AiChat_SetStyle( byval nPos as long, byval nLen as long, byval nStyle as long )
    if nLen <= 0 then exit sub
    SciExec( gAiChat.hTranscript, SCI_STARTSTYLING, nPos, 0 )
    SciExec( gAiChat.hTranscript, SCI_SETSTYLING, nLen, nStyle )
end sub

' Styles markdown-ish text that starts at nBase: ``` code blocks, `inline code`, # headings
private sub AiChat_StyleMarkdown( byval nBase as long, byref s as string )
    dim as boolean bCode = false
    dim as long p = 1
    do while p <= len(s)
        dim as long q = instr(p, s, chr(10))
        dim as long nLineEnd = iif(q, q, len(s) + 1)
        dim as string sLine = mid(s, p, nLineEnd - p)
        dim as long nLen = nLineEnd - p + iif(q, 1, 0)
        dim as string sTrim = ltrim(sLine)
        if left(sTrim, 3) = "```" then
            AiChat_SetStyle( nBase + p - 1, nLen, AIS_CODE )
            bCode = not bCode
        elseif bCode then
            AiChat_SetStyle( nBase + p - 1, nLen, AIS_CODE )
        elseif left(sTrim, 1) = "#" then
            AiChat_SetStyle( nBase + p - 1, nLen, AIS_HEADING )
        else
            AiChat_SetStyle( nBase + p - 1, nLen, AIS_TEXT )
            ' `inline code`
            dim as long a = instr(sLine, "`")
            do while a
                dim as long b = instr(a + 1, sLine, "`")
                if b = 0 then exit do
                AiChat_SetStyle( nBase + p - 1 + a - 1, b - a + 1, AIS_CODE )
                a = instr(b + 1, sLine, "`")
            loop
        end if
        if q = 0 then exit do
        p = q + 1
    loop
end sub

' One line of prose without its markdown: # headings, **bold**, `code`
private sub AiChat_AppendProse( byref sLine as string )
    dim as string t = sLine
    if left(ltrim(t), 1) = "#" then
        t = ltrim(ltrim(t), "#")
        AiChat_Append( ltrim(t) & chr(10), AIS_HEADING )
        exit sub
    end if
    dim as long p = 1
    do while p <= len(t)
        dim as long a = instr(p, t, "`"), b = instr(p, t, "**")
        dim as long nNext = 0, nKind = 0
        if a andalso ((b = 0) orelse (a < b)) then nNext = a : nKind = 1
        if b andalso ((a = 0) orelse (b < a)) then nNext = b : nKind = 2
        if nNext = 0 then exit do
        dim as string sMark = iif(nKind = 1, "`", "**")
        dim as long e = instr(nNext + len(sMark), t, sMark)
        if e = 0 then exit do
        AiChat_Append( mid(t, p, nNext - p), AIS_TEXT )
        AiChat_Append( mid(t, nNext + len(sMark), e - nNext - len(sMark)), iif(nKind = 1, AIS_CODE, AIS_HEADING) )
        p = e + len(sMark)
    loop
    AiChat_Append( mid(t, p) & chr(10), AIS_TEXT )
end sub

' Re-renders the finished reply without markdown; code blocks get their action links
private sub AiChat_FinishReply()
    dim as HWND h = gAiChat.hTranscript
    dim as string s = gAiChat.reply
    SciExec( h, SCI_SETREADONLY, 0, 0 )
    SciExec( h, SCI_DELETERANGE, gAiChat.replyStart, SciExec(h, SCI_GETLENGTH, 0, 0) - gAiChat.replyStart )
    SciExec( h, SCI_SETREADONLY, 1, 0 )
    if right(s, 1) = chr(10) then s = left(s, len(s) - 1)

    dim as string sBlockCode
    dim as boolean bCode = false
    dim as long p = 1
    do while p <= len(s) + 1
        dim as long q = instr(p, s, chr(10))
        dim as long nLineEnd = iif(q, q, len(s) + 1)
        dim as string sLine = mid(s, p, nLineEnd - p)
        if left(ltrim(sLine), 3) = "```" then
            if bCode then
                ' end of a code block: its actions
                dim as long nBlock = ubound(gAiCodeBlocks) + 1
                redim preserve gAiCodeBlocks(nBlock)
                if right(sBlockCode, 1) = chr(10) then sBlockCode = left(sBlockCode, len(sBlockCode) - 1)
                gAiCodeBlocks(nBlock) = sBlockCode
                dim as string sLabels(2) = { "Insert at cursor", "Replace selection", "Copy" }
                AiChat_Append( "  ", AIS_TEXT )
                for k as long = 0 to 2
                    dim as long nAt = AiChat_Append( sLabels(k), AIS_ACTION )
                    dim as long nA = ubound(gAiActions) + 1
                    redim preserve gAiActions(nA)
                    gAiActions(nA).posStart = nAt
                    gAiActions(nA).posEnd = nAt + len(sLabels(k))
                    gAiActions(nA).kind = k + 1
                    gAiActions(nA).block = nBlock
                    AiChat_Append( iif(k < 2, "     ", chr(10)), AIS_TEXT )
                next
                sBlockCode = ""
            end if
            bCode = not bCode
        elseif bCode then
            AiChat_Append( " " & sLine & chr(10), AIS_CODE )
            sBlockCode &= sLine & chr(10)
        else
            AiChat_AppendProse( sLine )
        end if
        if q = 0 then exit do
        p = q + 1
    loop
    AiChat_Append( chr(10), AIS_TEXT )
end sub

private sub AiChat_ClearTranscript()
    SciExec( gAiChat.hTranscript, SCI_SETREADONLY, 0, 0 )
    SciExec( gAiChat.hTranscript, SCI_CLEARALL, 0, 0 )
    SciExec( gAiChat.hTranscript, SCI_SETREADONLY, 1, 0 )
    erase gAiMsgs, gAiCodeBlocks, gAiActions
    AiChat_Append( "Ask about your code. Enter sends, Shift+Enter starts a new line." & chr(10) & _
                   "With 'Include current file' the open file (and the selection) goes with the question." & chr(10) & chr(10), AIS_INFO )
end sub

' ----------------------------------------------------------------------------------------
' Context: the language and the active file
' ----------------------------------------------------------------------------------------
private function AiChat_AssistantName() as string
    return iif(gAi.Provider = "openai", "ChatGPT", "Claude")
end function

private function AiChat_SystemPrompt() as string
    dim as string s
    s = "You are the coding assistant inside the Tiko editor, an IDE for two closely related languages: " & _
        "FreeBASIC (compiled with fbc) and the tiko programming language (compiled with the tiko compiler). " & _
        "Each question says which language the current file uses; answer for that language." & chr(10) & chr(10) & _
        "About tiko: tiko implements the FreeBASIC 'fb' dialect and adds to it. It compiles through LLVM (clang must be installed). " & _
        "Source files are .tiko, headers .ti. Differences and additions compared with FreeBASIC: fixed-width types int8 int16 int32 int64 " & _
        "uint8..uint64 float32 float64 (typedefs); ustring is the UTF-16 string type with O(1) len; nullptr; block strings opened by a """""" " & _
        "that ends its line and closed by a line starting with """""" (the closing line's indentation is the margin; !"""""" reads escapes); " & _
        "defer; for each x in ...; tuples; generics with 'of' (type List(of T)); lambdas (function(...) as T ... inside expressions, " & _
        "closures capture variables); interfaces and 'implements'; properties with any number of indices and auto-properties; spawn and " & _
        "channels (select channel, needs #import ""std/tasks"" and -mt); modules: #module ""name"" in a directory, #import ""./dir"" or " & _
        "#import ""std/strings"" (standard library: strings, fs, containers, chrono, tasks, outcome, numeric, money, logs, tui, memory), " & _
        "public marks exported names; packages described by tiko.toml and built with 'tiko build'; #cmdline ""options"" in the main file; " & _
        "#cinclude for C headers; Optional/Result types. Not in tiko: other dialects (#lang qb/fblite), gosub, line numbers, inline asm, " & _
        "threadcall, -gen/-lang options. Diagnostics have ids TKOnnnn. Build checked code with -exx while developing. " & _
        "__TIKO_COMPILER__ is defined by tiko only." & chr(10) & chr(10) & _
        "Answer concisely. Put code in fenced blocks tagged freebasic or tiko. When the user asks to change selected code, " & _
        "give the complete replacement for the selection as one code block (the editor offers 'Replace selection' for each block). " & _
        "Do not repeat the whole file unless asked."
    return s
end function

' The language of a file: tiko or FreeBASIC (follows the editor's compiler choice)
private function AiChat_LanguageOf( byval pDoc as clsDocument ptr ) as string
#ifdef TIKOLANG_SUPPORT
    if TikoLang_UsesTiko(pDoc->DiskFilename) then return "tiko"
#else
    select case lcase(AfxStrPathname("EXTN", pDoc->DiskFilename))
        case ".tiko", ".ti", ".tko": return "tiko"
    end select
#endif
    return "FreeBASIC"
end function

' The file (and selection) as context for the question; "" when there is no document
private function AiChat_FileContext( byref sNote as string ) as string
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    if pDoc = 0 then sNote = "(no file is open)" : return ""
    dim as HWND hEdit = pDoc->hWndActiveScintilla
    if hEdit = 0 then return ""
    dim as string sLang = AiChat_LanguageOf(pDoc)
    dim as string sName = WideToUtf8(@pDoc->DiskFilename)
    if pDoc->IsNewFlag then sName = "(unsaved new file)"
    dim as string sText = AiChat_DocUtf8( hEdit, AiChat_SciText(hEdit) )
    dim as string sTag = iif(sLang = "tiko", "tiko", "freebasic")
    dim as string s = "Current file: " & sName & " (language: " & sLang & ")" & chr(10)
    if len(sText) > 500000 then
        sNote = "(the file is larger than 500 KB and was not sent; select the part to ask about)"
        s &= "(The file is too large to include.)" & chr(10)
    else
        s &= "```" & sTag & chr(10) & sText & iif(right(sText, 1) = chr(10), "", chr(10)) & "```" & chr(10)
    end if
    dim as long nCaret = SciExec( hEdit, SCI_GETCURRENTPOS, 0, 0 )
    dim as long nSelA = SciExec( hEdit, SCI_GETSELECTIONSTART, 0, 0 )
    dim as long nSelB = SciExec( hEdit, SCI_GETSELECTIONEND, 0, 0 )
    s &= "The cursor is on line " & (SciExec(hEdit, SCI_LINEFROMPOSITION, nCaret, 0) + 1) & "." & chr(10)
    if nSelB > nSelA then
        dim as string sSel = AiChat_DocUtf8( hEdit, pDoc->GetTextRange(nSelA, nSelB) )
        s &= "Selected text (lines " & (SciExec(hEdit, SCI_LINEFROMPOSITION, nSelA, 0) + 1) & "-" & _
             (SciExec(hEdit, SCI_LINEFROMPOSITION, nSelB, 0) + 1) & "):" & chr(10) & _
             "```" & sTag & chr(10) & sSel & iif(right(sSel, 1) = chr(10), "", chr(10)) & "```" & chr(10)
    end if
    return s
end function

' The language of the active file only (when the file itself is not sent)
private function AiChat_LanguageNote() as string
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    if pDoc = 0 then return ""
    return "(The current file " & WideToUtf8(@pDoc->DiskFilename) & " is " & AiChat_LanguageOf(pDoc) & ".)" & chr(10)
end function

' ----------------------------------------------------------------------------------------
' Sending and receiving
' ----------------------------------------------------------------------------------------
private sub AiChat_SetBusy( byval bBusy as boolean )
    dim as DWSTRING wszLabel = "Send"
    if bBusy then wszLabel = "Stop"
    AfxSetWindowText( gAiChat.hSend, wszLabel )
    if bBusy then
        SetTimer( HWND_FRMAICHAT, AICHAT_TIMER, 40, NULL )
    else
        KillTimer( HWND_FRMAICHAT, AICHAT_TIMER )
    end if
end sub

private function AiChat_MessagesJson() as string
    dim as string s = "["
    for i as long = 0 to ubound(gAiMsgs)
        s &= iif(i, ",", "") & "{""role"":""" & gAiMsgs(i).role & """,""content"":" & TkJ_Esc(gAiMsgs(i).content) & "}"
    next
    return s & "]"
end function

' sQuestion: what the user typed (shown); bIncludeFile: send the file; sShown overrides the shown text
sub frmAiChat_Send( byref sQuestion as string, byval bIncludeFile as boolean, byref sShown as string = "" )
    if gAiChat.transfer then exit sub
    Ai_LoadSettings()
    if len(trim(sQuestion)) = 0 then exit sub
    if len(Ai_ApiKey(gAi.Provider)) = 0 then
        AiChat_Append( "No API key for " & iif(gAi.Provider = "openai", "OpenAI", "Anthropic") & ". Set " & _
            iif(gAi.Provider = "openai", "OPENAI_API_KEY", "ANTHROPIC_API_KEY") & " in the environment or enter the key in " & _
            "Environment Options > AI Assistant." & chr(10), AIS_ERROR )
        AiChat_Append( "Tiko looks in its environment and in ~/.profile; a key exported only in ~/.bashrc is " & _
            "seen only when Tiko is started from a terminal." & chr(10) & chr(10), AIS_INFO )
        exit sub
    end if

    dim as string sNote, sContent
    if bIncludeFile then
        sContent = AiChat_FileContext(sNote)
    else
        sContent = AiChat_LanguageNote()
    end if
    sContent &= iif(len(sContent), chr(10), "") & sQuestion

    AiChat_Append( "You" & chr(10), AIS_USERHDR )
    AiChat_Append( iif(len(sShown), sShown, sQuestion) & chr(10), AIS_USERTEXT )
    if len(sNote) then AiChat_Append( sNote & chr(10), AIS_INFO )
    AiChat_Append( chr(10) & AiChat_AssistantName() & chr(10), AIS_BOTHDR )

    dim as long n = ubound(gAiMsgs) + 1
    redim preserve gAiMsgs(n)
    gAiMsgs(n).role = "user"
    gAiMsgs(n).content = sContent

    gAiChat.pendingUser = sQuestion
    gAiChat.lineBuf = "" : gAiChat.rawAll = "" : gAiChat.reply = ""
    gAiChat.stopReason = "" : gAiChat.errorMsg = "" : gAiChat.servedModel = "" : gAiChat.fallbackNote = ""
    gAiChat.replyStart = SciExec( gAiChat.hTranscript, SCI_GETLENGTH, 0, 0 )
    gAiChat.transfer = Ai_StartChat( AiChat_SystemPrompt(), AiChat_MessagesJson() )
    AiChat_SetBusy( true )
end sub

' One server-sent event (the JSON after "data: ")
private sub AiChat_OnEvent( byref sData as string )
    if sData = "[DONE]" then exit sub
    dim as long nRoot = TkJ_Parse(sData)
    if (nRoot < 0) orelse (gTkj(nRoot).kind <> TKJK_OBJ) then exit sub
    dim as string sDelta
    if gAi.Provider = "openai" then
        dim as long nErr = TkJ_Get(nRoot, "error")
        if nErr >= 0 then gAiChat.errorMsg = TkJ_Str(TkJ_Get(nErr, "message")) : exit sub
        dim as long nChoice = TkJ_Item(TkJ_Get(nRoot, "choices"), 0)
        sDelta = TkJ_Str(TkJ_Get(TkJ_Get(nChoice, "delta"), "content"))
        dim as string sFinish = TkJ_Str(TkJ_Get(nChoice, "finish_reason"))
        if len(sFinish) then gAiChat.stopReason = sFinish
        if len(gAiChat.servedModel) = 0 then gAiChat.servedModel = TkJ_Str(TkJ_Get(nRoot, "model"))
    else
        select case TkJ_Str(TkJ_Get(nRoot, "type"))
            case "message_start"
                gAiChat.servedModel = TkJ_Str(TkJ_Get(TkJ_Get(nRoot, "message"), "model"))
            case "content_block_start"
                dim as long nBlock = TkJ_Get(nRoot, "content_block")
                if TkJ_Str(TkJ_Get(nBlock, "type")) = "fallback" then
                    ' another model continued after a refusal (server-side fallback)
                    dim as string sTo = TkJ_Str(TkJ_Get(TkJ_Get(nBlock, "to"), "model"))
                    gAiChat.fallbackNote = "(continued by " & iif(len(sTo), sTo, "a fallback model") & ")"
                end if
            case "content_block_delta"
                dim as long nDelta = TkJ_Get(nRoot, "delta")
                if TkJ_Str(TkJ_Get(nDelta, "type")) = "text_delta" then sDelta = TkJ_Str(TkJ_Get(nDelta, "text"))
            case "message_delta"
                dim as string sStop = TkJ_Str(TkJ_Get(TkJ_Get(nRoot, "delta"), "stop_reason"))
                if len(sStop) then gAiChat.stopReason = sStop
            case "error"
                gAiChat.errorMsg = TkJ_Str(TkJ_Get(TkJ_Get(nRoot, "error"), "message"))
        end select
    end if
    if len(sDelta) then
        gAiChat.reply &= sDelta
        AiChat_Append( sDelta, AIS_TEXT )
    end if
end sub

' The error message of a non-streamed error response body
private function AiChat_ErrorFromBody( byref sBody as string ) as string
    dim as long nRoot = TkJ_Parse(sBody)
    if (nRoot < 0) orelse (gTkj(nRoot).kind <> TKJK_OBJ) then return left(trim(sBody), 300)
    dim as long nErr = TkJ_Get(nRoot, "error")
    if nErr >= 0 then
        if gTkj(nErr).kind = TKJK_OBJ then return TkJ_Str(TkJ_Get(nErr, "message"))
        return TkJ_Str(nErr)
    end if
    return left(trim(sBody), 300)
end function

private sub AiChat_Finish( byref sTransportError as string, byval nHttp as long )
    dim as boolean bStopped = (sTransportError = "stopped")
    dim as boolean bOk = (len(gAiChat.reply) > 0) andalso (gAiChat.stopReason <> "refusal")

    if len(gAiChat.reply) then AiChat_FinishReply()
    if len(gAiChat.fallbackNote) then AiChat_Append( gAiChat.fallbackNote & chr(10), AIS_INFO )

    dim as string sProblem
    if bStopped then
        sProblem = "(stopped)"
    elseif (nHttp >= 400) orelse ((nHttp = 0) andalso (len(sTransportError) > 0)) then
        dim as string sMsg = iif(nHttp >= 400, AiChat_ErrorFromBody(gAiChat.rawAll), sTransportError)
        sProblem = "Request failed" & iif(nHttp >= 400, " (HTTP " & nHttp & ")", "") & ": " & sMsg
        if (nHttp = 401) orelse (nHttp = 403) then sProblem &= chr(10) & "Check the API key (Environment Options > AI Assistant)."
        if nHttp = 404 then sProblem &= chr(10) & "Check the model name (Environment Options > AI Assistant)."
        bOk = false
    elseif len(gAiChat.errorMsg) then
        sProblem = "Error: " & gAiChat.errorMsg
        bOk = false
    elseif gAiChat.stopReason = "refusal" then
        sProblem = "The model declined this request."
    elseif gAiChat.stopReason = "max_tokens" orelse gAiChat.stopReason = "length" then
        sProblem = "(the answer reached the length limit)"
    elseif len(sTransportError) then
        sProblem = "Connection problem: " & sTransportError
    end if
    if len(sProblem) then AiChat_Append( sProblem & chr(10) & chr(10), iif(bStopped, AIS_INFO, AIS_ERROR) )

    dim as boolean bKeep = bOk
    if bStopped andalso (len(gAiChat.reply) > 0) then bKeep = true
    if bKeep then
        dim as long n = ubound(gAiMsgs) + 1
        redim preserve gAiMsgs(n)
        gAiMsgs(n).role = "assistant"
        gAiMsgs(n).content = gAiChat.reply
    else
        ' the question did not get an answer: take it out of the history, give it back to edit
        if ubound(gAiMsgs) >= 0 then
            if ubound(gAiMsgs) = 0 then erase gAiMsgs else redim preserve gAiMsgs(ubound(gAiMsgs) - 1)
        end if
        if (len(AiChat_SciText(gAiChat.hInput)) = 0) andalso (bStopped = false) then
            dim as string sBack = gAiChat.pendingUser
            SciExec( gAiChat.hInput, SCI_SETTEXT, 0, cast(LPARAM, strptr(sBack)) )
        end if
    end if
    Ai_FreeTransfer( gAiChat.transfer )
    AiChat_SetBusy( false )
end sub

' Timer: takes what arrived and processes complete lines
private sub AiChat_Poll()
    if gAiChat.transfer = 0 then AiChat_SetBusy( false ) : exit sub
    dim as boolean bDone
    dim as string sChunkIn = Ai_TakeReceived( gAiChat.transfer, bDone )
    if len(sChunkIn) then
        if len(gAiChat.rawAll) < 65536 then gAiChat.rawAll &= sChunkIn
        gAiChat.lineBuf &= sChunkIn
        do
            dim as long q = instr(gAiChat.lineBuf, chr(10))
            if q = 0 then exit do
            dim as string sLine = left(gAiChat.lineBuf, q - 1)
            gAiChat.lineBuf = mid(gAiChat.lineBuf, q + 1)
            if right(sLine, 1) = chr(13) then sLine = left(sLine, len(sLine) - 1)
            if left(sLine, 5) = "data:" then AiChat_OnEvent( ltrim(mid(sLine, 6)) )
        loop
        if len(gAiChat.reply) then AiChat_StyleMarkdown( gAiChat.replyStart, gAiChat.reply )
    end if
    if bDone then
        if left(gAiChat.lineBuf, 5) = "data:" then AiChat_OnEvent( ltrim(mid(gAiChat.lineBuf, 6)) )
        MutexLock( gAiChat.transfer->mutex )
        dim as string sErr = gAiChat.transfer->errorText
        dim as long nHttp = gAiChat.transfer->httpCode
        MutexUnlock( gAiChat.transfer->mutex )
        AiChat_Finish( sErr, nHttp )
    end if
end sub

private sub AiChat_Stop()
    Ai_CancelTransfer( gAiChat.transfer )
end sub

' ----------------------------------------------------------------------------------------
' Code actions
' ----------------------------------------------------------------------------------------
private sub AiChat_RunAction( byval i as long )
    dim as string sBlockCode = gAiCodeBlocks(gAiActions(i).block)
    if gAiActions(i).kind = 3 then
        SciExec( gAiChat.hTranscript, SCI_COPYTEXT, len(sBlockCode), cast(LPARAM, strptr(sBlockCode)) )
        exit sub
    end if
    dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
    if pDoc = 0 then MessageBeep(MB_ICONWARNING) : exit sub
    dim as HWND hEdit = pDoc->hWndActiveScintilla
    if hEdit = 0 then exit sub
    dim as long nA = SciExec( hEdit, SCI_GETSELECTIONSTART, 0, 0 )
    dim as long nB = SciExec( hEdit, SCI_GETSELECTIONEND, 0, 0 )
    dim as boolean bReplace = (gAiActions(i).kind = 2) andalso (nB > nA)
    if bReplace = false then nA = SciExec( hEdit, SCI_GETCURRENTPOS, 0, 0 ) : nB = nA
    ' whole lines stay whole lines: keep the line break the replaced text ended with
    dim as string sText = sBlockCode
    if bReplace then
        dim as long nEndCol = SciExec( hEdit, SCI_GETCOLUMN, nB, 0 )
        if nEndCol = 0 then sText &= chr(10)
    elseif SciExec( hEdit, SCI_GETCOLUMN, nA, 0 ) = 0 then
        sText &= chr(10)
    end if
    dim as string sDoc = AiChat_ForDoc( hEdit, sText )
    SciExec( hEdit, SCI_BEGINUNDOACTION, 0, 0 )
    SciExec( hEdit, SCI_SETSEL, nA, nB )
    SciExec( hEdit, SCI_REPLACESEL, 0, cast(LPARAM, strptr(sDoc)) )
    SciExec( hEdit, SCI_ENDUNDOACTION, 0, 0 )
    SetFocus( hEdit )
end sub

' ----------------------------------------------------------------------------------------
' Public
' ----------------------------------------------------------------------------------------
function frmAiChat_IsVisible() as boolean
    return (HWND_FRMAICHAT <> 0) andalso (IsWindowVisible(HWND_FRMAICHAT) <> 0)
end function

sub frmAiChat_Toggle()
    Ai_LoadSettings()
    gAi.PanelVisible = not frmAiChat_IsVisible()
    ShowWindow( HWND_FRMAICHAT, iif(gAi.PanelVisible, SW_SHOW, SW_HIDE) )
    frmMain_PositionWindows()
    if gAi.PanelVisible then
        SetFocus( gAiChat.hInput )
    else
        dim pDoc as clsDocument ptr = gTTabCtl.GetActiveDocumentPtr()
        if pDoc then SetFocus( pDoc->hWndActiveScintilla )
    end if
    Ai_SaveSettings()
end sub

' "Explain errors": the last compile output and the file
sub frmAiChat_ExplainErrors()
    if frmAiChat_IsVisible() = false then frmAiChat_Toggle()
    dim as string sLog = trim(gCompile.wszFullLogFile.utf8, any chr(13, 10, 32))
    if len(sLog) = 0 then
        AiChat_Append( "There is no compiler output yet: compile first (F5 or Ctrl+F5)." & chr(10) & chr(10), AIS_INFO )
        exit sub
    end if
    dim as string sQ = "Explain the errors and warnings of the last compile and how to fix them." & chr(10) & _
                       "Compiler: " & gCompile.wszFullCommandLine.utf8 & chr(10) & _
                       "Compiler output:" & chr(10) & "```" & chr(10) & sLog & chr(10) & "```"
    frmAiChat_Send( sQ, true, "Explain the errors of the last compile." )
end sub

' Room for the panel at the right of the main window; returns the width taken
function frmAiChat_Layout( byref rc as RECT, byval nTop as long, byval nBottom as long ) as long
    if frmAiChat_IsVisible() = false then return 0
    dim as long w = AiChat_Scale(gAi.PanelWidth)
    dim as long wMax = (rc.right - rc.left) - AiChat_Scale(300)
    if w > wMax then w = wMax
    if w < AiChat_Scale(200) then w = AiChat_Scale(200)
    SetWindowPos( HWND_FRMAICHAT, 0, rc.right - w, nTop, w, nBottom - nTop, SWP_NOZORDER or SWP_SHOWWINDOW )
    return w
end function

' Keys typed in the chat controls: Ctrl+A/C/V/X/Z/Y go to them, not to the editor commands
function frmAiChat_PreTranslate( byref uMsg as MSG ) as boolean
    if HWND_FRMAICHAT = 0 then return false
    if (uMsg.hwnd <> gAiChat.hInput) andalso (uMsg.hwnd <> gAiChat.hTranscript) then return false
    if (uMsg.message <> WM_KEYDOWN) then return false
    if (GetKeyState(VK_CONTROL) and &h8000) = 0 then return false
    if (GetKeyState(VK_MENU) and &h8000) then return false
    select case uMsg.wParam
        case asc("A"), asc("C"), asc("V"), asc("X"), asc("Z"), asc("Y")
            if (uMsg.wParam = asc("A")) andalso (GetKeyState(VK_SHIFT) and &h8000) then return false  ' Ctrl+Shift+A: toggle
            TranslateMessage( @uMsg )
            DispatchMessage( @uMsg )
            return true
    end select
    return false
end function

' ----------------------------------------------------------------------------------------
' Window
' ----------------------------------------------------------------------------------------
private function frmAiChat_InputProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, _
                                       byval lParam as LPARAM, byval uIdSubclass as UINT_PTR, byval dwRefData as DWORD_PTR ) as LRESULT
    select case uMsg
        case WM_KEYDOWN
            if (wParam = VK_RETURN) andalso ((GetKeyState(VK_SHIFT) and &h8000) = 0) then
                ' Enter sends (Shift+Enter is a new line)
                if gAiChat.transfer = 0 then
                    dim as string sQ = AiChat_SciText(gAiChat.hInput)
                    if len(trim(sQ, any !" \t\r\n")) then
                        SciExec( gAiChat.hInput, SCI_CLEARALL, 0, 0 )
                        frmAiChat_Send( sQ, (Button_GetCheck(gAiChat.hInclude) <> 0) )
                    end if
                end if
                return 0
            end if
            if (wParam = VK_ESCAPE) andalso (gAiChat.transfer <> 0) then AiChat_Stop() : return 0
        case WM_CHAR
            if wParam = 13 then return 0
    end select
    return DefSubclassProc( hwnd, uMsg, wParam, lParam )
end function

private sub frmAiChat_SetupScintilla( byval h as HWND, byval bTranscript as boolean )
    SciExec( h, SCI_SETCODEPAGE, SC_CP_UTF8, 0 )
    SciExec( h, SCI_SETILEXER, 0, 0 )
    SciExec( h, SCI_SETWRAPMODE, SC_WRAP_WORD, 0 )
    SciExec( h, SCI_SETHSCROLLBAR, 0, 0 )
    for m as long = 0 to 4
        SciExec( h, SCI_SETMARGINWIDTHN, m, 0 )
    next
    SciExec( h, SCI_SETMARGINLEFT, 0, AiChat_Scale(8) )
    SciExec( h, SCI_SETMARGINRIGHT, 0, AiChat_Scale(6) )
#ifdef __FB_LINUX__
    dim as string sProse = "Selawik"
#else
    dim as string sProse = "Segoe UI"
#endif
    dim as string sMono = gConfig.EditorFontname.utf8
    dim as long nSize = valint(gConfig.EditorFontSize.utf8)
    if nSize < 8 then nSize = 10
    SciExec( h, SCI_STYLESETFONT, STYLE_DEFAULT, cast(LPARAM, strptr(sProse)) )
    SciExec( h, SCI_STYLESETSIZE, STYLE_DEFAULT, nSize )
    SciExec( h, SCI_STYLESETFORE, STYLE_DEFAULT, ghEditor.ForeColorText )
    SciExec( h, SCI_STYLESETBACK, STYLE_DEFAULT, ghEditor.BackColorText )
    SciExec( h, SCI_STYLECLEARALL, 0, 0 )
    SciExec( h, SCI_SETCARETFORE, ghEditor.ForeColorCaret, 0 )
    SciExec( h, SCI_SETSELBACK, 1, ghEditor.BackColorSelection )
    if bTranscript = false then
        ' the input box stands out from the transcript
        SciExec( h, SCI_STYLESETBACK, STYLE_DEFAULT, ghEditor.BackColorCurrentLine )
        SciExec( h, SCI_STYLECLEARALL, 0, 0 )
        SciExec( h, SCI_SETCARETLINEVISIBLE, 0, 0 )
    end if
    if bTranscript then
        SciExec( h, SCI_SETCARETWIDTH, 0, 0 )
        SciExec( h, SCI_SETREADONLY, 1, 0 )
        SciExec( h, SCI_STYLESETBOLD, AIS_USERHDR, 1 )
        SciExec( h, SCI_STYLESETFORE, AIS_USERHDR, ghEditor.ForeColorKeyword )
        SciExec( h, SCI_STYLESETBOLD, AIS_BOTHDR, 1 )
        SciExec( h, SCI_STYLESETFORE, AIS_BOTHDR, ghEditor.ForeColorStrings )
        SciExec( h, SCI_STYLESETFONT, AIS_CODE, cast(LPARAM, strptr(sMono)) )
        SciExec( h, SCI_STYLESETFORE, AIS_CODE, ghEditor.ForeColorText )
        SciExec( h, SCI_STYLESETBACK, AIS_CODE, ghEditor.BackColorCurrentLine )
        SciExec( h, SCI_STYLESETEOLFILLED, AIS_CODE, 1 )
        SciExec( h, SCI_STYLESETITALIC, AIS_INFO, 1 )
        SciExec( h, SCI_STYLESETFORE, AIS_INFO, ghEditor.ForeColorComments )
        SciExec( h, SCI_STYLESETFORE, AIS_ACTION, ghEditor.ForeColorKeyword )
        SciExec( h, SCI_STYLESETUNDERLINE, AIS_ACTION, 1 )
        SciExec( h, SCI_STYLESETHOTSPOT, AIS_ACTION, 1 )
        SciExec( h, SCI_STYLESETFORE, AIS_ERROR, BGR(240, 90, 90) )
        SciExec( h, SCI_STYLESETBOLD, AIS_HEADING, 1 )
        SciExec( h, SCI_STYLESETFORE, AIS_USERTEXT, ghEditor.ForeColorText )
    end if
end sub

private sub frmAiChat_PositionChildren()
    dim as RECT rc
    GetClientRect( HWND_FRMAICHAT, @rc )
    dim as long nSplit = AiChat_Scale(AICHAT_SPLITTER_W)
    dim as long nHead = AiChat_Scale(AICHAT_HEADER_H)
    dim as long nBottomBar = AiChat_Scale(AICHAT_BOTTOM_H)
    dim as long nInput = AiChat_Scale(AICHAT_INPUT_H)
    dim as long nPad = AiChat_Scale(6)
    dim as long x = nSplit, w = rc.right - nSplit
    dim as long yInput = rc.bottom - nBottomBar - nInput
    SetWindowPos( gAiChat.hTranscript, 0, x, nHead, w, yInput - nHead - nPad, SWP_NOZORDER or SWP_SHOWWINDOW )
    SetWindowPos( gAiChat.hInput, 0, x + nPad, yInput, w - 2 * nPad, nInput, SWP_NOZORDER or SWP_SHOWWINDOW )
    dim as long nBtnW = AiChat_Scale(76), nBtnH = AiChat_Scale(26)
    dim as long yBar = rc.bottom - nBottomBar + (nBottomBar - nBtnH) \ 2
    SetWindowPos( gAiChat.hInclude, 0, x + nPad, yBar, w - nBtnW - 3 * nPad, nBtnH, SWP_NOZORDER or SWP_SHOWWINDOW )
    SetWindowPos( gAiChat.hSend, 0, rc.right - nBtnW - nPad, yBar, nBtnW, nBtnH, SWP_NOZORDER or SWP_SHOWWINDOW )
    InvalidateRect( HWND_FRMAICHAT, NULL, TRUE )
end sub

private sub frmAiChat_Paint( byval hwnd as HWND )
    dim as clsDoubleBuffer b
    b.BeginDoubleBuffer(hwnd)
    b.SetForeColors( ghOutput.ForeColor, ghOutput.ForeColorHot )
    b.SetBackColors( ghOutput.BackColor, ghOutput.BackColorHot )
    b.PaintClientRect()
    dim as RECT rc = b.rcClient
    ' splitter line
    b.SetPenColor( ghOutput.Divider )
    b.PaintLine( 1, rc.left, rc.top, rc.left, rc.bottom )
    ' header: title + model, actions on the right
    dim as long nHead = AiChat_Scale(AICHAT_HEADER_H)
    dim as long nSplit = AiChat_Scale(AICHAT_SPLITTER_W)
    dim as long nPad = AiChat_Scale(8)
    b.SetFont( GUIFONT_9 )
    dim as long x = rc.right - nPad
    dim as long wClose = AiChat_Scale(22), wErrors = AiChat_Scale(84), wNew = AiChat_Scale(66)
    gAiChat.rcClose = type(x - wClose, 0, x, nHead) : x -= wClose + AiChat_Scale(4)
    gAiChat.rcErrors = type(x - wErrors, 0, x, nHead) : x -= wErrors + AiChat_Scale(4)
    gAiChat.rcNew = type(x - wNew, 0, x, nHead) : x -= wNew
    dim as RECT rcTitle = type(nSplit + nPad, 0, x - nPad, nHead)
    b.PaintText( "AI ASSISTANT  " & wchr(&hB7) & "  " & DWSTRING(Ai_ModelName(), CP_UTF8), @rcTitle, DT_LEFT or DT_VCENTER or DT_SINGLELINE or DT_END_ELLIPSIS )
    b.SetForeColors( ghOutput.ForeColorHot, ghOutput.ForeColorHot )
    b.PaintText( "New chat", @gAiChat.rcNew, DT_CENTER or DT_VCENTER or DT_SINGLELINE )
    b.PaintText( "Explain errors", @gAiChat.rcErrors, DT_CENTER or DT_VCENTER or DT_SINGLELINE )
    b.PaintText( wchr(&hD7), @gAiChat.rcClose, DT_CENTER or DT_VCENTER or DT_SINGLELINE )
    b.SetPenColor( ghOutput.Divider )
    b.PaintLine( 1, rc.left + nSplit, nHead - 1, rc.right, nHead - 1 )
    ' border around the input box
    if gAiChat.hInput then
        dim as RECT rcIn
        GetWindowRect( gAiChat.hInput, @rcIn )
        MapWindowPoints( NULL, hwnd, cast(LPPOINT, @rcIn), 2 )
        InflateRect( @rcIn, 1, 1 )
        b.SetPenColor( ghOutput.ForeColor )
        b.PaintLine( 1, rcIn.left, rcIn.top, rcIn.right, rcIn.top )
        b.PaintLine( 1, rcIn.left, rcIn.bottom, rcIn.right, rcIn.bottom )
        b.PaintLine( 1, rcIn.left, rcIn.top, rcIn.left, rcIn.bottom )
        b.PaintLine( 1, rcIn.right, rcIn.top, rcIn.right, rcIn.bottom )
    end if
    b.EndDoubleBuffer()
end sub

private function frmAiChat_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    select case uMsg
        case WM_PAINT
            frmAiChat_Paint( hwnd )
            return 0
        case WM_ERASEBKGND
            return 1
#ifndef __FB_LINUX__
        case WM_CTLCOLORSTATIC, WM_CTLCOLORBTN
            ' the "Include current file" checkbox in the panel's colours
            static as HBRUSH hbrPanel
            static as COLORREF clrPanel = &hFFFFFFFF
            if clrPanel <> ghOutput.BackColor then
                if hbrPanel then DeleteObject( hbrPanel )
                clrPanel = ghOutput.BackColor
                hbrPanel = CreateSolidBrush( clrPanel )
            end if
            SetTextColor( cast(HDC, wParam), ghOutput.ForeColorHot )
            SetBkColor( cast(HDC, wParam), clrPanel )
            return cast(LRESULT, hbrPanel)
#endif
        case WM_SIZE
            frmAiChat_PositionChildren()
            return 0
        case WM_TIMER
            if wParam = AICHAT_TIMER then AiChat_Poll()
            return 0
        case WM_COMMAND
            select case LOWORD(wParam)
                case IDC_AICHAT_SEND
                    if HIWORD(wParam) = BN_CLICKED then
                        if gAiChat.transfer then
                            AiChat_Stop()
                        else
                            dim as string sQ = AiChat_SciText(gAiChat.hInput)
                            if len(trim(sQ, any !" \t\r\n")) then
                                SciExec( gAiChat.hInput, SCI_CLEARALL, 0, 0 )
                                frmAiChat_Send( sQ, (Button_GetCheck(gAiChat.hInclude) <> 0) )
                            end if
                            SetFocus( gAiChat.hInput )
                        end if
                    end if
                case IDC_AICHAT_INCLUDE
                    if HIWORD(wParam) = BN_CLICKED then
                        gAi.IncludeFile = (Button_GetCheck(gAiChat.hInclude) <> 0)
                        Ai_SaveSettings()
                    end if
            end select
            return 0
        case WM_NOTIFY
            dim as SCNOTIFICATION ptr pNSC = cast(SCNOTIFICATION ptr, lParam)
            if (pNSC <> 0) andalso (cast(HWND, pNSC->hdr.hwndFrom) = gAiChat.hTranscript) andalso _
               (pNSC->hdr.code = SCN_HOTSPOTCLICK) then
                for i as long = 0 to ubound(gAiActions)
                    if (pNSC->position >= gAiActions(i).posStart) andalso (pNSC->position < gAiActions(i).posEnd) then
                        AiChat_RunAction( i )
                        exit for
                    end if
                next
            end if
            return 0
        case WM_LBUTTONDOWN
            dim as POINT pt = type(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
            if pt.x < AiChat_Scale(AICHAT_SPLITTER_W) then
                gAiChat.dragging = true
                SetCapture( hwnd )
            elseif PtInRect( @gAiChat.rcClose, pt ) then
                frmAiChat_Toggle()
            elseif PtInRect( @gAiChat.rcNew, pt ) then
                AiChat_Stop()
                if gAiChat.transfer = 0 then AiChat_ClearTranscript()
                SetFocus( gAiChat.hInput )
            elseif PtInRect( @gAiChat.rcErrors, pt ) then
                frmAiChat_ExplainErrors()
            end if
            return 0
        case WM_MOUSEMOVE
            dim as POINT pt = type(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
            if gAiChat.dragging then
                ' new width: from the mouse to the right edge of the panel
                dim as RECT rcWin
                GetWindowRect( hwnd, @rcWin )
                dim as POINT ptScreen = pt
                ClientToScreen( hwnd, @ptScreen )
                dim as long w = rcWin.right - ptScreen.x
                dim pWindow as CWindow ptr = AfxCWindowPtr(HWND_FRMMAIN)
                if pWindow then w = pWindow->UnScaleX(w)
                if w < 260 then w = 260
                if w > 1400 then w = 1400
                gAi.PanelWidth = w
                frmMain_PositionWindows()
            end if
            if gAiChat.dragging orelse (pt.x < AiChat_Scale(AICHAT_SPLITTER_W)) then
                SetCursor( LoadCursor(NULL, IDC_SIZEWE) )
            elseif PtInRect(@gAiChat.rcClose, pt) orelse PtInRect(@gAiChat.rcNew, pt) orelse PtInRect(@gAiChat.rcErrors, pt) then
                SetCursor( LoadCursor(NULL, IDC_HAND) )
            else
                SetCursor( LoadCursor(NULL, IDC_ARROW) )
            end if
            return 0
        case WM_LBUTTONUP
            if gAiChat.dragging then
                gAiChat.dragging = false
                ReleaseCapture()
                Ai_SaveSettings()
            end if
            return 0
        case WM_DESTROY
            if gAiChat.transfer then Ai_FreeTransfer( gAiChat.transfer )
            KillTimer( hwnd, AICHAT_TIMER )
            return 0
    end select
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function

function frmAiChat_Show( byval hWndParent as HWND ) as LRESULT
    Ai_LoadSettings()
    dim pWindow as CWindow ptr = new CWindow
    HWND_FRMAICHAT = pWindow->Create( hWndParent, "", @frmAiChat_WndProc, 0, 0, 0, 0, _
        WS_CHILD or iif(gAi.PanelVisible, WS_VISIBLE, 0) or WS_CLIPSIBLINGS or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING)

    gAiChat.hTranscript = CreateWindowEx( 0, "Scintilla", "", WS_CHILD or WS_VISIBLE or WS_TABSTOP or WS_CLIPCHILDREN, _
                                          0, 0, 0, 0, HWND_FRMAICHAT, cast(HMENU, IDC_AICHAT_TRANSCRIPT), GetModuleHandle(NULL), NULL )
    gAiChat.hInput = CreateWindowEx( 0, "Scintilla", "", WS_CHILD or WS_VISIBLE or WS_TABSTOP or WS_CLIPCHILDREN, _
                                     0, 0, 0, 0, HWND_FRMAICHAT, cast(HMENU, IDC_AICHAT_INPUT), GetModuleHandle(NULL), NULL )
    frmAiChat_SetupScintilla( gAiChat.hTranscript, true )
    frmAiChat_SetupScintilla( gAiChat.hInput, false )
    SetWindowSubclass( gAiChat.hInput, cast(SUBCLASSPROC, @frmAiChat_InputProc), 1, 0 )

    gAiChat.hInclude = pWindow->AddControl("CHECKBOX", , IDC_AICHAT_INCLUDE, "Include current file", 0, 0, 0, 0, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_NOTIFY or BS_AUTOCHECKBOX or BS_LEFT or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    gAiChat.hSend = pWindow->AddControl("BUTTON", , IDC_AICHAT_SEND, "Send", 0, 0, 0, 0, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    Button_SetCheck( gAiChat.hInclude, gAi.IncludeFile )

    AiChat_ClearTranscript()
    return 0
end function
