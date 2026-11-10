'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' CTextStream / CFileStream replacements for the AfxNova classes.
'
' Files are written byte-for-byte the way the Windows version writes them:
'   - "unicode" streams: UTF-16LE with BOM, CRLF line endings  (config, projects, sessions)
'   - other streams:     8-bit text, CRLF line endings          (keywords, batch files)
' When reading, the encoding is detected from the BOM (UTF-16LE, UTF-8) and otherwise
' the text is treated as UTF-8 (invalid UTF-8 bytes are mapped as Latin-1).

#pragma once

#ifndef IOMODE_FORREADING
#define IOMODE_FORREADING    1
#endif
#ifndef IOMODE_FORWRITING
#define IOMODE_FORWRITING    2
#endif
#ifndef IOMODE_FORAPPENDING
#define IOMODE_FORAPPENDING  8
#endif

#ifndef STGM_READ
#define STGM_READ            &h00000000
#endif
#ifndef STGM_WRITE
#define STGM_WRITE           &h00000001
#endif
#ifndef STGM_READWRITE
#define STGM_READWRITE       &h00000002
#endif
#ifndef STGM_CREATE
#define STGM_CREATE          &h00001000
#endif

' Decode a raw byte buffer into a DWSTRING by looking at its BOM.
private function DecodeTextBuffer( byref sRaw as const string, byref bIsUnicode as boolean = false ) as DWSTRING
    dim as long n = len(sRaw)
    dim as const ubyte ptr p = cast(const ubyte ptr, strptr(sRaw))
    bIsUnicode = false
    if n >= 2 andalso p[0] = &hFF andalso p[1] = &hFE then
        ' UTF-16LE (with surrogate pair handling)
        bIsUnicode = true
        dim as long nChars = (n - 2) \ 2
        dim as ulong ptr w = allocate( (nChars + 1) * 4 )
        dim as long o = 0, i = 2
        while i + 1 < n
            dim as ulong c = p[i] or (cast(ulong, p[i+1]) shl 8)
            i += 2
            if c >= &hD800 andalso c <= &hDBFF andalso i + 1 < n then
                dim as ulong c2 = p[i] or (cast(ulong, p[i+1]) shl 8)
                if c2 >= &hDC00 andalso c2 <= &hDFFF then
                    c = &h10000 + ((c - &hD800) shl 10) + (c2 - &hDC00)
                    i += 2
                end if
            end if
            w[o] = c : o += 1
        wend
        w[o] = 0
        dim as DWSTRING dws
        dws.SetBuffer( cast(wstring ptr, w), o )
        deallocate(w)
        return dws
    elseif n >= 3 andalso p[0] = &hEF andalso p[1] = &hBB andalso p[2] = &hBF then
        return DWSTRING( mid(sRaw, 4) )
    end if
    return DWSTRING( sRaw )
end function

' Encode a wide string as UTF-16LE (no BOM).
private function EncodeUtf16LE( byval w as const wstring ptr, byval n as long ) as string
    if w = 0 orelse n <= 0 then return ""
    dim as const ulong ptr p = cast(const ulong ptr, w)
    dim as string s = space(n * 4)
    dim as ubyte ptr d = cast(ubyte ptr, strptr(s))
    dim as long o = 0
    for i as long = 0 to n - 1
        dim as ulong c = p[i]
        if c >= &h10000 then
            c -= &h10000
            dim as ulong hi = &hD800 + (c shr 10), lo = &hDC00 + (c and &h3FF)
            d[o] = hi and 255 : d[o+1] = hi shr 8 : d[o+2] = lo and 255 : d[o+3] = lo shr 8 : o += 4
        else
            d[o] = c and 255 : d[o+1] = (c shr 8) and 255 : o += 2
        end if
    next
    return left(s, o)
end function

private function ReadFileBytes( byref wszFile as const wstring, byref sOut as string ) as boolean
    dim as string sPath = U8Path(wszFile)
    dim as long f = freefile
    if open( sPath for binary access read as #f ) <> 0 then return false
    sOut = ""
    if lof(f) > 0 then
        sOut = space(lof(f))
        get #f, , sOut
    end if
    close #f
    return true
end function


' File helpers that live outside the stream classes because the class methods named
' Open/Close/Write hide the FreeBASIC keywords of the same name.
private function LnxOpenWrite( byref sPath as const string, byval bTruncate as boolean ) as long
    dim as long f = freefile
    if bTruncate then
        if open( sPath for output as #f ) <> 0 then return 0
        close #f
        f = freefile
    end if
    if open( sPath for binary access write as #f ) <> 0 then return 0
    return f
end function
private function LnxOpenRead( byref sPath as const string ) as long
    dim as long f = freefile
    if open( sPath for binary access read as #f ) <> 0 then return 0
    return f
end function
private function LnxOpenReadWrite( byref sPath as const string ) as long
    dim as long f = freefile
    if open( sPath for binary as #f ) <> 0 then return 0
    return f
end function
private sub LnxClose( byval f as long )
    if f then close #f
end sub
private sub LnxPutString( byval f as long, byref s as const string )
    if f andalso len(s) then put #f, , s
end sub

type CTextStream
    private:
        m_Text      as DWSTRING      ' complete decoded text (reading)
        m_Pos       as long          ' 1-based read position into m_Text
        m_hFile     as long          ' file number (writing)
        m_bUnicode  as boolean
        m_bReading  as boolean
        m_bOpen     as boolean
        declare sub WriteRaw( byval w as const wstring ptr, byval n as long )
    public:
        declare destructor()
        declare function Open( byref wszFile as const wstring, byval IOMode as long = IOMODE_FORREADING, _
                               byval bCreate as boolean = false, byval bUnicode as boolean = false ) as HRESULT
        declare function OpenUnicode( byref wszFile as const wstring, byval IOMode as long = IOMODE_FORREADING, _
                               byval bCreate as boolean = false ) as HRESULT
        declare function OpenForOutputW( byref wszFile as const wstring, byval bOverwrite as boolean = true ) as HRESULT
        declare function OpenForAppendW( byref wszFile as const wstring ) as HRESULT
        declare function Create( byref wszFile as const wstring, byval bOverwrite as boolean = true, _
                                 byval bUnicode as boolean = false ) as HRESULT
        declare function ReadLine() as DWSTRING
        declare function ReadAll() as DWSTRING
        declare function EOS() as boolean
        declare function WriteLine overload ( ) as HRESULT
        declare function WriteLine overload ( byref wszText as const wstring ) as HRESULT
        declare function Write( byref wszText as const wstring ) as HRESULT
        declare function WriteBlankLines( byval n as long ) as HRESULT
        declare function Close() as HRESULT
end type

private destructor CTextStream()
    this.Close()
end destructor

private function CTextStream.Open( byref wszFile as const wstring, byval IOMode as long = IOMODE_FORREADING, _
                                   byval bCreate as boolean = false, byval bUnicode as boolean = false ) as HRESULT
    this.Close()
    if IOMode = IOMODE_FORREADING then
        dim as string sRaw
        if ReadFileBytes( wszFile, sRaw ) = false then return E_FAIL
        m_Text = DecodeTextBuffer( sRaw, m_bUnicode )
        m_Pos = 1
        m_bReading = true
        m_bOpen = true
        return S_OK
    elseif IOMode = IOMODE_FORAPPENDING then
        m_bUnicode = bUnicode
        return this.OpenForAppendW( wszFile )
    else
        return this.Create( wszFile, true, bUnicode )
    end if
end function

private function CTextStream.OpenUnicode( byref wszFile as const wstring, byval IOMode as long = IOMODE_FORREADING, _
                                          byval bCreate as boolean = false ) as HRESULT
    return this.Open( wszFile, IOMode, bCreate, true )
end function

private function CTextStream.Create( byref wszFile as const wstring, byval bOverwrite as boolean = true, _
                                     byval bUnicode as boolean = false ) as HRESULT
    this.Close()
    if bOverwrite = false andalso AfxFileExists(wszFile) then return E_FAIL
    dim as string sPath = U8Path(wszFile)
    m_hFile = LnxOpenWrite( sPath, true )
    if m_hFile = 0 then return E_FAIL
    m_bUnicode = bUnicode
    m_bReading = false
    m_bOpen = true
    if m_bUnicode then
        LnxPutString( m_hFile, chr(&hFF, &hFE) )
    end if
    return S_OK
end function

private function CTextStream.OpenForOutputW( byref wszFile as const wstring, byval bOverwrite as boolean = true ) as HRESULT
    return this.Create( wszFile, bOverwrite, true )
end function

private function CTextStream.OpenForAppendW( byref wszFile as const wstring ) as HRESULT
    this.Close()
    dim as boolean bExists = AfxFileExists(wszFile)
    if bExists then m_bUnicode = AfxIsFileUnicode(wszFile)
    dim as string sPath = U8Path(wszFile)
    m_hFile = LnxOpenWrite( sPath, false )
    if m_hFile = 0 then return E_FAIL
    seek m_hFile, lof(m_hFile) + 1
    m_bReading = false
    m_bOpen = true
    if bExists = false andalso m_bUnicode then
        LnxPutString( m_hFile, chr(&hFF, &hFE) )
    end if
    return S_OK
end function

private function CTextStream.ReadLine() as DWSTRING
    dim as DWSTRING dws
    if m_bReading = false then return dws
    dim as long n = len(m_Text)
    if m_Pos > n then return dws
    dim as const ulong ptr p = cast(const ulong ptr, m_Text.vptr)
    dim as long i = m_Pos - 1
    dim as long startIdx = i
    while i < n andalso p[i] <> 10 andalso p[i] <> 13
        i += 1
    wend
    dws.SetBuffer( cast(wstring ptr, p + startIdx), i - startIdx )
    if i < n then
        if p[i] = 13 andalso i + 1 < n andalso p[i+1] = 10 then i += 2 else i += 1
    end if
    m_Pos = i + 1
    return dws
end function

private function CTextStream.ReadAll() as DWSTRING
    if m_bReading = false then return DWSTRING()
    dim as DWSTRING dws = mid(m_Text, m_Pos)
    m_Pos = len(m_Text) + 1
    return dws
end function

private function CTextStream.EOS() as boolean
    if m_bReading = false then return true
    return ( m_Pos > len(m_Text) )
end function

private sub CTextStream.WriteRaw( byval w as const wstring ptr, byval n as long )
    if m_bOpen = false orelse m_bReading orelse m_hFile = 0 then exit sub
    if n <= 0 then exit sub
    dim as string s
    if m_bUnicode then
        s = EncodeUtf16LE( w, n )
    else
        s = WideToUtf8( w, n )
    end if
    LnxPutString( m_hFile, s )
end sub

private function CTextStream.Write( byref wszText as const wstring ) as HRESULT
    this.WriteRaw( @wszText, len(wszText) )
    return S_OK
end function

private function CTextStream.WriteLine( ) as HRESULT
    dim as wstring * 3 crlf = wstr(!"\r\n")
    this.WriteRaw( @crlf, 2 )
    return S_OK
end function

private function CTextStream.WriteLine( byref wszText as const wstring ) as HRESULT
    this.WriteRaw( @wszText, len(wszText) )
    return this.WriteLine()
end function

private function CTextStream.WriteBlankLines( byval n as long ) as HRESULT
    for i as long = 1 to n : this.WriteLine() : next
    return S_OK
end function

private function CTextStream.Close() as HRESULT
    LnxClose( m_hFile )
    m_hFile = 0
    m_bOpen = false
    m_bReading = false
    m_Text = ""
    m_Pos = 1
    return S_OK
end function


' ----------------------------------------------------------------------------
' CFileStream - raw byte stream
' ----------------------------------------------------------------------------
type CFileStream
    private:
        m_hFile as long
    public:
        declare destructor()
        declare function Open( byref wszFile as const wstring, byval grfMode as DWORD = STGM_READ ) as HRESULT
        declare function Write( byval pv as const any ptr, byval cb as ulong ) as HRESULT
        declare function Read( byval pv as any ptr, byval cb as ulong ) as ulong
        declare function GetSize() as ulongint
        declare function WriteTextA( byref s as const string ) as HRESULT
        declare function Close() as HRESULT
end type

private destructor CFileStream()
    this.Close()
end destructor

private function CFileStream.Open( byref wszFile as const wstring, byval grfMode as DWORD = STGM_READ ) as HRESULT
    this.Close()
    dim as string sPath = U8Path(wszFile)
    if (grfMode and STGM_CREATE) then
        ' truncating (instead of deleting) keeps permissions/ownership of an existing file
        m_hFile = LnxOpenWrite( sPath, true )
    elseif (grfMode and (STGM_WRITE or STGM_READWRITE)) then
        m_hFile = LnxOpenReadWrite( sPath )
    else
        m_hFile = LnxOpenRead( sPath )
    end if
    return iif( m_hFile, S_OK, E_FAIL )
end function

private function CFileStream.Write( byval pv as const any ptr, byval cb as ulong ) as HRESULT
    if m_hFile = 0 then return E_FAIL
    if cb = 0 then return S_OK
    put #m_hFile, , *cast(const ubyte ptr, pv), cb
    return S_OK
end function

private function CFileStream.Read( byval pv as any ptr, byval cb as ulong ) as ulong
    if m_hFile = 0 then return 0
    dim as uinteger nRead
    get #m_hFile, , *cast(ubyte ptr, pv), cb, nRead
    return nRead
end function

private function CFileStream.WriteTextA( byref s as const string ) as HRESULT
    if len(s) = 0 then return S_OK
    return this.Write( strptr(s), len(s) )
end function

private function CFileStream.GetSize() as ulongint
    if m_hFile = 0 then return 0
    return lof(m_hFile)
end function

private function CFileStream.Close() as HRESULT
    LnxClose( m_hFile )
    m_hFile = 0
    return S_OK
end function
