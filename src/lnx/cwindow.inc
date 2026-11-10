'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' CWindow class (subset of the AfxNova CWindow used by tiko), message crackers,
' accelerators, IsDialogMessage, popup menus, message boxes, tooltips and the
' Afx window helper procedures.

#pragma once

' ----------------------------------------------------------------------------
' Message crackers (windowsx.bi)
' ----------------------------------------------------------------------------
#ifndef CTRUE
    #define CTRUE 1
#endif
#ifndef CTLCOLOR_STATIC
#define CTLCOLOR_STATIC 6
#endif
type HDROP as any ptr
#undef HANDLE_MSG
#define HANDLE_MSG(hwnd,message,fn) case message: return HANDLE_##message(hwnd,wParam,lParam,fn)
#define HANDLE_WM_CREATE(hwnd, wParam, lParam, fn) cast(LRESULT, iif(fn((hwnd), cast(LPCREATESTRUCT, (lParam))), 0, -1))
#define HANDLE_WM_DESTROY(hwnd, wParam, lParam, fn) fn(hwnd)
#define HANDLE_WM_SIZE(hwnd, wParam, lParam, fn) fn((hwnd), cast(UINT, (wParam)), clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))))
#define HANDLE_WM_CLOSE(hwnd, wParam, lParam, fn) fn(hwnd)
#define HANDLE_WM_PAINT(hwnd, wParam, lParam, fn) fn(hwnd)
#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) fn((hwnd), cast(HDROP, (wParam)))
#define HANDLE_WM_ACTIVATEAPP(hwnd, wParam, lParam, fn) fn((hwnd), cast(boolean, (wParam) <> 0), cast(DWORD, (lParam)))
#define HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) fn((hwnd), clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))), cast(UINT, (wParam)))
#define HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) fn((hwnd), false, clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))), cast(UINT, (wParam)))
#define HANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) fn((hwnd), true, clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))), cast(UINT, (wParam)))
#define HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) fn((hwnd), clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))), cast(UINT, (wParam)))
#define HANDLE_WM_COMMAND(hwnd, wParam, lParam, fn) fn((hwnd), clng(LOWORD(wParam)), cast(HWND, (lParam)), cast(UINT, HIWORD(wParam)))
#define HANDLE_WM_CTLCOLORSTATIC(hwnd_, wParam, lParam, fn) cast(LRESULT, fn((hwnd_), cast(HDC, (wParam)), cast(HWND, (lParam)), CTLCOLOR_STATIC))
#define HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, fn) fn((hwnd), cptr(const DRAWITEMSTRUCT ptr, (lParam)))
#define HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, fn) fn((hwnd), cptr(MEASUREITEMSTRUCT ptr, (lParam)))
#define HANDLE_WM_CONTEXTMENU(hwnd_, wParam, lParam, fn) fn((hwnd_), cast(HWND, (wParam)), clng(cshort(LOWORD(lParam))), clng(cshort(HIWORD(lParam))))
#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) fn((hwnd), clng(wParam), cast(NMHDR ptr, (lParam)))

' Listbox/Combobox/Button/Edit macros (windowsx.bi)
#define ListBox_AddString(hwndCtl, lpsz)        clng(SendMessage((hwndCtl), LB_ADDSTRING, 0, cast(LPARAM, cast(LPCWSTR, (lpsz)))))
#define ListBox_InsertString(hwndCtl, i, lpsz)  clng(SendMessage((hwndCtl), LB_INSERTSTRING, cast(WPARAM, (i)), cast(LPARAM, cast(LPCWSTR, (lpsz)))))
#define ListBox_DeleteString(hwndCtl, i)        clng(SendMessage((hwndCtl), LB_DELETESTRING, cast(WPARAM, (i)), 0))
#define ListBox_ResetContent(hwndCtl)           cast(BOOL, SendMessage((hwndCtl), LB_RESETCONTENT, 0, 0))
#define ListBox_GetCount(hwndCtl)               clng(SendMessage((hwndCtl), LB_GETCOUNT, 0, 0))
#define ListBox_GetCurSel(hwndCtl)              clng(SendMessage((hwndCtl), LB_GETCURSEL, 0, 0))
#define ListBox_SetCurSel(hwndCtl, index)       clng(SendMessage((hwndCtl), LB_SETCURSEL, cast(WPARAM, (index)), 0))
#define ListBox_GetSel(hwndCtl, index)          clng(SendMessage((hwndCtl), LB_GETSEL, cast(WPARAM, (index)), 0))
#define ListBox_SetSel(hwndCtl, fSelect, index) clng(SendMessage((hwndCtl), LB_SETSEL, cast(WPARAM, (fSelect)), cast(LPARAM, (index))))
#define ListBox_GetSelCount(hwndCtl)            clng(SendMessage((hwndCtl), LB_GETSELCOUNT, 0, 0))
#define ListBox_GetItemData(hwndCtl, index)     cast(LRESULT, SendMessage((hwndCtl), LB_GETITEMDATA, cast(WPARAM, (index)), 0))
#define ListBox_SetItemData(hwndCtl, index, data_) clng(SendMessage((hwndCtl), LB_SETITEMDATA, cast(WPARAM, (index)), cast(LPARAM, (data_))))
#define ListBox_GetItemRect(hwndCtl, index, lprc) clng(SendMessage((hwndCtl), LB_GETITEMRECT, cast(WPARAM, (index)), cast(LPARAM, cast(RECT ptr, (lprc)))))
#define ListBox_GetTopIndex(hwndCtl)            clng(SendMessage((hwndCtl), LB_GETTOPINDEX, 0, 0))
#define ListBox_SetTopIndex(hwndCtl, indexTop)  clng(SendMessage((hwndCtl), LB_SETTOPINDEX, cast(WPARAM, (indexTop)), 0))
#define ListBox_GetCaretIndex(hwndCtl)          clng(SendMessage((hwndCtl), LB_GETCARETINDEX, 0, 0))
#define ListBox_SetCaretIndex(hwndCtl, index)   clng(SendMessage((hwndCtl), LB_SETCARETINDEX, cast(WPARAM, (index)), 0))
#define ListBox_SetAnchorIndex(hwndCtl, index)  clng(SendMessage((hwndCtl), LB_SETANCHORINDEX, cast(WPARAM, (index)), 0))
#define ListBox_GetTextLen(hwndCtl, index)      clng(SendMessage((hwndCtl), LB_GETTEXTLEN, cast(WPARAM, (index)), 0))
#define ListBox_GetText(hwndCtl, index, lpszBuffer) clng(SendMessage((hwndCtl), LB_GETTEXT, cast(WPARAM, (index)), cast(LPARAM, (lpszBuffer))))
#define ListBox_FindStringExact(hwndCtl, indexStart, lpszFind) clng(SendMessage((hwndCtl), LB_FINDSTRINGEXACT, cast(WPARAM, (indexStart)), cast(LPARAM, cast(LPCWSTR, (lpszFind)))))

private function ListBox_ReplaceString( byval hListBox as HWND, byval nIndex as long, byref wszNewText as const wstring ) as long
    dim as LPARAM pData = ListBox_GetItemData( hListBox, nIndex )
    ListBox_DeleteString( hListBox, nIndex )
    dim as long idx = ListBox_InsertString( hListBox, nIndex, @wszNewText )
    ListBox_SetItemData( hListBox, idx, pData )
    return idx
end function

' Returns the index of the item nearest the point (client coordinates)
private function Listbox_ItemFromPoint( byval hListBox as HWND, byval xPos as long, byval yPos as long ) as long
    return LOWORD( SendMessage( hListBox, LB_ITEMFROMPOINT, 0, MAKELPARAM(xPos, yPos) ) )
end function

#define ComboBox_AddString(hwndCtl, lpsz)       clng(SendMessage((hwndCtl), CB_ADDSTRING, 0, cast(LPARAM, cast(LPCWSTR, (lpsz)))))
#define ComboBox_InsertString(hwndCtl, index, lpsz) clng(SendMessage((hwndCtl), CB_INSERTSTRING, cast(WPARAM, (index)), cast(LPARAM, cast(LPCWSTR, (lpsz)))))
#define ComboBox_DeleteString(hwndCtl, index)   clng(SendMessage((hwndCtl), CB_DELETESTRING, cast(WPARAM, (index)), 0))
#define ComboBox_ResetContent(hwndCtl)          clng(SendMessage((hwndCtl), CB_RESETCONTENT, 0, 0))
#define ComboBox_GetCount(hwndCtl)              clng(SendMessage((hwndCtl), CB_GETCOUNT, 0, 0))
#define ComboBox_GetCurSel(hwndCtl)             clng(SendMessage((hwndCtl), CB_GETCURSEL, 0, 0))
#define ComboBox_SetCurSel(hwndCtl, index)      clng(SendMessage((hwndCtl), CB_SETCURSEL, cast(WPARAM, (index)), 0))
#define ComboBox_GetItemData(hwndCtl, index)    cast(LRESULT, SendMessage((hwndCtl), CB_GETITEMDATA, cast(WPARAM, (index)), 0))
#define ComboBox_SetItemData(hwndCtl, index, data_) clng(SendMessage((hwndCtl), CB_SETITEMDATA, cast(WPARAM, (index)), cast(LPARAM, (data_))))
#define ComboBox_FindStringExact(hwndCtl, indexStart, lpszFind) clng(SendMessage((hwndCtl), CB_FINDSTRINGEXACT, cast(WPARAM, (indexStart)), cast(LPARAM, cast(LPCWSTR, (lpszFind)))))
#define ComboBox_FindString(hwndCtl, indexStart, lpszFind) clng(SendMessage((hwndCtl), CB_FINDSTRING, cast(WPARAM, (indexStart)), cast(LPARAM, cast(LPCWSTR, (lpszFind)))))
#define ComboBox_GetLBTextLen(hwndCtl, index)   clng(SendMessage((hwndCtl), CB_GETLBTEXTLEN, cast(WPARAM, (index)), 0))
#define ComboBox_GetLBText(hwndCtl, index, lpszBuffer) clng(SendMessage((hwndCtl), CB_GETLBTEXT, cast(WPARAM, (index)), cast(LPARAM, (lpszBuffer))))
#define ComboBox_ShowDropdown(hwndCtl, fShow)   cast(BOOL, SendMessage((hwndCtl), CB_SHOWDROPDOWN, cast(WPARAM, (fShow)), 0))

#define Button_GetCheck(hwndCtl)                clng(SendMessage((hwndCtl), BM_GETCHECK, 0, 0))
#define Button_SetCheck(hwndCtl, check)         SendMessage((hwndCtl), BM_SETCHECK, cast(WPARAM, clng(check)), 0)
#define Edit_SetSel(hwndCtl, ichStart, ichEnd)  SendMessage((hwndCtl), EM_SETSEL, (ichStart), (ichEnd))
#define Edit_GetLineCount(hwndCtl)              clng(SendMessage((hwndCtl), EM_GETLINECOUNT, 0, 0))
#define Edit_LimitText(hwndCtl, cchMax)         SendMessage((hwndCtl), EM_LIMITTEXT, cast(WPARAM, (cchMax)), 0)
#define Edit_SetReadOnly(hwndCtl, fReadOnly)    cast(BOOL, SendMessage((hwndCtl), EM_SETREADONLY, cast(WPARAM, (fReadOnly)), 0))
#define Edit_ReplaceSel(hwndCtl, lpszReplace)   SendMessage((hwndCtl), EM_REPLACESEL, 0, cast(LPARAM, cast(LPCWSTR, (lpszReplace))))
#define Edit_SetCueBannerText(hwnd, lpcwText)   cast(BOOL, SendMessage((hwnd), EM_SETCUEBANNER, 0, cast(LPARAM, (lpcwText))))
#define Edit_GetCueBannerText(hwnd, lpwText, cchText) cast(BOOL, SendMessage((hwnd), EM_GETCUEBANNER, cast(WPARAM, (lpwText)), cast(LPARAM, (cchText))))

private function CheckRadioButton( byval hDlg as HWND, byval nIDFirst as long, byval nIDLast as long, byval nIDCheck as long ) as BOOL
    for id as long = nIDFirst to nIDLast
        dim as HWND h = GetDlgItem( hDlg, id )
        if h then SendMessage( h, BM_SETCHECK, iif(id = nIDCheck, BST_CHECKED, BST_UNCHECKED), 0 )
    next
    return 1
end function

private function IsDlgButtonChecked( byval hDlg as HWND, byval nIDButton as long ) as UINT
    return SendMessage( GetDlgItem(hDlg, nIDButton), BM_GETCHECK, 0, 0 )
end function

private function CheckDlgButton( byval hDlg as HWND, byval nIDButton as long, byval uCheck as UINT ) as BOOL
    SendMessage( GetDlgItem(hDlg, nIDButton), BM_SETCHECK, uCheck, 0 )
    return 1
end function


' ----------------------------------------------------------------------------
' Accelerators
' ----------------------------------------------------------------------------
type LNXACCELTABLE
    entries(any) as ACCEL
    count as long
end type

private function CreateAcceleratorTable( byval paccel as ACCEL ptr, byval cAccel as long ) as HACCEL
    dim as LNXACCELTABLE ptr t = new LNXACCELTABLE
    if cAccel > 0 then
        redim t->entries(cAccel - 1)
        for i as long = 0 to cAccel - 1 : t->entries(i) = paccel[i] : next
    end if
    t->count = cAccel
    return t
end function
#define CreateAcceleratorTableW CreateAcceleratorTable

private function DestroyAcceleratorTable( byval hAccel as HACCEL ) as BOOL
    if hAccel = 0 then return 0
    delete cast(LNXACCELTABLE ptr, hAccel)
    return 1
end function

private function TranslateAccelerator( byval hwnd as HWND, byval hAccTable as HACCEL, byval lpMsg as MSG ptr ) as long
    if hAccTable = 0 orelse lpMsg = 0 then return 0
    if lpMsg->message <> WM_KEYDOWN andalso lpMsg->message <> WM_SYSKEYDOWN then return 0
    dim as LNXACCELTABLE ptr t = hAccTable
    dim as boolean bCtrl = (GetKeyState(VK_CONTROL) and &h8000) <> 0
    dim as boolean bShift = (GetKeyState(VK_SHIFT) and &h8000) <> 0
    dim as boolean bAlt = (GetKeyState(VK_MENU) and &h8000) <> 0
    if lpMsg->lnxEvent then
        dim as GdkEventKey ptr ek = cast(GdkEventKey ptr, lpMsg->lnxEvent)
        bCtrl = (ek->state and GDK_CONTROL_MASK) <> 0
        bShift = (ek->state and GDK_SHIFT_MASK) <> 0
        bAlt = (ek->state and GDK_MOD1_MASK) <> 0
    end if
    for i as long = 0 to t->count - 1
        with t->entries(i)
            if (.fVirt and FVIRTKEY) = 0 then continue for
            if .key <> lpMsg->wParam then continue for
            if ((.fVirt and FCONTROL) <> 0) <> bCtrl then continue for
            if ((.fVirt and FSHIFT) <> 0) <> bShift then continue for
            if ((.fVirt and FALT) <> 0) <> bAlt then continue for
            SendMessage( hwnd, WM_COMMAND, MAKEWPARAM(.cmd, 1), 0 )
            return 1
        end with
    next
    return 0
end function
#define TranslateAcceleratorW TranslateAccelerator


' ----------------------------------------------------------------------------
' IsDialogMessage: TAB / ESC / ENTER handling for dialogs
' ----------------------------------------------------------------------------
private function LnxFindDefaultButton( byval hDlg as HWND ) as HWND
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND c = gLnxWindows(i)
        if c->parent = hDlg andalso c->kind = LK_BUTTON andalso (c->style and &hF) = BS_DEFPUSHBUTTON andalso c->visible andalso c->enabled then return c
    next
    return 0
end function

private function IsDialogMessage( byval hDlg as HWND, byval lpMsg as MSG ptr ) as BOOL
    if IsWindow(hDlg) = 0 orelse lpMsg = 0 orelse IsWindow(lpMsg->hwnd) = 0 then return 0
    if LnxIsDescendant( lpMsg->hwnd, hDlg ) = false then return 0
    if lpMsg->message = WM_KEYDOWN then
        dim as LRESULT dlgCode = SendMessage( lpMsg->hwnd, WM_GETDLGCODE, lpMsg->wParam, cast(LPARAM, lpMsg) )
        select case lpMsg->wParam
            case VK_ESCAPE
                if (dlgCode and DLGC_WANTALLKEYS) = 0 then
                    ' combobox with open popup closes itself
                    SendMessage( hDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), cast(LPARAM, GetDlgItem(hDlg, IDCANCEL)) )
                    return 1
                end if
            case VK_RETURN
                if (dlgCode and DLGC_WANTALLKEYS) = 0 andalso lpMsg->hwnd->kind <> LK_EDITMULTI andalso _
                   lpMsg->hwnd->kind <> LK_RICHEDIT andalso lpMsg->hwnd->kind <> LK_SCINTILLA then
                    dim as HWND hDef = LnxFindDefaultButton( hDlg )
                    dim as long id = iif(hDef, hDef->id, IDOK)
                    if lpMsg->hwnd->kind = LK_BUTTON then id = lpMsg->hwnd->id : hDef = lpMsg->hwnd
                    SendMessage( hDlg, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), cast(LPARAM, hDef) )
                    return 1
                end if
        end select
    end if
    ' everything else is processed like a normal message (TAB focus moves are done by GTK)
    TranslateMessage( lpMsg )
    DispatchMessage( lpMsg )
    return 1
end function
#define IsDialogMessageW IsDialogMessage

private function GetNextDlgTabItem( byval hDlg as HWND, byval hCtl as HWND, byval bPrevious as BOOL ) as HWND
    dim as HWND items(any)
    dim as long n = 0, cur = -1
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND c = gLnxWindows(i)
        if c->parent = hDlg andalso (c->style and WS_TABSTOP) andalso c->visible andalso c->enabled then
            redim preserve items(n) : items(n) = c
            if c = hCtl then cur = n
            n += 1
        end if
    next
    if n = 0 then return 0
    if cur = -1 then return items(0)
    if bPrevious then return items((cur - 1 + n) mod n)
    return items((cur + 1) mod n)
end function


' ----------------------------------------------------------------------------
' Popup menus (CreatePopupMenu / AppendMenu / TrackPopupMenu) -> GtkMenu
' ----------------------------------------------------------------------------
#ifndef MF_STRING
#define MF_STRING      &h0000
#endif
#ifndef MF_GRAYED
#define MF_GRAYED      &h0001
#endif
#ifndef MF_CHECKED
#define MF_CHECKED     &h0008
#endif
#ifndef MF_UNCHECKED
#define MF_UNCHECKED   &h0000
#endif
#ifndef MF_POPUP
#define MF_POPUP       &h0010
#endif
#ifndef MF_BYPOSITION
#define MF_BYPOSITION  &h0400
#endif
#ifndef MF_OWNERDRAW
#define MF_OWNERDRAW   &h0100
#endif
#ifndef TPM_LEFTALIGN
#define TPM_LEFTALIGN  &h0000
#endif
#ifndef TPM_CENTERALIGN
#define TPM_CENTERALIGN &h0004
#endif
#ifndef TPM_TOPALIGN
#define TPM_TOPALIGN   &h0000
#endif
#ifndef TPM_BOTTOMALIGN
#define TPM_BOTTOMALIGN &h0020
#endif
#ifndef TPM_LEFTBUTTON
#define TPM_LEFTBUTTON &h0000
#endif
#ifndef TPM_RIGHTBUTTON
#define TPM_RIGHTBUTTON &h0002
#endif
#ifndef MIIM_STATE
#define MIIM_STATE     &h0001
#endif
#ifndef MIIM_ID
#define MIIM_ID        &h0002
#endif
#ifndef MIIM_STRING
#define MIIM_STRING    &h0040
#endif
#ifndef MIIM_FTYPE
#define MIIM_FTYPE     &h0100
#endif
#ifndef MFS_CHECKED
#define MFS_CHECKED    &h0008
#endif
#ifndef MFS_DISABLED
#define MFS_DISABLED   &h0003
#endif
#ifndef MFS_GRAYED
#define MFS_GRAYED     &h0003
#endif

type LNXMENUITEM
    flags   as UINT
    id      as UINT_PTR
    text    as DWSTRING
    submenu as HMENU
    radio   as boolean
end type

type LNXMENU
    items(any) as LNXMENUITEM
    count as long
end type

private function CreatePopupMenu() as HMENU
    return new LNXMENU
end function
#define CreateMenu CreatePopupMenu

private function AppendMenu( byval hMenu as HMENU, byval uFlags as UINT, byval uIDNewItem as UINT_PTR, byval lpNewItem as LPCWSTR ) as BOOL
    if hMenu = 0 then return 0
    redim preserve hMenu->items(hMenu->count)
    with hMenu->items(hMenu->count)
        .flags = uFlags
        if (uFlags and MF_POPUP) then .submenu = cast(HMENU, uIDNewItem) else .id = uIDNewItem
        if lpNewItem andalso (uFlags and MF_SEPARATOR) = 0 then .text = *lpNewItem
    end with
    hMenu->count += 1
    return 1
end function
#define AppendMenuW AppendMenu

private function InsertMenu( byval hMenu as HMENU, byval uPosition as UINT, byval uFlags as UINT, byval uIDNewItem as UINT_PTR, byval lpNewItem as LPCWSTR ) as BOOL
    if hMenu = 0 then return 0
    dim as long pos_ = hMenu->count
    if (uFlags and MF_BYPOSITION) andalso uPosition < hMenu->count then pos_ = uPosition
    AppendMenu( hMenu, uFlags and not MF_BYPOSITION, uIDNewItem, lpNewItem )
    dim as LNXMENUITEM it = hMenu->items(hMenu->count - 1)
    for i as long = hMenu->count - 1 to pos_ + 1 step -1
        hMenu->items(i) = hMenu->items(i - 1)
    next
    hMenu->items(pos_) = it
    return 1
end function

private function LnxFindMenuItem( byval hMenu as HMENU, byval uItem as UINT, byval byPos as boolean ) as LNXMENUITEM ptr
    if hMenu = 0 then return 0
    if byPos then
        if uItem < hMenu->count then return @hMenu->items(uItem)
        return 0
    end if
    for i as long = 0 to hMenu->count - 1
        if hMenu->items(i).submenu = 0 andalso hMenu->items(i).id = uItem then return @hMenu->items(i)
        if hMenu->items(i).submenu then
            dim as LNXMENUITEM ptr p = LnxFindMenuItem( hMenu->items(i).submenu, uItem, false )
            if p then return p
        end if
    next
    return 0
end function

private function CheckMenuItem( byval hMenu as HMENU, byval uIDCheckItem as UINT, byval uCheck as UINT ) as DWORD
    dim as LNXMENUITEM ptr p = LnxFindMenuItem( hMenu, uIDCheckItem, (uCheck and MF_BYPOSITION) <> 0 )
    if p = 0 then return -1
    dim as DWORD prev = p->flags and MF_CHECKED
    p->flags = (p->flags and not MF_CHECKED) or (uCheck and MF_CHECKED)
    return prev
end function

private function EnableMenuItem( byval hMenu as HMENU, byval uIDEnableItem as UINT, byval uEnable as UINT ) as BOOL
    dim as LNXMENUITEM ptr p = LnxFindMenuItem( hMenu, uIDEnableItem, (uEnable and MF_BYPOSITION) <> 0 )
    if p = 0 then return -1
    p->flags = (p->flags and not (MF_GRAYED or MF_DISABLED)) or (uEnable and (MF_GRAYED or MF_DISABLED))
    return 0
end function

private function CheckMenuRadioItem( byval hMenu as HMENU, byval first as UINT, byval last as UINT, byval check as UINT, byval flags as UINT ) as BOOL
    if hMenu = 0 then return 0
    dim as boolean byPos = (flags and MF_BYPOSITION) <> 0
    for i as long = 0 to hMenu->count - 1
        dim as UINT key = iif(byPos, i, hMenu->items(i).id)
        if key >= first andalso key <= last then
            hMenu->items(i).radio = true
            if key = check then hMenu->items(i).flags or= MF_CHECKED else hMenu->items(i).flags and= not MF_CHECKED
        end if
    next
    return 1
end function

private function GetMenuItemCount( byval hMenu as HMENU ) as long
    if hMenu = 0 then return -1
    return hMenu->count
end function

private function DestroyMenu( byval hMenu as HMENU ) as BOOL
    if hMenu = 0 then return 0
    for i as long = 0 to hMenu->count - 1
        if hMenu->items(i).submenu then DestroyMenu( hMenu->items(i).submenu )
    next
    delete hMenu
    return 1
end function

extern "C"
    declare sub gtk_menu_popup_at_rect( byval menu as GtkMenu ptr, byval rect_window as GdkWindow ptr, byval rect as const GdkRectangle ptr, _
                                        byval rect_anchor as GdkGravity, byval menu_anchor as GdkGravity, byval trigger_event as const GdkEvent ptr )
    declare sub gtk_menu_popup_at_pointer( byval menu as GtkMenu ptr, byval trigger_event as const GdkEvent ptr )
end extern

dim shared as long gLnxMenuResult
dim shared as boolean gLnxMenuDone

private sub LnxOnMenuActivate cdecl ( byval item as GtkMenuItem ptr, byval data_ as gpointer )
    gLnxMenuResult = cast(integer, data_)
end sub

private sub LnxOnMenuDeactivate cdecl ( byval menu as GtkMenuShell ptr, byval data_ as gpointer )
    gLnxMenuDone = true
end sub

private function LnxBuildGtkMenu( byval hMenu as HMENU ) as GtkWidget ptr
    dim as GtkWidget ptr menu = gtk_menu_new()
    dim as GSList ptr radioGroup = NULL
    for i as long = 0 to hMenu->count - 1
        dim as GtkWidget ptr mi
        with hMenu->items(i)
            if (.flags and MF_SEPARATOR) then
                mi = gtk_separator_menu_item_new()
                radioGroup = NULL
            else
                ' "Text\tShortcut" -> label with accelerator text
                dim as DWSTRING t = .text, accelText
                dim as long tabPos = instr(t, !"\t")
                if tabPos then accelText = mid(t, tabPos + 1) : t = left(t, tabPos - 1)
                t = AfxStrReplace( AfxStrReplace( AfxStrReplace(t, "_", "__"), "&&", chr(1) ), "&", "_" )
                t = AfxStrReplace( t, chr(1), "&" )
                dim as string s = t.utf8
                if .radio then
                    mi = gtk_radio_menu_item_new_with_mnemonic( radioGroup, LnxZ(s) )
                    radioGroup = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(mi) )
                    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(mi), iif(.flags and MF_CHECKED, TRUE, FALSE) )
                elseif (.flags and MF_CHECKED) then
                    mi = gtk_check_menu_item_new_with_mnemonic( LnxZ(s) )
                    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(mi), TRUE )
                else
                    mi = gtk_menu_item_new_with_mnemonic( LnxZ(s) )
                end if
                if len(accelText) then
                    dim as GtkWidget ptr lbl = gtk_bin_get_child( GTK_BIN(mi) )
                    if lbl andalso GTK_IS_ACCEL_LABEL(lbl) then
                        ' show the shortcut text right aligned
                        dim as string a = accelText.utf8
                        dim as guint key
                        dim as GdkModifierType mods
                        gtk_accelerator_parse( LnxZ(a), @key, @mods )
                        if key then gtk_accel_label_set_accel( GTK_ACCEL_LABEL(lbl), key, mods )
                    end if
                end if
                if .submenu then
                    gtk_menu_item_set_submenu( GTK_MENU_ITEM(mi), LnxBuildGtkMenu(.submenu) )
                else
                    g_signal_connect( G_OBJECT(mi), "activate", G_CALLBACK(@LnxOnMenuActivate), cast(gpointer, cast(integer, .id)) )
                end if
                if (.flags and (MF_GRAYED or MF_DISABLED)) then gtk_widget_set_sensitive( mi, FALSE )
            end if
        end with
        gtk_menu_shell_append( GTK_MENU_SHELL(menu), mi )
    next
    gtk_widget_show_all( menu )
    return menu
end function

private function TrackPopupMenu( byval hMenu as HMENU, byval uFlags as UINT, byval x as long, byval y as long, _
                                 byval nReserved as long, byval hwnd as HWND, byval prcRect as const RECT ptr ) as BOOL
    if hMenu = 0 then return 0
    dim as GtkWidget ptr menu = LnxBuildGtkMenu( hMenu )
    dim as HWND top = LnxToplevelOf( hwnd )
    if top = 0 then top = GetActiveWindow()
    if top then gtk_menu_attach_to_widget( GTK_MENU(menu), top->widget, NULL )
    gLnxMenuResult = 0
    gLnxMenuDone = false
    g_signal_connect( G_OBJECT(menu), "deactivate", G_CALLBACK(@LnxOnMenuDeactivate), NULL )

    ' position the menu at the requested screen coordinates
    dim as GdkWindow ptr gw = iif(top, gtk_widget_get_window(top->client), NULL)
    if gw then
        dim as long ox, oy
        gdk_window_get_origin( gw, @ox, @oy )
        dim as GdkRectangle r
        r.x = x - ox : r.y = y - oy : r.width = 1 : r.height = 1
        dim as GdkGravity menuAnchor = GDK_GRAVITY_NORTH_WEST
        if (uFlags and TPM_RIGHTALIGN) then menuAnchor = GDK_GRAVITY_NORTH_EAST
        if (uFlags and TPM_BOTTOMALIGN) then menuAnchor = iif(uFlags and TPM_RIGHTALIGN, GDK_GRAVITY_SOUTH_EAST, GDK_GRAVITY_SOUTH_WEST)
        gtk_menu_popup_at_rect( GTK_MENU(menu), gw, @r, GDK_GRAVITY_NORTH_WEST, menuAnchor, gLnxLastInputEvent )
    else
        gtk_menu_popup_at_pointer( GTK_MENU(menu), gLnxLastInputEvent )
    end if

    ' modal loop like the Win32 TrackPopupMenu
    while gLnxMenuDone = false
        gtk_main_iteration_do( TRUE )
    wend
    ' let the "activate" signal (emitted after "deactivate") run
    while gtk_events_pending() : gtk_main_iteration_do( FALSE ) : wend
    gtk_widget_destroy( menu )

    dim as long result = gLnxMenuResult
    if (uFlags and TPM_RETURNCMD) then return result
    if result andalso (uFlags and TPM_NONOTIFY) = 0 andalso IsWindow(hwnd) then
        PostMessage( hwnd, WM_COMMAND, MAKEWPARAM(result, 0), 0 )
    end if
    return iif(result, 1, 0)
end function
#define TrackPopupMenuEx(hMenu, uFlags, x, y, hwnd, lptpm) TrackPopupMenu(hMenu, uFlags, x, y, 0, hwnd, NULL)


' ----------------------------------------------------------------------------
' Message boxes
' ----------------------------------------------------------------------------
#ifndef MB_OK
#define MB_OK              &h00000000
#endif
#ifndef MB_OKCANCEL
#define MB_OKCANCEL        &h00000001
#endif
#ifndef MB_ABORTRETRYIGNORE
#define MB_ABORTRETRYIGNORE &h00000002
#endif
#ifndef MB_YESNOCANCEL
#define MB_YESNOCANCEL     &h00000003
#endif
#ifndef MB_YESNO
#define MB_YESNO           &h00000004
#endif
#ifndef MB_RETRYCANCEL
#define MB_RETRYCANCEL     &h00000005
#endif
#ifndef MB_ICONSTOP
#define MB_ICONSTOP        MB_ICONERROR
#endif
#ifndef MB_ICONHAND
#define MB_ICONHAND        MB_ICONERROR
#endif
#ifndef MB_ICONEXCLAMATION
#define MB_ICONEXCLAMATION MB_ICONWARNING
#endif
#ifndef MB_ICONASTERISK
#define MB_ICONASTERISK    MB_ICONINFORMATION
#endif
#ifndef MB_DEFBUTTON3
#define MB_DEFBUTTON3      &h00000200
#endif
#ifndef MB_SYSTEMMODAL
#define MB_SYSTEMMODAL     &h00001000
#endif
#ifndef MB_TASKMODAL
#define MB_TASKMODAL       &h00002000
#endif
#ifndef MB_TOPMOST
#define MB_TOPMOST         &h00040000
#endif
#ifndef MB_SETFOREGROUND
#define MB_SETFOREGROUND   &h00010000
#endif

private function MessageBox( byval hwnd as HWND, byval lpText as LPCWSTR, byval lpCaption as LPCWSTR, byval uType as UINT ) as long
    dim as GtkMessageType mt = GTK_MESSAGE_OTHER
    select case (uType and &hF0)
        case MB_ICONERROR: mt = GTK_MESSAGE_ERROR
        case MB_ICONWARNING: mt = GTK_MESSAGE_WARNING
        case MB_ICONQUESTION: mt = GTK_MESSAGE_QUESTION
        case MB_ICONINFORMATION: mt = GTK_MESSAGE_INFO
    end select
    dim as HWND top = iif(hwnd, LnxToplevelOf(hwnd), GetActiveWindow())
    dim as string sText = iif(lpText, WideToUtf8(lpText), "")
    dim as string sCaption = iif(lpCaption, WideToUtf8(lpCaption), "")
    dim as GtkWidget ptr dlg = gtk_message_dialog_new( iif(top, GTK_WINDOW(top->widget), NULL), GTK_DIALOG_MODAL, mt, GTK_BUTTONS_NONE, "%s", LnxZ(sText) )
    gtk_window_set_title( GTK_WINDOW(dlg), LnxZ(sCaption) )
    dim as long defResp
    select case (uType and &hF)
        case MB_OKCANCEL
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_OK", IDOK, "_Cancel", IDCANCEL, NULL )
            defResp = iif((uType and MB_DEFBUTTON2), IDCANCEL, IDOK)
        case MB_YESNO
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_Yes", IDYES, "_No", IDNO, NULL )
            defResp = iif((uType and MB_DEFBUTTON2), IDNO, IDYES)
        case MB_YESNOCANCEL
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_Yes", IDYES, "_No", IDNO, "_Cancel", IDCANCEL, NULL )
            defResp = iif((uType and MB_DEFBUTTON3), IDCANCEL, iif((uType and MB_DEFBUTTON2), IDNO, IDYES))
        case MB_RETRYCANCEL
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_Retry", IDRETRY, "_Cancel", IDCANCEL, NULL )
            defResp = IDRETRY
        case MB_ABORTRETRYIGNORE
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_Abort", IDABORT, "_Retry", IDRETRY, "_Ignore", IDIGNORE, NULL )
            defResp = IDABORT
        case else
            gtk_dialog_add_buttons( GTK_DIALOG(dlg), "_OK", IDOK, NULL )
            defResp = IDOK
    end select
    gtk_dialog_set_default_response( GTK_DIALOG(dlg), defResp )
    dim as long r = gtk_dialog_run( GTK_DIALOG(dlg) )
    gtk_widget_destroy( dlg )
    if r < 0 then
        ' dialog closed with the window manager / ESC
        select case (uType and &hF)
            case MB_YESNO: r = IDNO
            case MB_OK: r = IDOK
            case else: r = IDCANCEL
        end select
    end if
    return r
end function
#define MessageBoxW MessageBox

private sub MessageBeep( byval uType as UINT )
    gdk_display_beep( gdk_display_get_default() )
end sub

private function AfxMsg( byref wszText as const wstring, byref wszCaption as const wstring = "Message", _
                         byval uType as UINT = MB_OK ) as long
    return MessageBox( GetActiveWindow(), @wszText, @wszCaption, uType )
end function


' ----------------------------------------------------------------------------
' Tooltips (GTK native tooltips on the tool window)
' ----------------------------------------------------------------------------
private function AfxAddTooltip( byval hwnd as HWND, byref wszText as const wstring = "", byval bBalloon as BOOL = false, _
                                byval bCentered as BOOL = false ) as HWND
    if IsWindow(hwnd) = 0 then return 0
    dim as HWND tt = LnxNewWindow( LK_TOOLTIP, 0 )
    tt->owner = hwnd
    tt->text = wszText
    dim as string s = WideToUtf8(@wszText)
    dim as GtkWidget ptr w = iif(hwnd->client, hwnd->client, iif(hwnd->inner, hwnd->inner, hwnd->widget))
    gtk_widget_set_tooltip_text( w, iif(len(s), LnxZ(s), NULL) )
    return tt
end function

private sub AfxSetTooltipText( byval hTooltip as HWND, byval hwnd as HWND, byref wszText as const wstring )
    if IsWindow(hTooltip) = 0 orelse IsWindow(hwnd) = 0 then exit sub
    hTooltip->text = wszText
    dim as string s = WideToUtf8(@wszText)
    dim as GtkWidget ptr w = iif(hwnd->client, hwnd->client, iif(hwnd->inner, hwnd->inner, hwnd->widget))
    gtk_widget_set_tooltip_text( w, iif(len(s), LnxZ(s), NULL) )
    gtk_widget_trigger_tooltip_query( w )
end sub


' ----------------------------------------------------------------------------
' CWindow
' ----------------------------------------------------------------------------
#ifndef CS_VREDRAW
#define CS_VREDRAW   &h0001
#endif
#ifndef CS_HREDRAW
#define CS_HREDRAW   &h0002
#endif
#ifndef CS_NOCLOSE
#define CS_NOCLOSE   &h0200
#endif
#ifndef CS_SAVEBITS
#define CS_SAVEBITS  &h0800
#endif

type CWindow
    private:
        m_hwnd            as HWND
        m_hFont           as HFONT
        m_hAccel          as HACCEL
        m_DPI             as long = 96
        m_rx              as single = 1
        m_ry              as single = 1
        m_wszClassName    as DWSTRING
        m_DefaultFontName as DWSTRING = "Segoe UI"
        m_DefaultFontSize as long = 9
        m_hbrBackground   as HBRUSH
        m_classStyle      as DWORD = CS_VREDRAW or CS_HREDRAW or CS_DBLCLKS
        m_rgUserData(0 to 99) as LONG_PTR
        m_rgAccelEntries(any) as ACCEL

    public:
        declare constructor( byref wszClassName as const wstring = "" )
        declare destructor()
        declare function Create( byval hParent as HWND = NULL, byref wszTitle as const wstring = "", byval lpfnWndProc as WNDPROC = NULL, _
            byval x as long = CW_USEDEFAULT, byval y as long = CW_USEDEFAULT, byval nWidth as long = CW_USEDEFAULT, byval nHeight as long = CW_USEDEFAULT, _
            byval dwStyle as DWORD = WS_OVERLAPPEDWINDOW or WS_CLIPCHILDREN or WS_CLIPSIBLINGS, _
            byval dwExStyle as DWORD = WS_EX_CONTROLPARENT or &h100 ) as HWND
        declare function AddControl( byref wszClassName as const wstring, byval hParent as HWND = NULL, byval cID as LONG_PTR = 0, _
            byref wszTitle as const wstring = "", byval x as long = 0, byval y as long = 0, byval nWidth as long = 0, _
            byval nHeight as long = 0, byval dwStyle as long = -1, byval dwExStyle as long = -1, byval lpParam as LONG_PTR = 0 ) as HWND
        declare function SetControlSubclass( byval hCtl as HWND, byval pfnSubclass as any ptr, byval uIdSubclass as UINT_PTR, byval dwRefData as any ptr ) as boolean
        declare function DoEvents( byval nCmdShow as long = 0 ) as long
        declare sub SetClientSize( byval nWidth as long, byval nHeight as long )
        declare sub Center( byval hwndTarget as HWND = NULL, byval hwndParent as HWND = NULL )
        declare function CreateFont( byref wszFaceName as const wstring, byval lPointSize as long, byval lWeight as long = 0, _
            byval bItalic as ubyte = FALSE, byval bUnderline as ubyte = FALSE, byval bStrikeOut as ubyte = FALSE, byval bCharSet as ubyte = DEFAULT_CHARSET ) as HFONT
        declare function SetFont( byref wszFaceName as const wstring, byval lPointSize as long, byval lWeight as long = 0, _
            byval bItalic as ubyte = FALSE, byval bUnderline as ubyte = FALSE, byval bStrikeOut as ubyte = FALSE, byval bCharSet as ubyte = DEFAULT_CHARSET ) as boolean
        declare function ScaleX( byval cx as single ) as single
        declare function ScaleY( byval cy as single ) as single
        declare function UnScaleX( byval cx as single ) as single
        declare function UnScaleY( byval cy as single ) as single
        declare property hWindow() as HWND
        declare property hWindow( byval hwnd as HWND )
        declare property InstanceHandle() as HINSTANCE
        declare property ClassStyle() as ULONG_PTR
        declare property ClassStyle( byval dwStyle as LONG_PTR )
        declare property Brush() as HBRUSH
        declare property Brush( byval hbr as HBRUSH )
        declare property Font() as HFONT
        declare property Font( byval hFont as HFONT )
        declare property BigIcon( byval hIcon as HICON )
        declare property SmallIcon( byval hIcon as HICON )
        declare property DPI() as single
        declare property DPI( byval newDpi as single )
        declare property rxRatio() as single
        declare property ryRatio() as single
        declare property DefaultFontName() as DWSTRING
        declare property DefaultFontName( byref wszName as const wstring )
        declare property DefaultFontSize() as long
        declare property DefaultFontSize( byval nSize as long )
        declare property UserData( byval idx as long ) as LONG_PTR
        declare property UserData( byval idx as long, byval newValue as LONG_PTR )
        declare property AccelHandle() as HACCEL
        declare property AccelHandle( byval hAccel as HACCEL )
        declare sub AddAccelerator overload ( byval fvirt as ubyte, byval wKey as WORD, byval cmd as WORD )
        declare sub AddAccelerator overload ( byval fvirt as ubyte, byref wszKey as const wstring, byval cmd as WORD )
        declare function CreateAcceleratorTable() as HACCEL
        declare sub DestroyAcceleratorTable()
        declare function GetWorkArea() as RECT
        declare function ClientWidth() as long
        declare function ClientHeight() as long
end type

private constructor CWindow( byref wszClassName as const wstring = "" )
    m_wszClassName = wszClassName
    m_hFont = this.CreateFont( m_DefaultFontName, m_DefaultFontSize )
    m_hbrBackground = GetSysColorBrush( COLOR_BTNFACE )
end constructor

private destructor CWindow()
    if m_hAccel then .DestroyAcceleratorTable( m_hAccel )
end destructor

private function CWindow.Create( byval hParent as HWND = NULL, byref wszTitle as const wstring = "", byval lpfnWndProc as WNDPROC = NULL, _
            byval x as long = CW_USEDEFAULT, byval y as long = CW_USEDEFAULT, byval nWidth as long = CW_USEDEFAULT, byval nHeight as long = CW_USEDEFAULT, _
            byval dwStyle as DWORD = WS_OVERLAPPEDWINDOW or WS_CLIPCHILDREN or WS_CLIPSIBLINGS, _
            byval dwExStyle as DWORD = WS_EX_CONTROLPARENT or &h100 ) as HWND
    if m_hwnd then return m_hwnd
    dim as LNX_KIND kind = iif((dwStyle and WS_CHILD), LK_CUSTOM, LK_TOPLEVEL)
    ' The window procedure gets WM_CREATE during creation; make AfxCWindowPtr work there.
    dim as HWND hwnd = LnxCreateWindow( kind, m_wszClassName, wszTitle, dwStyle, dwExStyle, _
                        iif(x = CW_USEDEFAULT, x, x * m_rx), iif(y = CW_USEDEFAULT, y, y * m_ry), _
                        iif(nWidth = CW_USEDEFAULT, nWidth, nWidth * m_rx), iif(nHeight = CW_USEDEFAULT, nHeight, nHeight * m_ry), _
                        hParent, 0, lpfnWndProc, NULL, @this, m_classStyle, m_hbrBackground, m_hFont )
    m_hwnd = hwnd
    return hwnd
end function

private function CWindow.AddControl( byref wszClassName as const wstring, byval hParent as HWND = NULL, byval cID as LONG_PTR = 0, _
            byref wszTitle as const wstring = "", byval x as long = 0, byval y as long = 0, byval nWidth as long = 0, _
            byval nHeight as long = 0, byval dwStyle as long = -1, byval dwExStyle as long = -1, byval lpParam as LONG_PTR = 0 ) as HWND
    if hParent = NULL then hParent = m_hwnd
    if dwStyle = -1 then dwStyle = WS_CHILD or WS_VISIBLE or WS_TABSTOP
    if dwExStyle = -1 then dwExStyle = 0
    select case ucase(wszClassName)
        case "CHECKBOX": dwStyle or= BS_AUTOCHECKBOX
        case "RADIOBUTTON", "OPTION": dwStyle or= BS_AUTORADIOBUTTON
        case "GROUPBOX": dwStyle or= BS_GROUPBOX
    end select
    dim as HWND hCtl = LnxCreateControl( wszClassName, hParent, cID, wszTitle, x * m_rx, y * m_ry, nWidth * m_rx, nHeight * m_ry, dwStyle, dwExStyle )
    if hCtl then
        hCtl->pCWindow = @this
        if hCtl->kind <> LK_CUSTOM andalso m_hFont andalso hCtl->kind <> LK_SCINTILLA then SendMessage( hCtl, WM_SETFONT, cast(WPARAM, m_hFont), 0 )
    end if
    return hCtl
end function

private function CWindow.SetControlSubclass( byval hCtl as HWND, byval pfnSubclass as any ptr, byval uIdSubclass as UINT_PTR, byval dwRefData as any ptr ) as boolean
    return SetWindowSubclass( hCtl, cast(SUBCLASSPROC, pfnSubclass), uIdSubclass, cast(DWORD_PTR, dwRefData) ) <> 0
end function

private function CWindow.DoEvents( byval nCmdShow as long = 0 ) as long
    dim as MSG uMsg
    if m_hwnd = NULL then exit function
    ShowWindow( m_hwnd, iif(nCmdShow = 0, SW_SHOW, nCmdShow) )
    UpdateWindow( m_hwnd )
    while GetMessage( @uMsg, NULL, 0, 0 )
        if m_hAccel = NULL orelse TranslateAccelerator( m_hwnd, m_hAccel, @uMsg ) = 0 then
            if IsDialogMessage( m_hwnd, @uMsg ) = 0 then
                TranslateMessage( @uMsg )
                DispatchMessage( @uMsg )
            end if
        end if
    wend
    return uMsg.wParam
end function

private sub CWindow.SetClientSize( byval nWidth as long, byval nHeight as long )
    if m_hwnd = 0 then exit sub
    nWidth *= m_rx : nHeight *= m_ry
    if m_hwnd->kind = LK_TOPLEVEL then
        gtk_window_set_default_size( GTK_WINDOW(m_hwnd->widget), nWidth, nHeight )
        gtk_window_resize( GTK_WINDOW(m_hwnd->widget), nWidth, nHeight )
        gtk_widget_set_size_request( m_hwnd->client, -1, -1 )
        m_hwnd->w = nWidth : m_hwnd->h = nHeight
        if (m_hwnd->style and WS_THICKFRAME) = 0 then
            ' fixed size dialogs
            gtk_widget_set_size_request( m_hwnd->client, nWidth, nHeight )
        end if
    else
        SetWindowPos( m_hwnd, 0, 0, 0, nWidth, nHeight, SWP_NOMOVE or SWP_NOZORDER )
    end if
end sub

private sub CWindow.Center( byval hwndTarget as HWND = NULL, byval hwndParent as HWND = NULL )
    if hwndTarget = NULL then hwndTarget = m_hwnd
    if IsWindow(hwndTarget) = 0 orelse hwndTarget->kind <> LK_TOPLEVEL then exit sub
    if hwndParent = NULL then hwndParent = hwndTarget->owner
    dim as HWND parentTop = LnxToplevelOf( hwndParent )
    if parentTop then
        gtk_window_set_transient_for( GTK_WINDOW(hwndTarget->widget), GTK_WINDOW(parentTop->widget) )
        gtk_window_set_position( GTK_WINDOW(hwndTarget->widget), GTK_WIN_POS_CENTER_ON_PARENT )
        ' explicit move for window managers ignoring the hint
        dim as RECT rcP
        GetWindowRect( parentTop, @rcP )
        dim as long w, h
        gtk_window_get_default_size( GTK_WINDOW(hwndTarget->widget), @w, @h )
        if w <= 0 then gtk_window_get_size( GTK_WINDOW(hwndTarget->widget), @w, @h )
        gtk_window_move( GTK_WINDOW(hwndTarget->widget), rcP.left + ((rcP.right - rcP.left) - w) \ 2, rcP.top + ((rcP.bottom - rcP.top) - h) \ 2 )
    else
        gtk_window_set_position( GTK_WINDOW(hwndTarget->widget), GTK_WIN_POS_CENTER )
    end if
end sub

private function CWindow.CreateFont( byref wszFaceName as const wstring, byval lPointSize as long, byval lWeight as long = 0, _
            byval bItalic as ubyte = FALSE, byval bUnderline as ubyte = FALSE, byval bStrikeOut as ubyte = FALSE, byval bCharSet as ubyte = DEFAULT_CHARSET ) as HFONT
    ' Win32: lfHeight = -MulDiv(pointSize, DPI, 72)
    dim as long nHeight = -MulDiv( lPointSize, m_DPI, 72 )
    return LnxCreateFontObj( nHeight, lWeight, bItalic <> 0, bUnderline <> 0, wszFaceName )
end function

private function CWindow.SetFont( byref wszFaceName as const wstring, byval lPointSize as long, byval lWeight as long = 0, _
            byval bItalic as ubyte = FALSE, byval bUnderline as ubyte = FALSE, byval bStrikeOut as ubyte = FALSE, byval bCharSet as ubyte = DEFAULT_CHARSET ) as boolean
    m_hFont = this.CreateFont( wszFaceName, lPointSize, lWeight, bItalic, bUnderline, bStrikeOut, bCharSet )
    if m_hwnd then m_hwnd->hFont = m_hFont
    return true
end function

private function CWindow.ScaleX( byval cx as single ) as single
    return cx * m_rx
end function
private function CWindow.ScaleY( byval cy as single ) as single
    return cy * m_ry
end function
private function CWindow.UnScaleX( byval cx as single ) as single
    return cx / m_rx
end function
private function CWindow.UnScaleY( byval cy as single ) as single
    return cy / m_ry
end function
private property CWindow.hWindow() as HWND
    return m_hwnd
end property
private property CWindow.hWindow( byval hwnd as HWND )
    m_hwnd = hwnd
end property
private property CWindow.InstanceHandle() as HINSTANCE
    return 0
end property
private property CWindow.ClassStyle() as ULONG_PTR
    return m_classStyle
end property
private property CWindow.ClassStyle( byval dwStyle as LONG_PTR )
    m_classStyle = dwStyle
    if m_hwnd then m_hwnd->classStyle = dwStyle
end property
private property CWindow.Brush() as HBRUSH
    return m_hbrBackground
end property
private property CWindow.Brush( byval hbr as HBRUSH )
    m_hbrBackground = hbr
    if m_hwnd then m_hwnd->hbrBackground = hbr : InvalidateRect( m_hwnd, NULL, 1 )
end property
private property CWindow.Font() as HFONT
    return m_hFont
end property
private property CWindow.Font( byval hFont as HFONT )
    m_hFont = hFont
    if m_hwnd then m_hwnd->hFont = hFont
end property
private property CWindow.BigIcon( byval hIcon as HICON )
    if m_hwnd andalso hIcon andalso m_hwnd->kind = LK_TOPLEVEL then gtk_window_set_icon( GTK_WINDOW(m_hwnd->widget), hIcon )
end property
private property CWindow.SmallIcon( byval hIcon as HICON )
end property
private property CWindow.DPI() as single
    return m_DPI
end property
private property CWindow.DPI( byval newDpi as single )
    m_DPI = newDpi
end property
private property CWindow.rxRatio() as single
    return m_rx
end property
private property CWindow.ryRatio() as single
    return m_ry
end property
private property CWindow.DefaultFontName() as DWSTRING
    return m_DefaultFontName
end property
private property CWindow.DefaultFontName( byref wszName as const wstring )
    m_DefaultFontName = wszName
end property
private property CWindow.DefaultFontSize() as long
    return m_DefaultFontSize
end property
private property CWindow.DefaultFontSize( byval nSize as long )
    m_DefaultFontSize = nSize
end property
private property CWindow.UserData( byval idx as long ) as LONG_PTR
    if idx < 0 orelse idx > 99 then return 0
    return m_rgUserData(idx)
end property
private property CWindow.UserData( byval idx as long, byval newValue as LONG_PTR )
    if idx < 0 orelse idx > 99 then exit property
    m_rgUserData(idx) = newValue
end property
private property CWindow.AccelHandle() as HACCEL
    return m_hAccel
end property
private property CWindow.AccelHandle( byval hAccel as HACCEL )
    if m_hAccel then .DestroyAcceleratorTable( m_hAccel )
    m_hAccel = hAccel
end property
private sub CWindow.AddAccelerator( byval fvirt as ubyte, byval wKey as WORD, byval cmd as WORD )
    dim as long n = ubound(m_rgAccelEntries) + 1
    redim preserve m_rgAccelEntries(n)
    m_rgAccelEntries(n).fVirt = fvirt
    m_rgAccelEntries(n).key = wKey
    m_rgAccelEntries(n).cmd = cmd
end sub
private sub CWindow.AddAccelerator( byval fvirt as ubyte, byref wszKey as const wstring, byval cmd as WORD )
    this.AddAccelerator( fvirt, cast(WORD, valint(wszKey)), cmd )
end sub
private function CWindow.CreateAcceleratorTable() as HACCEL
    dim as long n = ubound(m_rgAccelEntries) + 1
    if n = 0 then return NULL
    if m_hAccel then .DestroyAcceleratorTable( m_hAccel )
    m_hAccel = .CreateAcceleratorTable( @m_rgAccelEntries(0), n )
    erase m_rgAccelEntries
    return m_hAccel
end function
private sub CWindow.DestroyAcceleratorTable()
    if m_hAccel then .DestroyAcceleratorTable( m_hAccel )
    m_hAccel = NULL
end sub
private function CWindow.GetWorkArea() as RECT
    dim as RECT rc
    dim as GdkRectangle r
    gdk_screen_get_monitor_workarea( gdk_screen_get_default(), 0, @r )
    SetRect( @rc, r.x, r.y, r.x + r.width, r.y + r.height )
    return rc
end function
private function CWindow.ClientWidth() as long
    dim as RECT rc : GetClientRect( m_hwnd, @rc ) : return rc.right
end function
private function CWindow.ClientHeight() as long
    dim as RECT rc : GetClientRect( m_hwnd, @rc ) : return rc.bottom
end function


' ----------------------------------------------------------------------------
' Afx window helpers
' ----------------------------------------------------------------------------
private function AfxCWindowPtr overload ( byval hwnd as HWND ) as CWindow ptr
    while hwnd andalso IsWindow(hwnd)
        if hwnd->pCWindow andalso (hwnd->kind = LK_TOPLEVEL orelse hwnd->kind = LK_CUSTOM) then return hwnd->pCWindow
        hwnd = hwnd->parent
    wend
    return 0
end function

private function AfxCWindowOwnerPtr( byval hwnd as HWND ) as CWindow ptr
    if IsWindow(hwnd) = 0 then return 0
    return AfxCWindowPtr( iif(hwnd->owner, hwnd->owner, hwnd->parent) )
end function

private function AfxGetFormHandle( byval hwnd as HWND ) as HWND
    return LnxToplevelOf( hwnd )
end function

private function AfxGetWindowText( byval hwnd as HWND ) as DWSTRING
    if IsWindow(hwnd) = 0 then return DWSTRING()
    if hwnd->kind = LK_TOOLTIP then return hwnd->text
    dim as long n = GetWindowTextLength( hwnd )
    dim as DWSTRING dws
    dws.Reserve( n + 1 )
    GetWindowText( hwnd, dws.vptr, n + 1 )
    dws.m_BufferLen = n
    return dws
end function

private function AfxGetWindowTextLength( byval hwnd as HWND ) as long
    return GetWindowTextLength( hwnd )
end function

private function AfxSetWindowText overload ( byval hwnd as HWND, byref wszText as const wstring ) as BOOL
    return SetWindowText( hwnd, @wszText )
end function

private function AfxGetListBoxText( byval hListBox as HWND, byval nIndex as long ) as DWSTRING
    dim as long n = ListBox_GetTextLen( hListBox, nIndex )
    if n <= 0 then return DWSTRING()
    dim as DWSTRING dws
    dws.Reserve( n + 1 )
    ListBox_GetText( hListBox, nIndex, dws.vptr )
    dws.m_BufferLen = n
    return dws
end function

private function AfxGetComboBoxText( byval hComboBox as HWND, byval nIndex as long ) as DWSTRING
    dim as long n = ComboBox_GetLBTextLen( hComboBox, nIndex )
    if n <= 0 then return DWSTRING()
    dim as DWSTRING dws
    dws.Reserve( n + 1 )
    ComboBox_GetLBText( hComboBox, nIndex, dws.vptr )
    dws.m_BufferLen = n
    return dws
end function

private function AfxSetWindowFont( byval hwnd as HWND, byval hFont as HFONT, byval fRedraw as BOOL = TRUE ) as LRESULT
    return SendMessage( hwnd, WM_SETFONT, cast(WPARAM, hFont), fRedraw )
end function

private function AfxGetWindowFont( byval hwnd as HWND ) as HFONT
    return cast(HFONT, SendMessage( hwnd, WM_GETFONT, 0, 0 ))
end function

private function AfxGetWindowWidth( byval hwnd as HWND ) as long
    dim as RECT rc : GetWindowRect( hwnd, @rc ) : return rc.right - rc.left
end function

private function AfxGetWindowHeight( byval hwnd as HWND ) as long
    dim as RECT rc : GetWindowRect( hwnd, @rc ) : return rc.bottom - rc.top
end function

private function AfxGetWindowRect( byval hwnd as HWND ) as RECT
    dim as RECT rc : GetWindowRect( hwnd, @rc ) : return rc
end function

private function AfxGetWindowClientRect( byval hwnd as HWND ) as RECT
    dim as RECT rc : GetClientRect( hwnd, @rc ) : return rc
end function

private function AfxGetWindowClientWidth( byval hwnd as HWND ) as long
    dim as RECT rc : GetClientRect( hwnd, @rc ) : return rc.right
end function

private function AfxGetWindowClientHeight( byval hwnd as HWND ) as long
    dim as RECT rc : GetClientRect( hwnd, @rc ) : return rc.bottom
end function

private function AfxSetWindowSize( byval hwnd as HWND, byval nWidth as long, byval nHeight as long ) as BOOL
    return SetWindowPos( hwnd, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER or SWP_NOMOVE or SWP_NOACTIVATE )
end function

private function AfxSetWindowLocation( byval hwnd as HWND, byval nLeft as long, byval nTop as long ) as BOOL
    return SetWindowPos( hwnd, NULL, nLeft, nTop, 0, 0, SWP_NOZORDER or SWP_NOSIZE or SWP_NOACTIVATE )
end function

private function AfxScaleX( byval cx as single ) as single
    return cx
end function
private function AfxScaleY( byval cy as single ) as single
    return cy
end function

private function AfxGetWorkAreaHeight() as long
    dim as GdkRectangle r
    gdk_screen_get_monitor_workarea( gdk_screen_get_default(), 0, @r )
    return r.height
end function

' Ensure a top level window is visible on one of the monitors
private sub AfxForceVisibleDisplay( byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then exit sub
    dim as long x, y, w, h
    gtk_window_get_position( GTK_WINDOW(hwnd->widget), @x, @y )
    gtk_window_get_size( GTK_WINDOW(hwnd->widget), @w, @h )
    dim as GdkDisplay ptr disp = gdk_display_get_default()
    dim as GdkRectangle r
    gdk_screen_get_monitor_workarea( gdk_screen_get_default(), 0, @r )
    dim as boolean bVisible = false
    dim as long nMon = gdk_screen_get_n_monitors( gdk_screen_get_default() )
    for i as long = 0 to nMon - 1
        dim as GdkRectangle m
        gdk_screen_get_monitor_workarea( gdk_screen_get_default(), i, @m )
        if x + 50 >= m.x andalso x + 50 < m.x + m.width andalso y + 20 >= m.y andalso y < m.y + m.height then bVisible = true : exit for
    next
    if bVisible = false then gtk_window_move( GTK_WINDOW(hwnd->widget), r.x + 50, r.y + 50 )
end sub

#ifndef SPI_GETWORKAREA_
#define SPI_GETWORKAREA_ &h0030
#endif
private function SystemParametersInfo( byval uiAction as UINT, byval uiParam as UINT, byval pvParam as any ptr, byval fWinIni as UINT ) as BOOL
    if uiAction = SPI_GETWORKAREA andalso pvParam then
        dim as GdkRectangle r
        gdk_screen_get_monitor_workarea( gdk_screen_get_default(), 0, @r )
        SetRect( cast(RECT ptr, pvParam), r.x, r.y, r.x + r.width, r.y + r.height )
        return 1
    end if
    return 0
end function
#define SystemParametersInfoW SystemParametersInfo

private function SetLayeredWindowAttributes( byval hwnd as HWND, byval crKey as COLORREF, byval bAlpha as ubyte, byval dwFlags as DWORD ) as BOOL
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then return 0
    if (dwFlags and LWA_ALPHA) then gtk_widget_set_opacity( hwnd->widget, bAlpha / 255.0 )
    return 1
end function

private function GetModuleHandle( byval lpModuleName as LPCWSTR ) as HINSTANCE
    return 0
end function
#define GetModuleHandleW GetModuleHandle

private function FindWindow( byval lpClassName as LPCWSTR, byval lpWindowName as LPCWSTR ) as HWND
    ' Only windows of this process can be found on Linux
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND h = gLnxWindows(i)
        if h->kind = LK_TOPLEVEL andalso lpClassName andalso h->classname = *lpClassName then return h
    next
    return 0
end function
#define FindWindowW FindWindow

private function DwmSetWindowAttribute( byval hwnd as HWND, byval dwAttribute as DWORD, byval pvAttribute as const any ptr, byval cbAttribute as DWORD ) as HRESULT
    ' DWMWA_USE_IMMERSIVE_DARK_MODE: ask GTK for a dark title bar
    if dwAttribute = DWMWA_USE_IMMERSIVE_DARK_MODE andalso pvAttribute then
        dim as gboolean bDark = iif(*cast(const long ptr, pvAttribute), 1, 0)
        g_object_set( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", bDark, NULL )
    end if
    return S_OK
end function

' Images / icons: only cursors are needed (icons come from the application icon file)
private function LoadImage( byval hInst as HINSTANCE, byval name_ as LPCWSTR, byval uType as UINT, _
                            byval cx as long, byval cy as long, byval fuLoad as UINT ) as HANDLE
    if uType = IMAGE_CURSOR then return LnxCursorFromId( cast(integer, name_) )
    if uType = IMAGE_ICON then
        dim as string s = WideToUtf8( AfxGetExePathName.vptr ) & "bin/tiko.png"
        if g_file_test( LnxZ(s), G_FILE_TEST_EXISTS ) then return gdk_pixbuf_new_from_file( LnxZ(s), NULL )
    end if
    return 0
end function
#define LoadImageW LoadImage
#define MAKEINTRESOURCEW(i) cast(LPCWSTR, cast(ULONG_PTR, cast(WORD, (i))))
#define MAKEINTRESOURCE(i)  MAKEINTRESOURCEW(i)

' Drag & drop of files (WM_DROPFILES)
private function DragQueryFile( byval hDrop as HDROP, byval iFile as UINT, byval lpszFile as LPWSTR, byval cch as UINT ) as UINT
    dim as DWSTRING ptr files = hDrop
    if files = 0 then return 0
    dim as long n = valint( files[0] )
    if iFile = &hFFFFFFFF then return n
    if iFile >= n then return 0
    dim as DWSTRING ptr f = @files[iFile + 1]
    if lpszFile = 0 then return len(*f)
    dim as long l = len(*f)
    if l > cch - 1 then l = cch - 1
    memcpy( lpszFile, f->vptr, l * sizeof(wstring) )
    cast(ulong ptr, lpszFile)[l] = 0
    return l
end function
#define DragQueryFileW DragQueryFile
private sub DragFinish( byval hDrop as HDROP )
end sub
private sub DragAcceptFiles( byval hwnd as HWND, byval fAccept as BOOL )
end sub
