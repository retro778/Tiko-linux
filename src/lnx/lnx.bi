'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Platform layer umbrella include. Replaces windows.bi + AfxNova for the Linux build.

#pragma once

#include once "wintypes.bi"
#include once "dwstring.bi"
#include once "AfxStr.inc"
#include once "afxfile.bi"
#include once "textstream.bi"

#ifndef LNX_NO_GUI
#include once "gtk/gtk3.bi"
#include once "winconst.bi"
#include once "scintilla_gtk.bi"
#include once "winapi.bi"
#include once "gdi.inc"
#include once "window.inc"
#include once "controls.inc"
#include once "cwindow.inc"
#include once "comdlg.inc"
#include once "winmisc.inc"
#include once "process.inc"
#include once "winextra.inc"
#include once "afxextra.inc"
#include once "ipc.inc"

' CreateWindowEx: used for Scintilla windows and generic child windows
private function CreateWindowEx( byval dwExStyle as DWORD, byval lpClassName as LPCWSTR, byval lpWindowName as LPCWSTR, _
            byval dwStyle as DWORD, byval x as long, byval y as long, byval nWidth as long, byval nHeight as long, _
            byval hWndParent as HWND, byval hMenu as HMENU, byval hInstance as HINSTANCE, byval lpParam as any ptr ) as HWND
    dim as DWSTRING wszClass = iif(lpClassName, *lpClassName, wstr(""))
    dim as DWSTRING wszTitle = iif(lpWindowName, *lpWindowName, wstr(""))
    if (dwStyle and WS_CHILD) andalso LnxKindFromClass(wszClass, dwStyle) <> LK_CUSTOM then
        return LnxCreateControl( wszClass, hWndParent, cast(LONG_PTR, hMenu), wszTitle, x, y, nWidth, nHeight, dwStyle, dwExStyle )
    end if
    return LnxCreateWindow( iif(dwStyle and WS_CHILD, LK_CUSTOM, LK_TOPLEVEL), wszClass, wszTitle, dwStyle, dwExStyle, _
                            x, y, nWidth, nHeight, hWndParent, cast(LONG_PTR, hMenu), NULL, lpParam )
end function
#define CreateWindowExW CreateWindowEx

declare function setenv cdecl alias "setenv" ( byval as const zstring ptr, byval as const zstring ptr, byval as long ) as long

' Must be called once at program start (before any window is created).
private sub LnxInit()
    setlocale( LC_ALL, "" )
    gLnxTrace = (len(environ("LNX_TRACE")) > 0)
    ' Win32 style absolute window positioning needs X11 (XWayland on Wayland desktops).
    if len(environ("GDK_BACKEND")) = 0 then setenv( "GDK_BACKEND", "x11", 1 )
    ' Native controls use the light Adwaita theme (like Windows dialogs); the desktop's settings
    ' daemon would otherwise override the theme chosen at runtime.
    if len(environ("GTK_THEME")) = 0 then setenv( "GTK_THEME", "Adwaita", 1 )
    gtk_init( NULL, NULL )
    ' numbers must keep using "." (FreeBASIC val/str expect C locale numerics)
    setlocale( LC_NUMERIC, "C" )
    ' Windows dialogs/controls are light themed: use the light Adwaita theme for native controls
    ' (tiko paints its own dark/light themed windows itself).
    g_object_set( gtk_settings_get_default(), "gtk-theme-name", @"Adwaita", NULL )
    dim as gboolean bFalse = 0
    g_object_set( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", bFalse, NULL )
    ' Selawik: metric compatible Segoe UI replacement (bin/selawk*.ttf)
    dim as DWSTRING wszFont
    dim as string fonts(2) = { "selawk.ttf", "selawkb.ttf", "selawksb.ttf" }
    for i as long = 0 to 2
        wszFont = AfxGetExePathName & "bin/" & fonts(i)
        AddFontResourceEx( wszFont.vptr, 0, NULL )
    next
    LnxInstallControlCss()
end sub
#endif
