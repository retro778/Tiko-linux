'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Win32 style windows on top of GTK3.
'
' Every window is an LNXWINDOW (HWND). Custom windows (painted by their window procedure)
' and top level client areas are GtkLayout widgets; their GTK events are converted into
' WM_* messages. Input messages are queued and retrieved with GetMessage exactly like the
' Win32 message loop, so tiko's message loop, accelerators and pre-translate handlers work
' unchanged. Keyboard events for native GTK widgets (Scintilla, text boxes) are queued as
' WM_KEYDOWN/WM_CHAR too and the original GDK event is re-injected when the message reaches
' the end of the window procedure chain (i.e. if no subclass/handler swallowed it).

#pragma once

extern "C"
    declare function gdk_display_get_default_seat( byval display as GdkDisplay ptr ) as any ptr
    declare function gdk_seat_get_pointer( byval seat as any ptr ) as GdkDevice ptr
    declare function gdk_seat_get_keyboard( byval seat as any ptr ) as GdkDevice ptr
end extern

' ----------------------------------------------------------------------------
' Globals
' ----------------------------------------------------------------------------
dim shared as HWND gLnxWindows(any)          ' all live windows in creation order
dim shared as long gLnxWindowCount
dim shared as GHashTable ptr gLnxWindowSet    ' fast IsWindow lookup

dim shared as MSG gLnxQueue(any)
dim shared as long gLnxQHead, gLnxQTail, gLnxQCap

dim shared as GdkEvent ptr gLnxReinjectEvent  ' key event currently being re-injected
dim shared as HWND gLnxCapture
dim shared as boolean gLnxTrace
dim shared as HWND gLnxMouseHwnd              ' window under the mouse (last motion)
dim shared as HCURSOR gLnxCurrentCursor
dim shared as boolean gLnxAppActive
dim shared as long gLnxQuitCode
dim shared as ubyte gLnxKeyState(255)        ' VK -> down state

' double click detection
dim shared as HWND gLnxLastClickHwnd
dim shared as guint32 gLnxLastClickTime
dim shared as long gLnxLastClickX, gLnxLastClickY, gLnxLastClickButton

' event copies referenced by queued messages (freed in a ring)
#ifndef LNX_EVENT_RING
#define LNX_EVENT_RING 256
#endif
dim shared as GdkEvent ptr gLnxEventRing(LNX_EVENT_RING - 1)
dim shared as long gLnxEventRingPos

#ifndef MK_LBUTTON
#define MK_LBUTTON   &h0001
#endif
#ifndef MK_RBUTTON
#define MK_RBUTTON   &h0002
#endif
#ifndef MK_SHIFT
#define MK_SHIFT     &h0004
#endif
#ifndef MK_CONTROL
#define MK_CONTROL   &h0008
#endif
#ifndef MK_MBUTTON
#define MK_MBUTTON   &h0010
#endif

#ifndef WM_NULL
#define WM_NULL          &h0000
#endif
#ifndef WM_KILLFOCUS
#define WM_KILLFOCUS     &h0008
#endif
#ifndef WM_ENABLE
#define WM_ENABLE        &h000A
#endif
#ifndef WM_SETREDRAW
#define WM_SETREDRAW     &h000B
#endif
#ifndef WM_GETTEXT
#define WM_GETTEXT       &h000D
#endif
#ifndef WM_GETTEXTLENGTH
#define WM_GETTEXTLENGTH &h000E
#endif
#ifndef WM_SHOWWINDOW
#define WM_SHOWWINDOW    &h0018
#endif
#ifndef WM_SETFONT
#define WM_SETFONT       &h0030
#endif
#ifndef WM_GETFONT
#define WM_GETFONT       &h0031
#endif
#ifndef WM_WINDOWPOSCHANGED
#define WM_WINDOWPOSCHANGED &h0047
#endif
#ifndef WM_NCCREATE
#define WM_NCCREATE      &h0081
#endif
#ifndef WM_SYSKEYUP
#define WM_SYSKEYUP      &h0105
#endif
#ifndef WM_SYSCHAR
#define WM_SYSCHAR       &h0106
#endif
#ifndef WM_INITDIALOG
#define WM_INITDIALOG    &h0110
#endif
#ifndef WM_HSCROLL
#define WM_HSCROLL       &h0114
#endif
#ifndef WM_VSCROLL
#define WM_VSCROLL       &h0115
#endif
#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP     &h0205
#endif
#ifndef WM_RBUTTONDBLCLK
#define WM_RBUTTONDBLCLK &h0206
#endif
#ifndef WM_MBUTTONDOWN
#define WM_MBUTTONDOWN   &h0207
#endif
#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP     &h0208
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL   &h020E
#endif
#ifndef WM_ENTERIDLE
#define WM_ENTERIDLE     &h0121
#endif
#ifndef WM_APP
#define WM_APP           &h8000
#endif
#ifndef WA_INACTIVE
#define WA_INACTIVE      0
#endif
#ifndef WA_ACTIVE
#define WA_ACTIVE        1
#endif
#ifndef WA_CLICKACTIVE
#define WA_CLICKACTIVE   2
#endif
#ifndef SIZE_RESTORED
#define SIZE_RESTORED    0
#endif
#ifndef SIZE_MAXIMIZED
#define SIZE_MAXIMIZED   2
#endif
#ifndef SW_SHOWNA
#define SW_SHOWNA        8
#endif
#ifndef SW_RESTORE
#define SW_RESTORE       9
#endif
#ifndef SW_MINIMIZE
#define SW_MINIMIZE      6
#endif
#ifndef SW_SHOWDEFAULT
#define SW_SHOWDEFAULT   10
#endif
#ifndef SW_SHOWMINIMIZED
#define SW_SHOWMINIMIZED 2
#endif
#ifndef SW_SHOWMAXIMIZED
#define SW_SHOWMAXIMIZED 3
#endif
#ifndef SWP_NOZORDER
#define SWP_NOZORDER     &h0004
#endif
#ifndef SWP_FRAMECHANGED
#define SWP_FRAMECHANGED &h0020
#endif
#ifndef SWP_NOCOPYBITS
#define SWP_NOCOPYBITS   &h0100
#endif
#ifndef GWL_STYLE
#define GWL_STYLE        (-16)
#endif
#ifndef GWL_EXSTYLE
#define GWL_EXSTYLE      (-20)
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA    (-21)
#endif
#ifndef GWLP_ID
#define GWLP_ID          (-12)
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC     (-4)
#endif
#ifndef IDOK
#define IDOK             1
#endif
#ifndef IDCANCEL
#define IDCANCEL         2
#endif
#ifndef IDABORT
#define IDABORT          3
#endif
#ifndef IDRETRY
#define IDRETRY          4
#endif
#ifndef IDIGNORE
#define IDIGNORE         5
#endif
#ifndef IDYES
#define IDYES            6
#endif
#ifndef IDNO
#define IDNO             7
#endif
#ifndef DM_GETDEFID
#define DM_GETDEFID      (WM_USER + 0)
#endif
#ifndef DLGC_WANTARROWS
#define DLGC_WANTARROWS  &h0001
#endif
#ifndef DLGC_WANTTAB
#define DLGC_WANTTAB     &h0002
#endif
#ifndef DLGC_WANTCHARS
#define DLGC_WANTCHARS   &h0080
#endif
#ifndef RDW_ERASE
#define RDW_ERASE        &h0004
#endif
#ifndef RDW_FRAME
#define RDW_FRAME        &h0400
#endif
#ifndef RDW_ERASENOW
#define RDW_ERASENOW     &h0200
#endif

declare function SendMessage( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
declare function LnxDefProcBase( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
declare function DefWindowProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
declare function LnxControlProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
declare function DestroyWindow( byval hwnd as HWND ) as BOOL
declare function IsWindow( byval hwnd as HWND ) as BOOL
declare function GetParent( byval hwnd as HWND ) as HWND
declare function PostMessage( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as BOOL
declare sub LnxConnectInputSignals( byval hwnd as HWND, byval w as GtkWidget ptr, byval bNative as boolean )

#define DefWindowProcW  DefWindowProc
#define SendMessageW    SendMessage
#define PostMessageW    PostMessage


' ----------------------------------------------------------------------------
' Registry
' ----------------------------------------------------------------------------
private function IsWindow( byval hwnd as HWND ) as BOOL
    if hwnd = 0 orelse gLnxWindowSet = 0 then return 0
    if g_hash_table_contains( gLnxWindowSet, hwnd ) = 0 then return 0
    return iif( hwnd->destroyed, 0, 1 )
end function

private sub LnxRegisterWindow( byval hwnd as HWND )
    if gLnxWindowSet = 0 then gLnxWindowSet = g_hash_table_new( NULL, NULL )
    g_hash_table_add( gLnxWindowSet, hwnd )
    redim preserve gLnxWindows(gLnxWindowCount)
    gLnxWindows(gLnxWindowCount) = hwnd
    gLnxWindowCount += 1
end sub

private sub LnxUnregisterWindow( byval hwnd as HWND )
    if gLnxWindowSet then g_hash_table_remove( gLnxWindowSet, hwnd )
    for i as long = 0 to gLnxWindowCount - 1
        if gLnxWindows(i) = hwnd then
            for j as long = i to gLnxWindowCount - 2
                gLnxWindows(j) = gLnxWindows(j + 1)
            next
            gLnxWindowCount -= 1
            exit for
        end if
    next
    ' purge queued messages for this window
    dim as long i = gLnxQHead
    while i <> gLnxQTail
        if gLnxQueue(i).hwnd = hwnd then gLnxQueue(i).hwnd = 0 : gLnxQueue(i).message = WM_NULL
        i = (i + 1) mod gLnxQCap
    wend
end sub

' Finds the HWND owning a GtkWidget (walking up the widget hierarchy).
private function LnxHwndFromWidget( byval w as GtkWidget ptr ) as HWND
    while w
        dim as HWND h = g_object_get_data( G_OBJECT(w), "lnxhwnd" )
        if h andalso IsWindow(h) then return h
        w = gtk_widget_get_parent( w )
    wend
    return 0
end function

private function LnxToplevelOf( byval hwnd as HWND ) as HWND
    while hwnd andalso hwnd->kind <> LK_TOPLEVEL
        hwnd = hwnd->parent
    wend
    return hwnd
end function

private function LnxIsNative( byval hwnd as HWND ) as boolean
    select case hwnd->kind
        case LK_TOPLEVEL, LK_CUSTOM, LK_LISTBOX: return false
    end select
    return true
end function

private function LnxIsDescendant( byval hwnd as HWND, byval hParent as HWND ) as boolean
    while hwnd
        if hwnd = hParent then return true
        hwnd = hwnd->parent
    wend
    return false
end function


' ----------------------------------------------------------------------------
' Message queue
' ----------------------------------------------------------------------------
private sub LnxQueuePush( byref m as const MSG, byval bFront as boolean = false )
    if gLnxQCap = 0 then
        gLnxQCap = 1024
        redim gLnxQueue(gLnxQCap - 1)
    end if
    dim as long nextTail = (gLnxQTail + 1) mod gLnxQCap
    if nextTail = gLnxQHead then
        ' grow
        dim as MSG tmp(any)
        dim as long n = 0
        redim tmp(gLnxQCap * 2 - 1)
        dim as long i = gLnxQHead
        while i <> gLnxQTail
            tmp(n) = gLnxQueue(i) : n += 1
            i = (i + 1) mod gLnxQCap
        wend
        gLnxQCap *= 2
        redim gLnxQueue(gLnxQCap - 1)
        for k as long = 0 to n - 1 : gLnxQueue(k) = tmp(k) : next
        gLnxQHead = 0 : gLnxQTail = n
    end if
    if bFront then
        gLnxQHead = (gLnxQHead - 1 + gLnxQCap) mod gLnxQCap
        gLnxQueue(gLnxQHead) = m
    else
        gLnxQueue(gLnxQTail) = m
        gLnxQTail = (gLnxQTail + 1) mod gLnxQCap
    end if
end sub

private function LnxQueuePop( byref m as MSG ) as boolean
    while gLnxQHead <> gLnxQTail
        m = gLnxQueue(gLnxQHead)
        gLnxQHead = (gLnxQHead + 1) mod gLnxQCap
        if m.message = WM_QUIT orelse m.hwnd = 0 orelse IsWindow(m.hwnd) then
            if m.message <> WM_NULL orelse m.hwnd <> 0 then return true
        end if
    wend
    return false
end function

private function LnxKeepEvent( byval ev as GdkEvent ptr ) as GdkEvent ptr
    dim as GdkEvent ptr c = gdk_event_copy( ev )
    if gLnxEventRing(gLnxEventRingPos) then gdk_event_free( gLnxEventRing(gLnxEventRingPos) )
    gLnxEventRing(gLnxEventRingPos) = c
    gLnxEventRingPos = (gLnxEventRingPos + 1) mod LNX_EVENT_RING
    return c
end function

private function GetTickCount() as DWORD
    return cast(DWORD, g_get_monotonic_time() \ 1000)
end function

private function GetCursorPos( byval lpPoint as POINT ptr ) as BOOL
    if lpPoint = 0 then return 0
    dim as GdkDisplay ptr disp = gdk_display_get_default()
    dim as GdkDevice ptr dev = gdk_seat_get_pointer( gdk_display_get_default_seat(disp) )
    dim as gint x, y
    gdk_device_get_position( dev, NULL, @x, @y )
    lpPoint->x = x : lpPoint->y = y
    return 1
end function

private function LnxPostInput( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                               byval ev as GdkEvent ptr = 0, byval observe as long = 0 ) as boolean
    dim as MSG m
    m.hwnd = hwnd : m.message = uMsg : m.wParam = wParam : m.lParam = lParam
    m.time = GetTickCount()
    GetCursorPos( @m.pt )
    m.lnxEvent = ev
    m.lnxObserve = observe
    LnxQueuePush( m )
    return true
end function

private function PostMessage( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as BOOL
    if hwnd andalso IsWindow(hwnd) = 0 then return 0
    LnxPostInput( hwnd, uMsg, wParam, lParam )
    ' wake up a blocking gtk_main_iteration
    g_main_context_wakeup( NULL )
    return 1
end function

private sub PostQuitMessage( byval nExitCode as long )
    dim as MSG m
    m.message = WM_QUIT
    m.wParam = nExitCode
    LnxQueuePush( m )
    g_main_context_wakeup( NULL )
end sub


' ----------------------------------------------------------------------------
' Window procedure chain (subclassing)
' ----------------------------------------------------------------------------
private function LnxCallChain( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM, _
                               byval depth as long ) as LRESULT
    ' depth = number of subclasses still to call (from the top of the stack)
    if depth > 0 then
        dim as long idx = depth - 1
        if idx <= ubound(hwnd->subclasses) then
            dim as LNX_SUBCLASS sc = hwnd->subclasses(idx)
            dim as long saveDepth = hwnd->subclassDepth
            hwnd->subclassDepth = idx
            function = sc.pfn( hwnd, uMsg, wParam, lParam, sc.uId, sc.dwRefData )
            if IsWindow(hwnd) then hwnd->subclassDepth = saveDepth
            exit function
        end if
    end if
    if hwnd->wndproc then
        return hwnd->wndproc( hwnd, uMsg, wParam, lParam )
    end if
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function

private function SendMessage( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if IsWindow(hwnd) = 0 then return 0
    return LnxCallChain( hwnd, uMsg, wParam, lParam, ubound(hwnd->subclasses) + 1 )
end function

private function CallWindowProc( byval pfn as WNDPROC, byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if pfn = 0 then return DefWindowProc( hwnd, uMsg, wParam, lParam )
    return pfn( hwnd, uMsg, wParam, lParam )
end function

private function SetWindowSubclass( byval hwnd as HWND, byval pfn as SUBCLASSPROC, byval uId as UINT_PTR, byval dwRefData as DWORD_PTR ) as BOOL
    if IsWindow(hwnd) = 0 orelse pfn = 0 then return 0
    for i as long = 0 to ubound(hwnd->subclasses)
        if hwnd->subclasses(i).pfn = pfn andalso hwnd->subclasses(i).uId = uId then
            hwnd->subclasses(i).dwRefData = dwRefData
            return 1
        end if
    next
    dim as long n = ubound(hwnd->subclasses) + 1
    redim preserve hwnd->subclasses(n)
    hwnd->subclasses(n).pfn = pfn
    hwnd->subclasses(n).uId = uId
    hwnd->subclasses(n).dwRefData = dwRefData
    return 1
end function

private function RemoveWindowSubclass( byval hwnd as HWND, byval pfn as SUBCLASSPROC, byval uId as UINT_PTR ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    dim as long n = ubound(hwnd->subclasses)
    for i as long = 0 to n
        if hwnd->subclasses(i).pfn = pfn andalso hwnd->subclasses(i).uId = uId then
            for j as long = i to n - 1
                hwnd->subclasses(j) = hwnd->subclasses(j + 1)
            next
            if n = 0 then erase hwnd->subclasses else redim preserve hwnd->subclasses(n - 1)
            return 1
        end if
    next
    return 0
end function

private function DefSubclassProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if IsWindow(hwnd) = 0 then return 0
    return LnxCallChain( hwnd, uMsg, wParam, lParam, hwnd->subclassDepth )
end function


' ----------------------------------------------------------------------------
' Key translation GDK -> VK
' ----------------------------------------------------------------------------
private function LnxKeyvalToVK( byval kv as guint ) as long
    if kv >= GDK_KEY_a andalso kv <= GDK_KEY_z then return kv - GDK_KEY_a + asc("A")
    if kv >= GDK_KEY_A andalso kv <= GDK_KEY_Z then return kv - GDK_KEY_A + asc("A")
    if kv >= GDK_KEY_0 andalso kv <= GDK_KEY_9 then return kv
    if kv >= GDK_KEY_F1 andalso kv <= GDK_KEY_F24 then return VK_F1 + (kv - GDK_KEY_F1)
    if kv >= GDK_KEY_KP_0 andalso kv <= GDK_KEY_KP_9 then return VK_NUMPAD0 + (kv - GDK_KEY_KP_0)
    select case kv
        case GDK_KEY_BackSpace: return VK_BACK
        case GDK_KEY_Tab, GDK_KEY_ISO_Left_Tab, GDK_KEY_KP_Tab: return VK_TAB
        case GDK_KEY_Return, GDK_KEY_KP_Enter: return VK_RETURN
        case GDK_KEY_Shift_L, GDK_KEY_Shift_R: return VK_SHIFT
        case GDK_KEY_Control_L, GDK_KEY_Control_R: return VK_CONTROL
        case GDK_KEY_Alt_L, GDK_KEY_Alt_R, GDK_KEY_Meta_L, GDK_KEY_Meta_R, GDK_KEY_ISO_Level3_Shift: return VK_MENU
        case GDK_KEY_Pause: return &h13
        case GDK_KEY_Caps_Lock: return &h14
        case GDK_KEY_Escape: return VK_ESCAPE
        case GDK_KEY_space, GDK_KEY_KP_Space: return VK_SPACE
        case GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up: return VK_PRIOR
        case GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down: return VK_NEXT
        case GDK_KEY_End, GDK_KEY_KP_End: return VK_END
        case GDK_KEY_Home, GDK_KEY_KP_Home: return VK_HOME
        case GDK_KEY_Left, GDK_KEY_KP_Left: return VK_LEFT
        case GDK_KEY_Up, GDK_KEY_KP_Up: return VK_UP
        case GDK_KEY_Right, GDK_KEY_KP_Right: return VK_RIGHT
        case GDK_KEY_Down, GDK_KEY_KP_Down: return VK_DOWN
        case GDK_KEY_Insert, GDK_KEY_KP_Insert: return VK_INSERT
        case GDK_KEY_Delete, GDK_KEY_KP_Delete: return VK_DELETE
        case GDK_KEY_KP_Multiply: return VK_MULTIPLY
        case GDK_KEY_KP_Add: return VK_ADD
        case GDK_KEY_KP_Subtract: return VK_SUBTRACT
        case GDK_KEY_KP_Decimal: return VK_DECIMAL
        case GDK_KEY_KP_Divide: return VK_DIVIDE
        case GDK_KEY_Menu: return &h5D
        case GDK_KEY_semicolon, GDK_KEY_colon: return VK_OEM_1
        case GDK_KEY_equal, GDK_KEY_plus: return VK_OEM_PLUS
        case GDK_KEY_comma, GDK_KEY_less: return VK_OEM_COMMA
        case GDK_KEY_minus, GDK_KEY_underscore: return VK_OEM_MINUS
        case GDK_KEY_period, GDK_KEY_greater: return VK_OEM_PERIOD
        case GDK_KEY_slash, GDK_KEY_question: return VK_OEM_2
        case GDK_KEY_grave, GDK_KEY_asciitilde: return VK_OEM_3
        case GDK_KEY_bracketleft, GDK_KEY_braceleft: return VK_OEM_4
        case GDK_KEY_backslash, GDK_KEY_bar: return VK_OEM_5
        case GDK_KEY_bracketright, GDK_KEY_braceright: return VK_OEM_6
        case GDK_KEY_apostrophe, GDK_KEY_quotedbl: return VK_OEM_7
        ' shifted digits on US layout
        case GDK_KEY_exclam: return asc("1")
        case GDK_KEY_at: return asc("2")
        case GDK_KEY_numbersign: return asc("3")
        case GDK_KEY_dollar: return asc("4")
        case GDK_KEY_percent: return asc("5")
        case GDK_KEY_asciicircum: return asc("6")
        case GDK_KEY_ampersand: return asc("7")
        case GDK_KEY_asterisk: return asc("8")
        case GDK_KEY_parenleft: return asc("9")
        case GDK_KEY_parenright: return asc("0")
    end select
    return 0
end function

' Use the unshifted keyval of the hardware key so that VK codes are layout independent
' for letters (e.g. Ctrl+S works with non latin keyboard layouts like Windows does).
private function LnxEventToVK( byval ek as GdkEventKey ptr ) as long
    dim as guint kv
    dim as GdkKeymap ptr km = gdk_keymap_get_for_display( gdk_display_get_default() )
    if gdk_keymap_translate_keyboard_state( km, ek->hardware_keycode, 0, 0, @kv, NULL, NULL, NULL ) then
        dim as long vk = LnxKeyvalToVK( kv )
        if vk then return vk
    end if
    return LnxKeyvalToVK( ek->keyval )
end function

private function GetAsyncKeyState( byval vKey as long ) as short
    dim as guint st = gdk_keymap_get_modifier_state( gdk_keymap_get_for_display(gdk_display_get_default()) )
    dim as boolean down = false
    select case vKey
        case VK_SHIFT:   down = (st and GDK_SHIFT_MASK) <> 0
        case VK_CONTROL: down = (st and GDK_CONTROL_MASK) <> 0
        case VK_MENU:    down = (st and GDK_MOD1_MASK) <> 0
        case &h01:       down = (st and GDK_BUTTON1_MASK) <> 0      ' VK_LBUTTON
        case &h02:       down = (st and GDK_BUTTON3_MASK) <> 0      ' VK_RBUTTON
        case else
            if vKey >= 0 andalso vKey <= 255 then down = gLnxKeyState(vKey) <> 0
    end select
    return iif( down, cast(short, &h8000), 0 )
end function

private function GetKeyState( byval vKey as long ) as short
    return GetAsyncKeyState( vKey )
end function

private function LnxMouseKeyFlags( byval state as guint ) as WPARAM
    dim as WPARAM mk = 0
    if (state and GDK_BUTTON1_MASK) then mk or= MK_LBUTTON
    if (state and GDK_BUTTON3_MASK) then mk or= MK_RBUTTON
    if (state and GDK_BUTTON2_MASK) then mk or= MK_MBUTTON
    if (state and GDK_SHIFT_MASK) then mk or= MK_SHIFT
    if (state and GDK_CONTROL_MASK) then mk or= MK_CONTROL
    return mk
end function


' ----------------------------------------------------------------------------
' GTK signal handlers
' ----------------------------------------------------------------------------
private function LnxOnDraw cdecl ( byval w as GtkWidget ptr, byval cr as cairo_t ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    ' Like a Win32 window DC, painting is clipped to the window's own client area.
    cairo_save( cr )
    cairo_rectangle( cr, 0, 0, gtk_widget_get_allocated_width(w), gtk_widget_get_allocated_height(w) )
    cairo_clip( cr )
    hwnd->paintCr = cr
    SendMessage( hwnd, WM_PAINT, 0, 0 )
    if IsWindow(hwnd) then hwnd->paintCr = 0
    cairo_restore( cr )
    return FALSE   ' let GTK draw the child widgets
end function

' Track hover timer (TrackMouseEvent TME_HOVER)
private function LnxHoverTimeout cdecl ( byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    hwnd->hoverTimer = 0
    if (hwnd->trackFlags and TME_HOVER) andalso hwnd->mouseInside then
        hwnd->trackFlags and= not TME_HOVER
        dim as POINT pt
        GetCursorPos( @pt )
        dim as long ox, oy
        gdk_window_get_origin( gtk_widget_get_window(hwnd->client), @ox, @oy )
        LnxPostInput( hwnd, WM_MOUSEHOVER, 0, MAKELPARAM(pt.x - ox, pt.y - oy) )
    end if
    return FALSE
end function

private function LnxOnButtonPress cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventButton ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    if ev->type <> GDK_BUTTON_PRESS then return iif(LnxIsNative(hwnd), FALSE, TRUE)   ' own double click logic
    dim as boolean bNative = LnxIsNative(hwnd)
    dim as long x = ev->x, y = ev->y
    if bNative = false andalso hwnd->kind <> LK_TOPLEVEL andalso hwnd->kind <> LK_LISTBOX then
        ' coordinates relative to the custom window client area
    end if
    dim as UINT uMsg
    select case ev->button
        case 1: uMsg = WM_LBUTTONDOWN
        case 3: uMsg = WM_RBUTTONDOWN
        case 2: uMsg = WM_MBUTTONDOWN
        case else: return FALSE
    end select
    ' double click detection (Windows sends DOWN, UP, DBLCLK, UP)
    dim as gint dblTime = 400
    g_object_get( gtk_settings_get_default(), "gtk-double-click-time", @dblTime, NULL )
    if gLnxLastClickHwnd = hwnd andalso gLnxLastClickButton = ev->button andalso _
       (ev->time - gLnxLastClickTime) <= cast(guint32, dblTime) andalso _
       abs(x - gLnxLastClickX) <= 4 andalso abs(y - gLnxLastClickY) <= 4 then
        if (hwnd->classStyle and CS_DBLCLKS) orelse bNative then
            select case uMsg
                case WM_LBUTTONDOWN: uMsg = WM_LBUTTONDBLCLK
                case WM_RBUTTONDOWN: uMsg = WM_RBUTTONDBLCLK
            end select
        end if
        gLnxLastClickHwnd = 0
    else
        gLnxLastClickHwnd = hwnd
        gLnxLastClickTime = ev->time
        gLnxLastClickX = x : gLnxLastClickY = y
        gLnxLastClickButton = ev->button
    end if
    LnxPostInput( hwnd, uMsg, LnxMouseKeyFlags(ev->state) or iif(ev->button = 1, MK_LBUTTON, iif(ev->button = 3, MK_RBUTTON, MK_MBUTTON)), _
                  MAKELPARAM(x, y), 0, iif(bNative, 1, 0) )
    return iif( bNative, FALSE, TRUE )
end function

private function LnxOnButtonRelease cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventButton ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    dim as boolean bNative = LnxIsNative(hwnd)
    dim as HWND target = hwnd
    if gLnxCapture andalso IsWindow(gLnxCapture) andalso gLnxCapture <> hwnd then target = gLnxCapture
    dim as long x = ev->x, y = ev->y
    if target <> hwnd then
        ' translate into capture window coordinates
        dim as long ox, oy, tx, ty
        gdk_window_get_origin( ev->window, @ox, @oy )
        gdk_window_get_origin( gtk_widget_get_window(target->client), @tx, @ty )
        x = x + ox - tx : y = y + oy - ty
    end if
    dim as UINT uMsg
    select case ev->button
        case 1: uMsg = WM_LBUTTONUP
        case 3: uMsg = WM_RBUTTONUP
        case 2: uMsg = WM_MBUTTONUP
        case else: return FALSE
    end select
    LnxPostInput( target, uMsg, LnxMouseKeyFlags(ev->state) and not iif(ev->button = 1, MK_LBUTTON, MK_RBUTTON), _
                  MAKELPARAM(x, y), 0, iif(LnxIsNative(target), 1, 0) )
    ' native widgets: Windows sends WM_CONTEXTMENU to the control which is passed on to its parent
    if bNative andalso ev->button = 3 andalso hwnd->kind <> LK_TEXTBOX andalso hwnd->kind <> LK_EDITMULTI then
        LnxPostInput( hwnd, WM_CONTEXTMENU, cast(WPARAM, hwnd), MAKELPARAM(cint(ev->x_root), cint(ev->y_root)) )
    end if
    return iif( bNative, FALSE, TRUE )
end function

private function LnxOnMotion cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventMotion ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    dim as boolean bNative = LnxIsNative(hwnd)
    dim as HWND target = hwnd
    dim as long x = ev->x, y = ev->y
    if gLnxCapture andalso IsWindow(gLnxCapture) andalso gLnxCapture <> hwnd then
        target = gLnxCapture
        dim as long ox, oy, tx, ty
        gdk_window_get_origin( ev->window, @ox, @oy )
        gdk_window_get_origin( gtk_widget_get_window(target->client), @tx, @ty )
        x = x + ox - tx : y = y + oy - ty
    end if
    gLnxMouseHwnd = target
    target->mouseInside = true
    if bNative = false then
        ' default WM_SETCURSOR behaviour: the arrow cursor (window procedures may override with SetCursor)
        if target->cursor = 0 then
            gdk_window_set_cursor( gtk_widget_get_window(target->client), NULL )
        end if
    end if
    ' restart hover timer
    if (target->trackFlags and TME_HOVER) then
        if target->hoverTimer then g_source_remove_( target->hoverTimer )
        target->hoverTimer = g_timeout_add( iif(target->hoverTime = 0 orelse target->hoverTime = HOVER_DEFAULT, 400, target->hoverTime), _
                                            cast(GSourceFunc, @LnxHoverTimeout), target )
    end if
    LnxPostInput( target, WM_MOUSEMOVE, LnxMouseKeyFlags(ev->state), MAKELPARAM(x, y), 0, iif(LnxIsNative(target), 1, 0) )
    return iif( bNative, FALSE, TRUE )
end function

private function LnxOnLeave cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventCrossing ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    if ev->mode <> GDK_CROSSING_NORMAL then return FALSE
    hwnd->mouseInside = false
    if gLnxCapture = hwnd then return FALSE      ' captured windows keep receiving input
    if hwnd->hoverTimer then g_source_remove_( hwnd->hoverTimer ) : hwnd->hoverTimer = 0
    if (hwnd->trackFlags and TME_LEAVE) then
        hwnd->trackFlags = 0
        LnxPostInput( hwnd, WM_MOUSELEAVE, 0, 0 )
    end if
    return FALSE
end function

private function LnxOnScroll cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventScroll ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    if LnxIsNative(hwnd) then return FALSE       ' native widgets scroll themselves
    static as double accum
    dim as long delta = 0
    select case ev->direction
        case GDK_SCROLL_UP:   delta = 120
        case GDK_SCROLL_DOWN: delta = -120
        case GDK_SCROLL_SMOOTH
            accum -= ev->delta_y * 120
            delta = cint(accum)
            accum -= delta
        case else: return TRUE
    end select
    if delta = 0 then return TRUE
    LnxPostInput( hwnd, WM_MOUSEWHEEL, MAKEWPARAM(LnxMouseKeyFlags(ev->state), delta), _
                  MAKELPARAM(cint(ev->x_root), cint(ev->y_root)) )
    return TRUE
end function

' Keyboard input arrives at the top level GtkWindow first.
private function LnxOnKey cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventKey ptr, byval hwndTop as HWND ) as gboolean
    if cast(GdkEvent ptr, ev) = gLnxReinjectEvent then return FALSE     ' re-injected: let GTK handle it
    if IsWindow(hwndTop) = 0 then return FALSE
    ' Target is the window owning the focus widget
    dim as HWND target = 0
    dim as GtkWidget ptr fw = gtk_window_get_focus( GTK_WINDOW(w) )
    if fw then target = LnxHwndFromWidget( fw )
    if target = 0 then target = hwndTop
    ' popup windows (menus, autocomplete) do not take focus; keys go to the owner's focus window
    dim as long vk = LnxEventToVK( ev )
    dim as boolean bDown = (ev->type = GDK_KEY_PRESS)
    if vk >= 0 andalso vk <= 255 then gLnxKeyState(vk) = iif(bDown, 1, 0)
    if vk = 0 then
        ' keys without a VK (dead keys, IME) go straight to GTK
        return FALSE
    end if
    dim as boolean bAlt = (ev->state and GDK_MOD1_MASK) <> 0
    dim as UINT uMsg
    if bDown then
        uMsg = iif( bAlt orelse vk = VK_MENU, WM_SYSKEYDOWN, WM_KEYDOWN )
    else
        uMsg = iif( bAlt orelse vk = VK_MENU, WM_SYSKEYUP, WM_KEYUP )
    end if
    dim as LPARAM lp = 1 or (cast(LPARAM, ev->hardware_keycode and &hFF) shl 16)
    if bDown = false then lp or= (3 shl 30)
    LnxPostInput( target, uMsg, vk, lp, LnxKeepEvent(cast(GdkEvent ptr, ev)), 0 )
    return TRUE
end function

' Re-inject a queued GDK key event so that GTK processes it normally.
private sub LnxReinjectKey( byval ev as GdkEvent ptr )
    if ev = 0 then exit sub
    gLnxReinjectEvent = ev
    gtk_main_do_event( ev )
    gLnxReinjectEvent = 0
end sub

private function LnxOnFocusIn cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventFocus ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    SendMessage( hwnd, WM_SETFOCUS, 0, 0 )
    return FALSE
end function

private function LnxOnFocusOut cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventFocus ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    SendMessage( hwnd, WM_KILLFOCUS, 0, 0 )
    return FALSE
end function

' Size of a custom/toplevel client area changed
private sub LnxOnSizeAllocate cdecl ( byval w as GtkWidget ptr, byval alloc as GtkAllocation ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    ' Position every child exactly like Win32 does (ignoring GTK minimum sizes).
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND c = gLnxWindows(i)
        if c->parent = hwnd andalso c->kind <> LK_TOPLEVEL andalso c->visible andalso c->widget then
            dim as GtkAllocation a
            a.x = c->x : a.y = c->y : a.width = iif(c->w < 1, 1, c->w) : a.height = iif(c->h < 1, 1, c->h)
            gtk_widget_size_allocate( c->widget, @a )
            if gLnxTrace andalso hwnd->kind = LK_TOPLEVEL andalso len(hwnd->text) then
                print "  alloc child kind="; c->kind; " id="; c->id; " "; c->x; c->y; c->w; c->h; " gtkvis="; gtk_widget_get_visible(c->widget); " mapped="; gtk_widget_get_mapped(c->widget); " text="; left(c->text.utf8, 20)
            end if
        elseif gLnxTrace andalso c->parent = hwnd andalso hwnd->kind = LK_TOPLEVEL andalso len(hwnd->text) then
            print "  skip child kind="; c->kind; " id="; c->id; " visible="; c->visible; " widget="; c->widget
        end if
    next
    if hwnd->kind = LK_TOPLEVEL then
        if hwnd->w <> alloc->width orelse hwnd->h <> alloc->height then
            hwnd->w = alloc->width : hwnd->h = alloc->height
            dim as GdkWindowState wst = 0
            if gtk_widget_get_window(hwnd->widget) then wst = gdk_window_get_state( gtk_widget_get_window(hwnd->widget) )
            SendMessage( hwnd, WM_SIZE, iif(wst and GDK_WINDOW_STATE_MAXIMIZED, SIZE_MAXIMIZED, SIZE_RESTORED), _
                         MAKELPARAM(alloc->width, alloc->height) )
        end if
    end if
end sub

private function LnxOnWindowState cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventWindowState ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    if (ev->changed_mask and GDK_WINDOW_STATE_ICONIFIED) andalso (ev->new_window_state and GDK_WINDOW_STATE_ICONIFIED) then
        SendMessage( hwnd, WM_SIZE, SIZE_MINIMIZED, 0 )
    end if
    return FALSE
end function

private function LnxOnDelete cdecl ( byval w as GtkWidget ptr, byval ev as GdkEvent ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    LnxPostInput( hwnd, WM_SYSCOMMAND, &hF060, 0 )     ' SC_CLOSE
    return TRUE
end function

private function LnxOnConfigure cdecl ( byval w as GtkWidget ptr, byval ev as GdkEventConfigure ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    ' Win32 window sizes include the frame: once the window manager has decorated the window
    ' shrink the client so that the outer size matches the requested window rectangle.
    dim as RECT ptr prcOuter = g_object_get_data( G_OBJECT(w), "lnxouterrect" )
    if prcOuter andalso gtk_widget_get_mapped(w) then
        dim as GdkRectangle fr
        gdk_window_get_frame_extents( gtk_widget_get_window(w), @fr )
        dim as long cw, ch
        gtk_window_get_size( GTK_WINDOW(w), @cw, @ch )
        dim as long dw = fr.width - cw, dh = fr.height - ch
        if dw >= 0 andalso dh > 0 andalso dw < 100 andalso dh < 100 then
            g_object_set_data( G_OBJECT(w), "lnxouterrect", NULL )
            dim as long nw = (prcOuter->right - prcOuter->left) - dw
            dim as long nh = (prcOuter->bottom - prcOuter->top) - dh
            if gtk_window_is_maximized(GTK_WINDOW(w)) = 0 andalso nw > 0 andalso nh > 0 andalso (nw <> cw orelse nh <> ch) then
                gtk_window_resize( GTK_WINDOW(w), nw, nh )
            end if
        end if
    end if
    SendMessage( hwnd, WM_MOVE, 0, MAKELPARAM(ev->x, ev->y) )
    return FALSE
end function

' Application activation (WM_ACTIVATE / WM_ACTIVATEAPP)
private sub LnxCheckAppActive()
    dim as boolean bActive = false
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND h = gLnxWindows(i)
        if h->kind = LK_TOPLEVEL andalso h->widget andalso gtk_window_is_active( GTK_WINDOW(h->widget) ) then
            bActive = true : exit for
        end if
    next
    if bActive <> gLnxAppActive then
        gLnxAppActive = bActive
        for i as long = 0 to gLnxWindowCount - 1
            dim as HWND h = gLnxWindows(i)
            if h->kind = LK_TOPLEVEL andalso h->parent = 0 then
                PostMessage( h, WM_ACTIVATEAPP, iif(bActive, 1, 0), 0 )
            end if
        next
    end if
end sub

private sub LnxOnIsActive cdecl ( byval w as GtkWidget ptr, byval pspec as any ptr, byval hwnd as HWND )
    if IsWindow(hwnd) = 0 then exit sub
    dim as boolean bActive = gtk_window_is_active( GTK_WINDOW(w) ) <> 0
    PostMessage( hwnd, WM_ACTIVATE, iif(bActive, WA_ACTIVE, WA_INACTIVE), 0 )
    LnxCheckAppActive()
end sub

private sub LnxConnectInputSignals( byval hwnd as HWND, byval w as GtkWidget ptr, byval bNative as boolean )
    gtk_widget_add_events( w, GDK_BUTTON_PRESS_MASK or GDK_BUTTON_RELEASE_MASK or GDK_POINTER_MOTION_MASK or _
                              GDK_LEAVE_NOTIFY_MASK or GDK_ENTER_NOTIFY_MASK or GDK_SCROLL_MASK or GDK_SMOOTH_SCROLL_MASK or _
                              GDK_FOCUS_CHANGE_MASK or GDK_KEY_PRESS_MASK or GDK_KEY_RELEASE_MASK )
    g_signal_connect( G_OBJECT(w), "button-press-event", G_CALLBACK(@LnxOnButtonPress), hwnd )
    g_signal_connect( G_OBJECT(w), "button-release-event", G_CALLBACK(@LnxOnButtonRelease), hwnd )
    g_signal_connect( G_OBJECT(w), "motion-notify-event", G_CALLBACK(@LnxOnMotion), hwnd )
    g_signal_connect( G_OBJECT(w), "leave-notify-event", G_CALLBACK(@LnxOnLeave), hwnd )
    g_signal_connect( G_OBJECT(w), "scroll-event", G_CALLBACK(@LnxOnScroll), hwnd )
    g_signal_connect( G_OBJECT(w), "focus-in-event", G_CALLBACK(@LnxOnFocusIn), hwnd )
    g_signal_connect( G_OBJECT(w), "focus-out-event", G_CALLBACK(@LnxOnFocusOut), hwnd )
end sub


' ----------------------------------------------------------------------------
' Creation / destruction
' ----------------------------------------------------------------------------
private function LnxNewWindow( byval kind as LNX_KIND, byval parent as HWND ) as HWND
    dim as HWND hwnd = new LNXWINDOW
    hwnd->magic = LNX_MAGIC
    hwnd->kind = kind
    hwnd->parent = parent
    hwnd->enabled = true
    hwnd->lbCurSel = -1
    hwnd->lbAnchor = -1
    hwnd->lbItemHeight = 16
    LnxRegisterWindow( hwnd )
    return hwnd
end function

' Creates a GtkLayout used as client area for custom drawn windows.
private function LnxCreateClientLayout( byval hwnd as HWND ) as GtkWidget ptr
    dim as GtkWidget ptr lay = gtk_layout_new( NULL, NULL )
    g_object_set_data( G_OBJECT(lay), "lnxhwnd", hwnd )
    gtk_widget_set_can_focus( lay, TRUE )
    g_signal_connect( G_OBJECT(lay), "draw", G_CALLBACK(@LnxOnDraw), hwnd )
    g_signal_connect_after( G_OBJECT(lay), "size-allocate", G_CALLBACK(@LnxOnSizeAllocate), hwnd )
    LnxConnectInputSignals( hwnd, lay, false )
    return lay
end function

' Adds a child widget to its parent's client layout.
private sub LnxPlaceInParent( byval hwnd as HWND )
    if hwnd->parent = 0 orelse hwnd->parent->client = 0 orelse hwnd->widget = 0 then exit sub
    gtk_layout_put( GTK_LAYOUT(hwnd->parent->client), hwnd->widget, hwnd->x, hwnd->y )
    gtk_widget_set_size_request( hwnd->widget, iif(hwnd->w < 1, 1, hwnd->w), iif(hwnd->h < 1, 1, hwnd->h) )
    if hwnd->visible then gtk_widget_show( hwnd->widget )
end sub

' Generic window creation used by CWindow.Create / AddControl / CreateWindowEx.
private function LnxCreateWindow( byval kind as LNX_KIND, byref wszClass as const wstring, byref wszTitle as const wstring, _
            byval dwStyle as DWORD, byval dwExStyle as DWORD, byval x as long, byval y as long, _
            byval nWidth as long, byval nHeight as long, byval hParent as HWND, byval id as LONG_PTR, _
            byval lpfnWndProc as WNDPROC, byval lpParam as any ptr, _
            byval pCWin as any ptr = 0, byval clsStyle as DWORD = CS_DBLCLKS, _
            byval hbr as HBRUSH = 0, byval hFont as HFONT = 0 ) as HWND

    if (dwStyle and WS_CHILD) = 0 andalso kind = LK_CUSTOM then kind = LK_TOPLEVEL
    dim as HWND hwnd = LnxNewWindow( kind, iif(kind = LK_TOPLEVEL, 0, hParent) )
    hwnd->owner = iif(kind = LK_TOPLEVEL, hParent, 0)
    hwnd->classname = wszClass
    hwnd->text = wszTitle
    hwnd->style = dwStyle
    hwnd->exstyle = dwExStyle
    hwnd->id = id
    hwnd->wndproc = lpfnWndProc
    hwnd->visible = (dwStyle and WS_VISIBLE) <> 0
    hwnd->enabled = (dwStyle and WS_DISABLED) = 0
    hwnd->pCWindow = pCWin
    hwnd->classStyle = clsStyle
    hwnd->hbrBackground = hbr
    hwnd->hFont = hFont
    if x = CW_USEDEFAULT then x = 100
    if y = CW_USEDEFAULT then y = 100
    if nWidth = CW_USEDEFAULT then nWidth = 800
    if nHeight = CW_USEDEFAULT then nHeight = 600
    hwnd->x = x : hwnd->y = y : hwnd->w = nWidth : hwnd->h = nHeight

    select case kind
        case LK_TOPLEVEL
            ' WS_POPUP windows without a caption:
            '   - menus, shadows (untitled / layered / no-activate) -> GTK popup windows (never focused)
            '   - other popups (e.g. the Find/Replace panel)       -> undecorated, focusable windows
            dim as boolean bNoCaption = (dwStyle and WS_POPUP) <> 0 andalso (dwStyle and WS_CAPTION) <> WS_CAPTION
            '   - layered "shadow" windows                         -> undecorated, never focused managed windows
            '     (managed so that they stack below the window they shadow, like on Windows)
            dim as boolean bLayered = bNoCaption andalso (dwExStyle and WS_EX_LAYERED) <> 0
            dim as boolean bPopup = bNoCaption andalso bLayered = false andalso _
                                    ( len(wszTitle) = 0 orelse (dwExStyle and WS_EX_NOACTIVATE) <> 0 )
            dim as boolean bUndecorated = bNoCaption andalso bPopup = false
            hwnd->widget = gtk_window_new( iif(bPopup, GTK_WINDOW_POPUP, GTK_WINDOW_TOPLEVEL) )
            if bUndecorated then
                gtk_window_set_decorated( GTK_WINDOW(hwnd->widget), FALSE )
                gtk_window_set_skip_taskbar_hint( GTK_WINDOW(hwnd->widget), TRUE )
                gtk_window_set_skip_pager_hint( GTK_WINDOW(hwnd->widget), TRUE )
                gtk_window_set_type_hint( GTK_WINDOW(hwnd->widget), GDK_WINDOW_TYPE_HINT_UTILITY )
            end if
            g_object_set_data( G_OBJECT(hwnd->widget), "lnxhwnd", hwnd )
            dim as string sTitle = WideToUtf8(@wszTitle)
            gtk_window_set_title( GTK_WINDOW(hwnd->widget), LnxZ(sTitle) )
            if bPopup = false then
                gtk_window_set_resizable( GTK_WINDOW(hwnd->widget), iif((dwStyle and WS_THICKFRAME) orelse bUndecorated, TRUE, FALSE) )
            end if
            if hParent andalso IsWindow(hParent) then
                dim as HWND topOwner = LnxToplevelOf(hParent)
                if topOwner then gtk_window_set_transient_for( GTK_WINDOW(hwnd->widget), GTK_WINDOW(topOwner->widget) )
            end if
            if (dwExStyle and WS_EX_NOACTIVATE) orelse bPopup orelse bLayered then
                gtk_window_set_accept_focus( GTK_WINDOW(hwnd->widget), FALSE )
                gtk_window_set_focus_on_map( GTK_WINDOW(hwnd->widget), FALSE )
            end if
            if bPopup then gtk_window_set_type_hint( GTK_WINDOW(hwnd->widget), GDK_WINDOW_TYPE_HINT_POPUP_MENU )
            gtk_window_set_default_size( GTK_WINDOW(hwnd->widget), nWidth, nHeight )
            gtk_window_move( GTK_WINDOW(hwnd->widget), x, y )
            hwnd->client = LnxCreateClientLayout( hwnd )
            gtk_container_add( GTK_CONTAINER(hwnd->widget), hwnd->client )
            gtk_widget_show( hwnd->client )
            hwnd->w = 0 : hwnd->h = 0       ' client size is reported by size-allocate
            g_signal_connect( G_OBJECT(hwnd->widget), "key-press-event", G_CALLBACK(@LnxOnKey), hwnd )
            g_signal_connect( G_OBJECT(hwnd->widget), "key-release-event", G_CALLBACK(@LnxOnKey), hwnd )
            g_signal_connect( G_OBJECT(hwnd->widget), "delete-event", G_CALLBACK(@LnxOnDelete), hwnd )
            g_signal_connect( G_OBJECT(hwnd->widget), "window-state-event", G_CALLBACK(@LnxOnWindowState), hwnd )
            g_signal_connect( G_OBJECT(hwnd->widget), "configure-event", G_CALLBACK(@LnxOnConfigure), hwnd )
            g_signal_connect( G_OBJECT(hwnd->widget), "notify::is-active", G_CALLBACK(@LnxOnIsActive), hwnd )

        case LK_CUSTOM
            hwnd->client = LnxCreateClientLayout( hwnd )
            hwnd->widget = hwnd->client
            LnxPlaceInParent( hwnd )
    end select

    ' WM_CREATE
    dim as CREATESTRUCT cs
    cs.lpCreateParams = lpParam
    cs.hwndParent = hParent
    cs.cx = nWidth : cs.cy = nHeight : cs.x = x : cs.y = y
    cs.style = dwStyle : cs.dwExStyle = dwExStyle
    cs.lpszName = @wszTitle : cs.lpszClass = @wszClass
    if hwnd->wndproc then
        if SendMessage( hwnd, WM_CREATE, 0, cast(LPARAM, @cs) ) = -1 then
            DestroyWindow( hwnd )
            return 0
        end if
    end if
    if IsWindow(hwnd) = 0 then return 0
    if kind = LK_CUSTOM then
        SendMessage( hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(hwnd->w, hwnd->h) )
    end if
    if kind = LK_TOPLEVEL andalso hwnd->visible then gtk_widget_show( hwnd->widget )
    return hwnd
end function

private function DestroyWindow( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    SendMessage( hwnd, WM_DESTROY, 0, 0 )
    if IsWindow(hwnd) = 0 then return 1
    ' destroy children (and owned top level windows) first
    dim as long i = 0
    while i < gLnxWindowCount
        dim as HWND c = gLnxWindows(i)
        if c->parent = hwnd orelse c->owner = hwnd then
            DestroyWindow( c )
            i = 0
        else
            i += 1
        end if
    wend
    if IsWindow(hwnd) = 0 then return 1
    SendMessage( hwnd, WM_NCDESTROY, 0, 0 )
    hwnd->destroyed = true
    if gLnxCapture = hwnd then gLnxCapture = 0
    if gLnxMouseHwnd = hwnd then gLnxMouseHwnd = 0
    if hwnd->hoverTimer then g_source_remove_( hwnd->hoverTimer ) : hwnd->hoverTimer = 0
    LnxUnregisterWindow( hwnd )
    if hwnd->widget then
        g_object_set_data( G_OBJECT(hwnd->widget), "lnxhwnd", NULL )
        if hwnd->parent = 0 orelse IsWindow(hwnd->parent) then gtk_widget_destroy( hwnd->widget )
        hwnd->widget = 0
    end if
    ' quit when the last top level window is gone is handled by the application (PostQuitMessage)
    hwnd->magic = 0
    ' The LNXWINDOW memory is intentionally kept (tiny) so that stale handles never point to
    ' reused memory; IsWindow() rejects them through the registry.
    return 1
end function


' ----------------------------------------------------------------------------
' Message loop
' ----------------------------------------------------------------------------
private function GetMessage( byval lpMsg as MSG ptr, byval hwnd as HWND, byval wMsgFilterMin as UINT, byval wMsgFilterMax as UINT ) as BOOL
    do
        if LnxQueuePop( *lpMsg ) then
            if lpMsg->message = WM_QUIT then return 0
            return 1
        end if
        gtk_main_iteration_do( TRUE )
    loop
end function
#define GetMessageW GetMessage

#ifndef PM_NOREMOVE
#define PM_NOREMOVE  0
#endif
#ifndef PM_REMOVE
#define PM_REMOVE    1
#endif
private function PeekMessage( byval lpMsg as MSG ptr, byval hwnd as HWND, byval wMsgFilterMin as UINT, _
                              byval wMsgFilterMax as UINT, byval wRemoveMsg as UINT ) as BOOL
    while gtk_events_pending()
        gtk_main_iteration_do( FALSE )
    wend
    if gLnxQHead = gLnxQTail then return 0
    if wRemoveMsg = PM_REMOVE then
        return iif( LnxQueuePop(*lpMsg), 1, 0 )
    end if
    *lpMsg = gLnxQueue(gLnxQHead)
    return 1
end function
#define PeekMessageW PeekMessage

' Creates WM_CHAR from WM_KEYDOWN. For native widgets the GDK event is re-injected when the
' WM_CHAR message is dispatched (so filtering WM_CHAR in the message loop suppresses typing,
' exactly like on Windows).
private function TranslateMessage( byval lpMsg as MSG ptr ) as BOOL
    if lpMsg = 0 orelse lpMsg->lnxEvent = 0 then return 0
    if lpMsg->message <> WM_KEYDOWN andalso lpMsg->message <> WM_SYSKEYDOWN then return 0
    dim as GdkEventKey ptr ek = cast(GdkEventKey ptr, lpMsg->lnxEvent)
    dim as guint32 uc = gdk_keyval_to_unicode( ek->keyval )
    dim as boolean bCtrl = (ek->state and GDK_CONTROL_MASK) <> 0
    dim as boolean bAlt = (ek->state and GDK_MOD1_MASK) <> 0
    if uc = 0 then
        select case ek->keyval
            case GDK_KEY_Return, GDK_KEY_KP_Enter: uc = 13
            case GDK_KEY_BackSpace: uc = 8
            case GDK_KEY_Tab: uc = 9
            case GDK_KEY_Escape: uc = 27
        end select
    end if
    if uc = 0 then return 0
    if bCtrl andalso uc >= asc("a") andalso uc <= asc("z") then uc = uc - asc("a") + 1
    if bCtrl andalso uc >= asc("A") andalso uc <= asc("Z") then uc = uc - asc("A") + 1
    dim as MSG m = *lpMsg
    m.message = iif( lpMsg->message = WM_SYSKEYDOWN, WM_SYSCHAR, WM_CHAR )
    m.wParam = uc
    if uc >= 32 andalso bCtrl = false andalso bAlt = false then
        ' printable character: text is produced when WM_CHAR is dispatched
        lpMsg->lnxObserve = 2
        m.lnxObserve = 0
    else
        ' control characters: the key itself is processed with WM_KEYDOWN (like Scintilla on Windows)
        m.lnxObserve = 1
    end if
    LnxQueuePush( m, true )
    return 1
end function

private function DispatchMessage( byval lpMsg as const MSG ptr ) as LRESULT
    if lpMsg = 0 then return 0
    if lpMsg->hwnd = 0 then return 0
    if IsWindow(lpMsg->hwnd) = 0 then return 0
    if lpMsg->message = WM_TIMER andalso lpMsg->lParam <> 0 then
        dim as TIMERPROC pfn = cast(TIMERPROC, lpMsg->lParam)
        pfn( lpMsg->hwnd, WM_TIMER, lpMsg->wParam, GetTickCount() )
        return 0
    end if
    if lpMsg->lnxObserve = 1 then
        ' The native widget already handled this input; subclass procedures still see it.
        if ubound(lpMsg->hwnd->subclasses) >= 0 then
            return SendMessage( lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam )
        end if
        return 0
    end if
    ' Native widgets: the GDK key event is re-injected by LnxControlProc when the message
    ' reaches the end of the window procedure chain (lnxObserve = 2: the text is produced
    ' later by the WM_CHAR message, so nothing is re-injected for this WM_KEYDOWN).
    dim as boolean bPending = (lpMsg->lnxEvent <> 0) andalso LnxIsNative(lpMsg->hwnd) andalso (lpMsg->lnxObserve = 0)
    if bPending then g_object_set_data( G_OBJECT(lpMsg->hwnd->widget), "lnxkeyevent", lpMsg->lnxEvent )
    function = SendMessage( lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam )
    if bPending andalso IsWindow(lpMsg->hwnd) andalso lpMsg->hwnd->widget then
        g_object_set_data( G_OBJECT(lpMsg->hwnd->widget), "lnxkeyevent", NULL )
    end if
end function
#define DispatchMessageW DispatchMessage

' Processes pending events (AfxDoEvents)
private sub AfxDoEvents( byval hwnd as HWND = 0 )
    dim as MSG m
    while PeekMessage( @m, 0, 0, 0, PM_REMOVE )
        if m.message = WM_QUIT then
            PostQuitMessage( m.wParam )
            exit while
        end if
        TranslateMessage( @m )
        DispatchMessage( @m )
    wend
end sub


' ----------------------------------------------------------------------------
' Default window procedure
' ----------------------------------------------------------------------------
' Base default processing shared by all windows (LnxControlProc falls back to this).
private function LnxDefProcBase( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if IsWindow(hwnd) = 0 then return 0
    select case uMsg
        case WM_CLOSE
            DestroyWindow( hwnd )
            return 0
        case WM_PAINT
            ' paint the class background brush
            if hwnd->paintCr andalso hwnd->hbrBackground then
                LnxSetColor( hwnd->paintCr, hwnd->hbrBackground->color )
                cairo_paint( hwnd->paintCr )
            end if
            return 0
        case WM_ERASEBKGND
            return 1
        case WM_RBUTTONUP
            dim as POINT pt
            GetCursorPos( @pt )
            SendMessage( hwnd, WM_CONTEXTMENU, cast(WPARAM, hwnd), MAKELPARAM(pt.x, pt.y) )
            return 0
        case WM_CONTEXTMENU, WM_MOUSEWHEEL
            ' passed on to the parent
            if hwnd->parent then return SendMessage( hwnd->parent, uMsg, wParam, lParam )
            return 0
        case WM_SETTEXT
            if lParam then hwnd->text = *cast(wstring ptr, lParam) else hwnd->text = ""
            if hwnd->kind = LK_TOPLEVEL then
                dim as string s = hwnd->text.utf8
                gtk_window_set_title( GTK_WINDOW(hwnd->widget), LnxZ(s) )
            end if
            return 1
        case WM_GETTEXT
            if lParam = 0 orelse wParam = 0 then return 0
            dim as long n = len(hwnd->text)
            if n > cast(long, wParam) - 1 then n = wParam - 1
            memcpy( cast(any ptr, lParam), hwnd->text.vptr, n * sizeof(wstring) )
            cast(ulong ptr, lParam)[n] = 0
            return n
        case WM_GETTEXTLENGTH
            return len(hwnd->text)
        case WM_SETFONT
            hwnd->hFont = cast(HFONT, wParam)
            return 0
        case WM_GETFONT
            return cast(LRESULT, hwnd->hFont)
        case WM_SETREDRAW
            if wParam then gtk_widget_queue_draw( hwnd->widget )
            return 0
        case WM_SYSCOMMAND
            if (wParam and &hFFF0) = &hF060 then SendMessage( hwnd, WM_CLOSE, 0, 0 )
            return 0
        case WM_MOUSEACTIVATE
            return 1    ' MA_ACTIVATE
        case WM_GETDLGCODE
            return 0
    end select
    return 0
end function


private function DefWindowProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if IsWindow(hwnd) = 0 then return 0
    ' Controls: built-in control behaviour (subclass procedures may call DefWindowProc directly)
    if hwnd->kind <> LK_TOPLEVEL andalso hwnd->kind <> LK_CUSTOM then
        return LnxControlProc( hwnd, uMsg, wParam, lParam )
    end if
    return LnxDefProcBase( hwnd, uMsg, wParam, lParam )
end function

' ----------------------------------------------------------------------------
' Window state / geometry
' ----------------------------------------------------------------------------
private function GetParent( byval hwnd as HWND ) as HWND
    if IsWindow(hwnd) = 0 then return 0
    if hwnd->parent then return hwnd->parent
    return hwnd->owner
end function

private function GetAncestor( byval hwnd as HWND, byval gaFlags as UINT ) as HWND
    return LnxToplevelOf( hwnd )
end function

private function GetDlgItem( byval hDlg as HWND, byval nIDDlgItem as long ) as HWND
    if IsWindow(hDlg) = 0 then return 0
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND c = gLnxWindows(i)
        if c->parent = hDlg andalso c->id = nIDDlgItem andalso c->destroyed = false then return c
    next
    return 0
end function

private function GetDlgCtrlID( byval hwnd as HWND ) as long
    if IsWindow(hwnd) = 0 then return 0
    return hwnd->id
end function

private function IsWindowVisible( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    while hwnd
        if hwnd->visible = false then return 0
        hwnd = hwnd->parent
    wend
    return 1
end function

private function ShowWindow( byval hwnd as HWND, byval nCmdShow as long ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    if gLnxTrace andalso hwnd->kind = LK_TOPLEVEL then print "ShowWindow top '"; hwnd->text.utf8; "' cmd="; nCmdShow
    dim as boolean wasVisible = hwnd->visible
    select case nCmdShow
        case SW_HIDE
            hwnd->visible = false
            if hwnd->widget then gtk_widget_hide( hwnd->widget )
        case SW_MINIMIZE, SW_SHOWMINNOACTIVE, SW_SHOWMINIMIZED
            hwnd->visible = true
            if hwnd->kind = LK_TOPLEVEL then gtk_window_iconify( GTK_WINDOW(hwnd->widget) )
        case SW_MAXIMIZE
            hwnd->visible = true
            if hwnd->kind = LK_TOPLEVEL then gtk_window_maximize( GTK_WINDOW(hwnd->widget) ) : gtk_widget_show( hwnd->widget )
        case SW_RESTORE
            hwnd->visible = true
            if hwnd->kind = LK_TOPLEVEL then
                gtk_window_deiconify( GTK_WINDOW(hwnd->widget) )
                gtk_window_unmaximize( GTK_WINDOW(hwnd->widget) )
            end if
            gtk_widget_show( hwnd->widget )
        case else
            hwnd->visible = true
            if hwnd->widget then
                if hwnd->kind = LK_TOPLEVEL then
                    if nCmdShow = SW_SHOWNOACTIVATE orelse nCmdShow = SW_SHOWNA then
                        gtk_window_set_focus_on_map( GTK_WINDOW(hwnd->widget), FALSE )
                    end if
                    gtk_widget_show( hwnd->widget )
                    if nCmdShow = SW_SHOWNORMAL orelse nCmdShow = SW_SHOW then
                        if gtk_window_get_accept_focus( GTK_WINDOW(hwnd->widget) ) then gtk_window_present( GTK_WINDOW(hwnd->widget) )
                    end if
                else
                    gtk_widget_show( hwnd->widget )
                    if hwnd->parent andalso hwnd->parent->client then gtk_widget_queue_resize( hwnd->parent->client )
                end if
            end if
    end select
    if wasVisible <> hwnd->visible then SendMessage( hwnd, WM_SHOWWINDOW, iif(hwnd->visible, 1, 0), 0 )
    return iif( wasVisible, 1, 0 )
end function

private function EnableWindow( byval hwnd as HWND, byval bEnable as BOOL ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    dim as boolean wasDisabled = (hwnd->enabled = false)
    hwnd->enabled = (bEnable <> 0)
    if hwnd->widget then
        if hwnd->kind = LK_TOPLEVEL then
            gtk_widget_set_sensitive( hwnd->client, bEnable <> 0 )
        else
            gtk_widget_set_sensitive( hwnd->widget, bEnable <> 0 )
        end if
    end if
    return iif( wasDisabled, 1, 0 )
end function

private function IsWindowEnabled( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    return iif( hwnd->enabled, 1, 0 )
end function

private function GetClientRect( byval hwnd as HWND, byval lpRect as RECT ptr ) as BOOL
    if lpRect = 0 then return 0
    SetRectEmpty( lpRect )
    if IsWindow(hwnd) = 0 then return 0
    if hwnd->kind = LK_TOPLEVEL then
        if hwnd->w = 0 andalso hwnd->h = 0 then
            dim as long cw, ch
            gtk_window_get_size( GTK_WINDOW(hwnd->widget), @cw, @ch )
            lpRect->right = cw : lpRect->bottom = ch
        else
            lpRect->right = hwnd->w : lpRect->bottom = hwnd->h
        end if
    else
        lpRect->right = hwnd->w : lpRect->bottom = hwnd->h
    end if
    return 1
end function

' Screen origin of a window's client area
private sub LnxClientOrigin( byval hwnd as HWND, byref ox as long, byref oy as long )
    ox = 0 : oy = 0
    if IsWindow(hwnd) = 0 then exit sub
    if hwnd->kind = LK_TOPLEVEL then
        dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->client )
        if gw then
            gdk_window_get_origin( gw, @ox, @oy )
        else
            gtk_window_get_position( GTK_WINDOW(hwnd->widget), @ox, @oy )
        end if
        exit sub
    end if
    ' child: parent's client origin + our position
    LnxClientOrigin( hwnd->parent, ox, oy )
    ox += hwnd->x : oy += hwnd->y
end sub

private function GetWindowRect( byval hwnd as HWND, byval lpRect as RECT ptr ) as BOOL
    if lpRect = 0 then return 0
    SetRectEmpty( lpRect )
    if IsWindow(hwnd) = 0 then return 0
    if hwnd->kind = LK_TOPLEVEL then
        dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->widget )
        if gw andalso gtk_widget_get_visible( hwnd->widget ) then
            dim as GdkRectangle r
            gdk_window_get_frame_extents( gw, @r )
            SetRect( lpRect, r.x, r.y, r.x + r.width, r.y + r.height )
        else
            dim as long x, y, w, h
            gtk_window_get_position( GTK_WINDOW(hwnd->widget), @x, @y )
            gtk_window_get_size( GTK_WINDOW(hwnd->widget), @w, @h )
            SetRect( lpRect, x, y, x + w, y + h )
        end if
        return 1
    end if
    dim as long ox, oy
    LnxClientOrigin( hwnd, ox, oy )
    SetRect( lpRect, ox, oy, ox + hwnd->w, oy + hwnd->h )
    return 1
end function

private function MapWindowPoints( byval hwndFrom as HWND, byval hwndTo as HWND, byval lpPoints as POINT ptr, byval cPoints as UINT ) as long
    dim as long fx, fy, tx, ty
    if hwndFrom then LnxClientOrigin( hwndFrom, fx, fy )
    if hwndTo then LnxClientOrigin( hwndTo, tx, ty )
    for i as long = 0 to cPoints - 1
        lpPoints[i].x += fx - tx
        lpPoints[i].y += fy - ty
    next
    return MAKELONG( fx - tx, fy - ty )
end function

private function ScreenToClient( byval hwnd as HWND, byval lpPoint as POINT ptr ) as BOOL
    MapWindowPoints( HWND_DESKTOP, hwnd, lpPoint, 1 )
    return 1
end function

private function ClientToScreen( byval hwnd as HWND, byval lpPoint as POINT ptr ) as BOOL
    MapWindowPoints( hwnd, HWND_DESKTOP, lpPoint, 1 )
    return 1
end function

private function SetWindowPos( byval hwnd as HWND, byval hWndInsertAfter as HWND, byval X as long, byval Y as long, _
                               byval cx as long, byval cy as long, byval uFlags as UINT ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    if gLnxTrace andalso hwnd->kind = LK_TOPLEVEL then
        print "SetWindowPos top '"; hwnd->text.utf8; "' x="; X; " y="; Y; " cx="; cx; " cy="; cy; " flags=&h"; hex(uFlags)
    end if
    if hwnd->kind = LK_TOPLEVEL then
        if (uFlags and SWP_NOMOVE) = 0 then gtk_window_move( GTK_WINDOW(hwnd->widget), X, Y )
        if (uFlags and SWP_NOSIZE) = 0 then
            ' Win32 sizes include the frame; GTK sizes the client area. Compensate for decorations.
            dim as long fw = 0, fh = 0
            dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->widget )
            if gw andalso gtk_window_get_decorated( GTK_WINDOW(hwnd->widget) ) andalso gtk_widget_get_visible(hwnd->widget) then
                dim as GdkRectangle r
                gdk_window_get_frame_extents( gw, @r )
                dim as long iw, ih
                gtk_window_get_size( GTK_WINDOW(hwnd->widget), @iw, @ih )
                fw = r.width - iw : fh = r.height - ih
                if fw < 0 orelse fw > 100 then fw = 0
                if fh < 0 orelse fh > 100 then fh = 0
            end if
            dim as long nw = cx - fw, nh = cy - fh
            if nw < 1 then nw = 1
            if nh < 1 then nh = 1
            gtk_window_resize( GTK_WINDOW(hwnd->widget), nw, nh )
            if gtk_widget_get_visible(hwnd->widget) = 0 then
                gtk_window_set_default_size( GTK_WINDOW(hwnd->widget), nw, nh )
            end if
        end if
        if hWndInsertAfter = HWND_TOPMOST then gtk_window_set_keep_above( GTK_WINDOW(hwnd->widget), TRUE )
        if hWndInsertAfter = HWND_NOTOPMOST then gtk_window_set_keep_above( GTK_WINDOW(hwnd->widget), FALSE )
        if (uFlags and SWP_SHOWWINDOW) then ShowWindow( hwnd, iif(uFlags and SWP_NOACTIVATE, SW_SHOWNOACTIVATE, SW_SHOW) )
        if (uFlags and SWP_HIDEWINDOW) then ShowWindow( hwnd, SW_HIDE )
        return 1
    end if
    dim as boolean bMoved = false, bSized = false
    if (uFlags and SWP_NOMOVE) = 0 andalso (hwnd->x <> X orelse hwnd->y <> Y) then
        hwnd->x = X : hwnd->y = Y : bMoved = true
    end if
    if (uFlags and SWP_NOSIZE) = 0 then
        if cx < 0 then cx = 0
        if cy < 0 then cy = 0
        if hwnd->w <> cx orelse hwnd->h <> cy then
            hwnd->w = cx : hwnd->h = cy : bSized = true
        end if
    end if
    if hwnd->widget andalso hwnd->parent andalso hwnd->parent->client then
        if bMoved then gtk_layout_move( GTK_LAYOUT(hwnd->parent->client), hwnd->widget, hwnd->x, hwnd->y )
        if bSized then gtk_widget_set_size_request( hwnd->widget, iif(hwnd->w < 1, 1, hwnd->w), iif(hwnd->h < 1, 1, hwnd->h) )
        if bMoved orelse bSized then gtk_widget_queue_resize( hwnd->parent->client )
    end if
    if hWndInsertAfter = HWND_TOP andalso (uFlags and SWP_NOZORDER) = 0 andalso hwnd->widget then
        dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->widget )
        if gw andalso gtk_widget_get_has_window( hwnd->widget ) then gdk_window_raise( gw )
    end if
    if (uFlags and SWP_SHOWWINDOW) then ShowWindow( hwnd, SW_SHOW )
    if (uFlags and SWP_HIDEWINDOW) then ShowWindow( hwnd, SW_HIDE )
    if bMoved then SendMessage( hwnd, WM_MOVE, 0, MAKELPARAM(hwnd->x, hwnd->y) )
    if bSized then SendMessage( hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(hwnd->w, hwnd->h) )
    return 1
end function

private function MoveWindow( byval hwnd as HWND, byval X as long, byval Y as long, byval nWidth as long, byval nHeight as long, _
                             byval bRepaint as BOOL = 1 ) as BOOL
    return SetWindowPos( hwnd, 0, X, Y, nWidth, nHeight, SWP_NOZORDER )
end function

private function IsIconic( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then return 0
    dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->widget )
    if gw = 0 then return 0
    return iif( gdk_window_get_state(gw) and GDK_WINDOW_STATE_ICONIFIED, 1, 0 )
end function

private function IsZoomed( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then return 0
    return iif( gtk_window_is_maximized( GTK_WINDOW(hwnd->widget) ), 1, 0 )
end function

private function GetWindowPlacement( byval hwnd as HWND, byval lpwndpl as WINDOWPLACEMENT ptr ) as BOOL
    if IsWindow(hwnd) = 0 orelse lpwndpl = 0 then return 0
    dim as long x, y, w, h
    gtk_window_get_position( GTK_WINDOW(hwnd->widget), @x, @y )
    gtk_window_get_size( GTK_WINDOW(hwnd->widget), @w, @h )
    ' the normal (restored) position is remembered while maximized
    dim as RECT ptr prc = g_object_get_data( G_OBJECT(hwnd->widget), "lnxnormalrect" )
    if IsZoomed(hwnd) andalso prc then
        lpwndpl->rcNormalPosition = *prc
    else
        SetRect( @lpwndpl->rcNormalPosition, x, y, x + w, y + h )
    end if
    lpwndpl->showCmd = iif( IsZoomed(hwnd), SW_MAXIMIZE, iif(IsIconic(hwnd), SW_SHOWMINIMIZED, SW_SHOWNORMAL) )
    return 1
end function

private function SetWindowPlacement( byval hwnd as HWND, byval lpwndpl as const WINDOWPLACEMENT ptr ) as BOOL
    if IsWindow(hwnd) = 0 orelse lpwndpl = 0 then return 0
    with lpwndpl->rcNormalPosition
        gtk_window_move( GTK_WINDOW(hwnd->widget), .left, .top )
        gtk_window_set_default_size( GTK_WINDOW(hwnd->widget), .right - .left, .bottom - .top )
        gtk_window_resize( GTK_WINDOW(hwnd->widget), .right - .left, .bottom - .top )
    end with
    dim as RECT ptr prc = g_object_get_data( G_OBJECT(hwnd->widget), "lnxnormalrect" )
    if prc = 0 then prc = new RECT : g_object_set_data( G_OBJECT(hwnd->widget), "lnxnormalrect", prc )
    *prc = lpwndpl->rcNormalPosition
    static as RECT rcOuter
    rcOuter = lpwndpl->rcNormalPosition
    g_object_set_data( G_OBJECT(hwnd->widget), "lnxouterrect", @rcOuter )
    ShowWindow( hwnd, lpwndpl->showCmd )
    return 1
end function

private function SetWindowText( byval hwnd as HWND, byval lpString as LPCWSTR ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    return SendMessage( hwnd, WM_SETTEXT, 0, cast(LPARAM, lpString) )
end function
#define SetWindowTextW SetWindowText

private function GetWindowTextLength( byval hwnd as HWND ) as long
    return SendMessage( hwnd, WM_GETTEXTLENGTH, 0, 0 )
end function

private function GetWindowText( byval hwnd as HWND, byval lpString as LPWSTR, byval nMaxCount as long ) as long
    return SendMessage( hwnd, WM_GETTEXT, nMaxCount, cast(LPARAM, lpString) )
end function

private function GetWindowLongPtr( byval hwnd as HWND, byval nIndex as long ) as LONG_PTR
    if IsWindow(hwnd) = 0 then return 0
    select case nIndex
        case GWLP_USERDATA: return hwnd->userData
        case GWL_STYLE: return hwnd->style or iif(hwnd->visible, WS_VISIBLE, 0)
        case GWL_EXSTYLE: return hwnd->exstyle
        case GWLP_ID: return hwnd->id
        case GWLP_WNDPROC: return cast(LONG_PTR, hwnd->wndproc)
        case 0: return cast(LONG_PTR, hwnd->pCWindow)
    end select
    return 0
end function
#define GetWindowLongPtrW GetWindowLongPtr
#define GetWindowLong GetWindowLongPtr

private function SetWindowLongPtr( byval hwnd as HWND, byval nIndex as long, byval dwNewLong as LONG_PTR ) as LONG_PTR
    if IsWindow(hwnd) = 0 then return 0
    dim as LONG_PTR prev = GetWindowLongPtr( hwnd, nIndex )
    select case nIndex
        case GWLP_USERDATA: hwnd->userData = dwNewLong
        case GWL_STYLE: hwnd->style = dwNewLong
        case GWL_EXSTYLE: hwnd->exstyle = dwNewLong
        case GWLP_ID: hwnd->id = dwNewLong
        case GWLP_WNDPROC: hwnd->wndproc = cast(WNDPROC, dwNewLong)
        case 0: hwnd->pCWindow = cast(any ptr, dwNewLong)
    end select
    return prev
end function
#define SetWindowLongPtrW SetWindowLongPtr
#define SetWindowLong SetWindowLongPtr


' ----------------------------------------------------------------------------
' Painting
' ----------------------------------------------------------------------------
private function InvalidateRect( byval hwnd as HWND, byval lpRect as const RECT ptr, byval bErase as BOOL ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    dim as GtkWidget ptr w = iif(hwnd->client, hwnd->client, hwnd->widget)
    if w = 0 then return 0
    if lpRect then
        gtk_widget_queue_draw_area( w, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top )
    else
        gtk_widget_queue_draw( w )
    end if
    return 1
end function

private function ValidateRect( byval hwnd as HWND, byval lpRect as const RECT ptr ) as BOOL
    return 1
end function

private function UpdateWindow( byval hwnd as HWND ) as BOOL
    if IsWindow(hwnd) = 0 then return 0
    dim as GtkWidget ptr w = iif(hwnd->client, hwnd->client, hwnd->widget)
    dim as GdkWindow ptr gw = gtk_widget_get_window( w )
    if gw then gdk_window_process_updates( gw, TRUE )
    return 1
end function

private function RedrawWindow( byval hwnd as HWND, byval lprcUpdate as const RECT ptr, byval hrgnUpdate as any ptr, byval flags as UINT ) as BOOL
    InvalidateRect( hwnd, lprcUpdate, 1 )
    if (flags and RDW_UPDATENOW) then UpdateWindow( hwnd )
    return 1
end function

private function AfxRedrawWindow( byval hwnd as HWND ) as BOOL
    return RedrawWindow( hwnd, NULL, NULL, RDW_INVALIDATE or RDW_UPDATENOW )
end function

private function BeginPaint( byval hwnd as HWND, byval lpPaint as PAINTSTRUCT ptr ) as HDC
    if IsWindow(hwnd) = 0 then return 0
    dim as HDC hdc = new LNXDC
    LnxDCInit( hdc )
    hdc->hwnd = hwnd
    if hwnd->hFont then hdc->st.hFont = hwnd->hFont
    if hwnd->paintCr then
        hdc->cr = hwnd->paintCr
        hdc->ownsCr = false
    else
        ' BeginPaint outside of WM_PAINT dispatching: draw nowhere
        dim as cairo_surface_t ptr surf = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, 1, 1 )
        hdc->cr = cairo_create( surf )
        cairo_surface_destroy( surf )
        hdc->ownsCr = true
    end if
    if lpPaint then
        lpPaint->hdc = hdc
        GetClientRect( hwnd, @lpPaint->rcPaint )
        lpPaint->fErase = 0
    end if
    return hdc
end function

private function EndPaint( byval hwnd as HWND, byval lpPaint as const PAINTSTRUCT ptr ) as BOOL
    if lpPaint = 0 orelse lpPaint->hdc = 0 then return 0
    ReleaseDC( hwnd, lpPaint->hdc )
    return 1
end function

#define SetWindowRedraw(hwnd, fRedraw) SendMessage( (hwnd), WM_SETREDRAW, cast(WPARAM, cint(fRedraw)), 0 )


' ----------------------------------------------------------------------------
' Focus, capture, cursor
' ----------------------------------------------------------------------------
private function GetFocus() as HWND
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND h = gLnxWindows(i)
        if h->kind = LK_TOPLEVEL andalso h->widget andalso gtk_window_is_active( GTK_WINDOW(h->widget) ) then
            dim as GtkWidget ptr fw = gtk_window_get_focus( GTK_WINDOW(h->widget) )
            if fw then
                dim as HWND r = LnxHwndFromWidget( fw )
                if r then return r
            end if
            return h
        end if
    next
    return 0
end function

private function SetFocus( byval hwnd as HWND ) as HWND
    dim as HWND prev = GetFocus()
    if IsWindow(hwnd) = 0 then return prev
    dim as GtkWidget ptr w
    select case hwnd->kind
        case LK_TOPLEVEL: w = hwnd->client
        case LK_CUSTOM, LK_LISTBOX: w = hwnd->client
        case else: w = iif(hwnd->inner, hwnd->inner, hwnd->widget)
    end select
    if w then
        dim as HWND top = LnxToplevelOf( hwnd )
        if top andalso gtk_window_is_active( GTK_WINDOW(top->widget) ) = 0 andalso gtk_window_get_accept_focus( GTK_WINDOW(top->widget) ) then
            gtk_window_present( GTK_WINDOW(top->widget) )
        end if
        gtk_widget_grab_focus( w )
    end if
    return prev
end function

private function GetActiveWindow() as HWND
    for i as long = 0 to gLnxWindowCount - 1
        dim as HWND h = gLnxWindows(i)
        if h->kind = LK_TOPLEVEL andalso h->widget andalso gtk_window_is_active( GTK_WINDOW(h->widget) ) then return h
    next
    return 0
end function
#define GetForegroundWindow GetActiveWindow
#define GetForeGroundWindow GetActiveWindow

private function SetForegroundWindow( byval hwnd as HWND ) as BOOL
    dim as HWND top = LnxToplevelOf( hwnd )
    if top = 0 then return 0
    gtk_window_present( GTK_WINDOW(top->widget) )
    return 1
end function
#define SetActiveWindow SetForegroundWindow
#define BringWindowToTop SetForegroundWindow

private function AllowSetForegroundWindow( byval dwProcessId as DWORD ) as BOOL
    return 1
end function

private function SetCapture( byval hwnd as HWND ) as HWND
    dim as HWND prev = gLnxCapture
    gLnxCapture = hwnd
    return prev
end function

private function ReleaseCapture() as BOOL
    dim as HWND prev = gLnxCapture
    gLnxCapture = 0
    if prev andalso IsWindow(prev) then SendMessage( prev, WM_CAPTURECHANGED, 0, 0 )
    return 1
end function

private function GetCapture() as HWND
    return gLnxCapture
end function

' Cursors: HCURSOR is a GdkCursor ptr
#ifndef IDC_ARROW
#define IDC_ARROW     cast(LPCWSTR, 32512)
#endif
#ifndef IDC_IBEAM
#define IDC_IBEAM     cast(LPCWSTR, 32513)
#endif
#ifndef IDC_WAIT
#define IDC_WAIT      cast(LPCWSTR, 32514)
#endif
#ifndef IDC_CROSS
#define IDC_CROSS     cast(LPCWSTR, 32515)
#endif
#ifndef IDC_SIZENWSE
#define IDC_SIZENWSE  cast(LPCWSTR, 32642)
#endif
#ifndef IDC_SIZENESW
#define IDC_SIZENESW  cast(LPCWSTR, 32643)
#endif
#ifndef IDC_SIZEWE
#define IDC_SIZEWE    cast(LPCWSTR, 32644)
#endif
#ifndef IDC_SIZENS
#define IDC_SIZENS    cast(LPCWSTR, 32645)
#endif
#ifndef IDC_SIZEALL
#define IDC_SIZEALL   cast(LPCWSTR, 32646)
#endif
#ifndef IDC_HAND
#define IDC_HAND      cast(LPCWSTR, 32649)
#endif
#ifndef IDC_APPSTARTING
#define IDC_APPSTARTING cast(LPCWSTR, 32650)
#endif

private function LnxCursorFromId( byval id as long ) as HCURSOR
    dim as zstring ptr nm
    select case id
        case 32512: nm = @"default"
        case 32513: nm = @"text"
        case 32514: nm = @"wait"
        case 32515: nm = @"crosshair"
        case 32642: nm = @"nwse-resize"
        case 32643: nm = @"nesw-resize"
        case 32644, OCR_SIZEWE: nm = @"col-resize"
        case 32645, OCR_SIZENS: nm = @"row-resize"
        case 32646, OCR_SIZEALL: nm = @"move"
        case 32649: nm = @"pointer"
        case 32650: nm = @"progress"
        case else: nm = @"default"
    end select
    static as GHashTable ptr cache
    if cache = 0 then cache = g_hash_table_new( NULL, NULL )
    dim as HCURSOR c = g_hash_table_lookup( cache, cast(any ptr, cast(integer, id)) )
    if c = 0 then
        c = gdk_cursor_new_from_name( gdk_display_get_default(), nm )
        g_hash_table_insert( cache, cast(any ptr, cast(integer, id)), c )
    end if
    return c
end function

private function LoadCursor( byval hInstance as HINSTANCE, byval lpCursorName as LPCWSTR ) as HCURSOR
    return LnxCursorFromId( cast(integer, lpCursorName) )
end function
#define LoadCursorW LoadCursor

private function SetCursor( byval hCursor as HCURSOR ) as HCURSOR
    dim as HCURSOR prev = gLnxCurrentCursor
    gLnxCurrentCursor = hCursor
    dim as HWND h = iif(gLnxCapture andalso IsWindow(gLnxCapture), gLnxCapture, gLnxMouseHwnd)
    if h andalso IsWindow(h) andalso LnxIsNative(h) = false then
        dim as GdkWindow ptr gw = gtk_widget_get_window( h->client )
        if gw then gdk_window_set_cursor( gw, iif(hCursor = LnxCursorFromId(32512), NULL, cast(GdkCursor ptr, hCursor)) )
    end if
    return prev
end function

private function GetCursor() as HCURSOR
    if gLnxCurrentCursor = 0 then return LnxCursorFromId(32512)
    return gLnxCurrentCursor
end function

private function WindowFromPoint( byval pt as POINT ) as HWND
    dim as GdkDisplay ptr disp = gdk_display_get_default()
    dim as GdkDevice ptr dev = gdk_seat_get_pointer( gdk_display_get_default_seat(disp) )
    dim as gint wx, wy
    dim as GdkWindow ptr gw = gdk_device_get_window_at_position( dev, @wx, @wy )
    if gw = 0 then return 0
    dim as GtkWidget ptr w
    gdk_window_get_user_data( gw, cast(gpointer ptr, @w) )
    if w = 0 then return 0
    return LnxHwndFromWidget( w )
end function
#define RealChildWindowFrompoint(hParent, pt) WindowFromPoint(pt)
#define ChildWindowFromPoint(hParent, pt) WindowFromPoint(pt)

private function TrackMouseEvent( byval lpEventTrack as TRACKMOUSEEVENT ptr ) as BOOL
    if lpEventTrack = 0 then return 0
    dim as HWND hwnd = lpEventTrack->hwndTrack
    if IsWindow(hwnd) = 0 then return 0
    if (lpEventTrack->dwFlags and TME_CANCEL) then
        hwnd->trackFlags and= not (lpEventTrack->dwFlags and (TME_HOVER or TME_LEAVE))
        return 1
    end if
    if (lpEventTrack->dwFlags and TME_HOVER) andalso (hwnd->trackFlags and TME_HOVER) = 0 then
        hwnd->hoverTime = lpEventTrack->dwHoverTime
        if hwnd->hoverTimer then g_source_remove_( hwnd->hoverTimer )
        hwnd->hoverTimer = g_timeout_add( iif(hwnd->hoverTime = 0 orelse hwnd->hoverTime = HOVER_DEFAULT, 400, hwnd->hoverTime), _
                                          cast(GSourceFunc, @LnxHoverTimeout), hwnd )
    end if
    hwnd->trackFlags or= lpEventTrack->dwFlags and (TME_HOVER or TME_LEAVE)
    hwnd->mouseInside = true
    return 1
end function
#define _TrackMouseEvent TrackMouseEvent


' ----------------------------------------------------------------------------
' Timers
' ----------------------------------------------------------------------------
type LNX_TIMER
    hwnd   as HWND
    id     as UINT_PTR
    pfn    as TIMERPROC
    key    as long         ' unique key passed to the GLib callback
    source as guint        ' GLib source id
end type
dim shared as LNX_TIMER gLnxTimers(any)
dim shared as long gLnxTimerCount
dim shared as long gLnxTimerNextKey = 1

private function LnxTimerFire cdecl ( byval data_ as gpointer ) as gboolean
    dim as long key = cast(integer, data_)
    for i as long = 0 to gLnxTimerCount - 1
        if gLnxTimers(i).key = key then
            dim as HWND hwnd = gLnxTimers(i).hwnd
            dim as UINT_PTR id = gLnxTimers(i).id
            dim as TIMERPROC pfn = gLnxTimers(i).pfn
            if hwnd = 0 then
                ' timer without a window: call the timer procedure directly
                if pfn then pfn( 0, WM_TIMER, id, GetTickCount() )
                return TRUE
            end if
            if IsWindow(hwnd) = 0 then return TRUE
            ' Windows coalesces WM_TIMER: only post if one is not already queued.
            dim as long q = gLnxQHead
            while q <> gLnxQTail
                if gLnxQueue(q).message = WM_TIMER andalso gLnxQueue(q).hwnd = hwnd andalso _
                   gLnxQueue(q).wParam = id then return TRUE
                q = (q + 1) mod gLnxQCap
            wend
            dim as MSG m
            m.hwnd = hwnd
            m.message = WM_TIMER
            m.wParam = id
            m.lParam = cast(LPARAM, pfn)
            m.time = GetTickCount()
            LnxQueuePush( m )
            return TRUE
        end if
    next
    return FALSE   ' timer was killed
end function

private function KillTimer( byval hwnd as HWND, byval uIDEvent as UINT_PTR ) as BOOL
    for i as long = 0 to gLnxTimerCount - 1
        if gLnxTimers(i).hwnd = hwnd andalso gLnxTimers(i).id = uIDEvent then
            g_source_remove_( gLnxTimers(i).source )
            for j as long = i to gLnxTimerCount - 2
                gLnxTimers(j) = gLnxTimers(j + 1)
            next
            gLnxTimerCount -= 1
            return 1
        end if
    next
    return 0
end function

private function SetTimer( byval hwnd as HWND, byval nIDEvent as UINT_PTR, byval uElapse as UINT, byval lpTimerFunc as TIMERPROC ) as UINT_PTR
    KillTimer( hwnd, nIDEvent )
    if uElapse < 10 then uElapse = 10
    redim preserve gLnxTimers(gLnxTimerCount)
    with gLnxTimers(gLnxTimerCount)
        .hwnd = hwnd
        .id = nIDEvent
        .pfn = lpTimerFunc
        .key = gLnxTimerNextKey
        gLnxTimerNextKey += 1
        .source = g_timeout_add( uElapse, cast(GSourceFunc, @LnxTimerFire), cast(gpointer, cast(integer, .key)) )
    end with
    gLnxTimerCount += 1
    return nIDEvent
end function
