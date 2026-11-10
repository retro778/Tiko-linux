'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Cross process FindWindow / WM_COPYDATA, used for the single instance feature
' ("Allow multiple instances" unchecked): a second tiko finds the running editor by its
' window class, sends it the command line with WM_COPYDATA and exits.
'
' Every top level window that has a class name listens on a per user unix socket named after
' the class. FindWindow() for a class that does not exist in this process connects to that
' socket and returns a proxy HWND; SendMessage(proxy, WM_COPYDATA) and SetForegroundWindow(proxy)
' are forwarded over the socket.

#pragma once

extern "C"
    declare function g_unix_socket_address_new( byval path as const zstring ptr ) as GSocketAddress ptr
    declare function gdk_x11_window_get_xid( byval window as GdkWindow ptr ) as culong
end extern

type LNXIPCLISTENER
    hwnd    as HWND
    service as GSocketService ptr
    path    as string
end type
dim shared as LNXIPCLISTENER gLnxIpcListeners(any)

private function LnxIpcSocketPath( byref wszClass as const wstring ) as string
    dim as string sClass = lcase(WideToUtf8(@wszClass)), safe
    for i as long = 1 to len(sClass)
        dim as string c = mid(sClass, i, 1)
        safe &= iif( (c >= "a" andalso c <= "z") orelse (c >= "0" andalso c <= "9") orelse c = "_", c, "_" )
    next
    return *g_get_user_runtime_dir() & "/" & safe & ".sock"
end function

' Bring one of our own top level windows to the front even when another application is active
private sub LnxForceActivate( byval hwnd as HWND )
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then exit sub
    if IsIconic(hwnd) then gtk_window_deiconify( GTK_WINDOW(hwnd->widget) )
    gtk_window_present( GTK_WINDOW(hwnd->widget) )
    dim as GdkWindow ptr gw = gtk_widget_get_window( hwnd->widget )
    if gw = 0 then exit sub
    dim as any ptr d = gdk_x11_get_default_xdisplay()
    if d = 0 then exit sub
    dim as clong ev(23)
    cast(long ptr, @ev(0))[0] = 33                    ' ClientMessage
    cast(long ptr, @ev(2))[0] = 1
    ev(4) = gdk_x11_window_get_xid( gw )
    ev(5) = XInternAtom( d, "_NET_ACTIVE_WINDOW", 0 )
    cast(long ptr, @ev(6))[0] = 32
    ev(7) = 2                                         ' source = pager
    XSendEvent( d, XDefaultRootWindow(d), 0, (1 shl 20) or (1 shl 19), @ev(0) )
    XFlush( d )
end sub

' Incoming connection: "<command>\n<dwData>\n<utf8 payload>"
private function LnxIpcOnIncoming cdecl ( byval service as GSocketService ptr, byval conn as GSocketConnection ptr, _
                                          byval source as GObject ptr, byval hwnd as HWND ) as gboolean
    dim as GInputStream ptr inStream = g_io_stream_get_input_stream( G_IO_STREAM(conn) )
    dim as string buf = space(65536), msg
    do
        dim as gssize n = g_input_stream_read( inStream, strptr(buf), len(buf), NULL, NULL )
        if n <= 0 then exit do
        msg &= left(buf, n)
        if len(msg) > 4 * 1024 * 1024 then exit do
    loop
    if IsWindow(hwnd) = 0 then return TRUE
    dim as long p1 = instr(msg, !"\n")
    dim as string cmd = iif(p1, left(msg, p1 - 1), msg)
    LnxForceActivate( hwnd )
    if cmd = "COPYDATA" then
        dim as long p2 = instr(p1 + 1, msg, !"\n")
        if p2 = 0 then return TRUE
        dim as COPYDATASTRUCT cds
        cds.dwData = valulng( mid(msg, p1 + 1, p2 - p1 - 1) )
        dim as DWSTRING payload = DWSTRING( mid(msg, p2 + 1), CP_UTF8 )
        cds.cbData = (len(payload) + 1) * sizeof(wstring)
        cds.lpData = payload.vptr
        SendMessage( hwnd, WM_COPYDATA, 0, cast(LPARAM, @cds) )
    end if
    return TRUE
end function

private function LnxIpcSend( byref sPath as const string, byref sMsg as const string ) as boolean
    dim as GSocketAddress ptr addr = g_unix_socket_address_new( strptr(sPath) )
    dim as GSocketClient ptr client = g_socket_client_new()
    dim as GSocketConnection ptr conn = g_socket_client_connect( client, G_SOCKET_CONNECTABLE(addr), NULL, NULL )
    dim as boolean ok = false
    if conn then
        if len(sMsg) then
            dim as GOutputStream ptr outp = g_io_stream_get_output_stream( G_IO_STREAM(conn) )
            dim as gsize written
            ok = g_output_stream_write_all( outp, strptr(sMsg), len(sMsg), @written, NULL, NULL ) <> 0
        else
            ok = true
        end if
        g_io_stream_close( G_IO_STREAM(conn), NULL, NULL )
        g_object_unref( conn )
    end if
    g_object_unref( client )
    g_object_unref( addr )
    return ok
end function

' Called for every new top level window with a class name
private sub LnxIpcStartListener( byval hwnd as HWND )
    if len(hwnd->classname) = 0 then exit sub
    dim as string sPath = LnxIpcSocketPath( hwnd->classname )
    for i as long = 0 to ubound(gLnxIpcListeners)
        if gLnxIpcListeners(i).path = sPath then exit sub
    next
    ' Another instance already listening: keep its socket (multiple instances allowed)
    if LnxIpcSend( sPath, "" ) then exit sub
    remove( strptr(sPath) )        ' stale socket of a crashed instance
    dim as GSocketService ptr svc = g_socket_service_new()
    dim as GSocketAddress ptr addr = g_unix_socket_address_new( strptr(sPath) )
    dim as gboolean ok = g_socket_listener_add_address( G_SOCKET_LISTENER(svc), addr, G_SOCKET_TYPE_STREAM, _
                                                        G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, NULL )
    g_object_unref( addr )
    if ok = 0 then g_object_unref( svc ) : exit sub
    g_signal_connect( G_OBJECT(svc), "incoming", G_CALLBACK(@LnxIpcOnIncoming), hwnd )
    g_socket_service_start( svc )
    dim as long n = ubound(gLnxIpcListeners) + 1
    redim preserve gLnxIpcListeners(n)
    gLnxIpcListeners(n).hwnd = hwnd
    gLnxIpcListeners(n).service = svc
    gLnxIpcListeners(n).path = sPath
end sub

private sub LnxIpcStopListener( byval hwnd as HWND )
    for i as long = 0 to ubound(gLnxIpcListeners)
        if gLnxIpcListeners(i).hwnd = hwnd andalso gLnxIpcListeners(i).service then
            g_socket_service_stop( gLnxIpcListeners(i).service )
            g_socket_listener_close( G_SOCKET_LISTENER(gLnxIpcListeners(i).service) )
            g_object_unref( gLnxIpcListeners(i).service )
            gLnxIpcListeners(i).service = 0
            remove( strptr(gLnxIpcListeners(i).path) )
            gLnxIpcListeners(i).hwnd = 0
        end if
    next
end sub

' Proxy windows for top level windows of other tiko processes
private function LnxIpcFindRemote( byref wszClass as const wstring ) as HWND
    dim as string sPath = LnxIpcSocketPath( wszClass )
    if LnxIpcSend( sPath, "" ) = false then return 0
    for i as long = 0 to gLnxWindowCount - 1
        if gLnxWindows(i)->kind = LK_REMOTE andalso gLnxWindows(i)->classname = wszClass then return gLnxWindows(i)
    next
    dim as HWND h = LnxNewWindow( LK_REMOTE, 0 )
    h->classname = wszClass
    h->text = sPath
    return h
end function

private function LnxIpcRemoteMessage( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    dim as string sPath = hwnd->text.utf8
    select case uMsg
        case WM_COPYDATA
            dim as COPYDATASTRUCT ptr pcds = cast(COPYDATASTRUCT ptr, lParam)
            if pcds = 0 then return 0
            dim as string payload
            if pcds->lpData then payload = WideToUtf8( cast(wstring ptr, pcds->lpData) )
            return iif( LnxIpcSend( sPath, "COPYDATA" & !"\n" & pcds->dwData & !"\n" & payload ), 1, 0 )
    end select
    return 0
end function
