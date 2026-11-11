'    tiko editor - AI assistant: settings and the streaming HTTPS client
'    Licensed under the GNU GPLv3 or later.
'
' A request runs in a worker thread (libcurl). The thread only appends the raw response
' bytes to a shared buffer; the UI thread (frmAiChat timer) takes complete lines out of it
' and parses the server-sent events there, so all JSON parsing (tkjson.bi, shared with the
' tiko-lsp client) stays on the UI thread.
'
'   Anthropic: POST {base}/v1/messages, anthropic-version 2026-10-21, "stream": true.
'              Events: content_block_delta/text_delta (text), content_block_start of type
'              "fallback" (another model continued), message_delta.stop_reason ("refusal"),
'              error. Claude 5.x models opt into server-side refusal fallbacks.
'   OpenAI:    POST {base}/chat/completions, "stream": true. data: {choices[0].delta.content}
'              ... data: [DONE].

#pragma once

#include once "curl.bi"
#include once "../tikolang/tkjson.bi"

#define AI_DEFAULT_ANTHROPIC_MODEL  "claude-opus-5-5"
#define AI_DEFAULT_OPENAI_MODEL     "gpt-5"
#define AI_ANTHROPIC_VERSION        "2026-10-21"

extern "C"
    declare function ai_chmod alias "chmod"( byval path as const zstring ptr, byval mode as ulong ) as long
end extern

' ----------------------------------------------------------------------------------------
' Settings (settings/ai.ini)
' ----------------------------------------------------------------------------------------
private function Ai_SettingsFile() as string
    dim as DWSTRING w = AfxGetExePathName
    dim as string s = w.utf8
    if right(s, 1) <> "/" then s &= "/"
    return s & "settings/ai.ini"
end function

sub Ai_LoadSettings()
    if gAi.Loaded then exit sub
    gAi.Loaded = true
    gAi.Provider = "anthropic"
    gAi.AnthropicModel = AI_DEFAULT_ANTHROPIC_MODEL
    gAi.OpenAIModel = AI_DEFAULT_OPENAI_MODEL
    gAi.AnthropicBaseUrl = "https://api.anthropic.com"
    gAi.OpenAIBaseUrl = "https://api.openai.com/v1"
    gAi.PanelVisible = false
    gAi.PanelWidth = 420
    gAi.IncludeFile = true
    dim as long f = freefile
    if open(Ai_SettingsFile() for input as #f) <> 0 then
        ' first run: use the provider whose key is in the environment
        if (len(environ("ANTHROPIC_API_KEY")) = 0) andalso (len(environ("OPENAI_API_KEY")) > 0) then gAi.Provider = "openai"
        exit sub
    end if
    do until eof(f)
        dim as string sLine
        line input #f, sLine
        sLine = trim(sLine, any !" \t\r")
        dim as long p = instr(sLine, "=")
        if (p = 0) orelse (left(sLine, 1) = "'") orelse (left(sLine, 1) = "#") then continue do
        dim as string sKey = lcase(trim(left(sLine, p - 1))), v = trim(mid(sLine, p + 1))
        select case sKey
            case "provider":         gAi.Provider = iif(lcase(v) = "openai", "openai", "anthropic")
            case "anthropicmodel":   if len(v) then gAi.AnthropicModel = v
            case "openaimodel":      if len(v) then gAi.OpenAIModel = v
            case "anthropickey":     gAi.AnthropicKey = v
            case "openaikey":        gAi.OpenAIKey = v
            case "anthropicbaseurl": if len(v) then gAi.AnthropicBaseUrl = rtrim(v, "/")
            case "openaibaseurl":    if len(v) then gAi.OpenAIBaseUrl = rtrim(v, "/")
            case "panelvisible":     gAi.PanelVisible = (v = "1")
            case "panelwidth":       gAi.PanelWidth = valint(v)
            case "includefile":      gAi.IncludeFile = (v <> "0")
        end select
    loop
    close #f
    if gAi.PanelWidth < 260 then gAi.PanelWidth = 260
end sub

sub Ai_SaveSettings()
    dim as string sFile = Ai_SettingsFile()
    dim as long f = freefile
    if open(sFile for output as #f) <> 0 then exit sub
    print #f, "' AI assistant settings (Environment Options > AI Assistant). Keys here are used only"
    print #f, "' when ANTHROPIC_API_KEY / OPENAI_API_KEY are not set in the environment."
    print #f, "Provider=" & gAi.Provider
    print #f, "AnthropicModel=" & gAi.AnthropicModel
    print #f, "OpenAIModel=" & gAi.OpenAIModel
    print #f, "AnthropicKey=" & gAi.AnthropicKey
    print #f, "OpenAIKey=" & gAi.OpenAIKey
    print #f, "AnthropicBaseUrl=" & gAi.AnthropicBaseUrl
    print #f, "OpenAIBaseUrl=" & gAi.OpenAIBaseUrl
    print #f, "PanelVisible=" & iif(gAi.PanelVisible, "1", "0")
    print #f, "PanelWidth=" & gAi.PanelWidth
    print #f, "IncludeFile=" & iif(gAi.IncludeFile, "1", "0")
    close #f
    ai_chmod( strptr(sFile), &o600 )     ' it can hold API keys: readable by the user only
end sub

function Ai_KeyFromEnvironment( byref sProvider as string ) as boolean
    return len(environ(iif(sProvider = "openai", "OPENAI_API_KEY", "ANTHROPIC_API_KEY"))) > 0
end function

function Ai_ApiKey( byref sProvider as string ) as string
    dim as string sEnv = environ(iif(sProvider = "openai", "OPENAI_API_KEY", "ANTHROPIC_API_KEY"))
    if len(sEnv) then return sEnv
    return iif(sProvider = "openai", gAi.OpenAIKey, gAi.AnthropicKey)
end function

function Ai_ModelName() as string
    Ai_LoadSettings()
    return iif(gAi.Provider = "openai", gAi.OpenAIModel, gAi.AnthropicModel)
end function

' ----------------------------------------------------------------------------------------
' Worker thread
' ----------------------------------------------------------------------------------------
type AI_TRANSFER
    url       as string
    headers   as string        ' one header per line (chr(10))
    body      as string
    ' shared with the UI thread (under mutex)
    mutex     as any ptr
    received  as string        ' raw response bytes not yet taken by the UI
    done      as boolean
    cancel    as boolean
    httpCode  as long
    errorText as string        ' transport error (no connection, timeout, ...)
    thread    as any ptr
end type

private function Ai_OnWrite cdecl ( byval buffer as zstring ptr, byval size as uinteger, _
                                    byval nitems as uinteger, byval userp as any ptr ) as uinteger
    dim as AI_TRANSFER ptr t = userp
    dim as uinteger n = size * nitems
    dim as string s = space(n)
    memcpy( strptr(s), buffer, n )
    MutexLock( t->mutex )
    t->received &= s
    MutexUnlock( t->mutex )
    return n
end function

private function Ai_OnProgress cdecl ( byval clientp as any ptr, byval dltotal as curl_off_t, byval dlnow as curl_off_t, _
                                       byval ultotal as curl_off_t, byval ulnow as curl_off_t ) as long
    dim as AI_TRANSFER ptr t = clientp
    return iif(t->cancel, 1, 0)       ' non-zero aborts the transfer (Stop)
end function

private sub Ai_TransferThread( byval userdata as any ptr )
    dim as AI_TRANSFER ptr t = userdata
    dim as any ptr h = curl_easy_init()
    dim as curl_slist ptr hdrs = NULL
    dim as long p = 1
    do while p <= len(t->headers)
        dim as long q = instr(p, t->headers, chr(10))
        if q = 0 then q = len(t->headers) + 1
        dim as string sHdr = mid(t->headers, p, q - p)
        if len(sHdr) then hdrs = curl_slist_append( hdrs, strptr(sHdr) )
        p = q + 1
    loop
    dim as string sErr
    if h then
        curl_easy_setopt( h, CURLOPT_URL, strptr(t->url) )
        curl_easy_setopt( h, CURLOPT_HTTPHEADER, hdrs )
        curl_easy_setopt( h, CURLOPT_POST, clng(1) )
        curl_easy_setopt( h, CURLOPT_POSTFIELDS, strptr(t->body) )
        curl_easy_setopt( h, CURLOPT_POSTFIELDSIZE_LARGE, cast(curl_off_t, len(t->body)) )
        curl_easy_setopt( h, CURLOPT_WRITEFUNCTION, @Ai_OnWrite )
        curl_easy_setopt( h, CURLOPT_WRITEDATA, t )
        curl_easy_setopt( h, CURLOPT_NOPROGRESS, clng(0) )
        curl_easy_setopt( h, CURLOPT_XFERINFOFUNCTION, @Ai_OnProgress )
        curl_easy_setopt( h, CURLOPT_XFERINFODATA, t )
        curl_easy_setopt( h, CURLOPT_CONNECTTIMEOUT, clng(30) )
        ' a stream that sends nothing for 3 minutes is dead (the APIs send keep-alive pings)
        curl_easy_setopt( h, CURLOPT_LOW_SPEED_LIMIT, clng(1) )
        curl_easy_setopt( h, CURLOPT_LOW_SPEED_TIME, clng(180) )
        curl_easy_setopt( h, CURLOPT_NOSIGNAL, clng(1) )
        dim as CURLcode rc = curl_easy_perform( h )
        dim as clong nCode = 0
        curl_easy_getinfo( h, CURLINFO_RESPONSE_CODE, @nCode )
        MutexLock( t->mutex )
        t->httpCode = nCode
        if rc <> CURLE_OK then
            if rc = CURLE_ABORTED_BY_CALLBACK then
                t->errorText = "stopped"
            else
                t->errorText = *curl_easy_strerror( rc )
            end if
        end if
        MutexUnlock( t->mutex )
        curl_easy_cleanup( h )
    else
        t->errorText = "could not start libcurl"
    end if
    if hdrs then curl_slist_free_all( hdrs )
    MutexLock( t->mutex )
    t->done = true
    MutexUnlock( t->mutex )
end sub

' Starts a request. Returns the transfer (the caller frees it with Ai_FreeTransfer)
function Ai_StartTransfer( byref sUrl as string, byref sHeaders as string, byref sBody as string ) as AI_TRANSFER ptr
    static as boolean bInit
    if bInit = false then curl_global_init( CURL_GLOBAL_DEFAULT ) : bInit = true
    dim as AI_TRANSFER ptr t = new AI_TRANSFER
    t->url = sUrl
    t->headers = sHeaders
    t->body = sBody
    t->mutex = MutexCreate()
    t->thread = ThreadCreate( @Ai_TransferThread, t )
    return t
end function

' Takes what arrived so far; bDone tells whether the transfer has ended
function Ai_TakeReceived( byval t as AI_TRANSFER ptr, byref bDone as boolean ) as string
    MutexLock( t->mutex )
    dim as string s = t->received
    t->received = ""
    bDone = t->done
    MutexUnlock( t->mutex )
    return s
end function

sub Ai_FreeTransfer( byref t as AI_TRANSFER ptr )
    if t = 0 then exit sub
    t->cancel = true
    if t->thread then ThreadWait( t->thread )
    MutexDestroy( t->mutex )
    delete t
    t = 0
end sub

' ----------------------------------------------------------------------------------------
' Requests
' ----------------------------------------------------------------------------------------
' Claude 5.x models: opt into server-side refusal fallbacks
private function Ai_UseFallbacks( byref sModel as string ) as boolean
    dim as string m = lcase(sModel)
    return (left(m, 13) = "claude-opus-5") orelse (left(m, 14) = "claude-fable-5") orelse _
           (left(m, 15) = "claude-mythos-5") orelse (m = "claude-sonnet-5-5")
end function

' sMessagesJson: the conversation as a JSON array of {"role","content"} (content = string)
function Ai_StartChat( byref sSystem as string, byref sMessagesJson as string ) as AI_TRANSFER ptr
    Ai_LoadSettings()
    dim as string sUrl, sHeaders, sBody
    dim as string sKey = Ai_ApiKey(gAi.Provider)
    if gAi.Provider = "openai" then
        sUrl = gAi.OpenAIBaseUrl & "/chat/completions"
        sHeaders = "Content-Type: application/json" & chr(10) & "Authorization: Bearer " & sKey
        ' the system prompt goes first in messages
        dim as string sMsgs = sMessagesJson
        sMsgs = "[{""role"":""system"",""content"":" & TkJ_Esc(sSystem) & "}" & _
                iif(len(sMsgs) > 2, "," & mid(sMsgs, 2), "]")
        sBody = "{""model"":" & TkJ_Esc(gAi.OpenAIModel) & ",""stream"":true,""messages"":" & sMsgs & "}"
    else
        sUrl = gAi.AnthropicBaseUrl & "/v1/messages"
        sHeaders = "Content-Type: application/json" & chr(10) & "x-api-key: " & sKey & chr(10) & _
                   "anthropic-version: " & AI_ANTHROPIC_VERSION
        sBody = "{""model"":" & TkJ_Esc(gAi.AnthropicModel) & ",""max_tokens"":64000,""stream"":true," & _
                """cache_control"":{""type"":""ephemeral""}," & _
                """system"":" & TkJ_Esc(sSystem) & ",""messages"":" & sMessagesJson
        if Ai_UseFallbacks(gAi.AnthropicModel) then
            sHeaders &= chr(10) & "anthropic-beta: server-side-fallback-2026-07-01"
            sBody &= ",""fallbacks"":""default"""
        end if
        sBody &= "}"
    end if
    return Ai_StartTransfer( sUrl, sHeaders, sBody )
end function
