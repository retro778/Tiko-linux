'    tiko editor - AI assistant (Linux build, #ifdef AI_ASSISTANT)
'    Licensed under the GNU GPLv3 or later.
'
' A chat panel docked on the right of the main window (frmAiChat.inc) that talks to the
' Anthropic Messages API or the OpenAI Chat Completions API over HTTPS (libcurl, streamed,
' in a worker thread: modAiClient.inc). Settings: settings/ai.ini, the "AI Assistant" page
' of Environment Options (frmOptionsAi.inc).

#pragma once

' ---- settings ------------------------------------------------------------------------
type AI_SETTINGS
    Loaded           as boolean
    Provider         as string      ' "anthropic" | "openai"
    AnthropicModel   as string
    OpenAIModel      as string
    AnthropicKey     as string      ' used when ANTHROPIC_API_KEY is not set
    OpenAIKey        as string      ' used when OPENAI_API_KEY is not set
    AnthropicBaseUrl as string      ' https://api.anthropic.com
    OpenAIBaseUrl    as string      ' https://api.openai.com/v1
    PanelVisible     as boolean
    PanelWidth       as long        ' unscaled pixels
    IncludeFile      as boolean     ' send the active file with each question
end type
dim shared gAi as AI_SETTINGS

declare sub Ai_LoadSettings()
declare sub Ai_SaveSettings()
declare function Ai_ApiKey( byref sProvider as string ) as string
declare function Ai_KeyFromEnvironment( byref sProvider as string ) as boolean
declare function Ai_ModelName() as string

' ---- chat panel ------------------------------------------------------------------------
dim shared as HWND HWND_FRMAICHAT
declare function frmAiChat_Show( byval hWndParent as HWND ) as LRESULT
declare function frmAiChat_Layout( byref rc as RECT, byval nTop as long, byval nBottom as long ) as long
declare sub frmAiChat_Toggle()
declare function frmAiChat_IsVisible() as boolean
declare sub frmAiChat_ExplainErrors()

' ---- options page -----------------------------------------------------------------------
dim shared as HWND HWND_FRMOPTIONSAI
declare function frmOptionsAi_Show( byval hWndParent as HWND ) as LRESULT
declare sub frmOptionsAi_Save()
declare function frmAiChat_PreTranslate( byref uMsg as MSG ) as boolean
