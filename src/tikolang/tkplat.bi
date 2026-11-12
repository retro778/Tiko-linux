'    tiko editor - small platform helpers for the code added to upstream (src/tikolang, src/ai)
'    Licensed under the GNU GPLv3 or later.
'
' That code keeps paths and text as UTF-8 "string"s. These helpers hide what differs between
' the Linux build (GTK platform layer, src/lnx) and the Windows build (windows.bi + AfxNova).

#pragma once

#ifdef __FB_LINUX__
    #define TK_SEP          "/"
    #define TK_PATHLIST_SEP ":"
    #define TK_EXE          ""
    extern "C"
        declare function tk_realpath alias "realpath"( byval path as const zstring ptr, byval resolved as zstring ptr ) as zstring ptr
    end extern
#else
    #define TK_SEP          "\"
    #define TK_PATHLIST_SEP ";"
    #define TK_EXE          ".exe"

    ' UTF-8 text of a wide string (the Linux layer has the same function)
    private function WideToUtf8( byval w as const wstring ptr, byval n as long = -1 ) as string
        if w = 0 then return ""
        dim as DWSTRING dws = *cast(wstring ptr, w)
        if n >= 0 then dws = left(dws, n)
        return dws.utf8
    end function
#endif

' true for "/x", "\x", "C:\x", "C:/x"
private function TkIsAbsolute( byref sPath as string ) as boolean
    if (left(sPath, 1) = "/") orelse (left(sPath, 1) = "\") then return true
    return mid(sPath, 2, 1) = ":"
end function

' position of the last path separator (0 if none)
private function TkLastSep( byref sPath as string, byval nStart as long = -1 ) as long
    dim as long a = instrrev(sPath, "/", nStart), b = instrrev(sPath, "\", nStart)
    return iif(a > b, a, b)
end function

' "dir/" with its trailing separator ("" when the path has no folder)
private function TkDirOf( byref sPath as string ) as string
    return left(sPath, TkLastSep(sPath))
end function

' file name without its folder
private function TkNameOf( byref sPath as string ) as string
    return mid(sPath, TkLastSep(sPath) + 1)
end function

' A UTF-8 path as the run time library's file statements (open, kill, chdir) take it:
' UTF-8 on Linux, the ANSI code page on Windows
private function TkFs( byref sUtf8 as string ) as string
#ifdef __FB_LINUX__
    return sUtf8
#else
    dim as DWSTRING w = DWSTRING(sUtf8, CP_UTF8)
    dim as string s = w
    return s
#endif
end function

private function TkFileExists( byref sUtf8 as string ) as boolean
    return AfxFileExists(DWSTRING(sUtf8, CP_UTF8)) <> 0
end function

' Folder of the editor executable, with a trailing separator
private function TkExeDir() as string
    dim as DWSTRING w = AfxGetExePathName
    dim as string s = w.utf8
    if (right(s, 1) <> "/") andalso (right(s, 1) <> "\") then s &= TK_SEP
    return s
end function

' The user's temporary folder, with a trailing separator
private function TkTempDir() as string
#ifdef __FB_LINUX__
    return *g_get_tmp_dir() & "/"
#else
    dim as wstring * MAX_PATH wsz
    GetTempPathW( MAX_PATH, @wsz )
    return WideToUtf8(@wsz)
#endif
end function

private function TkPid() as long
#ifdef __FB_LINUX__
    return getpid()
#else
    return GetCurrentProcessId()
#endif
end function

' The user's home folder without a trailing separator
private function TkHomeDir() as string
#ifdef __FB_LINUX__
    return environ("HOME")
#else
    return environ("USERPROFILE")
#endif
end function

' A canonical form of a path for comparing two paths (links resolved / case folded)
private function TkRealPath( byref sPath as string ) as string
#ifdef __FB_LINUX__
    dim as zstring ptr p = tk_realpath( sPath, NULL )
    if p = 0 then return sPath
    dim as string s = *p
    free( p )
    return s
#else
    dim as wstring * (MAX_PATH * 2) wsz
    dim as DWSTRING w = DWSTRING(sPath, CP_UTF8)
    if GetFullPathNameW( w, MAX_PATH * 2, @wsz, NULL ) = 0 then return lcase(sPath)
    return lcase(WideToUtf8(@wsz))
#endif
end function
