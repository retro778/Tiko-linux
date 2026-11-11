'    tiko editor - "Compilers" page of the Environment Options dialog (Linux build)
'    Licensed under the GNU GPLv3 or later.
'
' Lets the user choose which compiler builds which files: fbc or tiko for .bas/.bi/.inc,
' tiko or fbc for .tiko/.ti/.tko, and where the tiko compiler is. Stored in
' settings/tikolang.ini (TikoLang_SaveSettings).

#pragma once

#define IDC_FRMOPTIONSTIKOLANG_LBLBAS       1101
#define IDC_FRMOPTIONSTIKOLANG_COMBOBAS     1102
#define IDC_FRMOPTIONSTIKOLANG_LBLTIKO      1103
#define IDC_FRMOPTIONSTIKOLANG_COMBOTIKO    1104
#define IDC_FRMOPTIONSTIKOLANG_LBLPATH      1105
#define IDC_FRMOPTIONSTIKOLANG_TXTPATH      1106
#define IDC_FRMOPTIONSTIKOLANG_CMDPATH      1107
#define IDC_FRMOPTIONSTIKOLANG_LBLFOUND     1108
#define IDC_FRMOPTIONSTIKOLANG_LBLNOTE      1109

' Shows which tiko compiler a build would use
private sub frmOptionsTikoLang_UpdateFound( byval hwnd as HWND )
    dim as DWSTRING wszPath = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_TXTPATH) )
    dim as string sSaved = gTikoLang.CompilerPath
    gTikoLang.CompilerPath = trim(wszPath.utf8)
    dim as DWSTRING wszCompiler = TikoLang_FindCompiler()
    gTikoLang.CompilerPath = sSaved

    dim as DWSTRING wszText
    if len(wszCompiler) = 0 then
        wszText = "tiko compiler: not found"
    elseif AfxFileExists(wszCompiler) = 0 then
        wszText = "tiko compiler: file not found: " & wszCompiler
    else
        wszText = "tiko compiler: " & wszCompiler & "  (" & DWSTRING(TikoLang_Version(wszCompiler), CP_UTF8) & ")"
    end if
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_LBLFOUND), wszText )
end sub

private function frmOptionsTikoLang_OnCommand( _
            byval hwnd as HWND, _
            byval id as long, _
            byval hwndCtl as HWND, _
            byval codeNotify as UINT _
            ) as LRESULT
    select case id
        case IDC_FRMOPTIONSTIKOLANG_CMDPATH
            if codeNotify = BN_CLICKED then
                dim pwszName as wstring ptr = AfxIFileOpenDialogW(hwnd, id)
                if pwszName then
                    SetWindowTextW( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_TXTPATH), pwszName )
                    CoTaskMemFree pwszName
                    frmOptionsTikoLang_UpdateFound( hwnd )
                end if
                exit function
            end if
        case IDC_FRMOPTIONSTIKOLANG_TXTPATH
            if codeNotify = EN_KILLFOCUS then frmOptionsTikoLang_UpdateFound( hwnd )
    end select
    function = 0
end function

private function frmOptionsTikoLang_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT
    select case uMsg
        HANDLE_MSG (hwnd, WM_COMMAND, frmOptionsTikoLang_OnCommand)
    end select
    function = DefWindowProcW(hwnd, uMsg, wParam, lParam)
end function

function frmOptionsTikoLang_Show( byval hWndParent as HWND ) as LRESULT
    TikoLang_LoadSettings()

    dim pWindow as CWindow ptr = new CWindow
    HWND_FRMOPTIONSTIKOLANG = pWindow->Create( hWndParent, "", @frmOptionsTikoLang_WndProc, 0, 0, 0, 0, _
        WS_CHILD, WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSTIKOLANG_LBLBAS, _
        "Compile FreeBASIC files (.bas .bi .inc) with:", 0, 2, 386, 16, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSTIKOLANG_COMBOBAS, "", 0, 21, 250, 20, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSTIKOLANG_LBLTIKO, _
        "Compile tiko files (.tiko .ti .tko) with:", 0, 56, 386, 16, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("COMBOBOX", , IDC_FRMOPTIONSTIKOLANG_COMBOTIKO, "", 0, 75, 250, 20, _
        WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or CBS_DROPDOWNLIST, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSTIKOLANG_LBLPATH, _
        "tiko compiler (leave empty to find it automatically):", 0, 110, 386, 16, _
        WS_CHILD or WS_VISIBLE or SS_LEFT or SS_NOTIFY, WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("TEXTBOX", , IDC_FRMOPTIONSTIKOLANG_TXTPATH, "", 0, 129, 356, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or ES_LEFT or ES_AUTOHSCROLL, _
        WS_EX_CLIENTEDGE or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)
    pWindow->AddControl("BUTTON", , IDC_FRMOPTIONSTIKOLANG_CMDPATH, "...", 362, 129, 24, 20, _
        WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_TEXT or BS_PUSHBUTTON or BS_NOTIFY or BS_CENTER or BS_VCENTER, _
        WS_EX_LEFT or WS_EX_LTRREADING)
    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSTIKOLANG_LBLFOUND, "", 0, 154, 400, 32, _
        WS_CHILD or WS_VISIBLE or SS_LEFT, WS_EX_LEFT or WS_EX_LTRREADING)

    pWindow->AddControl("LABEL", , IDC_FRMOPTIONSTIKOLANG_LBLNOTE, _
        "The editor follows the compiler: files built with tiko get the tiko keywords " & _
        "and F1 opens the tiko wiki. tiko also compiles FreeBASIC code; fbc only " & _
        "compiles tiko files that use no tiko-only features (#import, generics, ustring, ...).", _
        0, 196, 400, 64, WS_CHILD or WS_VISIBLE or SS_LEFT, WS_EX_LEFT or WS_EX_LTRREADING)

    dim as HWND hwnd = HWND_FRMOPTIONSTIKOLANG
    dim as HWND hCombo = GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_COMBOBAS)
    ComboBox_AddString( hCombo, @wstr("FreeBASIC (fbc)") )
    ComboBox_AddString( hCombo, @wstr("tiko") )
    ComboBox_SetCurSel( hCombo, iif(gTikoLang.BasWithTiko, 1, 0) )
    hCombo = GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_COMBOTIKO)
    ComboBox_AddString( hCombo, @wstr("tiko") )
    ComboBox_AddString( hCombo, @wstr("FreeBASIC (fbc)") )
    ComboBox_SetCurSel( hCombo, iif(gTikoLang.TikoWithFbc, 1, 0) )
    AfxSetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_TXTPATH), DWSTRING(gTikoLang.CompilerPath, CP_UTF8) )
    frmOptionsTikoLang_UpdateFound( hwnd )

    function = 0
end function

' Called when the Environment Options dialog is closed with OK
sub frmOptionsTikoLang_Save()
    dim as HWND hwnd = HWND_FRMOPTIONSTIKOLANG
    if hwnd = 0 then exit sub
    gTikoLang.BasWithTiko = (ComboBox_GetCurSel( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_COMBOBAS) ) = 1)
    gTikoLang.TikoWithFbc = (ComboBox_GetCurSel( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_COMBOTIKO) ) = 1)
    dim as DWSTRING wszPath = AfxGetWindowText( GetDlgItem(hwnd, IDC_FRMOPTIONSTIKOLANG_TXTPATH) )
    gTikoLang.CompilerPath = trim(wszPath.utf8)
    TikoLang_SaveSettings()
end sub
