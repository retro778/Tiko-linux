'    tiko editor - "AI Assistant" page of the Environment Options dialog
'    Licensed under the GNU GPLv3 or later.

#pragma once

#define IDC_FRMOPTIONSAI_LBLPROVIDER   1301
#define IDC_FRMOPTIONSAI_COMBOPROVIDER 1302
#define IDC_FRMOPTIONSAI_LBLAMODEL     1303
#define IDC_FRMOPTIONSAI_TXTAMODEL     1304
#define IDC_FRMOPTIONSAI_LBLAKEY       1305
#define IDC_FRMOPTIONSAI_TXTAKEY       1306
#define IDC_FRMOPTIONSAI_LBLOMODEL     1307
#define IDC_FRMOPTIONSAI_TXTOMODEL     1308
#define IDC_FRMOPTIONSAI_LBLOKEY       1309
#define IDC_FRMOPTIONSAI_TXTOKEY       1310
#define IDC_FRMOPTIONSAI_LBLNOTE       1311

private function frmOptionsAi_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    return DefWindowProcW(hwnd, uMsg, wParam, lParam)
end function

private function frmOptionsAi_KeyLabel( byref sProvider as string ) as DWSTRING
    dim as string sEnv = iif(sProvider = "openai", "OPENAI_API_KEY", "ANTHROPIC_API_KEY")
    if Ai_KeyFromEnvironment(sProvider) then
        return DWSTRING("API key (" & sEnv & " is set in the environment and is used):")
    end if
    return DWSTRING("API key (or set " & sEnv & " in the environment):")
end function

function frmOptionsAi_Show( byval hWndParent as HWND ) as LRESULT
    Ai_LoadSettings()
    dim pWindow as CWindow ptr = new CWindow
    HWND_FRMOPTIONSAI = pWindow->Create( hWndParent, "", @frmOptionsAi_WndProc, 0, 0, 0, 0, _
        WS_CHILD, WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    dim as long nLabel = WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY
    dim as long nText = WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL
    dim as long nTextEx = WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLPROVIDER, "AI provider for the chat panel (View > AI Assistant, Ctrl+Shift+A):", _
        0, 2, 400, 16, nLabel, WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSAI_COMBOPROVIDER, "", 0, 21, 250, 20, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, nTextEx)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLAMODEL, "Anthropic model:", 0, 58, 400, 16, nLabel, WS_EX_LEFT)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSAI_TXTAMODEL, "", 0, 76, 250, 20, nText, nTextEx)
    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLAKEY, frmOptionsAi_KeyLabel("anthropic"), 0, 100, 400, 16, nLabel, WS_EX_LEFT)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSAI_TXTAKEY, "", 0, 118, 386, 20, nText or ES_PASSWORD, nTextEx)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLOMODEL, "OpenAI model:", 0, 154, 400, 16, nLabel, WS_EX_LEFT)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSAI_TXTOMODEL, "", 0, 172, 250, 20, nText, nTextEx)
    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLOKEY, frmOptionsAi_KeyLabel("openai"), 0, 196, 400, 16, nLabel, WS_EX_LEFT)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSAI_TXTOKEY, "", 0, 214, 386, 20, nText or ES_PASSWORD, nTextEx)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSAI_LBLNOTE, _
        "Keys entered here are saved in settings/ai.ini (readable by you only). Questions are sent to the chosen " & _
        "provider with the current file when 'Include current file' is ticked in the panel.", _
        0, 250, 400, 64, WS_CHILD or WS_VISIBLE or SS_LEFT, WS_EX_LEFT or WS_EX_LTRREADING)

    dim as HWND hwnd = HWND_FRMOPTIONSAI
    dim as wstring * 32 wszA = "Anthropic (Claude)", wszO = "OpenAI"
    dim as HWND hCombo = GetDlgItem(hwnd, IDC_FRMOPTIONSAI_COMBOPROVIDER)
    ComboBox_AddString( hCombo, @wszA )       ' 0
    ComboBox_AddString( hCombo, @wszO )       ' 1
    ComboBox_SetCurSel( hCombo, iif(gAi.Provider = "openai", 1, 0) )
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTAMODEL), DWSTRING(gAi.AnthropicModel, CP_UTF8) )
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTOMODEL), DWSTRING(gAi.OpenAIModel, CP_UTF8) )
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTAKEY), DWSTRING(gAi.AnthropicKey, CP_UTF8) )
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTOKEY), DWSTRING(gAi.OpenAIKey, CP_UTF8) )
    return 0
end function

' Environment Options closed with OK
sub frmOptionsAi_Save()
    dim as HWND hwnd = HWND_FRMOPTIONSAI
    if hwnd = 0 then exit sub
    gAi.Provider = iif(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_FRMOPTIONSAI_COMBOPROVIDER)) = 1, "openai", "anthropic")
    dim as DWSTRING w
    w = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTAMODEL) ) : gAi.AnthropicModel = trim(w.utf8)
    w = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTOMODEL) ) : gAi.OpenAIModel = trim(w.utf8)
    w = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTAKEY) ) : gAi.AnthropicKey = trim(w.utf8)
    w = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSAI_TXTOKEY) ) : gAi.OpenAIKey = trim(w.utf8)
    if len(gAi.AnthropicModel) = 0 then gAi.AnthropicModel = AI_DEFAULT_ANTHROPIC_MODEL
    if len(gAi.OpenAIModel) = 0 then gAi.OpenAIModel = AI_DEFAULT_OPENAI_MODEL
    Ai_SaveSettings()
    if HWND_FRMAICHAT then InvalidateRect( HWND_FRMAICHAT, NULL, TRUE )     ' the model in the header
end sub
