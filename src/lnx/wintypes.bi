'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Minimal subset of Win32 data types and constants used by the tiko sources so that
' the platform-neutral code (config, parser, projects, compiling) compiles unchanged.

#pragma once

#include once "vbcompat.bi"
#include once "crt.bi"
#include once "crt/locale.bi"

type BOOL      as long
type DWORD     as ulong
type WORD      as ushort
type UINT      as ulong
type UINT_PTR  as uinteger
type LONG_PTR  as integer
type ULONG_PTR as uinteger
type DWORD_PTR as uinteger
type WPARAM    as uinteger
type LPARAM    as integer
type LRESULT   as integer
type HRESULT   as long
type COLORREF  as ulong
type ATOM      as ushort
type HANDLE    as any ptr
type LPCWSTR   as const wstring ptr
type LPWSTR    as wstring ptr
type LPCSTR    as const zstring ptr

#ifndef NULL
    #define NULL 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

#ifndef MAX_PATH
#define MAX_PATH      260
#endif
#ifndef S_OK
#define S_OK          0
#endif
#ifndef S_FALSE
#define S_FALSE       1
#endif
#ifndef E_FAIL
#define E_FAIL        &h80004005
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG  &h80070057
#endif
#ifndef CP_ACP
#define CP_ACP        0
#endif
#ifndef CP_UTF8
#define CP_UTF8       65001
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL     &h80
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY  &h10
#endif

type RECT
    left   as long
    top    as long
    right  as long
    bottom as long
end type
type LPRECT as RECT ptr

type POINT
    x as long
    y as long
end type

type SIZE
    cx as long
    cy as long
end type

type FILETIME
    dwLowDateTime  as DWORD
    dwHighDateTime as DWORD
end type

type SYSTEMTIME
    wYear         as WORD
    wMonth        as WORD
    wDayOfWeek    as WORD
    wDay          as WORD
    wHour         as WORD
    wMinute       as WORD
    wSecond       as WORD
    wMilliseconds as WORD
end type

type GUID
    Data1    as ulong
    Data2    as ushort
    Data3    as ushort
    Data4(7) as ubyte
end type

' Windows COLORREF layout is 0x00BBGGRR
#define BGR(r,g,b)        cast(COLORREF, ((cast(ulong,(b)) and 255) shl 16) or ((cast(ulong,(g)) and 255) shl 8) or (cast(ulong,(r)) and 255))
#define GetRValue(c)      (cast(ulong,(c)) and 255)
#define GetGValue(c)      ((cast(ulong,(c)) shr 8) and 255)
#define GetBValue(c)      ((cast(ulong,(c)) shr 16) and 255)
#ifndef LOWORD
#define LOWORD(l)         cast(WORD, cast(ulong, (l)) and &hFFFF)
#endif
#ifndef HIWORD
#define HIWORD(l)         cast(WORD, (cast(ulong, (l)) shr 16) and &hFFFF)
#endif
#define MAKELONG(a, b)    cast(long, (cast(ulong,(a)) and &hFFFF) or ((cast(ulong,(b)) and &hFFFF) shl 16))
#define MAKEWPARAM(l, h)  cast(WPARAM, cast(DWORD, MAKELONG(l, h)))
#define MAKELPARAM(l, h)  cast(LPARAM, cast(DWORD, MAKELONG(l, h)))
#define GET_X_LPARAM(lp)  cast(long, cast(short, LOWORD(lp)))
#define GET_Y_LPARAM(lp)  cast(long, cast(short, HIWORD(lp)))


'  FreeBASIC returns a NULL pointer for strptr("") which GTK rejects; LnxZ never does.
private function LnxZ( byref s as const string ) as zstring ptr
    static as zstring * 1 empty
    if len(s) = 0 then return @empty
    return cast(zstring ptr, strptr(s))
end function

private function SetRect( byval lprc as RECT ptr, byval l as long, byval t as long, byval r as long, byval b as long ) as BOOL
    if lprc = 0 then return 0
    lprc->left = l : lprc->top = t : lprc->right = r : lprc->bottom = b
    return 1
end function

private function SetRectEmpty( byval lprc as RECT ptr ) as BOOL
    return SetRect( lprc, 0, 0, 0, 0 )
end function

private function IsRectEmpty( byval lprc as const RECT ptr ) as BOOL
    if lprc = 0 then return 1
    return iif( lprc->right <= lprc->left orelse lprc->bottom <= lprc->top, 1, 0 )
end function

private function PtInRect( byval lprc as const RECT ptr, byval pt as POINT ) as BOOL
    if lprc = 0 then return 0
    return iif( pt.x >= lprc->left andalso pt.x < lprc->right andalso _
                pt.y >= lprc->top andalso pt.y < lprc->bottom, 1, 0 )
end function

private function InflateRect( byval lprc as RECT ptr, byval dx as long, byval dy as long ) as BOOL
    if lprc = 0 then return 0
    lprc->left -= dx : lprc->right += dx : lprc->top -= dy : lprc->bottom += dy
    return 1
end function

private function OffsetRect( byval lprc as RECT ptr, byval dx as long, byval dy as long ) as BOOL
    if lprc = 0 then return 0
    lprc->left += dx : lprc->right += dx : lprc->top += dy : lprc->bottom += dy
    return 1
end function

private function CopyRect( byval dst as RECT ptr, byval src as const RECT ptr ) as BOOL
    if dst = 0 orelse src = 0 then return 0
    *dst = *src
    return 1
end function

private function IntersectRect( byval dst as RECT ptr, byval a as const RECT ptr, byval b as const RECT ptr ) as BOOL
    dst->left = iif(a->left > b->left, a->left, b->left)
    dst->top = iif(a->top > b->top, a->top, b->top)
    dst->right = iif(a->right < b->right, a->right, b->right)
    dst->bottom = iif(a->bottom < b->bottom, a->bottom, b->bottom)
    if IsRectEmpty(dst) then SetRectEmpty(dst) : return 0
    return 1
end function
