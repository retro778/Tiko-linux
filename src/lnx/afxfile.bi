'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Linux implementations of the AfxNova file, path, time and misc procedures used by tiko.
' Paths inside the editor use "/" separators. Paths read from Windows-created files
' (".\src\file.bas") are accepted because every procedure here normalizes "\" to "/".

#pragma once

#include once "file.bi"

#include once "crt/unistd.bi"
#include once "crt/stdio.bi"
#include once "gio/gio.bi"

declare function lnx_chmod alias "chmod" ( byval as const zstring ptr, byval as ulong ) as long

' Query a file attribute block with GIO. Returns NULL when the file does not exist.
private function LnxFileInfo( byref sPath as const string ) as GFileInfo ptr
    dim as GFile ptr gf = g_file_new_for_path( strptr(sPath) )
    dim as GFileInfo ptr info = g_file_query_info( gf, _
        "standard::size,standard::type,time::modified,time::modified-usec,unix::mode,access::can-write", _
        G_FILE_QUERY_INFO_NONE, NULL, NULL )
    g_object_unref( gf )
    return info
end function

#define DIRSEP      "/"

' ----------------------------------------------------------------------------
' Path helpers
' ----------------------------------------------------------------------------

' Convert any Windows style separators to Linux separators.
private function NativePath( byref wszPath as const wstring ) as DWSTRING
    dim as DWSTRING dws = wszPath
    dim as ulong ptr p = cast(ulong ptr, dws.vptr)
    for i as long = 0 to len(dws) - 1
        if p[i] = asc("\") then p[i] = asc("/")
    next
    return dws
end function

' Convert a (relative) path to the Windows separator style so that files written by the
' Linux version (project files, sessions) remain readable by the Windows version.
private function PortablePath( byref wszPath as const wstring ) as DWSTRING
    dim as DWSTRING dws = wszPath
    ' Only relative paths are converted. Absolute Linux paths have no Windows meaning anyway.
    if left(dws, 1) = "/" then return dws
    dim as ulong ptr p = cast(ulong ptr, dws.vptr)
    for i as long = 0 to len(dws) - 1
        if p[i] = asc("/") then p[i] = asc("\")
    next
    return dws
end function

private function U8Path( byref wszPath as const wstring ) as string
    dim as DWSTRING dws = NativePath(wszPath)
    return dws.utf8
end function

' Returns the folder of the running executable including the trailing separator.
private function AfxGetExePathName() as DWSTRING
    static as DWSTRING dwsExePath
    if len(dwsExePath) = 0 then
        dim as zstring * 4097 buf
        dim as integer n = readlink( "/proc/self/exe", @buf, 4096 )
        if n > 0 then
            buf[n] = 0
            dim as string s = buf
            dwsExePath = DWSTRING( left(s, instrrev(s, "/")) )
        else
            dwsExePath = DWSTRING( exepath() & "/" )
        end if
    end if
    return dwsExePath
end function

private function AfxGetCurDir() as DWSTRING
    return DWSTRING( curdir() )
end function

private function AfxFileExists overload ( byval pwszFileSpec as const wstring ptr ) as boolean
    if pwszFileSpec = 0 then return false
    dim as string s = U8Path(*pwszFileSpec)
    if len(s) = 0 then return false
    ' Like FindFirstFile, wildcards are not supported here; directories count as existing.
    return ( g_file_test( strptr(s), G_FILE_TEST_EXISTS ) <> 0 )
end function
private function AfxFileExists overload ( byref wszFileSpec as const wstring ) as boolean
    return AfxFileExists( @wszFileSpec )
end function

private function AfxPathFileExists( byref wszPath as const wstring ) as boolean
    return AfxFileExists( @wszPath )
end function

private function AfxPathIsDirectory( byref wszPath as const wstring ) as boolean
    dim as string s = U8Path(wszPath)
    return ( g_file_test( strptr(s), G_FILE_TEST_IS_DIR ) <> 0 )
end function

private function AfxIsReadOnlyFile( byref wszFile as const wstring ) as boolean
    dim as string s = U8Path(wszFile)
    if AfxFileExists(wszFile) = false then return false
    return ( access_( strptr(s), W_OK ) <> 0 )
end function

private function AfxGetFileSize( byref wszFile as const wstring ) as ulongint
    dim as string s = U8Path(wszFile)
    dim as GFileInfo ptr info = LnxFileInfo(s)
    if info = 0 then return 0
    function = g_file_info_get_size( info )
    g_object_unref( info )
end function

private function AfxDeleteFile overload ( byval pwszFile as const wstring ptr ) as boolean
    if pwszFile = 0 then return false
    dim as string s = U8Path(*pwszFile)
    return ( remove( strptr(s) ) = 0 )
end function
private function AfxDeleteFile overload ( byref wszFile as const wstring ) as boolean
    return AfxDeleteFile( @wszFile )
end function

private function AfxRenameFile( byref wszOld as const wstring, byref wszNew as const wstring ) as boolean
    dim as string a = U8Path(wszOld), b = U8Path(wszNew)
    return ( rename( strptr(a), strptr(b) ) = 0 )
end function

private function AfxCopyFile( byval pwszExisting as const wstring ptr, byval pwszNew as const wstring ptr, _
                              byval bFailIfExists as boolean = false ) as boolean
    if pwszExisting = 0 orelse pwszNew = 0 then return false
    if bFailIfExists andalso AfxFileExists(pwszNew) then return false
    dim as string a = U8Path(*pwszExisting), b = U8Path(*pwszNew)
    dim as long fIn = freefile
    if open( a for binary access read as #fIn ) <> 0 then return false
    dim as string buffer
    if lof(fIn) > 0 then
        buffer = space(lof(fIn))
        get #fIn, , buffer
    end if
    close #fIn
    dim as long fOut = freefile
    if open( b for binary access write as #fOut ) <> 0 then return false
    if len(buffer) then put #fOut, , buffer
    close #fOut
    ' preserve permission bits (e.g. executables)
    dim as GFileInfo ptr info = LnxFileInfo(a)
    if info then
        lnx_chmod( strptr(b), g_file_info_get_attribute_uint32( info, "unix::mode" ) and &o7777 )
        g_object_unref( info )
    end if
    return true
end function

private function AfxPathIsRelative( byref wszPath as const wstring ) as boolean
    dim as DWSTRING dws = NativePath(wszPath)
    if len(dws) = 0 then return true
    if left(dws, 1) = "/" then return false
    if left(dws, 1) = "~" then return false
    ' A Windows drive specification (C:\...) is absolute as well.
    if mid(dws, 2, 1) = ":" then return false
    return true
end function

' Normalizes "." and ".." components of a path.
private function NormalizePathComponents( byref wszPath as const wstring ) as DWSTRING
    dim as DWSTRING dws = NativePath(wszPath)
    dim as boolean bAbs = ( left(dws, 1) = "/" )
    dim as boolean bTrail = ( right(dws, 1) = "/" ) andalso len(dws) > 1
    dim as long nParts = 0
    dim as DWSTRING parts(any)
    dim as long i = 1, n = len(dws)
    while i <= n
        dim as long j = instr(i, dws, "/")
        if j = 0 then j = n + 1
        dim as DWSTRING seg = mid(dws, i, j - i)
        if len(seg) = 0 orelse seg = "." then
            ' skip
        elseif seg = ".." then
            if nParts > 0 andalso parts(nParts - 1) <> ".." then
                nParts -= 1
            elseif bAbs = false then
                redim preserve parts(nParts) : parts(nParts) = seg : nParts += 1
            end if
        else
            redim preserve parts(nParts) : parts(nParts) = seg : nParts += 1
        end if
        i = j + 1
    wend
    dim as DWSTRING result = iif(bAbs, wstr("/"), wstr(""))
    for k as long = 0 to nParts - 1
        result &= parts(k)
        if k < nParts - 1 then result &= "/"
    next
    if bTrail andalso nParts > 0 then result &= "/"
    if len(result) = 0 then result = "."
    return result
end function

' Combines a folder and a (relative) file name into a single normalized path.
private function AfxPathCombine( byval pwszDir as const wstring ptr, byval pwszFile as const wstring ptr ) as DWSTRING
    dim as DWSTRING wszDir, wszFile
    if pwszDir then wszDir = NativePath(*pwszDir)
    if pwszFile then wszFile = NativePath(*pwszFile)
    if (len(wszFile) > 0) andalso AfxPathIsRelative(wszFile) = false then return NormalizePathComponents(wszFile)
    if len(wszDir) = 0 then return NormalizePathComponents(wszFile)
    if right(wszDir, 1) <> "/" then wszDir &= "/"
    return NormalizePathComponents(wszDir & wszFile)
end function

' Creates a relative path from one file or folder to another (like PathRelativePathToW).
' Returns ".\sub\file.bas" style paths using the native "/" separator: "./sub/file.bas".
private function AfxPathRelativePathTo( byref wszFrom as const wstring, byval dwAttrFrom as DWORD, _
                                        byref wszTo as const wstring, byval dwAttrTo as DWORD ) as DWSTRING
    dim as DWSTRING wFrom = NormalizePathComponents(wszFrom)
    dim as DWSTRING wTo = NormalizePathComponents(wszTo)
    ' If "from" is a file then use its folder
    if (dwAttrFrom and FILE_ATTRIBUTE_DIRECTORY) = 0 then
        wFrom = left(wFrom, instrrev(wFrom, "/"))
    end if
    if right(wFrom, 1) <> "/" then wFrom &= "/"
    if left(wFrom, 1) <> "/" orelse left(wTo, 1) <> "/" then return wszTo
    ' common prefix on folder boundary
    dim as long nCommon = 0
    for i as long = 1 to len(wFrom)
        if i > len(wTo) then exit for
        if mid(wFrom, i, 1) <> mid(wTo, i, 1) then exit for
        if mid(wFrom, i, 1) = "/" then nCommon = i
    next
    dim as DWSTRING remainFrom = mid(wFrom, nCommon + 1)
    dim as DWSTRING result
    dim as long nUp = 0
    for i as long = 1 to len(remainFrom)
        if mid(remainFrom, i, 1) = "/" then nUp += 1
    next
    if nUp = 0 then
        result = "./"
    else
        for i as long = 1 to nUp : result &= "../" : next
    end if
    result &= mid(wTo, nCommon + 1)
    ' Relative paths are only stored in project/session files: write them the way the Windows
    ' version does (".\src\file.bas") so the files stay interchangeable. Reading accepts both.
    return PortablePath( result )
end function

private function AfxGetFileName( byref wszPath as const wstring ) as DWSTRING
    return AfxStrPathname( "NAMEX", *cast(wstring ptr, @wszPath) )
end function

private function AfxGetFileExt( byref wszPath as const wstring ) as DWSTRING
    return AfxStrPathname( "EXTN", *cast(wstring ptr, @wszPath) )
end function

' Returns true when the file starts with a UTF-16 byte order mark.
private function AfxIsFileUnicode( byref wszFile as const wstring ) as boolean
    dim as string s = U8Path(wszFile)
    dim as long f = freefile
    if open( s for binary access read as #f ) <> 0 then return false
    dim as ubyte b1, b2
    if lof(f) >= 2 then get #f, , b1 : get #f, , b2
    close #f
    return ( b1 = &hFF andalso b2 = &hFE )
end function


' ----------------------------------------------------------------------------
' Time helpers. FILETIME holds 100ns intervals since 1601-01-01 (UTC), same as Windows.
' ----------------------------------------------------------------------------
#define EPOCH_DIFF_100NS   116444736000000000ull

private function AfxGetFileLastWriteTime( byref wszFileSpec as const wstring, byval bUTC as boolean = true ) as FILETIME
    dim as FILETIME ft
    dim as string s = U8Path(wszFileSpec)
    dim as GFileInfo ptr info = LnxFileInfo(s)
    if info then
        dim as ulongint v = g_file_info_get_attribute_uint64( info, "time::modified" ) * 10000000ull + _
                            cast(ulongint, g_file_info_get_attribute_uint32( info, "time::modified-usec" )) * 10ull + _
                            EPOCH_DIFF_100NS
        g_object_unref( info )
        ft.dwLowDateTime = v and &hFFFFFFFFull
        ft.dwHighDateTime = v shr 32
    end if
    return ft
end function

' Converts a FILETIME into an OLE automation date (days since 1899-12-30).
private function AfxFileTimeToVariantTime( byref ft as const FILETIME ) as double
    dim as ulongint v = (cast(ulongint, ft.dwHighDateTime) shl 32) or ft.dwLowDateTime
    if v = 0 then return 0
    ' 1601-01-01 to 1899-12-30 = 109205 days
    return (v / 864000000000.0) - 109205.0
end function

private function AfxLocalDateStr( byref wszFormat as const wstring = "dd/MM/yyyy" ) as DWSTRING
    return DWSTRING( format(now, wszFormat) )
end function

private function AfxLocalTimeStr( byref wszFormat as const wstring = "hh:mm:ss" ) as DWSTRING
    return DWSTRING( format(now, wszFormat) )
end function


' ----------------------------------------------------------------------------
' GUIDs
' ----------------------------------------------------------------------------
private function AfxGuid() as GUID
    dim as GUID g
    dim as long f = freefile
    dim as ubyte b(15)
    if open( "/dev/urandom" for binary access read as #f ) = 0 then
        get #f, , b()
        close #f
    else
        randomize timer
        for i as long = 0 to 15 : b(i) = int(rnd * 256) : next
    end if
    b(6) = (b(6) and &h0F) or &h40     ' version 4
    b(8) = (b(8) and &h3F) or &h80     ' variant
    g.Data1 = (cast(ulong, b(0)) shl 24) or (cast(ulong, b(1)) shl 16) or (cast(ulong, b(2)) shl 8) or b(3)
    g.Data2 = (cast(ushort, b(4)) shl 8) or b(5)
    g.Data3 = (cast(ushort, b(6)) shl 8) or b(7)
    for i as long = 0 to 7 : g.Data4(i) = b(8 + i) : next
    return g
end function

' Returns the guid in registry format: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
private function AfxGuidText( byref g as const GUID ) as DWSTRING
    dim as string s = "{" & hex(g.Data1, 8) & "-" & hex(g.Data2, 4) & "-" & hex(g.Data3, 4) & "-" & _
                      hex(g.Data4(0), 2) & hex(g.Data4(1), 2) & "-"
    for i as long = 2 to 7 : s &= hex(g.Data4(i), 2) : next
    s &= "}"
    return DWSTRING(s)
end function


' ----------------------------------------------------------------------------
' Command line
' ----------------------------------------------------------------------------
' nIndex = -1 returns the full command line; 0 the program; 1.. the arguments.
' An argument naming an existing file relative to the current folder (tiko-editor demo.bas
' typed in a terminal) is returned as a full path: upstream code would put the editor's own
' folder in front of a name without a path, as Windows' Explorer always passes full paths.
private function AfxCommand( byval nIndex as long = -1 ) as DWSTRING
    if nIndex = -1 then return DWSTRING( command(-1) )
    dim as string sArg = command(nIndex)
    if (nIndex >= 1) andalso (len(sArg) > 0) andalso (left(sArg, 1) <> "/") andalso (left(sArg, 1) <> "-") then
        dim as string sRel = sArg
        do while left(sRel, 2) = "./"
            sRel = mid(sRel, 3)
        loop
        dim as string sFull = curdir() & "/" & sRel
        if AfxFileExists( DWSTRING(sFull) ) then return DWSTRING( sFull )
    end if
    return DWSTRING( sArg )
end function

' Creates a unique temporary file (like GetTempFileName) and returns its name.
private function GetTempFileName( byval lpPathName as LPCWSTR, byval lpPrefixString as LPCWSTR, _
                                  byval uUnique as UINT, byval lpTempFileName as LPWSTR ) as UINT
    if lpTempFileName = 0 then return 0
    dim as DWSTRING folder = iif(lpPathName andalso len(*lpPathName), NativePath(*lpPathName), DWSTRING(*g_get_tmp_dir()))
    if right(folder, 1) <> "/" then folder &= "/"
    dim as DWSTRING prefix = iif(lpPrefixString, left(*lpPrefixString, 3), wstr("TMP"))
    for tries as long = 0 to 1000
        dim as UINT u = iif(uUnique, uUnique, (cast(UINT, timer * 1000) + tries) and &hFFFF)
        dim as DWSTRING nm = folder & prefix & lcase(hex(u, 4)) & ".tmp"
        if AfxFileExists(nm) = false then
            if uUnique = 0 then
                dim as long f = freefile
                if open( nm.utf8 for output as #f ) = 0 then close #f
            end if
            dim as long n = len(nm)
            if n > MAX_PATH - 1 then n = MAX_PATH - 1
            memcpy( lpTempFileName, nm.vptr, n * sizeof(wstring) )
            cast(ulong ptr, lpTempFileName)[n] = 0
            return u
        end if
        if uUnique then exit for
    next
    return 0
end function
#define GetTempFileNameW GetTempFileName

private function GetTempPath( byval nBufferLength as DWORD, byval lpBuffer as LPWSTR ) as DWORD
    dim as DWSTRING t = DWSTRING(*g_get_tmp_dir()) & "/"
    if lpBuffer andalso nBufferLength > len(t) then memcpy( lpBuffer, t.vptr, (len(t) + 1) * sizeof(wstring) )
    return len(t)
end function
#define GetTempPathW GetTempPath
