'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Win32 window/GDI data types used by tiko, implemented on top of GTK3.
' Only the subset of the Win32 API that tiko actually uses is provided.

#pragma once

#include once "gtk/gtk3.bi"

' ----------------------------------------------------------------------------
' Handles
' ----------------------------------------------------------------------------
type LNXWINDOW_ as LNXWINDOW
type LNXDC_ as LNXDC
type LNXGDIOBJ_ as LNXGDIOBJ
type LNXMENU_ as LNXMENU

type HWND      as LNXWINDOW ptr
type HDC       as LNXDC ptr
type HGDIOBJ   as LNXGDIOBJ ptr
type HFONT     as LNXGDIOBJ ptr
type HPEN      as LNXGDIOBJ ptr
type HBRUSH    as LNXGDIOBJ ptr
type HBITMAP   as LNXGDIOBJ ptr
type HICON     as any ptr
type HCURSOR   as any ptr
type HMENU     as LNXMENU ptr
type HACCEL    as any ptr
type HINSTANCE as any ptr
type HMODULE   as any ptr
type HGLOBAL   as any ptr
type HKL       as any ptr

type WNDPROC as function( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
type SUBCLASSPROC as function( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                               byval uIdSubclass as UINT_PTR, byval dwRefData as DWORD_PTR ) as LRESULT
type TIMERPROC as sub( byval hwnd as HWND, byval uMsg as UINT, byval idEvent as UINT_PTR, byval dwTime as DWORD )

#ifndef HWND_DESKTOP
#define HWND_DESKTOP      cast(HWND, 0)
#endif
#ifndef HWND_TOP
#define HWND_TOP          cast(HWND, 0)
#endif
#ifndef HWND_TOPMOST
#define HWND_TOPMOST      cast(HWND, -1)
#endif
#ifndef HWND_NOTOPMOST
#define HWND_NOTOPMOST    cast(HWND, -2)
#endif

' ----------------------------------------------------------------------------
' Messages and structures
' ----------------------------------------------------------------------------
type MSG
    hwnd    as HWND
    message as UINT
    wParam  as WPARAM
    lParam  as LPARAM
    time    as DWORD
    pt      as POINT
    ' --- Linux private fields ---
    lnxEvent   as GdkEvent ptr     ' original GDK event (keyboard events for native widgets)
    lnxObserve as long             ' 1 = observe only (the GTK widget handles it natively)
end type
type LPMSG as MSG ptr

type NMHDR
    hwndFrom as HWND
    idFrom   as UINT_PTR
    code     as UINT
end type
type LPNMHDR as NMHDR ptr

type PAINTSTRUCT
    hdc         as HDC
    fErase      as BOOL
    rcPaint     as RECT
    fRestore    as BOOL
    fIncUpdate  as BOOL
end type

type DRAWITEMSTRUCT
    CtlType    as UINT
    CtlID      as UINT
    itemID     as UINT
    itemAction as UINT
    itemState  as UINT
    hwndItem   as HWND
    hDC        as HDC
    rcItem     as RECT
    itemData   as ULONG_PTR
end type
type LPDRAWITEMSTRUCT as DRAWITEMSTRUCT ptr

type MEASUREITEMSTRUCT
    CtlType    as UINT
    CtlID      as UINT
    itemID     as UINT
    itemWidth  as UINT
    itemHeight as UINT
    itemData   as ULONG_PTR
end type
type LPMEASUREITEMSTRUCT as MEASUREITEMSTRUCT ptr

#ifndef ODT_MENU
#define ODT_MENU      1
#endif
#ifndef ODT_LISTBOX
#define ODT_LISTBOX   2
#endif
#ifndef ODT_COMBOBOX
#define ODT_COMBOBOX  3
#endif
#ifndef ODT_BUTTON
#define ODT_BUTTON    4
#endif
#ifndef ODS_FOCUS
#define ODS_FOCUS     &h0010
#endif

type TRACKMOUSEEVENT
    cbSize      as DWORD
    dwFlags     as DWORD
    hwndTrack   as HWND
    dwHoverTime as DWORD
end type
type LPTRACKMOUSEEVENT as TRACKMOUSEEVENT ptr
#ifndef TME_CANCEL
#define TME_CANCEL   &h80000000
#endif

type WINDOWPLACEMENT
    length           as UINT
    flags            as UINT
    showCmd          as UINT
    ptMinPosition    as POINT
    ptMaxPosition    as POINT
    rcNormalPosition as RECT
end type

type COPYDATASTRUCT
    dwData as ULONG_PTR
    cbData as DWORD
    lpData as any ptr
end type

type ACCEL
    fVirt as ubyte
    key   as WORD
    cmd   as WORD
end type
#ifndef FVIRTKEY
type LPACCEL as ACCEL ptr
#define FVIRTKEY   1
#endif
#ifndef FNOINVERT
#define FNOINVERT  2
#endif
#ifndef FSHIFT
#define FSHIFT     4
#endif
#ifndef FCONTROL
#define FCONTROL   8
#endif
#ifndef FALT
#define FALT       16
#endif

type CREATESTRUCT
    lpCreateParams as any ptr
    hInstance      as HINSTANCE
    hMenu          as HMENU
    hwndParent     as HWND
    cy             as long
    cx             as long
    y              as long
    x              as long
    style          as long
    lpszName       as LPCWSTR
    lpszClass      as LPCWSTR
    dwExStyle      as DWORD
end type
type LPCREATESTRUCT as CREATESTRUCT ptr

type TEXTMETRIC
    tmHeight           as long
    tmAscent           as long
    tmDescent          as long
    tmInternalLeading  as long
    tmExternalLeading  as long
    tmAveCharWidth     as long
    tmMaxCharWidth     as long
    tmWeight           as long
    tmOverhang         as long
    tmDigitizedAspectX as long
    tmDigitizedAspectY as long
    tmFirstChar        as wstring * 1
    tmLastChar         as wstring * 1
    tmDefaultChar      as wstring * 1
    tmBreakChar        as wstring * 1
    tmItalic           as ubyte
    tmUnderlined       as ubyte
    tmStruckOut        as ubyte
    tmPitchAndFamily   as ubyte
    tmCharSet          as ubyte
end type
type TEXTMETRICW as TEXTMETRIC

#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif
type LOGFONTW
    lfHeight         as long
    lfWidth          as long
    lfEscapement     as long
    lfOrientation    as long
    lfWeight         as long
    lfItalic         as ubyte
    lfUnderline      as ubyte
    lfStrikeOut      as ubyte
    lfCharSet        as ubyte
    lfOutPrecision   as ubyte
    lfClipPrecision  as ubyte
    lfQuality        as ubyte
    lfPitchAndFamily as ubyte
    lfFaceName       as wstring * LF_FACESIZE
end type
type LOGFONT as LOGFONTW

' ----------------------------------------------------------------------------
' Window class names (CWindow.AddControl)
' ----------------------------------------------------------------------------
enum LNX_KIND
    LK_NONE = 0
    LK_TOPLEVEL        ' top level window (CWindow.Create with no WS_CHILD)
    LK_CUSTOM          ' child window painted by its window procedure
    LK_BUTTON
    LK_CHECKBOX
    LK_RADIOBUTTON
    LK_GROUPBOX
    LK_LABEL
    LK_TEXTBOX         ' single line
    LK_EDITMULTI       ' multi line textbox
    LK_COMBOBOX
    LK_LISTBOX         ' custom drawn by the layer (supports owner draw)
    LK_LISTVIEW
    LK_TREEVIEW
    LK_RICHEDIT
    LK_SCINTILLA
    LK_TOOLTIP
    LK_REMOTE          ' top level window of another tiko process (ipc.inc)
end enum

type LNX_SUBCLASS
    pfn       as SUBCLASSPROC
    uId       as UINT_PTR
    dwRefData as DWORD_PTR
end type

type LNX_LBITEM
    text     as DWSTRING
    itemData as LONG_PTR
    selected as boolean
end type

type LNXWINDOW
    magic        as ulong                ' LNX_MAGIC while alive
    kind         as LNX_KIND
    widget       as GtkWidget ptr        ' outer widget placed in the parent's container
    client       as GtkWidget ptr        ' GtkFixed holding child windows (toplevel and custom)
    inner        as GtkWidget ptr        ' real control widget (entry, button, treeview...)
    scroller     as GtkWidget ptr        ' optional GtkScrolledWindow
    parent       as HWND
    owner        as HWND
    wndproc      as WNDPROC
    subclasses(any) as LNX_SUBCLASS
    subclassDepth as long                ' during DefSubclassProc chaining
    id           as LONG_PTR
    style        as DWORD
    exstyle      as DWORD
    classStyle   as DWORD
    classname    as DWSTRING
    text         as DWSTRING
    x            as long                 ' position relative to parent client area
    y            as long
    w            as long
    h            as long
    visible      as boolean
    enabled      as boolean
    destroyed    as boolean
    hFont        as HFONT
    hbrBackground as HBRUSH
    userData     as LONG_PTR             ' GWLP_USERDATA
    pCWindow     as any ptr              ' owning CWindow class (AfxCWindowPtr)
    hoverTime    as DWORD                ' TrackMouseEvent
    trackFlags   as DWORD
    hoverTimer   as ulong
    mouseInside  as boolean
    paintCr      as cairo_t ptr          ' valid during WM_PAINT
    blockSignals as long                 ' > 0 while changing native controls programmatically
    ' listbox / combobox data
    lbItems(any) as LNX_LBITEM
    lbCount      as long
    lbCurSel     as long
    lbTopIndex   as long
    lbItemHeight as long
    lbAnchor     as long
    lbCaret      as long
    ' modal / dialog
    modalResult  as long
    isModal      as boolean
    hAccel       as HACCEL
    layeredAlpha as long
    cursor       as HCURSOR
end type

' Upstream modParser.bi uses PARSEACTION (a type that happens to exist in windows.bi)
' for its PARSINGACTION enum parameters.
type PARSEACTION as long

#ifndef LNX_MAGIC
#define LNX_MAGIC  &h4C4E5857   ' "LNXW"
#endif
