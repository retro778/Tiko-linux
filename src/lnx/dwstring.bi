'    tiko editor (Linux port) - Programmer's Code Editor for the FreeBASIC Compiler
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' DWSTRING - dynamic wide string compatible with the AfxNova DWSTRING used by
' the Windows version. On Linux a wstring character is 4 bytes (UTF-32).
' Narrow `string` values are treated as UTF-8 when converted.

#pragma once

#include once "crt/string.bi"

declare function Utf8ToWide( byref s as const string ) as wstring ptr        ' caller frees with Deallocate
declare function WideToUtf8( byval w as const wstring ptr, byval n as long = -1 ) as string

type DWSTRING extends wstring
    m_pBuffer  as wstring ptr
    m_BufferLen as long          ' length in characters
    m_Capacity  as long          ' capacity in characters (excluding terminator)

    declare constructor()
    declare constructor( byval pwsz as const wstring ptr )
    declare constructor( byref ws as const DWSTRING )
    declare constructor( byref s as const string )
    declare constructor( byref s as const string, byval codepage as long )
    declare constructor( byval n as longint )
    declare constructor( byval n as double )
    declare destructor()

    declare operator let( byval pwsz as const wstring ptr )
    declare operator let( byref ws as const DWSTRING )
    declare operator let( byref s as const string )
    declare operator let( byval n as longint )
    declare operator let( byval n as double )

    declare operator &=( byval pwsz as const wstring ptr )
    declare operator &=( byref ws as const DWSTRING )
    declare operator &=( byref s as const string )
    declare operator &=( byval n as longint )
    declare operator &=( byval n as double )
    declare operator +=( byval pwsz as const wstring ptr )
    declare operator +=( byref ws as const DWSTRING )
    declare operator +=( byref s as const string )

    declare operator cast() byref as wstring
    declare operator cast() as any ptr
    declare operator []( byval nIndex as ulong ) byref as ulong
    declare constructor( byval nChars as long, byref wszFill as const wstring )
    declare function Add overload ( byval pwszStr as const wstring ptr ) as boolean
    declare function Add overload ( byref s as const string ) as boolean
    declare function Add overload ( byref dws as const DWSTRING ) as boolean
    declare function GetBuffer() as wstring ptr
    declare function ResizeBuffer( byval nChars as ulong, byval bClear as boolean = false ) as wstring ptr
    declare property Capacity() as ulong
    declare property Capacity( byval nValue as ulong )
    declare function ChrW( byval codepoint as ulong ) as DWSTRING

    declare sub Reserve( byval nChars as long )
    declare sub AppendBuffer( byval p as const wstring ptr, byval n as long )
    declare sub SetBuffer( byval p as const wstring ptr, byval n as long )
    declare sub Clear()

    declare function vptr() as wstring ptr
    declare function sptr() as wstring ptr
    declare function wchar() as wstring ptr
    declare function bstr() as wstring ptr
    declare property utf8() as string
    declare property utf8( byref s as const string )
    declare function Length() as long
end type

declare operator len( byref ws as const DWSTRING ) as integer
declare operator * ( byref ws as const DWSTRING ) as wstring ptr


' ----------------------------------------------------------------------------
' UTF-8 <-> UTF-32
' ----------------------------------------------------------------------------
' Windows-1252 code points for bytes &h80..&h9F
dim shared as ushort LnxCp1252High(31) = { _
    &h20AC, &h0081, &h201A, &h0192, &h201E, &h2026, &h2020, &h2021, &h02C6, &h2030, &h0160, &h2039, &h0152, &h008D, &h017D, &h008F, _
    &h0090, &h2018, &h2019, &h201C, &h201D, &h2022, &h2013, &h2014, &h02DC, &h2122, &h0161, &h203A, &h0153, &h009D, &h017E, &h0178 }

private function Utf8ToWide( byref s as const string ) as wstring ptr
    dim as long n = len(s)
    dim as ulong ptr pOut = allocate( (n + 1) * 4 )
    dim as ubyte ptr p = cast(ubyte ptr, strptr(s))
    dim as long i = 0, o = 0
    while i < n
        dim as ulong c = p[i]
        if c < &h80 then
            i += 1
        elseif (c and &hE0) = &hC0 andalso i + 1 < n then
            c = ((c and &h1F) shl 6) or (p[i+1] and &h3F) : i += 2
        elseif (c and &hF0) = &hE0 andalso i + 2 < n then
            c = ((c and &h0F) shl 12) or ((p[i+1] and &h3F) shl 6) or (p[i+2] and &h3F) : i += 3
        elseif (c and &hF8) = &hF0 andalso i + 3 < n then
            c = ((c and &h07) shl 18) or ((p[i+1] and &h3F) shl 12) or ((p[i+2] and &h3F) shl 6) or (p[i+3] and &h3F) : i += 4
        else
            ' invalid UTF-8: the text is Windows "ANSI" (code page 1252)
            if c >= &h80 andalso c <= &h9F then c = LnxCp1252High(c - &h80)
            i += 1
        end if
        pOut[o] = c : o += 1
    wend
    pOut[o] = 0
    return cast(wstring ptr, pOut)
end function

private function WideToUtf8( byval w as const wstring ptr, byval n as long = -1 ) as string
    if w = 0 then return ""
    dim as const ulong ptr p = cast(const ulong ptr, w)
    if n < 0 then
        n = 0 : while p[n] : n += 1 : wend
    end if
    dim as string s = space(n * 4)
    dim as ubyte ptr d = cast(ubyte ptr, strptr(s))
    dim as long o = 0
    for i as long = 0 to n - 1
        dim as ulong c = p[i]
        if c < &h80 then
            d[o] = c : o += 1
        elseif c < &h800 then
            d[o] = &hC0 or (c shr 6) : d[o+1] = &h80 or (c and &h3F) : o += 2
        elseif c < &h10000 then
            d[o] = &hE0 or (c shr 12) : d[o+1] = &h80 or ((c shr 6) and &h3F) : d[o+2] = &h80 or (c and &h3F) : o += 3
        else
            d[o] = &hF0 or (c shr 18) : d[o+1] = &h80 or ((c shr 12) and &h3F)
            d[o+2] = &h80 or ((c shr 6) and &h3F) : d[o+3] = &h80 or (c and &h3F) : o += 4
        end if
    next
    return left(s, o)
end function

private function wlen_( byval p as const wstring ptr ) as long
    if p = 0 then return 0
    dim as const ulong ptr q = cast(const ulong ptr, p)
    dim as long n = 0
    while q[n] : n += 1 : wend
    return n
end function


' ----------------------------------------------------------------------------
' DWSTRING implementation
' ----------------------------------------------------------------------------
private sub DWSTRING.Reserve( byval nChars as long )
    if nChars <= m_Capacity andalso m_pBuffer <> 0 then exit sub
    dim as long newCap = iif(m_Capacity < 16, 16, m_Capacity)
    while newCap < nChars : newCap *= 2 : wend
    m_pBuffer = reallocate( m_pBuffer, (newCap + 1) * sizeof(wstring) )
    if m_Capacity = 0 then cast(ulong ptr, m_pBuffer)[0] = 0
    m_Capacity = newCap
end sub

private sub DWSTRING.SetBuffer( byval p as const wstring ptr, byval n as long )
    this.Reserve(n)
    if n > 0 then memmove( m_pBuffer, p, n * sizeof(wstring) )
    cast(ulong ptr, m_pBuffer)[n] = 0
    m_BufferLen = n
end sub

private sub DWSTRING.AppendBuffer( byval p as const wstring ptr, byval n as long )
    if n <= 0 then exit sub
    this.Reserve(m_BufferLen + n)
    memmove( cast(ulong ptr, m_pBuffer) + m_BufferLen, p, n * sizeof(wstring) )
    m_BufferLen += n
    cast(ulong ptr, m_pBuffer)[m_BufferLen] = 0
end sub

private sub DWSTRING.Clear()
    this.Reserve(0)
    m_BufferLen = 0
    cast(ulong ptr, m_pBuffer)[0] = 0
end sub

private constructor DWSTRING()
    this.Reserve(16)
end constructor
private constructor DWSTRING( byval pwsz as const wstring ptr )
    this.SetBuffer( pwsz, wlen_(pwsz) )
end constructor
private constructor DWSTRING( byref ws as const DWSTRING )
    this.SetBuffer( ws.m_pBuffer, ws.m_BufferLen )
end constructor
private constructor DWSTRING( byref s as const string )
    dim as wstring ptr w = Utf8ToWide(s)
    this.SetBuffer( w, wlen_(w) ) : deallocate(w)
end constructor
private constructor DWSTRING( byref s as const string, byval codepage as long )
    dim as wstring ptr w = Utf8ToWide(s)
    this.SetBuffer( w, wlen_(w) ) : deallocate(w)
end constructor
private constructor DWSTRING( byval n as longint )
    dim as wstring * 32 w = wstr(n)
    this.SetBuffer( @w, len(w) )
end constructor
private constructor DWSTRING( byval n as double )
    dim as wstring * 64 w = wstr(n)
    this.SetBuffer( @w, len(w) )
end constructor
private destructor DWSTRING()
    if m_pBuffer then deallocate(m_pBuffer)
    m_pBuffer = 0 : m_Capacity = 0 : m_BufferLen = 0
end destructor

private operator DWSTRING.let( byval pwsz as const wstring ptr )
    if pwsz = m_pBuffer then exit operator
    this.SetBuffer( pwsz, wlen_(pwsz) )
end operator
private operator DWSTRING.let( byref ws as const DWSTRING )
    if @ws = @this then exit operator
    this.SetBuffer( ws.m_pBuffer, ws.m_BufferLen )
end operator
private operator DWSTRING.let( byref s as const string )
    dim as wstring ptr w = Utf8ToWide(s)
    this.SetBuffer( w, wlen_(w) ) : deallocate(w)
end operator
private operator DWSTRING.let( byval n as longint )
    dim as wstring * 32 w = wstr(n)
    this.SetBuffer( @w, len(w) )
end operator
private operator DWSTRING.let( byval n as double )
    dim as wstring * 64 w = wstr(n)
    this.SetBuffer( @w, len(w) )
end operator

private operator DWSTRING.&=( byval pwsz as const wstring ptr )
    if pwsz = m_pBuffer then
        dim as DWSTRING tmp = this
        this.AppendBuffer( tmp.m_pBuffer, tmp.m_BufferLen )
    else
        this.AppendBuffer( pwsz, wlen_(pwsz) )
    end if
end operator
private operator DWSTRING.&=( byref ws as const DWSTRING )
    if @ws = @this then
        dim as DWSTRING tmp = ws
        this.AppendBuffer( tmp.m_pBuffer, tmp.m_BufferLen )
    else
        this.AppendBuffer( ws.m_pBuffer, ws.m_BufferLen )
    end if
end operator
private operator DWSTRING.&=( byref s as const string )
    dim as wstring ptr w = Utf8ToWide(s)
    this.AppendBuffer( w, wlen_(w) ) : deallocate(w)
end operator
private operator DWSTRING.&=( byval n as longint )
    dim as wstring * 32 w = wstr(n)
    this.AppendBuffer( @w, len(w) )
end operator
private operator DWSTRING.&=( byval n as double )
    dim as wstring * 64 w = wstr(n)
    this.AppendBuffer( @w, len(w) )
end operator
private operator DWSTRING.+=( byval pwsz as const wstring ptr )
    this &= pwsz
end operator
private operator DWSTRING.+=( byref ws as const DWSTRING )
    this &= ws
end operator
private operator DWSTRING.+=( byref s as const string )
    this &= s
end operator

private operator DWSTRING.cast() byref as wstring
    return *m_pBuffer
end operator

private operator DWSTRING.cast() as any ptr
    return m_pBuffer
end operator
private operator DWSTRING.[]( byval nIndex as ulong ) byref as ulong
    return cast(ulong ptr, m_pBuffer)[nIndex]
end operator
private constructor DWSTRING( byval nChars as long, byref wszFill as const wstring )
    this.Reserve( nChars )
    dim as ulong c = iif(len(wszFill), cast(const ulong ptr, @wszFill)[0], 32)
    for i as long = 0 to nChars - 1 : cast(ulong ptr, m_pBuffer)[i] = c : next
    cast(ulong ptr, m_pBuffer)[nChars] = 0
    m_BufferLen = nChars
end constructor
private function DWSTRING.Add( byval pwszStr as const wstring ptr ) as boolean
    this &= pwszStr : return true
end function
private function DWSTRING.Add( byref s as const string ) as boolean
    this &= s : return true
end function
private function DWSTRING.Add( byref dws as const DWSTRING ) as boolean
    this &= dws : return true
end function
private function DWSTRING.GetBuffer() as wstring ptr
    return m_pBuffer
end function
private function DWSTRING.ResizeBuffer( byval nChars as ulong, byval bClear as boolean = false ) as wstring ptr
    this.Reserve( nChars )
    if bClear then memset( m_pBuffer, 0, (nChars + 1) * sizeof(wstring) ) : m_BufferLen = 0
    if m_BufferLen > nChars then m_BufferLen = nChars : cast(ulong ptr, m_pBuffer)[nChars] = 0
    return m_pBuffer
end function
private property DWSTRING.Capacity() as ulong
    return m_Capacity
end property
private property DWSTRING.Capacity( byval nValue as ulong )
    this.Reserve( nValue )
end property
private function DWSTRING.ChrW( byval codepoint as ulong ) as DWSTRING
    dim as ulong buf(1) = { codepoint, 0 }
    return DWSTRING( cast(wstring ptr, @buf(0)) )
end function

private function DWSTRING.vptr() as wstring ptr
    return m_pBuffer
end function
private function DWSTRING.sptr() as wstring ptr
    return m_pBuffer
end function
private function DWSTRING.wchar() as wstring ptr
    return m_pBuffer
end function
private function DWSTRING.bstr() as wstring ptr
    return m_pBuffer
end function
private function DWSTRING.Length() as long
    return m_BufferLen
end function
private property DWSTRING.utf8() as string
    return WideToUtf8( m_pBuffer, m_BufferLen )
end property
private property DWSTRING.utf8( byref s as const string )
    this = s
end property

private operator len( byref ws as const DWSTRING ) as integer
    return ws.m_BufferLen
end operator
private operator * ( byref ws as const DWSTRING ) as wstring ptr
    return ws.m_pBuffer
end operator
