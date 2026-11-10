'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Win32 controls on GTK3.
'   BUTTON / CHECKBOX / RADIOBUTTON / GROUPBOX / LABEL / TEXTBOX / COMBOBOX -> native GTK widgets
'   LISTBOX  -> drawn by this layer (supports LBS_OWNERDRAWFIXED / LBS_NODATA / multi select)
'   LISTVIEW / TREEVIEW -> GtkTreeView,  RICHEDIT -> GtkTextView
' All of them answer the Win32 control messages (LB_*, CB_*, BM_*, EM_*, LVM_*, TVM_*) and
' notify their parent with WM_COMMAND / WM_NOTIFY / WM_DRAWITEM / WM_MEASUREITEM.

#pragma once


#ifndef LBS_SORT
    #define LBS_SORT &h0002
#endif
#ifndef SS_NOPREFIX
    #define SS_NOPREFIX &h0080
#endif
#ifndef ES_PASSWORD
    #define ES_PASSWORD &h0020
#endif
#ifndef ES_CENTER
    #define ES_CENTER &h0001
#endif
#ifndef ES_RIGHT
    #define ES_RIGHT &h0002
#endif
#ifndef ES_UPPERCASE
    #define ES_UPPERCASE &h0008
#endif
#ifndef LB_OKAY
    #define LB_OKAY 0
#endif
#ifndef LB_ERR
    #define LB_ERR (-1)
#endif
#ifndef LB_ADDSTRING
    #define LB_ADDSTRING &h0180
#endif
#ifndef LB_INSERTSTRING
    #define LB_INSERTSTRING &h0181
#endif
#ifndef LB_DELETESTRING
    #define LB_DELETESTRING &h0182
#endif
#ifndef LB_SELITEMRANGEEX
    #define LB_SELITEMRANGEEX &h0183
#endif
#ifndef LB_RESETCONTENT
    #define LB_RESETCONTENT &h0184
#endif
#ifndef LB_SETSEL
    #define LB_SETSEL &h0185
#endif
#ifndef LB_SETCURSEL
    #define LB_SETCURSEL &h0186
#endif
#ifndef LB_GETSEL
    #define LB_GETSEL &h0187
#endif
#ifndef LB_GETCURSEL
    #define LB_GETCURSEL &h0188
#endif
#ifndef LB_GETTEXT
    #define LB_GETTEXT &h0189
#endif
#ifndef LB_GETTEXTLEN
    #define LB_GETTEXTLEN &h018A
#endif
#ifndef LB_GETCOUNT
    #define LB_GETCOUNT &h018B
#endif
#ifndef LB_SELECTSTRING
    #define LB_SELECTSTRING &h018C
#endif
#ifndef LB_GETTOPINDEX
    #define LB_GETTOPINDEX &h018E
#endif
#ifndef LB_FINDSTRING
    #define LB_FINDSTRING &h018F
#endif
#ifndef LB_GETSELCOUNT
    #define LB_GETSELCOUNT &h0190
#endif
#ifndef LB_GETSELITEMS
    #define LB_GETSELITEMS &h0191
#endif
#ifndef LB_SETTOPINDEX
    #define LB_SETTOPINDEX &h0197
#endif
#ifndef LB_GETITEMRECT
    #define LB_GETITEMRECT &h0198
#endif
#ifndef LB_GETITEMDATA
    #define LB_GETITEMDATA &h0199
#endif
#ifndef LB_SETITEMDATA
    #define LB_SETITEMDATA &h019A
#endif
#ifndef LB_SELITEMRANGE
    #define LB_SELITEMRANGE &h019B
#endif
#ifndef LB_SETANCHORINDEX
    #define LB_SETANCHORINDEX &h019C
#endif
#ifndef LB_GETANCHORINDEX
    #define LB_GETANCHORINDEX &h019D
#endif
#ifndef LB_SETCARETINDEX
    #define LB_SETCARETINDEX &h019E
#endif
#ifndef LB_GETCARETINDEX
    #define LB_GETCARETINDEX &h019F
#endif
#ifndef LB_SETITEMHEIGHT
    #define LB_SETITEMHEIGHT &h01A0
#endif
#ifndef LB_GETITEMHEIGHT
    #define LB_GETITEMHEIGHT &h01A1
#endif
#ifndef LB_FINDSTRINGEXACT
    #define LB_FINDSTRINGEXACT &h01A2
#endif
#ifndef LB_SETCOUNT
    #define LB_SETCOUNT &h01A7
#endif
#ifndef LB_ITEMFROMPOINT
    #define LB_ITEMFROMPOINT &h01A9
#endif
#ifndef LBN_SELCHANGE
    #define LBN_SELCHANGE 1
#endif
#ifndef LBN_DBLCLK
    #define LBN_DBLCLK 2
#endif
#ifndef LBN_SETFOCUS
    #define LBN_SETFOCUS 4
#endif
#ifndef LBN_KILLFOCUS
    #define LBN_KILLFOCUS 5
#endif
#ifndef CB_ERR
    #define CB_ERR (-1)
#endif
#ifndef CB_GETEDITSEL
    #define CB_GETEDITSEL &h0140
#endif
#ifndef CB_LIMITTEXT
    #define CB_LIMITTEXT &h0141
#endif
#ifndef CB_SETEDITSEL
    #define CB_SETEDITSEL &h0142
#endif
#ifndef CB_ADDSTRING
    #define CB_ADDSTRING &h0143
#endif
#ifndef CB_DELETESTRING
    #define CB_DELETESTRING &h0144
#endif
#ifndef CB_GETCOUNT
    #define CB_GETCOUNT &h0146
#endif
#ifndef CB_GETCURSEL
    #define CB_GETCURSEL &h0147
#endif
#ifndef CB_GETLBTEXT
    #define CB_GETLBTEXT &h0148
#endif
#ifndef CB_GETLBTEXTLEN
    #define CB_GETLBTEXTLEN &h0149
#endif
#ifndef CB_INSERTSTRING
    #define CB_INSERTSTRING &h014A
#endif
#ifndef CB_RESETCONTENT
    #define CB_RESETCONTENT &h014B
#endif
#ifndef CB_FINDSTRING
    #define CB_FINDSTRING &h014C
#endif
#ifndef CB_SELECTSTRING
    #define CB_SELECTSTRING &h014D
#endif
#ifndef CB_SETCURSEL
    #define CB_SETCURSEL &h014E
#endif
#ifndef CB_SHOWDROPDOWN
    #define CB_SHOWDROPDOWN &h014F
#endif
#ifndef CB_GETITEMDATA
    #define CB_GETITEMDATA &h0150
#endif
#ifndef CB_SETITEMDATA
    #define CB_SETITEMDATA &h0151
#endif
#ifndef CB_SETITEMHEIGHT
    #define CB_SETITEMHEIGHT &h0153
#endif
#ifndef CB_GETITEMHEIGHT
    #define CB_GETITEMHEIGHT &h0154
#endif
#ifndef CB_FINDSTRINGEXACT
    #define CB_FINDSTRINGEXACT &h0158
#endif
#ifndef CB_GETDROPPEDSTATE
    #define CB_GETDROPPEDSTATE &h0157
#endif
#ifndef CB_SETDROPPEDWIDTH
    #define CB_SETDROPPEDWIDTH &h0160
#endif
#ifndef CBN_SELCHANGE
    #define CBN_SELCHANGE 1
#endif
#ifndef CBN_DBLCLK
    #define CBN_DBLCLK 2
#endif
#ifndef CBN_SETFOCUS
    #define CBN_SETFOCUS 3
#endif
#ifndef CBN_KILLFOCUS
    #define CBN_KILLFOCUS 4
#endif
#ifndef CBN_EDITCHANGE
    #define CBN_EDITCHANGE 5
#endif
#ifndef CBN_EDITUPDATE
    #define CBN_EDITUPDATE 6
#endif
#ifndef CBN_DROPDOWN
    #define CBN_DROPDOWN 7
#endif
#ifndef CBN_CLOSEUP
    #define CBN_CLOSEUP 8
#endif
#ifndef CBN_SELENDOK
    #define CBN_SELENDOK 9
#endif
#ifndef BM_GETCHECK
    #define BM_GETCHECK &h00F0
#endif
#ifndef BM_SETCHECK
    #define BM_SETCHECK &h00F1
#endif
#ifndef BM_GETSTATE
    #define BM_GETSTATE &h00F2
#endif
#ifndef BM_SETSTATE
    #define BM_SETSTATE &h00F3
#endif
#ifndef BM_SETSTYLE
    #define BM_SETSTYLE &h00F4
#endif
#ifndef BM_CLICK
    #define BM_CLICK &h00F5
#endif
#ifndef BN_CLICKED
    #define BN_CLICKED 0
#endif
#ifndef BN_SETFOCUS
    #define BN_SETFOCUS 6
#endif
#ifndef BN_KILLFOCUS
    #define BN_KILLFOCUS 7
#endif
#ifndef BST_INDETERMINATE
    #define BST_INDETERMINATE 2
#endif
#ifndef STN_CLICKED
    #define STN_CLICKED 0
#endif
#ifndef STN_DBLCLK
    #define STN_DBLCLK 1
#endif
#ifndef EM_GETSEL
    #define EM_GETSEL &h00B0
#endif
#ifndef EM_SETSEL
    #define EM_SETSEL &h00B1
#endif
#ifndef EM_GETRECT
    #define EM_GETRECT &h00B2
#endif
#ifndef EM_SETRECT
    #define EM_SETRECT &h00B3
#endif
#ifndef EM_SCROLL
    #define EM_SCROLL &h00B5
#endif
#ifndef EM_LINESCROLL
    #define EM_LINESCROLL &h00B6
#endif
#ifndef EM_SCROLLCARET
    #define EM_SCROLLCARET &h00B7
#endif
#ifndef EM_GETMODIFY
    #define EM_GETMODIFY &h00B8
#endif
#ifndef EM_SETMODIFY
    #define EM_SETMODIFY &h00B9
#endif
#ifndef EM_GETLINECOUNT
    #define EM_GETLINECOUNT &h00BA
#endif
#ifndef EM_LINEINDEX
    #define EM_LINEINDEX &h00BB
#endif
#ifndef EM_LINELENGTH
    #define EM_LINELENGTH &h00C1
#endif
#ifndef EM_REPLACESEL
    #define EM_REPLACESEL &h00C2
#endif
#ifndef EM_GETLINE
    #define EM_GETLINE &h00C4
#endif
#ifndef EM_LIMITTEXT
    #define EM_LIMITTEXT &h00C5
#endif
#ifndef EM_CANUNDO
    #define EM_CANUNDO &h00C6
#endif
#ifndef EM_UNDO
    #define EM_UNDO &h00C7
#endif
#ifndef EM_LINEFROMCHAR
    #define EM_LINEFROMCHAR &h00C9
#endif
#ifndef EM_GETFIRSTVISIBLELINE
    #define EM_GETFIRSTVISIBLELINE &h00CE
#endif
#ifndef EM_SETREADONLY
    #define EM_SETREADONLY &h00CF
#endif
#ifndef EM_SETMARGINS
    #define EM_SETMARGINS &h00D3
#endif
#ifndef EM_SETLIMITTEXT
    #define EM_SETLIMITTEXT &h00C5
#endif
#ifndef EM_CANPASTE
    #define EM_CANPASTE (WM_USER + 50)
#endif
#ifndef EM_EXGETSEL
    #define EM_EXGETSEL (WM_USER + 52)
#endif
#ifndef EM_EXSETSEL
    #define EM_EXSETSEL (WM_USER + 55)
#endif
#ifndef EM_GETSELTEXT
    #define EM_GETSELTEXT (WM_USER + 62)
#endif
#ifndef EM_SETZOOM
    #define EM_SETZOOM (WM_USER + 225)
#endif
#ifndef EM_SETTYPOGRAPHYOPTIONS
    #define EM_SETTYPOGRAPHYOPTIONS (WM_USER + 202)
#endif
#ifndef EM_SETCUEBANNER
    #define EM_SETCUEBANNER &h1501
#endif
#ifndef EM_GETCUEBANNER
    #define EM_GETCUEBANNER &h1502
#endif
#ifndef EN_SETFOCUS
    #define EN_SETFOCUS &h0100
#endif
#ifndef EN_KILLFOCUS
    #define EN_KILLFOCUS &h0200
#endif
#ifndef EN_CHANGE
    #define EN_CHANGE &h0300
#endif
#ifndef EN_UPDATE
    #define EN_UPDATE &h0400
#endif
#ifndef WM_CLEAR
    #define WM_CLEAR &h0303
#endif
#ifndef NM_FIRST
    #define NM_FIRST 0
#endif
#ifndef NM_CLICK
    #define NM_CLICK (culng(NM_FIRST - 2))
#endif
#ifndef NM_DBLCLK
    #define NM_DBLCLK (culng(NM_FIRST - 3))
#endif
#ifndef NM_RETURN
    #define NM_RETURN (culng(NM_FIRST - 4))
#endif
#ifndef NM_RCLICK
    #define NM_RCLICK (culng(NM_FIRST - 5))
#endif
#ifndef NM_SETFOCUS
    #define NM_SETFOCUS (culng(NM_FIRST - 7))
#endif
' RichEdit character formatting
#ifndef CFM_BOLD
const CFM_BOLD      = &h00000001
const CFM_ITALIC    = &h00000002
const CFM_UNDERLINE = &h00000004
const CFM_SIZE      = &h80000000
const CFM_FACE      = &h20000000
const CFE_BOLD      = &h0001
const CFE_ITALIC    = &h0002
const CFE_AUTOCOLOR = &h40000000
#endif
type CHARFORMATW
    cbSize          as UINT
    dwMask          as DWORD
    dwEffects       as DWORD
    yHeight         as long
    yOffset         as long
    crTextColor     as COLORREF
    bCharSet        as ubyte
    bPitchAndFamily as ubyte
    szFaceName      as wstring * 32
end type
type CHARFORMAT as CHARFORMATW

' ----------------------------------------------------------------------------
' Compact Windows-like styling for native controls (dialog layouts are pixel based).
' ----------------------------------------------------------------------------
private sub LnxInstallControlCss()
    static as boolean done
    if done then exit sub
    done = true
    dim as string css = _
        ".lnxctl { font-family: 'Segoe UI', 'Selawik', 'Noto Sans', 'DejaVu Sans', sans-serif; font-size: 9pt; }" & _
        "entry.lnxctl { min-height: 0; padding: 0px 4px; border-radius: 0; border: 1px solid #7a7a7a; background: #ffffff; color: #000000; box-shadow: none; }" & _
        "entry.lnxctl:focus { border-color: #0078d7; }" & _
        "entry.lnxctl:disabled { background: #f0f0f0; color: #6d6d6d; border-color: #cccccc; }" & _
        "button.lnxctl { min-height: 0; min-width: 0; padding: 0px 6px; border-radius: 0; border: 1px solid #adadad; background: #e1e1e1; color: #000000; box-shadow: none; text-shadow: none; -gtk-icon-shadow: none; }" & _
        "button.lnxctl:hover { background: #e5f1fb; border-color: #0078d7; }" & _
        "button.lnxctl:active { background: #cce4f7; border-color: #005499; }" & _
        "button.lnxctl.default { border: 2px solid #0078d7; }" & _
        "button.lnxctl:disabled { background: #cccccc; color: #838383; border-color: #bfbfbf; }" & _
        "checkbutton.lnxctl, radiobutton.lnxctl { min-height: 0; padding: 0; color: #000000; }" & _
        "checkbutton.lnxctl check, radiobutton.lnxctl radio { min-width: 13px; min-height: 13px; margin: 0 3px 0 0; padding: 0; -gtk-icon-transform: scale(0.93); }" & _
        "checkbutton.lnxctl > label, radiobutton.lnxctl > label { padding: 0; margin: 0; }" & _
        "combobox.lnxctl button { min-height: 0; padding: 0px 4px; border-radius: 0; border: 1px solid #adadad; background: #e1e1e1; box-shadow: none; }" & _
        "combobox.lnxctl entry { min-height: 0; padding: 0px 4px; border-radius: 0; }" & _
        "combobox.lnxctl box.linked > entry { border-right-width: 0; }" & _
        "combobox.lnxctl box.linked > button { border-left-width: 1px; }" & _
        "frame.lnxctl > border { border-radius: 0; border: 1px solid #dcdcdc; }" & _
        "frame.lnxctl > label { color: #000000; }" & _
        "label.lnxctl { color: #000000; padding: 0; }" & _
        "textview.lnxctl, textview.lnxctl text { background: #ffffff; color: #000000; }" & _
        "scrolledwindow.lnxedge { border: 1px solid #828790; }" & _
        "treeview.lnxctl { background: #ffffff; color: #000000; }" & _
        "treeview.lnxctl:selected { background: #cce8ff; color: #000000; }" & _
        "tooltip { border-radius: 0; }" & _
        "tooltip.background { background-color: #ffffff; border: 1px solid #767676; }" & _
        "tooltip label { color: #575757; padding: 2px 4px; }"
    dim as GtkCssProvider ptr prov = gtk_css_provider_new()
    gtk_css_provider_load_from_data( prov, LnxZ(css), len(css), NULL )
    gtk_style_context_add_provider_for_screen( gdk_screen_get_default(), GTK_STYLE_PROVIDER(prov), _
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION )
end sub

' Default dialog font (CWindow default: Segoe UI 9pt)
private function hFontDefault() as HFONT
    static as HFONT hf
    if hf = 0 then
        hf = LnxCreateFontObj( -12, 400, false, false, "Segoe UI" )
        hf->stock = true
    end if
    return hf
end function

private sub LnxAddClass( byval w as GtkWidget ptr, byval clsName as const zstring ptr )
    gtk_style_context_add_class( gtk_widget_get_style_context(w), clsName )
end sub

' Apply an HFONT to a native widget
private sub LnxApplyFont( byval hwnd as HWND, byval hFont as HFONT )
    if hFont = 0 orelse hFont->pfd = 0 then exit sub
    dim as GtkWidget ptr w = iif(hwnd->inner, hwnd->inner, hwnd->widget)
    if w = 0 then exit sub
    ' gtk_widget_override_font is deprecated but remains the simplest per widget font setter
    gtk_widget_override_font( w, hFont->pfd )
    if hwnd->kind = LK_COMBOBOX then
        dim as GtkWidget ptr child = gtk_bin_get_child( GTK_BIN(hwnd->inner) )
        if child then gtk_widget_override_font( child, hFont->pfd )
    elseif hwnd->kind = LK_BUTTON orelse hwnd->kind = LK_CHECKBOX orelse hwnd->kind = LK_RADIOBUTTON then
        dim as GtkWidget ptr child = gtk_bin_get_child( GTK_BIN(hwnd->inner) )
        if child then gtk_widget_override_font( child, hFont->pfd )
    end if
end sub

private sub LnxNotifyParent( byval hwnd as HWND, byval code as long )
    if IsWindow(hwnd) = 0 orelse hwnd->parent = 0 then exit sub
    if hwnd->blockSignals > 0 then exit sub
    SendMessage( hwnd->parent, WM_COMMAND, MAKEWPARAM(hwnd->id, code), cast(LPARAM, hwnd) )
end sub

private sub LnxNotifyParentNM( byval hwnd as HWND, byval code as UINT )
    if IsWindow(hwnd) = 0 orelse hwnd->parent = 0 then exit sub
    if hwnd->blockSignals > 0 then exit sub
    dim as NMHDR nm
    nm.hwndFrom = hwnd : nm.idFrom = hwnd->id : nm.code = code
    SendMessage( hwnd->parent, WM_NOTIFY, hwnd->id, cast(LPARAM, @nm) )
end sub


' ----------------------------------------------------------------------------
' Native widget signal handlers
' ----------------------------------------------------------------------------
private sub LnxOnButtonClicked cdecl ( byval w as GtkWidget ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    LnxNotifyParent( hwnd, BN_CLICKED )
end sub

private sub LnxOnToggled cdecl ( byval w as GtkWidget ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    ' radio buttons: only notify for the button being checked (Windows behaviour)
    if hwnd->kind = LK_RADIOBUTTON andalso gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w) ) = 0 then exit sub
    LnxNotifyParent( hwnd, BN_CLICKED )
end sub

private sub LnxOnEntryChanged cdecl ( byval w as GtkWidget ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    if hwnd->kind = LK_COMBOBOX then
        LnxNotifyParent( hwnd, CBN_EDITUPDATE )
        LnxNotifyParent( hwnd, CBN_EDITCHANGE )
    else
        LnxNotifyParent( hwnd, EN_UPDATE )
        LnxNotifyParent( hwnd, EN_CHANGE )
    end if
end sub

private sub LnxOnComboChanged cdecl ( byval w as GtkWidget ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    if gtk_combo_box_get_active( GTK_COMBO_BOX(w) ) < 0 then exit sub   ' text typed into the entry
    hwnd->lbCurSel = gtk_combo_box_get_active( GTK_COMBO_BOX(w) )
    LnxNotifyParent( hwnd, CBN_SELCHANGE )
    LnxNotifyParent( hwnd, CBN_SELENDOK )
end sub

private sub LnxOnComboPopup cdecl ( byval w as GtkWidget ptr, byval pspec as any ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    dim as gboolean shown
    g_object_get( G_OBJECT(w), "popup-shown", @shown, NULL )
    LnxNotifyParent( hwnd, iif(shown, CBN_DROPDOWN, CBN_CLOSEUP) )
end sub

private function LnxOnCtlFocusIn cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventFocus ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    select case hwnd->kind
        case LK_TEXTBOX, LK_EDITMULTI, LK_RICHEDIT: LnxNotifyParent( hwnd, EN_SETFOCUS )
        case LK_COMBOBOX: LnxNotifyParent( hwnd, CBN_SETFOCUS )
        case LK_BUTTON: LnxNotifyParent( hwnd, BN_SETFOCUS )
        case LK_LISTVIEW, LK_TREEVIEW: LnxNotifyParentNM( hwnd, NM_SETFOCUS )
    end select
    SendMessage( hwnd, WM_SETFOCUS, 0, 0 )
    return FALSE
end function

private function LnxOnCtlFocusOut cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventFocus ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    select case hwnd->kind
        case LK_TEXTBOX, LK_EDITMULTI, LK_RICHEDIT: LnxNotifyParent( hwnd, EN_KILLFOCUS )
        case LK_COMBOBOX: LnxNotifyParent( hwnd, CBN_KILLFOCUS )
        case LK_BUTTON: LnxNotifyParent( hwnd, BN_KILLFOCUS )
    end select
    SendMessage( hwnd, WM_KILLFOCUS, 0, 0 )
    return FALSE
end function

' Static controls are painted through WM_PAINT so that subclass procedures can owner draw
' them (tiko paints tab strips and list headers this way). If the procedure chain reaches the
' default handler, GTK draws the label normally.
private function LnxOnLabelDraw cdecl ( byval w as GtkWidget ptr, byval cr as cairo_t ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 orelse ubound(hwnd->subclasses) < 0 then return FALSE
    cairo_save( cr )
    cairo_rectangle( cr, 0, 0, gtk_widget_get_allocated_width(w), gtk_widget_get_allocated_height(w) )
    cairo_clip( cr )
    hwnd->paintCr = cr
    hwnd->lbAnchor = 0          ' reused as "default paint reached" flag for labels
    SendMessage( hwnd, WM_PAINT, 0, 0 )
    dim as boolean bDefault = (hwnd->lbAnchor <> 0)
    if IsWindow(hwnd) then hwnd->paintCr = 0
    cairo_restore( cr )
    return iif( bDefault, FALSE, TRUE )
end function

private function LnxOnLabelClick cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventButton ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    if ev->button = 1 andalso (hwnd->style and SS_NOTIFY) then
        if ev->type = GDK_2BUTTON_PRESS then
            LnxNotifyParent( hwnd, STN_DBLCLK )
        elseif ev->type = GDK_BUTTON_PRESS then
            LnxNotifyParent( hwnd, STN_CLICKED )
        end if
    end if
    return FALSE
end function


' ----------------------------------------------------------------------------
' Listbox (custom drawn)
' ----------------------------------------------------------------------------
#ifndef LNX_SCROLLBAR_W
#define LNX_SCROLLBAR_W 17
#endif

private function LnxLbHasScrollbar( byval hwnd as HWND ) as boolean
    if (hwnd->style and WS_VSCROLL) = 0 then return false
    dim as long visibleItems = hwnd->h \ iif(hwnd->lbItemHeight < 1, 1, hwnd->lbItemHeight)
    return hwnd->lbCount > visibleItems
end function

private function LnxLbBorder( byval hwnd as HWND ) as long
    return iif( (hwnd->exstyle and WS_EX_CLIENTEDGE), 1, 0 )
end function

private function LnxLbVisibleCount( byval hwnd as HWND ) as long
    dim as long ih = iif(hwnd->lbItemHeight < 1, 1, hwnd->lbItemHeight)
    return (hwnd->h - 2 * LnxLbBorder(hwnd)) \ ih
end function

private sub LnxLbClampTop( byval hwnd as HWND )
    dim as long maxTop = hwnd->lbCount - LnxLbVisibleCount(hwnd)
    if maxTop < 0 then maxTop = 0
    if hwnd->lbTopIndex > maxTop then hwnd->lbTopIndex = maxTop
    if hwnd->lbTopIndex < 0 then hwnd->lbTopIndex = 0
end sub

private sub LnxLbEnsureVisible( byval hwnd as HWND, byval idx as long )
    if idx < 0 then exit sub
    dim as long vis = LnxLbVisibleCount(hwnd)
    if vis < 1 then vis = 1
    if idx < hwnd->lbTopIndex then hwnd->lbTopIndex = idx
    if idx >= hwnd->lbTopIndex + vis then hwnd->lbTopIndex = idx - vis + 1
    LnxLbClampTop( hwnd )
end sub

private function LnxLbIsOwnerDraw( byval hwnd as HWND ) as boolean
    return (hwnd->style and (LBS_OWNERDRAWFIXED or &h0020)) <> 0      ' LBS_OWNERDRAWVARIABLE = &h20
end function

private function LnxLbIsMulti( byval hwnd as HWND ) as boolean
    return (hwnd->style and (LBS_MULTIPLESEL or LBS_EXTENDEDSEL)) <> 0
end function

' Ask the parent for the item height of owner drawn listboxes (WM_MEASUREITEM)
private sub LnxLbMeasure( byval hwnd as HWND )
    if LnxLbIsOwnerDraw(hwnd) andalso hwnd->parent then
        dim as MEASUREITEMSTRUCT mis
        mis.CtlType = ODT_LISTBOX
        mis.CtlID = hwnd->id
        mis.itemID = 0
        mis.itemHeight = 16
        SendMessage( hwnd->parent, WM_MEASUREITEM, hwnd->id, cast(LPARAM, @mis) )
        if mis.itemHeight > 0 then hwnd->lbItemHeight = mis.itemHeight
    else
        ' standard listbox: font height + 1 (Segoe UI 9pt -> 15px at 96 DPI)
        hwnd->lbItemHeight = 15
    end if
end sub

private function LnxLbItemRect( byval hwnd as HWND, byval idx as long ) as RECT
    dim as RECT rc
    dim as long b = LnxLbBorder(hwnd)
    rc.left = b
    rc.right = hwnd->w - b - iif(LnxLbHasScrollbar(hwnd), LNX_SCROLLBAR_W, 0)
    rc.top = b + (idx - hwnd->lbTopIndex) * hwnd->lbItemHeight
    rc.bottom = rc.top + hwnd->lbItemHeight
    return rc
end function

private function LnxLbItemFromPoint( byval hwnd as HWND, byval x as long, byval y as long, byref bOutside as boolean ) as long
    dim as long ih = iif(hwnd->lbItemHeight < 1, 1, hwnd->lbItemHeight)
    dim as long idx = hwnd->lbTopIndex + (y - LnxLbBorder(hwnd)) \ ih
    bOutside = false
    if y < 0 then idx = hwnd->lbTopIndex : bOutside = true
    if idx >= hwnd->lbCount then idx = hwnd->lbCount - 1 : bOutside = true
    if idx < 0 then idx = 0 : bOutside = true
    return idx
end function

private sub LnxLbPaint( byval hwnd as HWND )
    dim as cairo_t ptr cr = hwnd->paintCr
    if cr = 0 then exit sub
    dim as HDC hdc = new LNXDC
    LnxDCInit( hdc )
    hdc->cr = cr : hdc->ownsCr = false : hdc->hwnd = hwnd
    if hwnd->hFont then hdc->st.hFont = hwnd->hFont

    ' background: like Windows, WM_ERASEBKGND goes through the window procedure chain first so
    ' that subclass procedures can paint the area that is not covered by items.
    cairo_save( cr )
    if SendMessage( hwnd, WM_ERASEBKGND, cast(WPARAM, hdc), 0 ) = 0 then
        LnxSetColor( cr, GetSysColor(COLOR_WINDOW) )
        cairo_paint( cr )
    end if
    cairo_restore( cr )

    dim as long b = LnxLbBorder(hwnd)
    dim as long ih = iif(hwnd->lbItemHeight < 1, 1, hwnd->lbItemHeight)
    dim as long nVisible = (hwnd->h - 2 * b) \ ih + 1
    dim as HWND hFocus = GetFocus()

    cairo_save( cr )
    cairo_rectangle( cr, b, b, hwnd->w - 2 * b - iif(LnxLbHasScrollbar(hwnd), LNX_SCROLLBAR_W, 0), hwnd->h - 2 * b )
    cairo_clip( cr )
    for i as long = hwnd->lbTopIndex to hwnd->lbTopIndex + nVisible
        if i >= hwnd->lbCount then exit for
        dim as RECT rc = LnxLbItemRect( hwnd, i )
        dim as boolean bSel = false
        if LnxLbIsMulti(hwnd) then
            if i <= ubound(hwnd->lbItems) then bSel = hwnd->lbItems(i).selected
        else
            bSel = (i = hwnd->lbCurSel)
        end if
        if LnxLbIsOwnerDraw(hwnd) andalso hwnd->parent then
            dim as DRAWITEMSTRUCT dis
            dis.CtlType = ODT_LISTBOX
            dis.CtlID = hwnd->id
            dis.itemID = i
            dis.itemAction = ODA_DRAWENTIRE
            dis.itemState = iif(bSel, ODS_SELECTED, 0) or iif(hFocus = hwnd andalso i = hwnd->lbCaret, ODS_FOCUS, 0)
            dis.hwndItem = hwnd
            dis.hDC = hdc
            dis.rcItem = rc
            if i <= ubound(hwnd->lbItems) then dis.itemData = hwnd->lbItems(i).itemData
            cairo_save( cr )
            SendMessage( hwnd->parent, WM_DRAWITEM, hwnd->id, cast(LPARAM, @dis) )
            cairo_restore( cr )
        else
            if bSel then
                LnxSetColor( cr, iif(hFocus = hwnd, GetSysColor(COLOR_HIGHLIGHT), BGR(204,232,255)) )
                cairo_rectangle( cr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top )
                cairo_fill( cr )
            end if
            hdc->st.bkMode = TRANSPARENT
            hdc->st.textColor = iif(bSel andalso hFocus = hwnd, GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_WINDOWTEXT))
            dim as RECT rcText = rc
            rcText.left += 2
            if i <= ubound(hwnd->lbItems) then
                DrawText( hdc, hwnd->lbItems(i).text.vptr, -1, @rcText, DT_LEFT or DT_VCENTER or DT_SINGLELINE or DT_NOPREFIX )
            end if
        end if
    next
    cairo_restore( cr )

    ' scrollbar (Windows 10 style)
    if LnxLbHasScrollbar(hwnd) then
        dim as long sx = hwnd->w - b - LNX_SCROLLBAR_W
        LnxSetColor( cr, BGR(240,240,240) )
        cairo_rectangle( cr, sx, b, LNX_SCROLLBAR_W, hwnd->h - 2 * b )
        cairo_fill( cr )
        dim as long trackTop = b + LNX_SCROLLBAR_W, trackH = hwnd->h - 2 * b - 2 * LNX_SCROLLBAR_W
        dim as long vis = LnxLbVisibleCount(hwnd)
        if trackH > 0 andalso hwnd->lbCount > 0 then
            dim as long thumbH = trackH * vis \ hwnd->lbCount
            if thumbH < 17 then thumbH = 17
            dim as long maxTop = hwnd->lbCount - vis
            dim as long thumbY = trackTop + iif(maxTop > 0, (trackH - thumbH) * hwnd->lbTopIndex \ maxTop, 0)
            LnxSetColor( cr, BGR(205,205,205) )
            cairo_rectangle( cr, sx + 2, thumbY, LNX_SCROLLBAR_W - 4, thumbH )
            cairo_fill( cr )
        end if
        ' arrows
        LnxSetColor( cr, BGR(96,96,96) )
        dim as double cx = sx + LNX_SCROLLBAR_W / 2
        cairo_move_to( cr, cx - 4, b + 10 ) : cairo_line_to( cr, cx, b + 6 ) : cairo_line_to( cr, cx + 4, b + 10 )
        cairo_close_path( cr ) : cairo_fill( cr )
        dim as double by_ = hwnd->h - b - LNX_SCROLLBAR_W
        cairo_move_to( cr, cx - 4, by_ + 7 ) : cairo_line_to( cr, cx, by_ + 11 ) : cairo_line_to( cr, cx + 4, by_ + 7 )
        cairo_close_path( cr ) : cairo_fill( cr )
    end if

    ' client edge
    if b then
        LnxSetColor( cr, BGR(130,135,144) )
        cairo_set_line_width( cr, 1 )
        cairo_rectangle( cr, 0.5, 0.5, hwnd->w - 1, hwnd->h - 1 )
        cairo_stroke( cr )
    end if
    hdc->cr = 0
    delete hdc
end sub

private sub LnxLbSetCount( byval hwnd as HWND, byval n as long )
    if n < 0 then n = 0
    if n = 0 then
        erase hwnd->lbItems
    else
        redim preserve hwnd->lbItems(n - 1)
    end if
    hwnd->lbCount = n
    if hwnd->lbCurSel >= n then hwnd->lbCurSel = -1
    if hwnd->lbCaret >= n then hwnd->lbCaret = iif(n > 0, n - 1, 0)
    LnxLbClampTop( hwnd )
end sub

private function LnxLbInsert( byval hwnd as HWND, byval idx as long, byval pText as const wstring ptr ) as long
    if idx < 0 orelse idx > hwnd->lbCount then idx = hwnd->lbCount
    ' LBS_SORT
    if (hwnd->style and LBS_SORT) andalso pText then
        idx = hwnd->lbCount
        for i as long = 0 to hwnd->lbCount - 1
            if lcase(hwnd->lbItems(i).text) > lcase(*pText) then idx = i : exit for
        next
    end if
    redim preserve hwnd->lbItems(hwnd->lbCount)
    for i as long = hwnd->lbCount to idx + 1 step -1
        hwnd->lbItems(i) = hwnd->lbItems(i - 1)
    next
    hwnd->lbItems(idx).text = iif(pText, *pText, wstr(""))
    hwnd->lbItems(idx).itemData = 0
    hwnd->lbItems(idx).selected = false
    hwnd->lbCount += 1
    if hwnd->lbCurSel >= idx then hwnd->lbCurSel += 1
    return idx
end function

private sub LnxLbDelete( byval hwnd as HWND, byval idx as long )
    if idx < 0 orelse idx >= hwnd->lbCount then exit sub
    for i as long = idx to hwnd->lbCount - 2
        hwnd->lbItems(i) = hwnd->lbItems(i + 1)
    next
    hwnd->lbCount -= 1
    if hwnd->lbCount = 0 then erase hwnd->lbItems else redim preserve hwnd->lbItems(hwnd->lbCount - 1)
    if hwnd->lbCurSel = idx then
        hwnd->lbCurSel = -1
    elseif hwnd->lbCurSel > idx then
        hwnd->lbCurSel -= 1
    end if
    LnxLbClampTop( hwnd )
end sub

private sub LnxLbSelectRange( byval hwnd as HWND, byval a as long, byval b as long, byval bSel as boolean )
    if a > b then swap a, b
    for i as long = a to b
        if i >= 0 andalso i <= ubound(hwnd->lbItems) then hwnd->lbItems(i).selected = bSel
    next
end sub

private sub LnxLbClearSel( byval hwnd as HWND )
    for i as long = 0 to ubound(hwnd->lbItems)
        hwnd->lbItems(i).selected = false
    next
end sub

' Mouse/keyboard selection handling; returns true if the selection changed
private function LnxLbSelectByUser( byval hwnd as HWND, byval idx as long, byval mk as WPARAM ) as boolean
    if idx < 0 orelse idx >= hwnd->lbCount then return false
    dim as long prevSel = hwnd->lbCurSel
    if (hwnd->style and LBS_EXTENDEDSEL) then
        if (mk and MK_SHIFT) andalso hwnd->lbAnchor >= 0 then
            if (mk and MK_CONTROL) = 0 then LnxLbClearSel( hwnd )
            LnxLbSelectRange( hwnd, hwnd->lbAnchor, idx, true )
        elseif (mk and MK_CONTROL) then
            if idx <= ubound(hwnd->lbItems) then hwnd->lbItems(idx).selected = not hwnd->lbItems(idx).selected
            hwnd->lbAnchor = idx
        else
            LnxLbClearSel( hwnd )
            if idx <= ubound(hwnd->lbItems) then hwnd->lbItems(idx).selected = true
            hwnd->lbAnchor = idx
        end if
    elseif (hwnd->style and LBS_MULTIPLESEL) then
        if idx <= ubound(hwnd->lbItems) then hwnd->lbItems(idx).selected = not hwnd->lbItems(idx).selected
    end if
    hwnd->lbCurSel = idx
    hwnd->lbCaret = idx
    return true
end function

private sub LnxLbScrollTo( byval hwnd as HWND, byval newTop as long )
    dim as long prev = hwnd->lbTopIndex
    hwnd->lbTopIndex = newTop
    LnxLbClampTop( hwnd )
    if prev <> hwnd->lbTopIndex then gtk_widget_queue_draw( hwnd->client )
end sub

private function LnxListBoxProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                                 byref handled as boolean ) as LRESULT
    static as boolean bThumbDrag, bSelDrag
    static as long dragStartY, dragStartTop
    handled = true
    select case uMsg
        case WM_PAINT
            LnxLbPaint( hwnd )
            return 0
        case WM_ERASEBKGND
            dim as HDC hdc = cast(HDC, wParam)
            if hdc andalso hdc->cr then
                LnxSetColor( hdc->cr, GetSysColor(COLOR_WINDOW) )
                cairo_paint( hdc->cr )
            end if
            return 1
        case WM_SIZE
            LnxLbClampTop( hwnd )
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case WM_SETFOCUS, WM_KILLFOCUS
            gtk_widget_queue_draw( hwnd->client )
            LnxNotifyParent( hwnd, iif(uMsg = WM_SETFOCUS, LBN_SETFOCUS, LBN_KILLFOCUS) )
            return 0
        case WM_LBUTTONDOWN, WM_LBUTTONDBLCLK
            dim as long x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam)
            gtk_widget_grab_focus( hwnd->client )
            if LnxLbHasScrollbar(hwnd) andalso x >= hwnd->w - LnxLbBorder(hwnd) - LNX_SCROLLBAR_W then
                dim as long b = LnxLbBorder(hwnd)
                dim as long vis = LnxLbVisibleCount(hwnd)
                if y < b + LNX_SCROLLBAR_W then
                    LnxLbScrollTo( hwnd, hwnd->lbTopIndex - 1 )
                elseif y > hwnd->h - b - LNX_SCROLLBAR_W then
                    LnxLbScrollTo( hwnd, hwnd->lbTopIndex + 1 )
                else
                    bThumbDrag = true
                    dragStartY = y : dragStartTop = hwnd->lbTopIndex
                    SetCapture( hwnd )
                end if
                return 0
            end if
            dim as boolean bOutside
            dim as long idx = LnxLbItemFromPoint( hwnd, x, y, bOutside )
            if bOutside = false then
                if LnxLbSelectByUser( hwnd, idx, wParam ) then
                    LnxLbEnsureVisible( hwnd, idx )
                    gtk_widget_queue_draw( hwnd->client )
                    if (hwnd->style and LBS_NOTIFY) then LnxNotifyParent( hwnd, LBN_SELCHANGE )
                end if
                if uMsg = WM_LBUTTONDBLCLK andalso (hwnd->style and LBS_NOTIFY) then LnxNotifyParent( hwnd, LBN_DBLCLK )
                bSelDrag = (LnxLbIsMulti(hwnd) = false)
                SetCapture( hwnd )
            end if
            return 0
        case WM_MOUSEMOVE
            if bThumbDrag andalso GetCapture() = hwnd then
                dim as long y = GET_Y_LPARAM(lParam)
                dim as long trackH = hwnd->h - 2 * LnxLbBorder(hwnd) - 2 * LNX_SCROLLBAR_W
                if trackH > 0 then
                    LnxLbScrollTo( hwnd, dragStartTop + ((y - dragStartY) * hwnd->lbCount) \ trackH )
                end if
            elseif bSelDrag andalso GetCapture() = hwnd andalso (wParam and MK_LBUTTON) then
                dim as boolean bOutside
                dim as long idx = LnxLbItemFromPoint( hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), bOutside )
                if idx <> hwnd->lbCurSel andalso idx >= 0 andalso idx < hwnd->lbCount then
                    hwnd->lbCurSel = idx : hwnd->lbCaret = idx
                    LnxLbEnsureVisible( hwnd, idx )
                    gtk_widget_queue_draw( hwnd->client )
                    if (hwnd->style and LBS_NOTIFY) then LnxNotifyParent( hwnd, LBN_SELCHANGE )
                end if
            end if
            return 0
        case WM_LBUTTONUP
            if GetCapture() = hwnd then ReleaseCapture()
            bThumbDrag = false : bSelDrag = false
            return 0
        case WM_MOUSEWHEEL
            dim as long delta = cast(short, HIWORD(wParam))
            LnxLbScrollTo( hwnd, hwnd->lbTopIndex - (delta \ 120) * 3 )
            return 0
        case WM_KEYDOWN
            dim as long idx = hwnd->lbCaret
            dim as long vis = LnxLbVisibleCount(hwnd)
            select case wParam
                case VK_UP: idx -= 1
                case VK_DOWN: idx += 1
                case VK_PRIOR: idx -= iif(vis > 1, vis - 1, 1)
                case VK_NEXT: idx += iif(vis > 1, vis - 1, 1)
                case VK_HOME: idx = 0
                case VK_END: idx = hwnd->lbCount - 1
                case else: return 0
            end select
            if hwnd->lbCount = 0 then return 0
            if idx < 0 then idx = 0
            if idx >= hwnd->lbCount then idx = hwnd->lbCount - 1
            dim as WPARAM mk = 0
            if (GetKeyState(VK_SHIFT) and &h8000) then mk or= MK_SHIFT
            if (hwnd->style and LBS_EXTENDEDSEL) = 0 orelse mk = 0 then hwnd->lbAnchor = idx
            LnxLbSelectByUser( hwnd, idx, mk )
            LnxLbEnsureVisible( hwnd, idx )
            gtk_widget_queue_draw( hwnd->client )
            if (hwnd->style and LBS_NOTIFY) then LnxNotifyParent( hwnd, LBN_SELCHANGE )
            return 0
        case WM_CHAR, WM_KEYUP, WM_SYSKEYDOWN, WM_SYSKEYUP
            return 0

        case LB_ADDSTRING
            dim as long idx = LnxLbInsert( hwnd, -1, cast(wstring ptr, lParam) )
            gtk_widget_queue_draw( hwnd->client )
            return idx
        case LB_INSERTSTRING
            dim as long idx = LnxLbInsert( hwnd, cast(long, wParam), cast(wstring ptr, lParam) )
            gtk_widget_queue_draw( hwnd->client )
            return idx
        case LB_DELETESTRING
            if cast(long, wParam) < 0 orelse cast(long, wParam) >= hwnd->lbCount then return LB_ERR
            LnxLbDelete( hwnd, wParam )
            gtk_widget_queue_draw( hwnd->client )
            return hwnd->lbCount
        case LB_RESETCONTENT
            LnxLbSetCount( hwnd, 0 )
            hwnd->lbCurSel = -1 : hwnd->lbTopIndex = 0 : hwnd->lbCaret = 0 : hwnd->lbAnchor = -1
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_SETCOUNT
            LnxLbSetCount( hwnd, wParam )
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_GETCOUNT
            return hwnd->lbCount
        case LB_GETCURSEL
            if LnxLbIsMulti(hwnd) then return hwnd->lbCaret
            return hwnd->lbCurSel
        case LB_SETCURSEL
            dim as long idx = cast(long, wParam)
            if idx < -1 orelse idx >= hwnd->lbCount then idx = -1
            hwnd->lbCurSel = idx
            if idx >= 0 then
                hwnd->lbCaret = idx
                LnxLbEnsureVisible( hwnd, idx )
            end if
            gtk_widget_queue_draw( hwnd->client )
            return iif(idx = -1, LB_ERR, idx)
        case LB_GETSEL
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx >= hwnd->lbCount then return LB_ERR
            if LnxLbIsMulti(hwnd) then
                if idx <= ubound(hwnd->lbItems) then return iif(hwnd->lbItems(idx).selected, 1, 0)
                return 0
            end if
            return iif(idx = hwnd->lbCurSel, 1, 0)
        case LB_SETSEL
            ' wParam = select flag, lParam = index (-1 = all)
            dim as long idx = cast(long, lParam)
            if idx = -1 then
                LnxLbSelectRange( hwnd, 0, hwnd->lbCount - 1, wParam <> 0 )
            elseif idx >= 0 andalso idx < hwnd->lbCount then
                if idx <= ubound(hwnd->lbItems) then hwnd->lbItems(idx).selected = (wParam <> 0)
                if wParam then hwnd->lbCaret = idx : hwnd->lbCurSel = idx
            end if
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_SELITEMRANGE
            LnxLbSelectRange( hwnd, LOWORD(lParam), HIWORD(lParam), wParam <> 0 )
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_SELITEMRANGEEX
            LnxLbSelectRange( hwnd, cast(long, wParam), cast(long, lParam), true )
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_GETSELCOUNT
            if LnxLbIsMulti(hwnd) = false then return LB_ERR
            dim as long n = 0
            for i as long = 0 to ubound(hwnd->lbItems)
                if hwnd->lbItems(i).selected then n += 1
            next
            return n
        case LB_GETSELITEMS
            dim as long n = 0
            dim as long ptr arr = cast(long ptr, lParam)
            for i as long = 0 to ubound(hwnd->lbItems)
                if n >= cast(long, wParam) then exit for
                if hwnd->lbItems(i).selected then arr[n] = i : n += 1
            next
            return n
        case LB_GETITEMDATA
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return LB_ERR
            return hwnd->lbItems(idx).itemData
        case LB_SETITEMDATA
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return LB_ERR
            hwnd->lbItems(idx).itemData = lParam
            return 0
        case LB_GETTEXT
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) orelse lParam = 0 then return LB_ERR
            dim as long n = len(hwnd->lbItems(idx).text)
            memcpy( cast(any ptr, lParam), hwnd->lbItems(idx).text.vptr, (n + 1) * sizeof(wstring) )
            return n
        case LB_GETTEXTLEN
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return LB_ERR
            return len(hwnd->lbItems(idx).text)
        case LB_GETITEMRECT
            dim as long idx = cast(long, wParam)
            if lParam = 0 then return LB_ERR
            *cast(RECT ptr, lParam) = LnxLbItemRect( hwnd, idx )
            return 1
        case LB_GETITEMHEIGHT
            return hwnd->lbItemHeight
        case LB_SETITEMHEIGHT
            hwnd->lbItemHeight = iif(LOWORD(lParam) > 0, LOWORD(lParam), 1)
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_GETTOPINDEX
            return hwnd->lbTopIndex
        case LB_SETTOPINDEX
            hwnd->lbTopIndex = cast(long, wParam)
            LnxLbClampTop( hwnd )
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_GETCARETINDEX
            return hwnd->lbCaret
        case LB_SETCARETINDEX
            hwnd->lbCaret = cast(long, wParam)
            gtk_widget_queue_draw( hwnd->client )
            return 0
        case LB_SETANCHORINDEX
            hwnd->lbAnchor = cast(long, wParam)
            return 0
        case LB_GETANCHORINDEX
            return hwnd->lbAnchor
        case LB_ITEMFROMPOINT
            dim as boolean bOutside
            dim as long idx = LnxLbItemFromPoint( hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), bOutside )
            if hwnd->lbCount = 0 then return MAKELONG(0, 1)
            return MAKELONG(idx, iif(bOutside, 1, 0))
        case LB_FINDSTRING, LB_FINDSTRINGEXACT, LB_SELECTSTRING
            if lParam = 0 then return LB_ERR
            dim as DWSTRING s = lcase(*cast(wstring ptr, lParam))
            dim as long start = cast(long, wParam) + 1
            for k as long = 0 to hwnd->lbCount - 1
                dim as long i = (start + k) mod iif(hwnd->lbCount = 0, 1, hwnd->lbCount)
                if i > ubound(hwnd->lbItems) then continue for
                dim as DWSTRING t = lcase(hwnd->lbItems(i).text)
                dim as boolean bMatch = iif(uMsg = LB_FINDSTRINGEXACT, t = s, left(t, len(s)) = s)
                if bMatch then
                    if uMsg = LB_SELECTSTRING then SendMessage( hwnd, LB_SETCURSEL, i, 0 )
                    return i
                end if
            next
            return LB_ERR
    end select
    handled = false
    return 0
end function


' ----------------------------------------------------------------------------
' Combobox helpers (items mirrored in lbItems for item data)
' ----------------------------------------------------------------------------
private function LnxComboEntry( byval hwnd as HWND ) as GtkEntry ptr
    if gtk_combo_box_get_has_entry( GTK_COMBO_BOX(hwnd->inner) ) = 0 then return 0
    return GTK_ENTRY( gtk_bin_get_child( GTK_BIN(hwnd->inner) ) )
end function

private function LnxComboProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                               byref handled as boolean ) as LRESULT
    dim as GtkComboBoxText ptr cb = GTK_COMBO_BOX_TEXT(hwnd->inner)
    handled = true
    select case uMsg
        case CB_ADDSTRING, CB_INSERTSTRING
            if lParam = 0 then return CB_ERR
            dim as long idx = iif(uMsg = CB_ADDSTRING, -1, cast(long, wParam))
            if (hwnd->style and CBS_SORT) then
                idx = hwnd->lbCount
                for i as long = 0 to hwnd->lbCount - 1
                    if lcase(hwnd->lbItems(i).text) > lcase(*cast(wstring ptr, lParam)) then idx = i : exit for
                next
            end if
            hwnd->blockSignals += 1
            dim as long savedCurSel = hwnd->lbCurSel
            idx = LnxLbInsert( hwnd, idx, cast(wstring ptr, lParam) )
            hwnd->lbCurSel = savedCurSel
            dim as string s = WideToUtf8( cast(wstring ptr, lParam) )
            gtk_combo_box_text_insert_text( cb, idx, LnxZ(s) )
            hwnd->blockSignals -= 1
            return idx
        case CB_DELETESTRING
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx >= hwnd->lbCount then return CB_ERR
            hwnd->blockSignals += 1
            LnxLbDelete( hwnd, idx )
            gtk_combo_box_text_remove( cb, idx )
            hwnd->blockSignals -= 1
            return hwnd->lbCount
        case CB_RESETCONTENT
            hwnd->blockSignals += 1
            LnxLbSetCount( hwnd, 0 )
            gtk_combo_box_text_remove_all( cb )
            hwnd->lbCurSel = -1
            hwnd->blockSignals -= 1
            return 0
        case CB_GETCOUNT
            return hwnd->lbCount
        case CB_GETCURSEL
            dim as long idx = gtk_combo_box_get_active( GTK_COMBO_BOX(cb) )
            if idx < 0 andalso LnxComboEntry(hwnd) then
                ' editable combobox: the selection is valid while the text still matches it
                if hwnd->lbCurSel >= 0 andalso hwnd->lbCurSel < hwnd->lbCount then
                    dim as string t = *gtk_entry_get_text( LnxComboEntry(hwnd) )
                    if DWSTRING(t) = hwnd->lbItems(hwnd->lbCurSel).text then return hwnd->lbCurSel
                end if
                return CB_ERR
            end if
            return iif(idx < 0, CB_ERR, idx)
        case CB_SETCURSEL
            dim as long idx = cast(long, wParam)
            hwnd->blockSignals += 1
            if idx < 0 orelse idx >= hwnd->lbCount then
                gtk_combo_box_set_active( GTK_COMBO_BOX(cb), -1 )
                if LnxComboEntry(hwnd) then gtk_entry_set_text( LnxComboEntry(hwnd), "" )
                hwnd->lbCurSel = -1
                hwnd->blockSignals -= 1
                return CB_ERR
            end if
            gtk_combo_box_set_active( GTK_COMBO_BOX(cb), idx )
            hwnd->lbCurSel = idx
            hwnd->blockSignals -= 1
            return idx
        case CB_GETLBTEXT
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) orelse lParam = 0 then return CB_ERR
            dim as long n = len(hwnd->lbItems(idx).text)
            memcpy( cast(any ptr, lParam), hwnd->lbItems(idx).text.vptr, (n + 1) * sizeof(wstring) )
            return n
        case CB_GETLBTEXTLEN
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return CB_ERR
            return len(hwnd->lbItems(idx).text)
        case CB_GETITEMDATA
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return CB_ERR
            return hwnd->lbItems(idx).itemData
        case CB_SETITEMDATA
            dim as long idx = cast(long, wParam)
            if idx < 0 orelse idx > ubound(hwnd->lbItems) then return CB_ERR
            hwnd->lbItems(idx).itemData = lParam
            return 0
        case CB_FINDSTRING, CB_FINDSTRINGEXACT, CB_SELECTSTRING
            if lParam = 0 then return CB_ERR
            dim as DWSTRING s = lcase(*cast(wstring ptr, lParam))
            dim as long start = cast(long, wParam) + 1
            for k as long = 0 to hwnd->lbCount - 1
                dim as long i = (start + k) mod iif(hwnd->lbCount = 0, 1, hwnd->lbCount)
                dim as DWSTRING t = lcase(hwnd->lbItems(i).text)
                dim as boolean bMatch = iif(uMsg = CB_FINDSTRINGEXACT, t = s, left(t, len(s)) = s)
                if bMatch then
                    if uMsg = CB_SELECTSTRING then SendMessage( hwnd, CB_SETCURSEL, i, 0 )
                    return i
                end if
            next
            return CB_ERR
        case CB_SHOWDROPDOWN
            if wParam then gtk_combo_box_popup( GTK_COMBO_BOX(cb) ) else gtk_combo_box_popdown( GTK_COMBO_BOX(cb) )
            return 1
        case CB_GETDROPPEDSTATE
            dim as gboolean shown
            g_object_get( G_OBJECT(cb), "popup-shown", @shown, NULL )
            return iif(shown, 1, 0)
        case CB_SETEDITSEL
            dim as GtkEntry ptr e = LnxComboEntry(hwnd)
            if e then gtk_editable_select_region( GTK_EDITABLE(e), cast(short, LOWORD(lParam)), cast(short, HIWORD(lParam)) )
            return 1
        case CB_LIMITTEXT
            dim as GtkEntry ptr e = LnxComboEntry(hwnd)
            if e then gtk_entry_set_max_length( e, wParam )
            return 1
        case CB_SETITEMHEIGHT, CB_SETDROPPEDWIDTH
            return 0
        case CB_GETITEMHEIGHT
            return 18
    end select
    handled = false
    return 0
end function


' ----------------------------------------------------------------------------
' Edit (GtkEntry / GtkTextView) helpers
' ----------------------------------------------------------------------------
private function LnxEditGetText( byval hwnd as HWND ) as DWSTRING
    select case hwnd->kind
        case LK_TEXTBOX
            return DWSTRING( *gtk_entry_get_text( GTK_ENTRY(hwnd->inner) ) )
        case LK_COMBOBOX
            dim as GtkEntry ptr e = LnxComboEntry(hwnd)
            if e then return DWSTRING( *gtk_entry_get_text(e) )
            dim as long idx = gtk_combo_box_get_active( GTK_COMBO_BOX(hwnd->inner) )
            if idx >= 0 andalso idx <= ubound(hwnd->lbItems) then return hwnd->lbItems(idx).text
            return DWSTRING()
        case LK_EDITMULTI, LK_RICHEDIT
            dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
            dim as GtkTextIter a, b
            gtk_text_buffer_get_bounds( buf, @a, @b )
            dim as zstring ptr p = gtk_text_buffer_get_text( buf, @a, @b, FALSE )
            dim as string s = *p
            g_free( p )
            ' Windows multiline edits use CRLF
            dim as DWSTRING dws = s
            return AfxStrReplace( AfxStrReplace(dws, !"\r\n", !"\n"), !"\n", !"\r\n" )
    end select
    return hwnd->text
end function

private sub LnxEditSetText( byval hwnd as HWND, byref wszText as const wstring )
    dim as string s = WideToUtf8( @wszText )
    select case hwnd->kind
        case LK_TEXTBOX
            gtk_entry_set_text( GTK_ENTRY(hwnd->inner), LnxZ(s) )
        case LK_COMBOBOX
            dim as GtkEntry ptr e = LnxComboEntry(hwnd)
            if e then
                gtk_entry_set_text( e, LnxZ(s) )
            else
                ' non editable combobox: select the matching item
                for i as long = 0 to hwnd->lbCount - 1
                    if hwnd->lbItems(i).text = wszText then
                        hwnd->blockSignals += 1
                        gtk_combo_box_set_active( GTK_COMBO_BOX(hwnd->inner), i )
                        hwnd->blockSignals -= 1
                        exit for
                    end if
                next
            end if
        case LK_EDITMULTI, LK_RICHEDIT
            dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
            dim as string t = s
            ' strip CR (GtkTextView uses LF)
            dim as DWSTRING dws = AfxStrReplace( wszText, !"\r\n", !"\n" )
            t = dws.utf8
            gtk_text_buffer_set_text( buf, LnxZ(t), len(t) )
        case LK_BUTTON, LK_CHECKBOX, LK_RADIOBUTTON
            ' Win32 uses & for mnemonics; GTK uses _
            dim as DWSTRING m = AfxStrReplace( AfxStrReplace(wszText, "_", "__"), "&&", chr(1) )
            m = AfxStrReplace( AfxStrReplace(m, "&", "_"), chr(1), "&" )
            dim as string ms = m.utf8
            gtk_button_set_label( GTK_BUTTON(hwnd->inner), LnxZ(ms) )
            gtk_button_set_use_underline( GTK_BUTTON(hwnd->inner), TRUE )
            dim as GtkWidget ptr lbl = gtk_bin_get_child( GTK_BIN(hwnd->inner) )
            if lbl andalso GTK_IS_LABEL(lbl) then
                gtk_label_set_ellipsize( GTK_LABEL(lbl), PANGO_ELLIPSIZE_END )
                if hwnd->kind = LK_BUTTON then gtk_label_set_xalign( GTK_LABEL(lbl), iif((hwnd->style and BS_LEFT) andalso (hwnd->style and BS_CENTER) <> BS_CENTER, 0.0, 0.5) )
                if hFontDefault() then gtk_widget_override_font( lbl, iif(hwnd->hFont, hwnd->hFont->pfd, hFontDefault()->pfd) )
            end if
        case LK_GROUPBOX
            gtk_frame_set_label( GTK_FRAME(hwnd->inner), iif(len(s), LnxZ(s), NULL) )
        case LK_LABEL
            dim as DWSTRING m = AfxStrReplace( wszText, "&&", chr(1) )
            if (hwnd->style and SS_NOPREFIX) = 0 then m = AfxStrReplace( m, "&", "" )
            m = AfxStrReplace( m, chr(1), "&" )
            dim as string ms = m.utf8
            gtk_label_set_text( GTK_LABEL(hwnd->inner), LnxZ(ms) )
    end select
end sub


' ----------------------------------------------------------------------------
' ListView (GtkTreeView, report mode)
' ----------------------------------------------------------------------------
#define LVM_GETITEMCOUNT      (LVM_FIRST + 4)
#define LVM_DELETEITEM        (LVM_FIRST + 8)
#define LVM_DELETEALLITEMS    (LVM_FIRST + 9)
#define LVM_GETNEXTITEM       (LVM_FIRST + 12)
#define LVM_ENSUREVISIBLE     (LVM_FIRST + 19)
#define LVM_INSERTCOLUMN      (LVM_FIRST + 97)
#define LVM_SETCOLUMNWIDTH    (LVM_FIRST + 30)
#define LVM_SETITEMSTATE      (LVM_FIRST + 43)
#define LVM_GETITEMSTATE      (LVM_FIRST + 44)
#define LVM_GETITEMTEXT       (LVM_FIRST + 115)
#define LVM_SETITEMTEXT       (LVM_FIRST + 116)
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define LVM_GETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 55)
#define LVM_GETHEADER         (LVM_FIRST + 31)
#ifndef LVNI_SELECTED
#define LVNI_SELECTED         &h0002
#endif
#ifndef LVNI_FOCUSED
#define LVNI_FOCUSED          &h0001
#endif
#ifndef LVIS_SELECTED
#define LVIS_SELECTED         &h0002
#endif
#ifndef LVIS_FOCUSED
#define LVIS_FOCUSED          &h0001
#endif
#ifndef LVIF_STATE
#define LVIF_STATE            &h0008
#endif
#ifndef LVCF_FMT
#define LVCF_FMT              &h0001
#endif
#ifndef LVCF_WIDTH
#define LVCF_WIDTH            &h0002
#endif
#ifndef LVCF_TEXT
#define LVCF_TEXT             &h0004
#endif
#ifndef LVCFMT_LEFT
#define LVCFMT_LEFT           0
#endif
#ifndef LVCFMT_RIGHT
#define LVCFMT_RIGHT          1
#endif
#ifndef LVCFMT_CENTER
#define LVCFMT_CENTER         2
#endif
#ifndef LV_MAXCOLS
#define LV_MAXCOLS            16
#endif

type LVITEMW
    mask       as UINT
    iItem      as long
    iSubItem   as long
    state      as UINT
    stateMask  as UINT
    pszText    as LPWSTR
    cchTextMax as long
    iImage     as long
    lParam     as LPARAM
    iIndent    as long
end type
type LVITEM as LVITEMW
type LV_ITEM as LVITEMW

type LVCOLUMNW
    mask     as UINT
    fmt      as long
    cx       as long
    pszText  as LPWSTR
    cchTextMax as long
    iSubItem as long
end type
type LVCOLUMN as LVCOLUMNW

type NMLISTVIEW
    hdr       as NMHDR
    iItem     as long
    iSubItem  as long
    uNewState as UINT
    uOldState as UINT
    uChanged  as UINT
    ptAction  as POINT
    lParam    as LPARAM
end type
type LPNMLISTVIEW as NMLISTVIEW ptr

type NMITEMACTIVATE
    hdr       as NMHDR
    iItem     as long
    iSubItem  as long
    uNewState as UINT
    uOldState as UINT
    uChanged  as UINT
    ptAction  as POINT
    lParam    as LPARAM
    uKeyFlags as UINT
end type

private function LnxLvStore( byval hwnd as HWND ) as GtkListStore ptr
    return GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW(hwnd->inner) ) )
end function

private function LnxLvSelected( byval hwnd as HWND ) as long
    dim as GtkTreeSelection ptr sel = gtk_tree_view_get_selection( GTK_TREE_VIEW(hwnd->inner) )
    dim as GtkTreeModel ptr model
    dim as GList ptr rows = gtk_tree_selection_get_selected_rows( sel, @model )
    dim as long idx = -1
    if rows then
        idx = gtk_tree_path_get_indices( cast(GtkTreePath ptr, rows->data) )[0]
        g_list_free_full( rows, cast(GDestroyNotify, @gtk_tree_path_free) )
    end if
    return idx
end function

private sub LnxOnLvSelChanged cdecl ( byval sel as GtkTreeSelection ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->blockSignals > 0 orelse hwnd->parent = 0 then exit sub
    dim as NMLISTVIEW nm
    nm.hdr.hwndFrom = hwnd : nm.hdr.idFrom = hwnd->id : nm.hdr.code = LVN_ITEMCHANGED
    nm.iItem = LnxLvSelected( hwnd )
    nm.uNewState = LVIS_SELECTED or LVIS_FOCUSED
    nm.uChanged = LVIF_STATE
    SendMessage( hwnd->parent, WM_NOTIFY, hwnd->id, cast(LPARAM, @nm) )
end sub

private sub LnxOnLvRowActivated cdecl ( byval tv as GtkTreeView ptr, byval path as GtkTreePath ptr, byval col as any ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->parent = 0 then exit sub
    dim as NMITEMACTIVATE nm
    nm.hdr.hwndFrom = hwnd : nm.hdr.idFrom = hwnd->id : nm.hdr.code = NM_DBLCLK
    nm.iItem = gtk_tree_path_get_indices( path )[0]
    SendMessage( hwnd->parent, WM_NOTIFY, hwnd->id, cast(LPARAM, @nm) )
end sub

private function LnxListViewProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                                  byref handled as boolean ) as LRESULT
    dim as GtkTreeView ptr tv = GTK_TREE_VIEW(hwnd->inner)
    dim as GtkListStore ptr store = LnxLvStore(hwnd)
    handled = true
    select case uMsg
        case LVM_INSERTCOLUMN
            dim as LVCOLUMNW ptr pc = cast(LVCOLUMNW ptr, lParam)
            dim as long ncol = gtk_tree_view_get_n_columns( tv )
            if ncol >= LV_MAXCOLS then return -1
            dim as GtkCellRenderer ptr rend = gtk_cell_renderer_text_new()
            g_object_set( G_OBJECT(rend), "ypad", 0, "ellipsize", PANGO_ELLIPSIZE_END, NULL )
            if hwnd->hFont then g_object_set( G_OBJECT(rend), "font-desc", hwnd->hFont->pfd, NULL )
            dim as string title = iif(pc andalso pc->pszText, WideToUtf8(pc->pszText), "")
            dim as GtkTreeViewColumn ptr col = gtk_tree_view_column_new_with_attributes( LnxZ(title), rend, "text", ncol, NULL )
            gtk_tree_view_column_set_sizing( col, GTK_TREE_VIEW_COLUMN_FIXED )
            if pc then gtk_tree_view_column_set_fixed_width( col, iif(pc->cx > 0, pc->cx, 1) )
            if pc andalso pc->cx <= 0 then gtk_tree_view_column_set_visible( col, FALSE )
            gtk_tree_view_column_set_resizable( col, TRUE )
            if pc andalso (pc->mask and LVCF_FMT) then
                if pc->fmt = LVCFMT_RIGHT then g_object_set( G_OBJECT(rend), "xalign", 1.0, NULL )
                if pc->fmt = LVCFMT_CENTER then g_object_set( G_OBJECT(rend), "xalign", 0.5, NULL )
            end if
            gtk_tree_view_append_column( tv, col )
            return ncol
        case LVM_SETCOLUMNWIDTH
            dim as GtkTreeViewColumn ptr col = gtk_tree_view_get_column( tv, wParam )
            if col then
                gtk_tree_view_column_set_fixed_width( col, iif(cast(long, lParam) > 0, lParam, 1) )
                gtk_tree_view_column_set_visible( col, iif(cast(long, lParam) > 0, TRUE, FALSE) )
            end if
            return 1
        case LVM_INSERTITEM
            dim as LVITEMW ptr pi = cast(LVITEMW ptr, lParam)
            if pi = 0 then return -1
            dim as GtkTreeIter it
            dim as long n = gtk_tree_model_iter_n_children( GTK_TREE_MODEL(store), NULL )
            dim as long idx = iif(pi->iItem < 0 orelse pi->iItem > n, n, pi->iItem)
            hwnd->blockSignals += 1
            gtk_list_store_insert( store, @it, idx )
            if (pi->mask and LVIF_TEXT) andalso pi->pszText then
                dim as string s = WideToUtf8( pi->pszText )
                gtk_list_store_set( store, @it, 0, LnxZ(s), -1 )
            end if
            if (pi->mask and LVIF_PARAM) then gtk_list_store_set( store, @it, LV_MAXCOLS, cast(gint64, pi->lParam), -1 )
            hwnd->blockSignals -= 1
            return idx
        case LVM_SETITEM, LVM_SETITEMTEXT
            dim as LVITEMW ptr pi = cast(LVITEMW ptr, lParam)
            if pi = 0 then return 0
            dim as long idx = iif(uMsg = LVM_SETITEMTEXT, cast(long, wParam), pi->iItem)
            dim as GtkTreeIter it
            if gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(store), @it, NULL, idx ) = 0 then return 0
            if (uMsg = LVM_SETITEMTEXT orelse (pi->mask and LVIF_TEXT)) andalso pi->iSubItem < LV_MAXCOLS then
                dim as string s = iif(pi->pszText, WideToUtf8(pi->pszText), "")
                gtk_list_store_set( store, @it, pi->iSubItem, LnxZ(s), -1 )
            end if
            if uMsg = LVM_SETITEM andalso (pi->mask and LVIF_PARAM) then gtk_list_store_set( store, @it, LV_MAXCOLS, cast(gint64, pi->lParam), -1 )
            return 1
        case LVM_GETITEM, LVM_GETITEMTEXT
            dim as LVITEMW ptr pi = cast(LVITEMW ptr, lParam)
            if pi = 0 then return 0
            dim as long idx = iif(uMsg = LVM_GETITEMTEXT, cast(long, wParam), pi->iItem)
            dim as GtkTreeIter it
            if gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(store), @it, NULL, idx ) = 0 then return 0
            if (uMsg = LVM_GETITEMTEXT orelse (pi->mask and LVIF_TEXT)) andalso pi->pszText andalso pi->cchTextMax > 0 then
                dim as zstring ptr p
                gtk_tree_model_get( GTK_TREE_MODEL(store), @it, pi->iSubItem, @p, -1 )
                dim as DWSTRING dws = iif(p, *p, "")
                if p then g_free( p )
                dim as long n = len(dws)
                if n > pi->cchTextMax - 1 then n = pi->cchTextMax - 1
                memcpy( pi->pszText, dws.vptr, n * sizeof(wstring) )
                cast(ulong ptr, pi->pszText)[n] = 0
                if uMsg = LVM_GETITEMTEXT then return n
            end if
            if uMsg = LVM_GETITEM andalso (pi->mask and LVIF_PARAM) then
                dim as gint64 v
                gtk_tree_model_get( GTK_TREE_MODEL(store), @it, LV_MAXCOLS, @v, -1 )
                pi->lParam = v
            end if
            return 1
        case LVM_GETITEMCOUNT
            return gtk_tree_model_iter_n_children( GTK_TREE_MODEL(store), NULL )
        case LVM_DELETEITEM
            dim as GtkTreeIter it
            if gtk_tree_model_iter_nth_child( GTK_TREE_MODEL(store), @it, NULL, wParam ) = 0 then return 0
            hwnd->blockSignals += 1
            gtk_list_store_remove( store, @it )
            hwnd->blockSignals -= 1
            return 1
        case LVM_DELETEALLITEMS
            hwnd->blockSignals += 1
            gtk_list_store_clear( store )
            hwnd->blockSignals -= 1
            return 1
        case LVM_GETNEXTITEM
            dim as long sel = LnxLvSelected( hwnd )
            if (lParam and (LVNI_SELECTED or LVNI_FOCUSED)) then
                if sel > cast(long, wParam) then return sel
                return -1
            end if
            return iif(cast(long, wParam) + 1 < gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL), wParam + 1, -1)
        case LVM_SETITEMSTATE
            dim as LVITEMW ptr pi = cast(LVITEMW ptr, lParam)
            dim as GtkTreeSelection ptr sel = gtk_tree_view_get_selection( tv )
            if pi = 0 then return 0
            if cast(long, wParam) = -1 then
                if (pi->state and LVIS_SELECTED) then gtk_tree_selection_select_all( sel ) else gtk_tree_selection_unselect_all( sel )
                return 1
            end if
            dim as GtkTreePath ptr path = gtk_tree_path_new_from_indices( wParam, -1 )
            if (pi->stateMask and LVIS_SELECTED) then
                if (pi->state and LVIS_SELECTED) then
                    gtk_tree_selection_select_path( sel, path )
                    gtk_tree_view_set_cursor( tv, path, NULL, FALSE )
                else
                    gtk_tree_selection_unselect_path( sel, path )
                end if
            end if
            gtk_tree_path_free( path )
            return 1
        case LVM_GETITEMSTATE
            dim as long sel = LnxLvSelected( hwnd )
            return iif(sel = cast(long, wParam), LVIS_SELECTED or LVIS_FOCUSED, 0) and lParam
        case LVM_ENSUREVISIBLE
            dim as GtkTreePath ptr path = gtk_tree_path_new_from_indices( wParam, -1 )
            gtk_tree_view_scroll_to_cell( tv, path, NULL, FALSE, 0, 0 )
            gtk_tree_path_free( path )
            return 1
        case LVM_SETEXTENDEDLISTVIEWSTYLE
            dim as long prev = hwnd->userData
            hwnd->userData = (hwnd->userData and not wParam) or (lParam and iif(wParam, wParam, -1))
            gtk_tree_view_set_grid_lines( tv, iif(hwnd->userData and LVS_EX_GRIDLINES, GTK_TREE_VIEW_GRID_LINES_BOTH, GTK_TREE_VIEW_GRID_LINES_NONE) )
            return prev
        case LVM_GETEXTENDEDLISTVIEWSTYLE
            return hwnd->userData
        case LVM_GETHEADER
            return 0
    end select
    handled = false
    return 0
end function


' ----------------------------------------------------------------------------
' TreeView (GtkTreeView + GtkTreeStore). HTREEITEM = GtkTreeRowReference ptr
' ----------------------------------------------------------------------------
#ifndef TV_FIRST
#define TV_FIRST            &h1100
#endif
#define TVM_INSERTITEM      (TV_FIRST + 50)
#define TVM_DELETEITEM      (TV_FIRST + 1)
#define TVM_EXPAND          (TV_FIRST + 2)
#define TVM_GETNEXTITEM     (TV_FIRST + 10)
#define TVM_SELECTITEM      (TV_FIRST + 11)
#define TVM_GETITEM         (TV_FIRST + 62)
#define TVM_SETITEM         (TV_FIRST + 63)
#define TVM_GETCOUNT        (TV_FIRST + 5)
#ifndef TVI_FIRST
#define TVI_FIRST           (cast(HTREEITEM, cast(ULONG_PTR, -&hFFFF)))
#endif
#ifndef TVI_LAST
#define TVI_LAST            (cast(HTREEITEM, cast(ULONG_PTR, -&hFFFE)))
#endif
#ifndef TVGN_ROOT
#define TVGN_ROOT           &h0000
#endif
#ifndef TVGN_CARET
#define TVGN_CARET          &h0009
#endif
#ifndef TVGN_CHILD
#define TVGN_CHILD          &h0004
#endif
#ifndef TVGN_NEXT
#define TVGN_NEXT           &h0001
#endif
#ifndef TVE_EXPAND
#define TVE_EXPAND          &h0002
#endif
#ifndef TVE_COLLAPSE
#define TVE_COLLAPSE        &h0001
#endif
#ifndef TVIF_TEXT
#define TVIF_TEXT           &h0001
#endif
#ifndef TVIF_PARAM
#define TVIF_PARAM          &h0004
#endif
#ifndef TVIF_HANDLE
#define TVIF_HANDLE         &h0010
#endif

type TVITEMW
    mask           as UINT
    hItem          as HTREEITEM
    state          as UINT
    stateMask      as UINT
    pszText        as LPWSTR
    cchTextMax     as long
    iImage         as long
    iSelectedImage as long
    cChildren      as long
    lParam         as LPARAM
end type
type TVITEM as TVITEMW

type TVINSERTSTRUCTW
    hParent      as HTREEITEM
    hInsertAfter as HTREEITEM
    item         as TVITEMW
end type
type TVINSERTSTRUCT as TVINSERTSTRUCTW

type NMTREEVIEWW
    hdr     as NMHDR
    action  as UINT
    itemOld as TVITEMW
    itemNew as TVITEMW
    ptDrag  as POINT
end type
type NMTREEVIEW as NMTREEVIEWW
type NM_TREEVIEW as NMTREEVIEWW
type LPNMTREEVIEW as NMTREEVIEWW ptr

private function LnxTvIter( byval hwnd as HWND, byval hItem as HTREEITEM, byval it as GtkTreeIter ptr ) as boolean
    if hItem = 0 orelse hItem = TVI_ROOT then return false
    dim as GtkTreePath ptr path = gtk_tree_row_reference_get_path( cast(GtkTreeRowReference ptr, hItem) )
    if path = 0 then return false
    dim as boolean ok = gtk_tree_model_get_iter( gtk_tree_view_get_model(GTK_TREE_VIEW(hwnd->inner)), it, path ) <> 0
    gtk_tree_path_free( path )
    return ok
end function

private function LnxTvRef( byval hwnd as HWND, byval it as GtkTreeIter ptr ) as HTREEITEM
    dim as GtkTreeModel ptr model = gtk_tree_view_get_model( GTK_TREE_VIEW(hwnd->inner) )
    dim as GtkTreePath ptr path = gtk_tree_model_get_path( model, it )
    dim as GtkTreeRowReference ptr ref = gtk_tree_row_reference_new( model, path )
    gtk_tree_path_free( path )
    ' keep the reference in the row so it is unique per row
    dim as gpointer existing
    gtk_tree_model_get( model, it, 2, @existing, -1 )
    if existing then
        gtk_tree_row_reference_free( ref )
        return existing
    end if
    gtk_tree_store_set( GTK_TREE_STORE(model), it, 2, ref, -1 )
    return cast(HTREEITEM, ref)
end function

private sub LnxOnTvSelChanged cdecl ( byval sel as GtkTreeSelection ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->blockSignals > 0 orelse hwnd->parent = 0 then exit sub
    dim as GtkTreeModel ptr model
    dim as GtkTreeIter it
    dim as NMTREEVIEWW nm
    nm.hdr.hwndFrom = hwnd : nm.hdr.idFrom = hwnd->id : nm.hdr.code = TVN_SELCHANGED
    if gtk_tree_selection_get_selected( sel, @model, @it ) then
        nm.itemNew.hItem = LnxTvRef( hwnd, @it )
        dim as gint64 v
        gtk_tree_model_get( model, @it, 1, @v, -1 )
        nm.itemNew.lParam = v
        nm.itemNew.mask = TVIF_HANDLE or TVIF_PARAM
    end if
    SendMessage( hwnd->parent, WM_NOTIFY, hwnd->id, cast(LPARAM, @nm) )
end sub

private function LnxTreeViewProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                                  byref handled as boolean ) as LRESULT
    dim as GtkTreeView ptr tv = GTK_TREE_VIEW(hwnd->inner)
    dim as GtkTreeStore ptr store = GTK_TREE_STORE( gtk_tree_view_get_model(tv) )
    handled = true
    select case uMsg
        case TVM_INSERTITEM
            dim as TVINSERTSTRUCTW ptr pis = cast(TVINSERTSTRUCTW ptr, lParam)
            if pis = 0 then return 0
            dim as GtkTreeIter itParent, it
            dim as boolean hasParent = LnxTvIter( hwnd, pis->hParent, @itParent )
            hwnd->blockSignals += 1
            if pis->hInsertAfter = TVI_FIRST then
                gtk_tree_store_prepend( store, @it, iif(hasParent, @itParent, NULL) )
            else
                gtk_tree_store_append( store, @it, iif(hasParent, @itParent, NULL) )
            end if
            dim as string s = iif(pis->item.pszText, WideToUtf8(pis->item.pszText), "")
            gtk_tree_store_set( store, @it, 0, LnxZ(s), 1, cast(gint64, pis->item.lParam), 2, NULL, -1 )
            hwnd->blockSignals -= 1
            return cast(LRESULT, LnxTvRef( hwnd, @it ))
        case TVM_DELETEITEM
            dim as GtkTreeIter it
            hwnd->blockSignals += 1
            if lParam = 0 orelse cast(HTREEITEM, lParam) = TVI_ROOT then
                gtk_tree_store_clear( store )
            elseif LnxTvIter( hwnd, cast(HTREEITEM, lParam), @it ) then
                gtk_tree_store_remove( store, @it )
            end if
            hwnd->blockSignals -= 1
            return 1
        case TVM_EXPAND
            dim as GtkTreeIter it
            if LnxTvIter( hwnd, cast(HTREEITEM, lParam), @it ) then
                dim as GtkTreePath ptr path = gtk_tree_model_get_path( GTK_TREE_MODEL(store), @it )
                if (wParam and TVE_EXPAND) then gtk_tree_view_expand_row( tv, path, FALSE ) else gtk_tree_view_collapse_row( tv, path )
                gtk_tree_path_free( path )
            end if
            return 1
        case TVM_SELECTITEM
            dim as GtkTreeIter it
            if LnxTvIter( hwnd, cast(HTREEITEM, lParam), @it ) then
                dim as GtkTreePath ptr path = gtk_tree_model_get_path( GTK_TREE_MODEL(store), @it )
                gtk_tree_view_expand_to_path( tv, path )
                gtk_tree_view_set_cursor( tv, path, NULL, FALSE )
                gtk_tree_view_scroll_to_cell( tv, path, NULL, FALSE, 0, 0 )
                gtk_tree_path_free( path )
            end if
            return 1
        case TVM_GETITEM
            dim as TVITEMW ptr pi = cast(TVITEMW ptr, lParam)
            if pi = 0 then return 0
            dim as GtkTreeIter it
            if LnxTvIter( hwnd, pi->hItem, @it ) = false then return 0
            if (pi->mask and TVIF_PARAM) then
                dim as gint64 v
                gtk_tree_model_get( GTK_TREE_MODEL(store), @it, 1, @v, -1 )
                pi->lParam = v
            end if
            if (pi->mask and TVIF_TEXT) andalso pi->pszText andalso pi->cchTextMax > 0 then
                dim as zstring ptr p
                gtk_tree_model_get( GTK_TREE_MODEL(store), @it, 0, @p, -1 )
                dim as DWSTRING dws = iif(p, *p, "")
                if p then g_free( p )
                dim as long n = len(dws)
                if n > pi->cchTextMax - 1 then n = pi->cchTextMax - 1
                memcpy( pi->pszText, dws.vptr, n * sizeof(wstring) )
                cast(ulong ptr, pi->pszText)[n] = 0
            end if
            return 1
        case TVM_GETNEXTITEM
            dim as GtkTreeIter it
            select case wParam
                case TVGN_CARET
                    dim as GtkTreeModel ptr model
                    if gtk_tree_selection_get_selected( gtk_tree_view_get_selection(tv), @model, @it ) then return cast(LRESULT, LnxTvRef(hwnd, @it))
                case TVGN_ROOT
                    if gtk_tree_model_get_iter_first( GTK_TREE_MODEL(store), @it ) then return cast(LRESULT, LnxTvRef(hwnd, @it))
                case TVGN_CHILD
                    dim as GtkTreeIter p
                    if LnxTvIter( hwnd, cast(HTREEITEM, lParam), @p ) andalso gtk_tree_model_iter_children( GTK_TREE_MODEL(store), @it, @p ) then
                        return cast(LRESULT, LnxTvRef(hwnd, @it))
                    end if
                case TVGN_NEXT
                    if LnxTvIter( hwnd, cast(HTREEITEM, lParam), @it ) andalso gtk_tree_model_iter_next( GTK_TREE_MODEL(store), @it ) then
                        return cast(LRESULT, LnxTvRef(hwnd, @it))
                    end if
            end select
            return 0
        case TVM_GETCOUNT
            return gtk_tree_model_iter_n_children( GTK_TREE_MODEL(store), NULL )
    end select
    handled = false
    return 0
end function


' ----------------------------------------------------------------------------
' Scintilla notifications: "sci-notify" -> WM_NOTIFY, "command" -> WM_COMMAND (like Win32)
' ----------------------------------------------------------------------------
private sub LnxOnSciNotify cdecl ( byval w as GtkWidget ptr, byval param as gint, byval notification as any ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->parent = 0 orelse notification = 0 then exit sub
    ' The notification header (hwndFrom, idFrom, code) has the same layout as NMHDR;
    ' hwndFrom must identify our HWND, not the GTK widget.
    dim as NMHDR ptr hdr = notification
    dim as any ptr savedFrom = hdr->hwndFrom
    hdr->hwndFrom = hwnd
    hdr->idFrom = hwnd->id
    SendMessage( hwnd->parent, WM_NOTIFY, hwnd->id, cast(LPARAM, notification) )
    hdr->hwndFrom = savedFrom
end sub

private sub LnxOnSciCommand cdecl ( byval w as GtkWidget ptr, byval wParam as gint, byval lParam as GtkWidget ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->parent = 0 then exit sub
    ' wParam = MAKEWPARAM(id, SCEN_*)
    SendMessage( hwnd->parent, WM_COMMAND, MAKEWPARAM(hwnd->id, HIWORD(wParam)), cast(LPARAM, hwnd) )
end sub


' ----------------------------------------------------------------------------
' Control creation
' ----------------------------------------------------------------------------
private function LnxKindFromClass( byref wszClass as const wstring, byval dwStyle as DWORD ) as LNX_KIND
    select case ucase(wszClass)
        case "BUTTON"
            select case (dwStyle and &hF)
                case BS_AUTOCHECKBOX, &h2, &h5, &h6: return LK_CHECKBOX       ' BS_CHECKBOX, BS_3STATE, BS_AUTO3STATE
                case BS_AUTORADIOBUTTON, &h4: return LK_RADIOBUTTON           ' BS_RADIOBUTTON
                case BS_GROUPBOX: return LK_GROUPBOX
            end select
            return LK_BUTTON
        case "OPTION", "RADIOBUTTON": return LK_RADIOBUTTON
        case "CHECKBOX", "CHECK3STATE": return LK_CHECKBOX
        case "GROUPBOX", "FRAME": return LK_GROUPBOX
        case "LABEL", "STATIC": return LK_LABEL
        case "TEXTBOX", "EDIT", "EDITMULTILINE", "MULTILINETEXTBOX"
            if (dwStyle and ES_MULTILINE) orelse ucase(wszClass) = "EDITMULTILINE" orelse ucase(wszClass) = "MULTILINETEXTBOX" then return LK_EDITMULTI
            return LK_TEXTBOX
        case "COMBOBOX", "COMBOBOXEX", "COMBOBOXEX32": return LK_COMBOBOX
        case "LISTBOX": return LK_LISTBOX
        case "LISTVIEW", "SYSLISTVIEW32": return LK_LISTVIEW
        case "TREEVIEW", "SYSTREEVIEW32": return LK_TREEVIEW
        case "RICHEDIT", "RICHEDIT50W", "RICHEDIT20W", "RICHEDIT_CLASS": return LK_RICHEDIT
        case "SCINTILLA": return LK_SCINTILLA
    end select
    return LK_CUSTOM
end function

private function LnxCreateControl( byref wszClass as const wstring, byval hParent as HWND, byval id as LONG_PTR, _
            byref wszTitle as const wstring, byval x as long, byval y as long, byval nWidth as long, byval nHeight as long, _
            byval dwStyle as DWORD, byval dwExStyle as DWORD ) as HWND

    LnxInstallControlCss()
    dim as LNX_KIND kind = LnxKindFromClass( wszClass, dwStyle )
    if kind = LK_CUSTOM then
        return LnxCreateWindow( LK_CUSTOM, wszClass, wszTitle, dwStyle or WS_CHILD, dwExStyle, x, y, nWidth, nHeight, hParent, id, NULL, NULL )
    end if

    dim as HWND hwnd = LnxNewWindow( kind, hParent )
    hwnd->classname = wszClass
    hwnd->text = wszTitle
    hwnd->style = dwStyle or WS_CHILD
    hwnd->exstyle = dwExStyle
    hwnd->id = id
    hwnd->visible = (dwStyle and WS_VISIBLE) <> 0
    hwnd->enabled = (dwStyle and WS_DISABLED) = 0
    hwnd->x = x : hwnd->y = y : hwnd->w = nWidth : hwnd->h = nHeight
    hwnd->wndproc = @LnxControlProc
    hwnd->hFont = hFontDefault()

    ' Win32 combobox heights include the drop down list; only the edit part is visible.
    if kind = LK_COMBOBOX andalso hwnd->h > 30 then hwnd->h = 23

    dim as GtkWidget ptr inner, outer
    select case kind
        case LK_BUTTON
            inner = gtk_button_new()
            if (dwStyle and &hF) = BS_DEFPUSHBUTTON then LnxAddClass( inner, "default" )
            g_signal_connect( G_OBJECT(inner), "clicked", G_CALLBACK(@LnxOnButtonClicked), hwnd )
        case LK_CHECKBOX
            inner = gtk_check_button_new()
            g_signal_connect( G_OBJECT(inner), "toggled", G_CALLBACK(@LnxOnToggled), hwnd )
        case LK_RADIOBUTTON
            ' radio groups: WS_GROUP starts a new group (Win32 semantics)
            static as GtkWidget ptr lastRadio
            static as HWND lastRadioParent
            if (dwStyle and WS_GROUP) orelse lastRadioParent <> hParent orelse lastRadio = 0 then
                inner = gtk_radio_button_new( NULL )
            else
                inner = gtk_radio_button_new_from_widget( GTK_RADIO_BUTTON(lastRadio) )
            end if
            lastRadio = inner : lastRadioParent = hParent
            g_signal_connect( G_OBJECT(inner), "toggled", G_CALLBACK(@LnxOnToggled), hwnd )
        case LK_GROUPBOX
            inner = gtk_frame_new( NULL )
            gtk_frame_set_label_align( GTK_FRAME(inner), 0.02, 0.5 )
        case LK_LABEL
            inner = gtk_label_new( NULL )
            gtk_label_set_ellipsize( GTK_LABEL(inner), PANGO_ELLIPSIZE_NONE )
            gtk_label_set_line_wrap( GTK_LABEL(inner), TRUE )
            gtk_label_set_line_wrap_mode( GTK_LABEL(inner), PANGO_WRAP_WORD_CHAR )
            gtk_label_set_xalign( GTK_LABEL(inner), iif((dwStyle and &hF) = SS_RIGHT, 1.0, iif((dwStyle and &hF) = SS_CENTER, 0.5, 0.0)) )
            gtk_label_set_yalign( GTK_LABEL(inner), iif((dwStyle and SS_CENTERIMAGE), 0.5, 0.0) )
            if (dwStyle and &hF) = SS_RIGHT then gtk_label_set_justify( GTK_LABEL(inner), GTK_JUSTIFY_RIGHT )
            if (dwStyle and &hF) = SS_CENTER then gtk_label_set_justify( GTK_LABEL(inner), GTK_JUSTIFY_CENTER )
            ' event box so labels can be clicked (SS_NOTIFY)
            outer = gtk_event_box_new()
            gtk_event_box_set_visible_window( GTK_EVENT_BOX(outer), FALSE )
            gtk_container_add( GTK_CONTAINER(outer), inner )
            g_signal_connect( G_OBJECT(outer), "button-press-event", G_CALLBACK(@LnxOnLabelClick), hwnd )
            g_signal_connect( G_OBJECT(outer), "draw", G_CALLBACK(@LnxOnLabelDraw), hwnd )
        case LK_TEXTBOX
            inner = gtk_entry_new()
            gtk_entry_set_width_chars( GTK_ENTRY(inner), 1 )
            gtk_entry_set_has_frame( GTK_ENTRY(inner), iif((dwExStyle and WS_EX_CLIENTEDGE) orelse (dwStyle and WS_BORDER), TRUE, FALSE) )
            if (dwStyle and ES_READONLY) then gtk_editable_set_editable( GTK_EDITABLE(inner), FALSE )
            if (dwStyle and ES_PASSWORD) then gtk_entry_set_visibility( GTK_ENTRY(inner), FALSE )
            if (dwStyle and ES_NUMBER) then gtk_entry_set_input_purpose( GTK_ENTRY(inner), GTK_INPUT_PURPOSE_DIGITS )
            if (dwStyle and ES_RIGHT) then gtk_entry_set_alignment( GTK_ENTRY(inner), 1.0 )
            if (dwStyle and ES_CENTER) then gtk_entry_set_alignment( GTK_ENTRY(inner), 0.5 )
            g_signal_connect( G_OBJECT(inner), "changed", G_CALLBACK(@LnxOnEntryChanged), hwnd )
        case LK_EDITMULTI, LK_RICHEDIT
            inner = gtk_text_view_new()
            gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(inner), iif((dwStyle and ES_AUTOHSCROLL) andalso kind = LK_EDITMULTI, GTK_WRAP_NONE, GTK_WRAP_WORD_CHAR) )
            gtk_text_view_set_left_margin( GTK_TEXT_VIEW(inner), 3 )
            gtk_text_view_set_right_margin( GTK_TEXT_VIEW(inner), 3 )
            if (dwStyle and ES_READONLY) then gtk_text_view_set_editable( GTK_TEXT_VIEW(inner), FALSE )
            outer = gtk_scrolled_window_new( NULL, NULL )
            gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(outer), _
                iif((dwStyle and WS_HSCROLL), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER), _
                iif((dwStyle and WS_VSCROLL), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER) )
            if (dwExStyle and WS_EX_CLIENTEDGE) then LnxAddClass( outer, "lnxedge" )
            gtk_container_add( GTK_CONTAINER(outer), inner )
            g_signal_connect( G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(inner))), "changed", G_CALLBACK(@LnxOnEntryChanged), hwnd )
        case LK_COMBOBOX
            if (dwStyle and &h3) = CBS_DROPDOWNLIST then
                inner = gtk_combo_box_text_new()
            else
                inner = gtk_combo_box_text_new_with_entry()
                dim as GtkWidget ptr e = gtk_bin_get_child( GTK_BIN(inner) )
                gtk_entry_set_width_chars( GTK_ENTRY(e), 1 )
                LnxAddClass( e, "lnxctl" )
                g_signal_connect( G_OBJECT(e), "changed", G_CALLBACK(@LnxOnEntryChanged), hwnd )
                g_signal_connect( G_OBJECT(e), "focus-in-event", G_CALLBACK(@LnxOnCtlFocusIn), hwnd )
                g_signal_connect( G_OBJECT(e), "focus-out-event", G_CALLBACK(@LnxOnCtlFocusOut), hwnd )
                g_object_set_data( G_OBJECT(e), "lnxhwnd", hwnd )
            end if
            ' keep the popup a simple list like Windows
            dim as GList ptr cells = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT(inner) )
            if cells then
                g_object_set( G_OBJECT(cells->data), "ellipsize", PANGO_ELLIPSIZE_END, NULL )
                g_list_free( cells )
            end if
            g_signal_connect( G_OBJECT(inner), "changed", G_CALLBACK(@LnxOnComboChanged), hwnd )
            g_signal_connect( G_OBJECT(inner), "notify::popup-shown", G_CALLBACK(@LnxOnComboPopup), hwnd )
        case LK_LISTBOX
            hwnd->client = LnxCreateClientLayout( hwnd )
            hwnd->widget = hwnd->client
            gtk_widget_set_can_focus( hwnd->client, TRUE )
            LnxPlaceInParent( hwnd )
            LnxLbMeasure( hwnd )
            SendMessage( hwnd, WM_SIZE, 0, MAKELPARAM(nWidth, nHeight) )
            return hwnd
        case LK_LISTVIEW
            dim as GType types(LV_MAXCOLS)
            for i as long = 0 to LV_MAXCOLS - 1 : types(i) = G_TYPE_STRING : next
            types(LV_MAXCOLS) = G_TYPE_INT64
            dim as GtkListStore ptr store = gtk_list_store_newv( LV_MAXCOLS + 1, @types(0) )
            inner = gtk_tree_view_new_with_model( GTK_TREE_MODEL(store) )
            g_object_unref( store )
            gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(inner), iif((dwStyle and &h4000), FALSE, TRUE) )  ' LVS_NOCOLUMNHEADER
            gtk_tree_view_set_enable_search( GTK_TREE_VIEW(inner), FALSE )
            g_signal_connect( G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(inner))), "changed", G_CALLBACK(@LnxOnLvSelChanged), hwnd )
            g_signal_connect( G_OBJECT(inner), "row-activated", G_CALLBACK(@LnxOnLvRowActivated), hwnd )
            outer = gtk_scrolled_window_new( NULL, NULL )
            gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(outer), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC )
            if (dwExStyle and WS_EX_CLIENTEDGE) then LnxAddClass( outer, "lnxedge" )
            gtk_container_add( GTK_CONTAINER(outer), inner )
        case LK_TREEVIEW
            dim as GtkTreeStore ptr store = gtk_tree_store_new( 3, G_TYPE_STRING, G_TYPE_INT64, G_TYPE_POINTER )
            inner = gtk_tree_view_new_with_model( GTK_TREE_MODEL(store) )
            g_object_unref( store )
            gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(inner), FALSE )
            gtk_tree_view_set_enable_search( GTK_TREE_VIEW(inner), FALSE )
            gtk_tree_view_set_enable_tree_lines( GTK_TREE_VIEW(inner), iif((dwStyle and TVS_HASLINES), TRUE, FALSE) )
            dim as GtkCellRenderer ptr rend = gtk_cell_renderer_text_new()
            g_object_set( G_OBJECT(rend), "ypad", 0, NULL )
            gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW(inner), -1, "", rend, "text", 0, NULL )
            g_signal_connect( G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(inner))), "changed", G_CALLBACK(@LnxOnTvSelChanged), hwnd )
            outer = gtk_scrolled_window_new( NULL, NULL )
            gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(outer), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC )
            if (dwExStyle and WS_EX_CLIENTEDGE) then LnxAddClass( outer, "lnxedge" )
            gtk_container_add( GTK_CONTAINER(outer), inner )
        case LK_SCINTILLA
            inner = scintilla_new()
            scintilla_set_id( inner, id )
            g_signal_connect( G_OBJECT(inner), "sci-notify", G_CALLBACK(@LnxOnSciNotify), hwnd )
            g_signal_connect( G_OBJECT(inner), "command", G_CALLBACK(@LnxOnSciCommand), hwnd )
    end select

    hwnd->inner = inner
    hwnd->widget = iif(outer, outer, inner)
    g_object_set_data( G_OBJECT(hwnd->widget), "lnxhwnd", hwnd )
    if inner <> hwnd->widget then g_object_set_data( G_OBJECT(inner), "lnxhwnd", hwnd )
    if kind <> LK_SCINTILLA then LnxAddClass( inner, "lnxctl" )
    gtk_widget_set_can_focus( inner, iif((dwStyle and WS_TABSTOP) orelse kind = LK_TEXTBOX orelse kind = LK_EDITMULTI orelse _
                                         kind = LK_RICHEDIT orelse kind = LK_SCINTILLA orelse kind = LK_COMBOBOX, TRUE, FALSE) )
    if kind = LK_LABEL orelse kind = LK_GROUPBOX then gtk_widget_set_can_focus( inner, FALSE )
    gtk_widget_set_sensitive( hwnd->widget, iif(hwnd->enabled, TRUE, FALSE) )

    ' Mouse input for native widgets is observed (so the message loop pre-filters see it)
    LnxConnectInputSignals( hwnd, iif(kind = LK_LABEL, outer, inner), true )
    g_signal_connect( G_OBJECT(inner), "focus-in-event", G_CALLBACK(@LnxOnCtlFocusIn), hwnd )
    g_signal_connect( G_OBJECT(inner), "focus-out-event", G_CALLBACK(@LnxOnCtlFocusOut), hwnd )

    if kind <> LK_SCINTILLA then
        LnxApplyFont( hwnd, hwnd->hFont )
        hwnd->blockSignals += 1
        LnxEditSetText( hwnd, wszTitle )
        hwnd->blockSignals -= 1
    end if
    gtk_widget_show_all( hwnd->widget )
    if hwnd->visible = false then gtk_widget_hide( hwnd->widget )
    LnxPlaceInParent( hwnd )
    return hwnd
end function


' ----------------------------------------------------------------------------
' Built-in control window procedure
' ----------------------------------------------------------------------------
private function LnxControlProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if IsWindow(hwnd) = 0 then return 0
    dim as boolean handled

    select case hwnd->kind
        case LK_LISTBOX
            dim as LRESULT r = LnxListBoxProc( hwnd, uMsg, wParam, lParam, handled )
            if handled then return r
        case LK_COMBOBOX
            dim as LRESULT r = LnxComboProc( hwnd, uMsg, wParam, lParam, handled )
            if handled then return r
        case LK_LISTVIEW
            dim as LRESULT r = LnxListViewProc( hwnd, uMsg, wParam, lParam, handled )
            if handled then return r
        case LK_TREEVIEW
            dim as LRESULT r = LnxTreeViewProc( hwnd, uMsg, wParam, lParam, handled )
            if handled then return r
        case LK_SCINTILLA
            ' Scintilla messages go straight to the widget
            if uMsg >= 2000 then return scintilla_send_message( hwnd->inner, uMsg, wParam, lParam )
    end select

    select case uMsg
        case WM_KEYDOWN, WM_KEYUP, WM_CHAR, WM_SYSKEYDOWN, WM_SYSKEYUP, WM_SYSCHAR
            ' end of the procedure chain: let GTK process the original key event
            if hwnd->widget then
                dim as GdkEvent ptr ev = g_object_get_data( G_OBJECT(hwnd->widget), "lnxkeyevent" )
                if ev then
                    g_object_set_data( G_OBJECT(hwnd->widget), "lnxkeyevent", NULL )
                    LnxReinjectKey( ev )
                end if
            end if
            return 0

        case WM_SETTEXT
            hwnd->text = iif(lParam, *cast(wstring ptr, lParam), wstr(""))
            LnxEditSetText( hwnd, hwnd->text )
            return 1
        case WM_GETTEXT
            if lParam = 0 orelse wParam = 0 then return 0
            dim as DWSTRING t = LnxEditGetText( hwnd )
            dim as long n = len(t)
            if n > cast(long, wParam) - 1 then n = wParam - 1
            memcpy( cast(any ptr, lParam), t.vptr, n * sizeof(wstring) )
            cast(ulong ptr, lParam)[n] = 0
            return n
        case WM_GETTEXTLENGTH
            return len( LnxEditGetText(hwnd) )
        case WM_SETFONT
            hwnd->hFont = cast(HFONT, wParam)
            if hwnd->kind = LK_LISTBOX then
                LnxLbMeasure( hwnd )
                gtk_widget_queue_draw( hwnd->client )
            else
                LnxApplyFont( hwnd, hwnd->hFont )
            end if
            return 0
        case WM_GETFONT
            return cast(LRESULT, hwnd->hFont)

        case BM_GETCHECK
            if hwnd->kind = LK_CHECKBOX orelse hwnd->kind = LK_RADIOBUTTON then
                if gtk_toggle_button_get_inconsistent( GTK_TOGGLE_BUTTON(hwnd->inner) ) then return BST_INDETERMINATE
                return iif( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hwnd->inner)), BST_CHECKED, BST_UNCHECKED )
            end if
            return 0
        case BM_SETCHECK
            if hwnd->kind = LK_CHECKBOX orelse hwnd->kind = LK_RADIOBUTTON then
                hwnd->blockSignals += 1
                gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON(hwnd->inner), iif(wParam = BST_INDETERMINATE, TRUE, FALSE) )
                gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(hwnd->inner), iif(wParam = BST_CHECKED, TRUE, FALSE) )
                hwnd->blockSignals -= 1
            end if
            return 0
        case BM_CLICK
            if hwnd->inner andalso GTK_IS_BUTTON(hwnd->inner) then gtk_button_clicked( GTK_BUTTON(hwnd->inner) )
            return 0
        case BM_SETSTYLE
            if hwnd->kind = LK_BUTTON then
                if (wParam and &hF) = BS_DEFPUSHBUTTON then LnxAddClass( hwnd->inner, "default" ) _
                else gtk_style_context_remove_class( gtk_widget_get_style_context(hwnd->inner), "default" )
                hwnd->style = (hwnd->style and not &hF) or (wParam and &hF)
            end if
            return 0

        ' ---- edit controls ----
        case EM_SETSEL
            if hwnd->kind = LK_TEXTBOX then
                gtk_editable_select_region( GTK_EDITABLE(hwnd->inner), cast(long, wParam), cast(long, lParam) )
            elseif hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
                dim as GtkTextIter a, b
                if cast(long, wParam) = -1 then
                    gtk_text_buffer_get_iter_at_mark( buf, @a, gtk_text_buffer_get_insert(buf) )
                    gtk_text_buffer_select_range( buf, @a, @a )
                else
                    gtk_text_buffer_get_iter_at_offset( buf, @a, cast(long, wParam) )
                    gtk_text_buffer_get_iter_at_offset( buf, @b, cast(long, lParam) )
                    gtk_text_buffer_select_range( buf, @b, @a )
                end if
            elseif hwnd->kind = LK_COMBOBOX then
                dim as GtkEntry ptr e = LnxComboEntry(hwnd)
                if e then gtk_editable_select_region( GTK_EDITABLE(e), cast(long, wParam), cast(long, lParam) )
            end if
            return 0
        case EM_GETSEL
            dim as long a = 0, b = 0
            if hwnd->kind = LK_TEXTBOX then
                gtk_editable_get_selection_bounds( GTK_EDITABLE(hwnd->inner), @a, @b )
                if a = b then a = gtk_editable_get_position( GTK_EDITABLE(hwnd->inner) ) : b = a
            elseif hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
                dim as GtkTextIter ia, ib
                gtk_text_buffer_get_selection_bounds( buf, @ia, @ib )
                a = gtk_text_iter_get_offset( @ia ) : b = gtk_text_iter_get_offset( @ib )
            end if
            if wParam then *cast(DWORD ptr, wParam) = a
            if lParam then *cast(DWORD ptr, lParam) = b
            return MAKELONG(a, b)
        case EM_REPLACESEL
            dim as string s = iif(lParam, WideToUtf8(cast(wstring ptr, lParam)), "")
            if hwnd->kind = LK_TEXTBOX then
                gtk_editable_delete_selection( GTK_EDITABLE(hwnd->inner) )
                dim as gint pos_ = gtk_editable_get_position( GTK_EDITABLE(hwnd->inner) )
                gtk_editable_insert_text( GTK_EDITABLE(hwnd->inner), LnxZ(s), len(s), @pos_ )
                gtk_editable_set_position( GTK_EDITABLE(hwnd->inner), pos_ )
            elseif hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
                dim as DWSTRING dws = AfxStrReplace( *cast(wstring ptr, lParam), !"\r\n", !"\n" )
                s = dws.utf8
                gtk_text_buffer_delete_selection( buf, TRUE, TRUE )
                gtk_text_buffer_insert_at_cursor( buf, LnxZ(s), len(s) )
            end if
            return 0
        case EM_SETLIMITTEXT
            if hwnd->kind = LK_TEXTBOX then gtk_entry_set_max_length( GTK_ENTRY(hwnd->inner), iif(wParam > 65535, 0, wParam) )
            return 0
        case EM_SETREADONLY
            if hwnd->kind = LK_TEXTBOX then gtk_editable_set_editable( GTK_EDITABLE(hwnd->inner), iif(wParam, FALSE, TRUE) )
            if hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then gtk_text_view_set_editable( GTK_TEXT_VIEW(hwnd->inner), iif(wParam, FALSE, TRUE) )
            return 1
        case EM_GETLINECOUNT
            if hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                return gtk_text_buffer_get_line_count( gtk_text_view_get_buffer(GTK_TEXT_VIEW(hwnd->inner)) )
            end if
            return 1
        case EM_GETFIRSTVISIBLELINE
            if hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GdkRectangle vr
                gtk_text_view_get_visible_rect( GTK_TEXT_VIEW(hwnd->inner), @vr )
                dim as GtkTextIter it
                dim as gint lineTop
                gtk_text_view_get_line_at_y( GTK_TEXT_VIEW(hwnd->inner), @it, vr.y, @lineTop )
                return gtk_text_iter_get_line( @it )
            end if
            return 0
        case EM_LINESCROLL
            if hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GtkAdjustment ptr adj = gtk_scrollable_get_vadjustment( GTK_SCROLLABLE(hwnd->inner) )
                if adj then
                    dim as double lh = 16
                    gtk_adjustment_set_value( adj, gtk_adjustment_get_value(adj) + cast(long, lParam) * lh )
                end if
            end if
            return 1
        case EM_SCROLLCARET
            if hwnd->kind = LK_EDITMULTI orelse hwnd->kind = LK_RICHEDIT then
                dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hwnd->inner) )
                gtk_text_view_scroll_mark_onscreen( GTK_TEXT_VIEW(hwnd->inner), gtk_text_buffer_get_insert(buf) )
            end if
            return 1
        case EM_GETRECT
            if lParam then SetRect( cast(RECT ptr, lParam), 0, 0, hwnd->w, hwnd->h )
            return 0
        case EM_SETCHARFORMAT
            dim as CHARFORMATW ptr pcf = cast(CHARFORMATW ptr, lParam)
            if pcf andalso hwnd->inner then
                if (pcf->dwMask and CFM_COLOR) andalso (pcf->dwEffects and CFE_AUTOCOLOR) = 0 then
                    dim as GdkRGBA c
                    c.red = GetRValue(pcf->crTextColor) / 255 : c.green = GetGValue(pcf->crTextColor) / 255
                    c.blue = GetBValue(pcf->crTextColor) / 255 : c.alpha = 1
                    gtk_widget_override_color( hwnd->inner, GTK_STATE_FLAG_NORMAL, @c )
                end if
                if (pcf->dwMask and (CFM_FACE or CFM_SIZE)) then
                    dim as long px = iif((pcf->dwMask and CFM_SIZE) andalso pcf->yHeight > 0, pcf->yHeight \ 15, 12)
                    dim as DWSTRING face = iif((pcf->dwMask and CFM_FACE) andalso len(pcf->szFaceName), pcf->szFaceName, wstr("Segoe UI"))
                    dim as HFONT hf = LnxCreateFontObj( -px, iif(pcf->dwEffects and CFE_BOLD, 700, 400), (pcf->dwEffects and CFE_ITALIC) <> 0, false, face )
                    LnxApplyFont( hwnd, hf )
                    hwnd->hFont = hf
                end if
            end if
            return 1
        case EM_GETCHARFORMAT
            dim as CHARFORMATW ptr pcf = cast(CHARFORMATW ptr, lParam)
            if pcf then
                dim as long px = 12
                if hwnd->hFont then px = abs(hwnd->hFont->fontHeight)
                pcf->yHeight = px * 15          ' twips at 96 DPI
                pcf->dwMask = CFM_SIZE or CFM_FACE or CFM_COLOR
                if hwnd->hFont then pcf->szFaceName = hwnd->hFont->fontFace
            end if
            return 0
        case EM_SETRECT, EM_SETMARGINS, EM_SETEVENTMASK, EM_SETTARGETDEVICE, EM_SHOWSCROLLBAR, EM_SETZOOM, _
             EM_SETTYPOGRAPHYOPTIONS
            return 1
        case EM_SETBKGNDCOLOR
            if hwnd->inner andalso wParam = 0 then
                dim as GdkRGBA c
                c.red = GetRValue(lParam) / 255 : c.green = GetGValue(lParam) / 255 : c.blue = GetBValue(lParam) / 255 : c.alpha = 1
                gtk_widget_override_background_color( hwnd->inner, GTK_STATE_FLAG_NORMAL, @c )
            end if
            return 0
        case EM_CANPASTE
            return iif( gtk_clipboard_wait_is_text_available( gtk_clipboard_get(GDK_SELECTION_CLIPBOARD) ), 1, 0 )
        case EM_UNDO, WM_UNDO
            return 0
        case EM_SETCUEBANNER
            if hwnd->kind = LK_TEXTBOX andalso lParam then
                dim as string s = WideToUtf8( cast(wstring ptr, lParam) )
                gtk_entry_set_placeholder_text( GTK_ENTRY(hwnd->inner), LnxZ(s) )
            end if
            return 1
        case EM_GETCUEBANNER
            if hwnd->kind = LK_TEXTBOX andalso wParam then
                dim as DWSTRING dws = *gtk_entry_get_placeholder_text( GTK_ENTRY(hwnd->inner) )
                dim as long n = len(dws)
                if n > cast(long, lParam) - 1 then n = lParam - 1
                memcpy( cast(any ptr, wParam), dws.vptr, n * sizeof(wstring) )
                cast(ulong ptr, wParam)[n] = 0
                return 1
            end if
            return 0
        case WM_CUT, WM_COPY, WM_PASTE, WM_CLEAR
            dim as GtkWidget ptr ed = hwnd->inner
            if hwnd->kind = LK_COMBOBOX then ed = cast(GtkWidget ptr, LnxComboEntry(hwnd))
            if ed then
                if GTK_IS_EDITABLE(ed) then
                    select case uMsg
                        case WM_CUT: gtk_editable_cut_clipboard( GTK_EDITABLE(ed) )
                        case WM_COPY: gtk_editable_copy_clipboard( GTK_EDITABLE(ed) )
                        case WM_PASTE: gtk_editable_paste_clipboard( GTK_EDITABLE(ed) )
                        case WM_CLEAR: gtk_editable_delete_selection( GTK_EDITABLE(ed) )
                    end select
                elseif GTK_IS_TEXT_VIEW(ed) then
                    dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(ed) )
                    dim as GtkClipboard ptr cb = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD )
                    select case uMsg
                        case WM_CUT: gtk_text_buffer_cut_clipboard( buf, cb, gtk_text_view_get_editable(GTK_TEXT_VIEW(ed)) )
                        case WM_COPY: gtk_text_buffer_copy_clipboard( buf, cb )
                        case WM_PASTE: gtk_text_buffer_paste_clipboard( buf, cb, NULL, gtk_text_view_get_editable(GTK_TEXT_VIEW(ed)) )
                        case WM_CLEAR: gtk_text_buffer_delete_selection( buf, TRUE, TRUE )
                    end select
                end if
            end if
            return 0

        case WM_PAINT, WM_ERASEBKGND
            if hwnd->kind = LK_LABEL andalso uMsg = WM_PAINT then hwnd->lbAnchor = 1   ' let GTK draw the label
            return 0
        case WM_CONTEXTMENU
            ' passed to the parent like DefWindowProc does for child windows
            if hwnd->parent then return SendMessage( hwnd->parent, uMsg, wParam, lParam )
            return 0
        case WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, WM_RBUTTONDOWN, WM_RBUTTONUP, WM_MOUSEMOVE, WM_MOUSEWHEEL
            ' native widgets handle the mouse themselves
            return 0
    end select
    return LnxDefProcBase( hwnd, uMsg, wParam, lParam )
end function
