'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Miscellaneous Win32 functions used by tiko:
'   - process enumeration (Toolhelp32) on /proc
'   - private profile (INI) strings that keep the file's UTF-16 encoding
'   - font family enumeration through Pango
'   - small type aliases that only existed in windows.bi

#pragma once

#include once "crt/linux/unistd.bi"

type SIZEL as SIZE
type UNIT as UINT                ' typo used in upstream modRoutines.inc (also exists in windows.bi)
type WINBOOL as long
type PDWORD as DWORD ptr
type LPDWORD as DWORD ptr
type LPVOID as any ptr
type BYTE_ as ubyte
type BSTRING as DWSTRING          ' AfxNova COM string class; only used as a plain string by tiko

#ifndef MB_ERR_INVALID_CHARS
#define MB_ERR_INVALID_CHARS      &h00000008
#endif

#ifndef SYNCHRONIZE
#define SYNCHRONIZE               &h00100000
#endif
#ifndef PROCESS_TERMINATE
#define PROCESS_TERMINATE         &h0001
#endif

' ----------------------------------------------------------------------------
' Processes
' ----------------------------------------------------------------------------
type PROCESSENTRY32W
    dwSize              as DWORD
    cntUsage            as DWORD
    th32ProcessID       as DWORD
    th32DefaultHeapID   as ULONG_PTR
    th32ModuleID        as DWORD
    cntThreads          as DWORD
    th32ParentProcessID as DWORD
    pcPriClassBase      as long
    dwFlags             as DWORD
    szExeFile           as wstring * MAX_PATH
end type

type LNXSNAPSHOT
    pids(any) as long
    count     as long
    pos_      as long
end type

type LNXPROCHANDLE
    magic as ulong         ' &h50524F43 "PROC"
    pid   as long
end type

private function LnxProcExeName( byval pid as long ) as DWSTRING
    ' /proc/<pid>/comm is truncated to 15 chars; use the exe link when possible
    dim as zstring * 4097 buf
    dim as string lnk = "/proc/" & pid & "/exe"
    dim as integer n = readlink( strptr(lnk), @buf, 4096 )
    if n > 0 then
        buf[n] = 0
        return AfxStrPathname( "NAMEX", DWSTRING(buf) )
    end if
    dim as long f = freefile
    dim as string s
    if open( "/proc/" & pid & "/comm" for input as #f ) = 0 then
        line input #f, s
        close #f
    end if
    return DWSTRING( s )
end function

private function CreateToolhelp32Snapshot( byval dwFlags as DWORD, byval th32ProcessID as DWORD ) as HANDLE
    dim as LNXSNAPSHOT ptr snap = new LNXSNAPSHOT
    dim as string d = dir( "/proc/*", fbDirectory )
    while len(d)
        if val(d) > 0 andalso str(val(d)) = d then
            redim preserve snap->pids(snap->count)
            snap->pids(snap->count) = val(d)
            snap->count += 1
        end if
        d = dir()
    wend
    return snap
end function

private function Process32NextW( byval hSnapshot as HANDLE, byval lppe as PROCESSENTRY32W ptr ) as BOOL
    dim as LNXSNAPSHOT ptr snap = hSnapshot
    if snap = 0 orelse lppe = 0 then return 0
    if snap->pos_ >= snap->count then return 0
    dim as long pid = snap->pids(snap->pos_)
    snap->pos_ += 1
    lppe->th32ProcessID = pid
    lppe->szExeFile = LnxProcExeName( pid )
    return 1
end function

private function Process32FirstW( byval hSnapshot as HANDLE, byval lppe as PROCESSENTRY32W ptr ) as BOOL
    dim as LNXSNAPSHOT ptr snap = hSnapshot
    if snap = 0 then return 0
    snap->pos_ = 0
    return Process32NextW( hSnapshot, lppe )
end function

private function OpenProcess( byval dwDesiredAccess as DWORD, byval bInheritHandle as BOOL, byval dwProcessId as DWORD ) as HANDLE
    if dwProcessId = 0 then return 0
    if AfxFileExists( "/proc/" & dwProcessId ) = false then return 0
    dim as LNXPROCHANDLE ptr p = new LNXPROCHANDLE
    p->magic = &h50524F43
    p->pid = dwProcessId
    return p
end function

private function GetCurrentProcessId() as DWORD
    return getpid()
end function

' ----------------------------------------------------------------------------
' INI files (GetPrivateProfileString / WritePrivateProfileString)
' The file keeps its encoding (UTF-16LE with BOM when it already was unicode).
' ----------------------------------------------------------------------------
private function LnxIniLoad( byref wszFile as const wstring, lines() as DWSTRING, byref bUnicode as boolean ) as long
    dim as string raw
    erase lines
    bUnicode = false
    if ReadFileBytes( wszFile, raw ) = false then return 0
    dim as DWSTRING txt = DecodeTextBuffer( raw, bUnicode )
    dim as long n = 0, i = 1, l = len(txt)
    while i <= l
        dim as long j = i
        while j <= l andalso txt[j - 1] <> 10 andalso txt[j - 1] <> 13 : j += 1 : wend
        redim preserve lines(n)
        lines(n) = mid(txt, i, j - i)
        n += 1
        if j <= l andalso txt[j - 1] = 13 andalso j < l andalso txt[j] = 10 then j += 1
        i = j + 1
    wend
    return n
end function

private function LnxIniSave( byref wszFile as const wstring, lines() as DWSTRING, byval n as long, byval bUnicode as boolean ) as boolean
    dim as CTextStream st
    if st.Create( wszFile, true, bUnicode ) <> S_OK then return false
    for i as long = 0 to n - 1
        st.WriteLine( lines(i) )
    next
    st.Close()
    return true
end function

private function WritePrivateProfileString( byval lpAppName as LPCWSTR, byval lpKeyName as LPCWSTR, _
                                            byval lpString as LPCWSTR, byval lpFileName as LPCWSTR ) as BOOL
    if lpAppName = 0 orelse lpFileName = 0 then return 0
    dim as DWSTRING lines(any)
    dim as boolean bUnicode = true
    dim as long n = 0
    if AfxFileExists(*lpFileName) then n = LnxIniLoad( *lpFileName, lines(), bUnicode )
    dim as DWSTRING sect = "[" & ucase(*lpAppName) & "]"
    ' locate the section
    dim as long sStart = -1, sEnd = n
    for i as long = 0 to n - 1
        dim as DWSTRING t = ucase(trim(lines(i)))
        if sStart = -1 then
            if t = sect then sStart = i
        elseif left(t, 1) = "[" then
            sEnd = i : exit for
        end if
    next
    dim as DWSTRING outLines(any)
    dim as long m = 0
    #macro LNX_ADDLINE(x)
        redim preserve outLines(m) : outLines(m) = x : m += 1
    #endmacro
    if lpKeyName = 0 then
        ' delete the whole section
        for i as long = 0 to n - 1
            if sStart <> -1 andalso i >= sStart andalso i < sEnd then continue for
            LNX_ADDLINE(lines(i))
        next
    else
        dim as DWSTRING key = ucase(*lpKeyName)
        dim as boolean done = false
        if sStart = -1 then
            for i as long = 0 to n - 1 : LNX_ADDLINE(lines(i)) : next
            if lpString then
                LNX_ADDLINE("[" & *lpAppName & "]")
                LNX_ADDLINE(*lpKeyName & "=" & *lpString)
            end if
        else
            for i as long = 0 to n - 1
                if i > sStart andalso i < sEnd andalso done = false then
                    dim as long eq = instr(lines(i), "=")
                    if eq andalso ucase(trim(left(lines(i), eq - 1))) = key then
                        if lpString then LNX_ADDLINE(*lpKeyName & "=" & *lpString)
                        done = true
                        continue for
                    end if
                end if
                if i = sEnd andalso done = false andalso lpString then
                    ' insert before the trailing blank lines of the section
                    dim as long ins = m
                    while ins > 0 andalso len(trim(outLines(ins - 1))) = 0 : ins -= 1 : wend
                    LNX_ADDLINE("")
                    for k as long = m - 1 to ins + 1 step -1 : outLines(k) = outLines(k - 1) : next
                    outLines(ins) = *lpKeyName & "=" & *lpString
                    done = true
                end if
                LNX_ADDLINE(lines(i))
            next
            if done = false andalso lpString then LNX_ADDLINE(*lpKeyName & "=" & *lpString)
        end if
    end if
    return iif( LnxIniSave( *lpFileName, outLines(), m, bUnicode ), 1, 0 )
end function
#define WritePrivateProfileStringW WritePrivateProfileString

private function GetPrivateProfileString( byval lpAppName as LPCWSTR, byval lpKeyName as LPCWSTR, byval lpDefault as LPCWSTR, _
                                          byval lpReturnedString as LPWSTR, byval nSize as DWORD, byval lpFileName as LPCWSTR ) as DWORD
    dim as DWSTRING result = iif(lpDefault, *lpDefault, wstr(""))
    if lpAppName andalso lpKeyName andalso lpFileName then
        dim as DWSTRING lines(any)
        dim as boolean bUnicode
        dim as long n = LnxIniLoad( *lpFileName, lines(), bUnicode )
        dim as boolean inSect = false
        for i as long = 0 to n - 1
            dim as DWSTRING t = trim(lines(i))
            if left(t, 1) = "[" then
                inSect = (ucase(t) = "[" & ucase(*lpAppName) & "]")
            elseif inSect then
                dim as long eq = instr(t, "=")
                if eq andalso ucase(trim(left(t, eq - 1))) = ucase(*lpKeyName) then result = mid(t, eq + 1) : exit for
            end if
        next
    end if
    if lpReturnedString = 0 orelse nSize = 0 then return 0
    dim as long l = len(result)
    if l > nSize - 1 then l = nSize - 1
    memcpy( lpReturnedString, result.vptr, l * sizeof(wstring) )
    cast(ulong ptr, lpReturnedString)[l] = 0
    return l
end function
#define GetPrivateProfileStringW GetPrivateProfileString

' ----------------------------------------------------------------------------
' Fonts
' ----------------------------------------------------------------------------
#ifndef TRUETYPE_FONTTYPE
#define TRUETYPE_FONTTYPE &h0004
#endif
type FONTENUMPROCW as function( byref lf as LOGFONTW, byref tm as TEXTMETRIC, byval FontType as long, byval lParam as LPARAM ) as long

private function EnumFontFamilies( byval hdc as HDC, byval lpszFamily as LPCWSTR, byval lpProc as FONTENUMPROCW, byval lParam as LPARAM ) as long
    dim as PangoFontMap ptr fm = pango_cairo_font_map_get_default()
    dim as PangoFontFamily ptr ptr families
    dim as long n
    pango_font_map_list_families( fm, @families, @n )
    dim as long r = 1
    for i as long = 0 to n - 1
        dim as LOGFONTW lf
        dim as TEXTMETRIC tm
        lf.lfFaceName = DWSTRING( *pango_font_family_get_name(families[i]) )
        if pango_font_family_is_monospace( families[i] ) then lf.lfPitchAndFamily = 1 : tm.tmPitchAndFamily = 0 _
        else lf.lfPitchAndFamily = 2 : tm.tmPitchAndFamily = 1        ' TMPF_FIXED_PITCH bit is set for VARIABLE pitch
        tm.tmPitchAndFamily or= TMPF_TRUETYPE or TMPF_VECTOR
        r = lpProc( lf, tm, TRUETYPE_FONTTYPE, lParam )
        if r = 0 then exit for
    next
    g_free( families )
    return r
end function
#define EnumFontFamiliesW EnumFontFamilies
#define EnumFontFamiliesEx(hdc, lpLogfont, lpProc, lParam, dwFlags) EnumFontFamilies(hdc, NULL, lpProc, lParam)

' ----------------------------------------------------------------------------
' Code page conversions. CP_ACP is Windows-1252 (the "ANSI" encoding of files that
' were created on Windows). Wide characters are UTF-32 on Linux.
' ----------------------------------------------------------------------------
private function MultiByteToWideChar( byval CodePage as UINT, byval dwFlags as DWORD, byval lpMultiByteStr as const zstring ptr, _
            byval cbMultiByte as long, byval lpWideCharStr as LPWSTR, byval cchWideChar as long ) as long
    if lpMultiByteStr = 0 then return 0
    if cbMultiByte < 0 then cbMultiByte = len(*lpMultiByteStr) + 1
    dim as string src = space(cbMultiByte)
    memcpy( strptr(src), lpMultiByteStr, cbMultiByte )
    dim as string u8
    if CodePage = CP_UTF8 then
        if (dwFlags and MB_ERR_INVALID_CHARS) then
            if g_utf8_validate( strptr(src), cbMultiByte, NULL ) = 0 then return 0
        end if
        u8 = src
    else
        dim as gsize nOut
        dim as zstring ptr p = g_convert( strptr(src), cbMultiByte, "UTF-8", "WINDOWS-1252", NULL, @nOut, NULL )
        if p = 0 then return 0
        u8 = space(nOut) : memcpy( strptr(u8), p, nOut ) : g_free( p )
    end if
    dim as wstring ptr w = Utf8ToWide( u8 )
    dim as long n = 0
    while cast(ulong ptr, w)[n] : n += 1 : wend
    ' embedded NULs are not preserved by Utf8ToWide; the length is what callers use
    if lpWideCharStr andalso cchWideChar > 0 then
        dim as long c = iif(n < cchWideChar, n, cchWideChar)
        memcpy( lpWideCharStr, w, c * sizeof(wstring) )
    end if
    deallocate( w )
    return n
end function

private function WideCharToMultiByte( byval CodePage as UINT, byval dwFlags as DWORD, byval lpWideCharStr as const wstring ptr, _
            byval cchWideChar as long, byval lpMultiByteStr as zstring ptr, byval cbMultiByte as long, _
            byval lpDefaultChar as const zstring ptr, byval lpUsedDefaultChar as long ptr ) as long
    if lpWideCharStr = 0 then return 0
    dim as string out_ = WideToUtf8( lpWideCharStr, cchWideChar )
    if CodePage <> CP_UTF8 then
        dim as gsize nOut
        dim as zstring ptr p = g_convert_with_fallback( strptr(out_), len(out_), "WINDOWS-1252", "UTF-8", "?", NULL, @nOut, NULL )
        if p = 0 then return 0
        out_ = space(nOut) : memcpy( strptr(out_), p, nOut ) : g_free( p )
    end if
    if lpMultiByteStr andalso cbMultiByte > 0 then
        dim as long c = iif(len(out_) < cbMultiByte, len(out_), cbMultiByte)
        memcpy( lpMultiByteStr, strptr(out_), c )
    end if
    return len(out_)
end function

' Converts UTF-8 text to Windows "ANSI" (code page 1252)
private function LnxUtf8ToAnsi( byref sUtf8 as const string ) as string
    if len(sUtf8) = 0 then return ""
    dim as gsize nOut
    dim as zstring ptr p = g_convert_with_fallback( strptr(sUtf8), len(sUtf8), "WINDOWS-1252", "UTF-8", "?", NULL, @nOut, NULL )
    if p = 0 then return sUtf8
    dim as string r = space(nOut)
    memcpy( strptr(r), p, nOut )
    g_free( p )
    return r
end function

#ifndef FILE_NAME_NORMALIZED_
#define FILE_NAME_NORMALIZED_ 0
#endif
private function GetFinalPathNameByHandleW( byval hFile as HANDLE, byval lpszFilePath as LPWSTR, byval cchFilePath as DWORD, byval dwFlags as DWORD ) as DWORD
    if hFile = 0 orelse hFile = INVALID_HANDLE_VALUE then return 0
    dim as long fd = *cast(long ptr, cast(ubyte ptr, hFile) + 8)    ' LNXKHANDLE.fd
    dim as zstring * 4097 buf
    dim as string lnk = "/proc/self/fd/" & fd
    dim as integer n = readlink( strptr(lnk), @buf, 4096 )
    if n <= 0 then return 0
    buf[n] = 0
    dim as DWSTRING dws = buf
    if lpszFilePath andalso cchFilePath > len(dws) then memcpy( lpszFilePath, dws.vptr, (len(dws) + 1) * sizeof(wstring) )
    return len(dws)
end function
