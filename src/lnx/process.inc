'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Win32 kernel handles on POSIX: files, anonymous pipes and processes.
' Implements the subset used by tiko for reading files, running the compiler with
' redirected output and driving gdb through pipes.

#pragma once

extern "C"
    declare function lnx_pipe alias "pipe" ( byval fds as long ptr ) as long
    declare function lnx_read alias "read" ( byval fd as long, byval buf as any ptr, byval n as uinteger ) as integer
    declare function lnx_write alias "write" ( byval fd as long, byval buf as const any ptr, byval n as uinteger ) as integer
    declare function lnx_close alias "close" ( byval fd as long ) as long
    declare function lnx_open alias "open" ( byval path as const zstring ptr, byval flags as long, ... ) as long
    declare function lnx_ioctl alias "ioctl" ( byval fd as long, byval req as culong, ... ) as long
    declare function lnx_fcntl alias "fcntl" ( byval fd as long, byval cmd as long, ... ) as long
    declare function lnx_lseek alias "lseek" ( byval fd as long, byval offset as longint, byval whence as long ) as longint
    declare function lnx_waitpid alias "waitpid" ( byval pid as long, byval status as long ptr, byval options as long ) as long
    declare function lnx_kill alias "kill" ( byval pid as long, byval sig as long ) as long
    declare function lnx_errno_location alias "__errno_location" () as long ptr
    declare function g_spawn_async_with_fds( byval working_directory as const zstring ptr, byval argv as zstring ptr ptr, _
            byval envp as zstring ptr ptr, byval flags as long, byval child_setup as any ptr, byval user_data as any ptr, _
            byval child_pid as long ptr, byval stdin_fd as long, byval stdout_fd as long, byval stderr_fd as long, _
            byval error_ as GError ptr ptr ) as gboolean
end extern

#ifndef LNX_O_RDONLY
#define LNX_O_RDONLY     0
#endif
#ifndef LNX_O_WRONLY
#define LNX_O_WRONLY     1
#endif
#ifndef LNX_O_CLOEXEC
#define LNX_O_CLOEXEC    &o2000000
#endif
#ifndef LNX_FIONREAD
#define LNX_FIONREAD     &h541B
#endif
#ifndef LNX_F_SETFD
#define LNX_F_SETFD      2
#endif
#ifndef LNX_FD_CLOEXEC
#define LNX_FD_CLOEXEC   1
#endif
#ifndef LNX_WNOHANG
#define LNX_WNOHANG      1
#endif
#ifndef LNX_SIGKILL
#define LNX_SIGKILL      9
#endif

#ifndef GENERIC_WRITE
#define GENERIC_WRITE          &h40000000
#endif
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS          2
#endif
#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS            4
#endif
#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE       2
#endif
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND   2
#endif
#ifndef ERROR_BROKEN_PIPE
#define ERROR_BROKEN_PIPE      109
#endif
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0          0
#endif
#ifndef INFINITE
#define INFINITE               &hFFFFFFFF
#endif
#ifndef STARTF_USESTDHANDLES_
#define STARTF_USESTDHANDLES_  &h100
#endif

type PHANDLE as HANDLE

enum LNX_HKIND
    HK_FILE = 1
    HK_PIPE
    HK_PROCESS
end enum

type LNXKHANDLE
    magic    as ulong          ' &h4B484E44 "KHND"
    kind     as LNX_HKIND
    fd       as long
    pid      as long
    exitCode as long
    exited   as boolean
    inherit  as boolean
end type
#ifndef LNX_KMAGIC
#define LNX_KMAGIC &h4B484E44
#endif

type SECURITY_ATTRIBUTES
    nLength              as DWORD
    lpSecurityDescriptor as any ptr
    bInheritHandle       as BOOL
end type
type LPSECURITY_ATTRIBUTES as SECURITY_ATTRIBUTES ptr

type STARTUPINFO
    cb              as DWORD
    lpReserved      as LPWSTR
    lpDesktop       as LPWSTR
    lpTitle         as LPWSTR
    dwX             as DWORD
    dwY             as DWORD
    dwXSize         as DWORD
    dwYSize         as DWORD
    dwXCountChars   as DWORD
    dwYCountChars   as DWORD
    dwFillAttribute as DWORD
    dwFlags         as DWORD
    wShowWindow     as WORD
    cbReserved2     as WORD
    lpReserved2     as ubyte ptr
    hStdInput       as HANDLE
    hStdOutput      as HANDLE
    hStdError       as HANDLE
end type
type STARTUPINFOW as STARTUPINFO

type PROCESS_INFORMATION
    hProcess    as HANDLE
    hThread     as HANDLE
    dwProcessId as DWORD
    dwThreadId  as DWORD
end type

dim shared as DWORD gLnxLastError

private function GetLastError() as DWORD
    return gLnxLastError
end function

private function LnxNewKHandle( byval kind as LNX_HKIND, byval fd as long ) as LNXKHANDLE ptr
    dim as LNXKHANDLE ptr h = new LNXKHANDLE
    h->magic = LNX_KMAGIC
    h->kind = kind
    h->fd = fd
    return h
end function

private function LnxIsKHandle( byval h as HANDLE ) as boolean
    if h = 0 orelse h = INVALID_HANDLE_VALUE then return false
    return cast(LNXKHANDLE ptr, h)->magic = LNX_KMAGIC
end function

private function CloseHandle( byval hObject as HANDLE ) as BOOL
    if hObject = 0 orelse hObject = INVALID_HANDLE_VALUE then return 0
    dim as ulong magic = *cast(ulong ptr, hObject)
    if magic = LNX_KMAGIC then
        dim as LNXKHANDLE ptr h = hObject
        if h->fd >= 0 andalso h->kind <> HK_PROCESS then lnx_close( h->fd )
        h->magic = 0
        delete h
    elseif magic = &h50524F43 then         ' OpenProcess handle (winmisc.inc)
        delete cast(LNXPROCHANDLE ptr, hObject)
    else
        ' Toolhelp snapshot
        delete cast(LNXSNAPSHOT ptr, hObject)
    end if
    return 1
end function

private function CreateFileW( byval lpFileName as LPCWSTR, byval dwDesiredAccess as DWORD, byval dwShareMode as DWORD, _
            byval lpSecurityAttributes as any ptr, byval dwCreationDisposition as DWORD, byval dwFlagsAndAttributes as DWORD, _
            byval hTemplateFile as HANDLE ) as HANDLE
    if lpFileName = 0 then return INVALID_HANDLE_VALUE
    dim as string s = U8Path(*lpFileName)
    dim as long flags = LNX_O_CLOEXEC
    if (dwDesiredAccess and GENERIC_WRITE) then
        flags or= LNX_O_WRONLY or &o100     ' O_CREAT
        if dwCreationDisposition = CREATE_ALWAYS then flags or= &o1000   ' O_TRUNC
    else
        flags or= LNX_O_RDONLY
    end if
    dim as long fd = lnx_open( strptr(s), flags, &o644 )
    if fd < 0 then gLnxLastError = ERROR_FILE_NOT_FOUND : return INVALID_HANDLE_VALUE
    return LnxNewKHandle( HK_FILE, fd )
end function
#define CreateFile CreateFileW

private function GetFileSize( byval hFile as HANDLE, byval lpFileSizeHigh as DWORD ptr ) as DWORD
    if LnxIsKHandle(hFile) = false then return &hFFFFFFFF
    dim as LNXKHANDLE ptr h = hFile
    dim as longint cur = lnx_lseek( h->fd, 0, 1 )
    dim as longint sz = lnx_lseek( h->fd, 0, 2 )
    lnx_lseek( h->fd, cur, 0 )
    if lpFileSizeHigh then *lpFileSizeHigh = sz shr 32
    return sz and &hFFFFFFFFull
end function

private function ReadFile( byval hFile as HANDLE, byval lpBuffer as any ptr, byval nNumberOfBytesToRead as DWORD, _
                           byval lpNumberOfBytesRead as any ptr, byval lpOverlapped as any ptr ) as BOOL
    if lpNumberOfBytesRead then *cast(DWORD ptr, lpNumberOfBytesRead) = 0
    if LnxIsKHandle(hFile) = false then return 0
    dim as LNXKHANDLE ptr h = hFile
    dim as integer n = lnx_read( h->fd, lpBuffer, nNumberOfBytesToRead )
    if n < 0 then
        dim as long e = *lnx_errno_location()
        if e = 4 then return ReadFile( hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped )   ' EINTR
        return 0
    end if
    if lpNumberOfBytesRead then *cast(DWORD ptr, lpNumberOfBytesRead) = n
    ' Windows: reading a pipe whose writer has closed fails with ERROR_BROKEN_PIPE
    if n = 0 andalso h->kind = HK_PIPE then gLnxLastError = ERROR_BROKEN_PIPE : return 0
    return 1
end function

private function WriteFile( byval hFile as HANDLE, byval lpBuffer as const any ptr, byval nNumberOfBytesToWrite as DWORD, _
                            byval lpNumberOfBytesWritten as any ptr, byval lpOverlapped as any ptr ) as BOOL
    if lpNumberOfBytesWritten then *cast(DWORD ptr, lpNumberOfBytesWritten) = 0
    if LnxIsKHandle(hFile) = false then return 0
    dim as LNXKHANDLE ptr h = hFile
    dim as integer total = 0
    while total < nNumberOfBytesToWrite
        dim as integer n = lnx_write( h->fd, cast(const ubyte ptr, lpBuffer) + total, nNumberOfBytesToWrite - total )
        if n <= 0 then exit while
        total += n
    wend
    if lpNumberOfBytesWritten then *cast(DWORD ptr, lpNumberOfBytesWritten) = total
    return iif( total = nNumberOfBytesToWrite, 1, 0 )
end function

private function FlushFileBuffers( byval hFile as HANDLE ) as BOOL
    return 1
end function

private function CreatePipe( byval hReadPipe as HANDLE ptr, byval hWritePipe as HANDLE ptr, _
                             byval lpPipeAttributes as SECURITY_ATTRIBUTES ptr, byval nSize as DWORD ) as BOOL
    dim as long fds(1)
    if lnx_pipe( @fds(0) ) <> 0 then return 0
    ' Our side of the pipe must not leak into spawned children
    lnx_fcntl( fds(0), LNX_F_SETFD, LNX_FD_CLOEXEC )
    lnx_fcntl( fds(1), LNX_F_SETFD, LNX_FD_CLOEXEC )
    *hReadPipe = LnxNewKHandle( HK_PIPE, fds(0) )
    *hWritePipe = LnxNewKHandle( HK_PIPE, fds(1) )
    return 1
end function

private function SetHandleInformation( byval hObject as HANDLE, byval dwMask as DWORD, byval dwFlags as DWORD ) as BOOL
    if LnxIsKHandle(hObject) = false then return 0
    cast(LNXKHANDLE ptr, hObject)->inherit = (dwFlags and HANDLE_FLAG_INHERIT) <> 0
    return 1
end function

private function PeekNamedPipe( byval hNamedPipe as HANDLE, byval lpBuffer as any ptr, byval nBufferSize as DWORD, _
                                byval lpBytesRead as DWORD ptr, byval lpTotalBytesAvail as DWORD ptr, byval lpBytesLeftThisMessage as DWORD ptr ) as BOOL
    if LnxIsKHandle(hNamedPipe) = false then return 0
    dim as long n = 0
    if lnx_ioctl( cast(LNXKHANDLE ptr, hNamedPipe)->fd, LNX_FIONREAD, @n ) <> 0 then return 0
    if lpTotalBytesAvail then *lpTotalBytesAvail = n
    if lpBytesRead then *lpBytesRead = 0
    return 1
end function

private sub GetStartupInfo( byval lpStartupInfo as STARTUPINFO ptr )
    if lpStartupInfo then memset( lpStartupInfo, 0, sizeof(STARTUPINFO) ) : lpStartupInfo->cb = sizeof(STARTUPINFO)
end sub
#define GetStartupInfoW GetStartupInfo

' Splits a Windows style command line into arguments (CommandLineToArgvW rules).
private function LnxSplitCommandLine( byref wszCmd as const wstring, args() as string ) as long
    dim as string s = WideToUtf8(@wszCmd)
    dim as long n = 0, i = 1, l = len(s)
    erase args
    while i <= l
        while i <= l andalso (mid(s, i, 1) = " " orelse mid(s, i, 1) = !"\t") : i += 1 : wend
        if i > l then exit while
        dim as string arg
        dim as boolean inQuote = false
        while i <= l
            dim as string c = mid(s, i, 1)
            if c = chr(34) then
                inQuote = not inQuote
            elseif (c = " " orelse c = !"\t") andalso inQuote = false then
                exit while
            else
                arg &= c
            end if
            i += 1
        wend
        redim preserve args(n)
        args(n) = arg
        n += 1
    wend
    return n
end function

private function LnxFdOf( byval h as HANDLE ) as long
    if LnxIsKHandle(h) = false then return -1
    return cast(LNXKHANDLE ptr, h)->fd
end function

private function CreateProcessW( byval lpApplicationName as LPCWSTR, byval lpCommandLine as LPWSTR, _
            byval lpProcessAttributes as any ptr, byval lpThreadAttributes as any ptr, byval bInheritHandles as BOOL, _
            byval dwCreationFlags as DWORD, byval lpEnvironment as any ptr, byval lpCurrentDirectory as LPCWSTR, _
            byval lpStartupInfo as STARTUPINFO ptr, byval lpProcessInformation as PROCESS_INFORMATION ptr ) as BOOL
    dim as string args(any)
    dim as long nArgs = 0
    if lpCommandLine then nArgs = LnxSplitCommandLine( *lpCommandLine, args() )
    if lpApplicationName andalso len(*lpApplicationName) then
        ' Windows: argv[0] of the command line is ignored for execution when an application name is given
        dim as string app = U8Path(*lpApplicationName)
        if nArgs = 0 then
            redim args(0) : nArgs = 1
        end if
        args(0) = app
    end if
    if nArgs = 0 then gLnxLastError = ERROR_FILE_NOT_FOUND : return 0
    args(0) = WideToUtf8( NativePath(DWSTRING(args(0))).vptr )

    dim as zstring ptr argv(nArgs)
    for i as long = 0 to nArgs - 1 : argv(i) = strptr(args(i)) : next
    argv(nArgs) = NULL

    dim as long fdIn = -1, fdOut = -1, fdErr = -1
    if lpStartupInfo andalso (lpStartupInfo->dwFlags and STARTF_USESTDHANDLES) then
        fdIn = LnxFdOf( lpStartupInfo->hStdInput )
        fdOut = LnxFdOf( lpStartupInfo->hStdOutput )
        fdErr = LnxFdOf( lpStartupInfo->hStdError )
    end if
    dim as string wd
    if lpCurrentDirectory then wd = U8Path(*lpCurrentDirectory)
    dim as long pid
    dim as GError ptr gerr
    ' G_SPAWN_DO_NOT_REAP_CHILD (2) | G_SPAWN_SEARCH_PATH (4)
    if g_spawn_async_with_fds( iif(len(wd), strptr(wd), NULL), @argv(0), NULL, 2 or 4, NULL, NULL, @pid, _
                               fdIn, fdOut, fdErr, @gerr ) = 0 then
        if gerr then g_error_free( gerr )
        gLnxLastError = ERROR_FILE_NOT_FOUND
        return 0
    end if
    if lpProcessInformation then
        dim as LNXKHANDLE ptr hp = LnxNewKHandle( HK_PROCESS, -1 )
        hp->pid = pid
        lpProcessInformation->hProcess = hp
        lpProcessInformation->hThread = 0
        lpProcessInformation->dwProcessId = pid
    end if
    return 1
end function
#define CreateProcess CreateProcessW

private sub LnxReapProcess( byval h as LNXKHANDLE ptr, byval bBlock as boolean )
    if h->exited then exit sub
    dim as long status
    dim as long r = lnx_waitpid( h->pid, @status, iif(bBlock, 0, LNX_WNOHANG) )
    if r = h->pid then
        h->exited = true
        ' WIFEXITED / WEXITSTATUS
        if (status and &h7F) = 0 then h->exitCode = (status shr 8) and &hFF else h->exitCode = 128 + (status and &h7F)
    elseif r < 0 then
        h->exited = true
    end if
end sub

private function GetExitCodeProcess( byval hProcess as HANDLE, byval lpExitCode as DWORD ptr ) as BOOL
    if LnxIsKHandle(hProcess) = false then return 0
    dim as LNXKHANDLE ptr h = hProcess
    LnxReapProcess( h, false )
    if lpExitCode then *lpExitCode = iif(h->exited, h->exitCode, STILL_ACTIVE)
    return 1
end function

private function WaitForSingleObject( byval hHandle as HANDLE, byval dwMilliseconds as DWORD ) as DWORD
    if LnxIsKHandle(hHandle) = false then return WAIT_OBJECT_0
    dim as LNXKHANDLE ptr h = hHandle
    dim as double t0 = timer
    do
        LnxReapProcess( h, false )
        if h->exited then return WAIT_OBJECT_0
        if dwMilliseconds <> INFINITE andalso (timer - t0) * 1000 >= dwMilliseconds then return WAIT_TIMEOUT
        sleep 5, 1
    loop
end function

private function TerminateProcess( byval hProcess as HANDLE, byval uExitCode as UINT ) as BOOL
    if LnxIsKHandle(hProcess) = false then return 0
    dim as LNXKHANDLE ptr h = hProcess
    if h->exited = false then lnx_kill( h->pid, LNX_SIGKILL ) : LnxReapProcess( h, true )
    h->exitCode = uExitCode
    return 1
end function


' ----------------------------------------------------------------------------
' ShellExecute: URLs and documents open with the desktop's default application,
' executables are started directly.
' ----------------------------------------------------------------------------
#ifndef SEE_MASK_DEFAULT
#define SEE_MASK_DEFAULT          &h00000000
#endif
#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC          &h00000100
#endif
#ifndef SEE_MASK_FLAG_NO_UI
#define SEE_MASK_FLAG_NO_UI       &h00000400
#endif

type SHELLEXECUTEINFOW
    cbSize       as DWORD
    fMask        as ULONG
    hwnd         as HWND
    lpVerb       as LPCWSTR
    lpFile       as LPCWSTR
    lpParameters as LPCWSTR
    lpDirectory  as LPCWSTR
    nShow        as long
    hInstApp     as HINSTANCE
    lpIDList     as any ptr
    lpClass      as LPCWSTR
    hkeyClass    as any ptr
    dwHotKey     as DWORD
    hIcon        as HANDLE
    hProcess     as HANDLE
end type
type SHELLEXECUTEINFO as SHELLEXECUTEINFOW

private function LnxIsExecutableFile( byref wszFile as const wstring ) as boolean
    dim as string s = U8Path(wszFile)
    if g_file_test( strptr(s), G_FILE_TEST_IS_DIR ) then return false
    return g_file_test( strptr(s), G_FILE_TEST_IS_EXECUTABLE ) <> 0
end function

private function ShellExecuteEx( byval pExecInfo as SHELLEXECUTEINFOW ptr ) as BOOL
    if pExecInfo = 0 orelse pExecInfo->lpFile = 0 then return 0
    dim as DWSTRING wszFile = *pExecInfo->lpFile
    dim as boolean isUrl = (instr(wszFile, "://") > 0) orelse (lcase(left(wszFile, 7)) = "mailto:")
    if isUrl = false andalso LnxIsExecutableFile(wszFile) then
        dim as DWSTRING cmd = chr(34) & NativePath(wszFile) & chr(34)
        if pExecInfo->lpParameters andalso len(*pExecInfo->lpParameters) then cmd &= " " & *pExecInfo->lpParameters
        dim as PROCESS_INFORMATION pi
        dim as STARTUPINFO si
        if CreateProcessW( NULL, cmd.vptr, NULL, NULL, 0, 0, NULL, pExecInfo->lpDirectory, @si, @pi ) = 0 then return 0
        if (pExecInfo->fMask and SEE_MASK_NOCLOSEPROCESS) then pExecInfo->hProcess = pi.hProcess else CloseHandle( pi.hProcess )
        return 1
    end if
    ' documents / folders / URLs
    dim as string uri
    if isUrl then
        uri = wszFile.utf8
    else
        dim as string sPath = U8Path(wszFile)
        dim as GFile ptr gf = g_file_new_for_path( strptr(sPath) )
        dim as zstring ptr u = g_file_get_uri( gf )
        uri = *u
        g_free( u ) : g_object_unref( gf )
    end if
    dim as GError ptr gerr
    dim as gboolean ok = g_app_info_launch_default_for_uri( strptr(uri), NULL, @gerr )
    if gerr then g_error_free( gerr )
    pExecInfo->hProcess = 0
    return iif(ok, 1, 0)
end function
#define ShellExecuteExW ShellExecuteEx

private function ShellExecute( byval hwnd as HWND, byval lpOperation as LPCWSTR, byval lpFile as LPCWSTR, _
                               byval lpParameters as LPCWSTR, byval lpDirectory as LPCWSTR, byval nShowCmd as long ) as HINSTANCE
    dim as SHELLEXECUTEINFOW sei
    sei.cbSize = sizeof(sei)
    sei.lpFile = lpFile
    sei.lpParameters = lpParameters
    sei.lpDirectory = lpDirectory
    sei.nShow = nShowCmd
    return cast(HINSTANCE, iif(ShellExecuteEx(@sei), 33, 2))
end function
#define ShellExecuteW ShellExecute

' ----------------------------------------------------------------------------
' Linux helpers for compiling and running programs
' ----------------------------------------------------------------------------
private function LnxFindSystemFbc() as DWSTRING
    dim as zstring ptr p = g_find_program_in_path( "fbc" )
    if p = 0 then return DWSTRING()
    dim as DWSTRING r = *p
    g_free( p )
    return r
end function

' Finds a terminal emulator and returns the command prefix used to run a shell script in it.
private function LnxTerminalCommand( byref sScript as const string ) as string
    dim as string terms(5) = { "x-terminal-emulator", "xfce4-terminal", "gnome-terminal", "konsole", "mate-terminal", "xterm" }
    for i as long = 0 to 5
        dim as zstring ptr p = g_find_program_in_path( strptr(terms(i)) )
        if p then
            dim as string t = *p
            g_free( p )
            select case terms(i)
                case "gnome-terminal", "mate-terminal": return t & " -- /bin/sh " & chr(34) & sScript & chr(34)
                case "xfce4-terminal": return t & " -x /bin/sh " & chr(34) & sScript & chr(34)
                case else: return t & " -e /bin/sh " & chr(34) & sScript & chr(34)
            end select
        end if
    next
    return ""
end function


' ----------------------------------------------------------------------------
' Bring another application's top level window to the front (by title), e.g. the terminal
' that shows a console program. Uses the EWMH _NET_ACTIVE_WINDOW request (source = pager,
' which window managers honour despite focus stealing prevention).
' ----------------------------------------------------------------------------
extern "C"
    declare function gdk_x11_get_default_xdisplay() as any ptr
    declare function XDefaultRootWindow( byval d as any ptr ) as culong
    declare function XInternAtom( byval d as any ptr, byval name_ as const zstring ptr, byval only_if_exists as long ) as culong
    declare function XGetWindowProperty( byval d as any ptr, byval w as culong, byval prop as culong, byval long_offset as clong, _
            byval long_length as clong, byval del as long, byval req_type as culong, byval actual_type as culong ptr, _
            byval actual_format as long ptr, byval nitems as culong ptr, byval bytes_after as culong ptr, byval prop_return as ubyte ptr ptr ) as long
    declare function XFree( byval p as any ptr ) as long
    declare function XSendEvent( byval d as any ptr, byval w as culong, byval propagate as long, byval mask as clong, byval ev as any ptr ) as long
    declare function XFlush( byval d as any ptr ) as long
end extern

private function LnxXWindowTitle( byval d as any ptr, byval w as culong ) as string
    dim as culong actualType, nItems, bytesAfter
    dim as long actualFormat
    dim as ubyte ptr prop
    dim as culong aName = XInternAtom( d, "_NET_WM_NAME", 0 )
    dim as culong aUtf8 = XInternAtom( d, "UTF8_STRING", 0 )
    dim as string r
    if XGetWindowProperty( d, w, aName, 0, 1024, 0, aUtf8, @actualType, @actualFormat, @nItems, @bytesAfter, @prop ) = 0 andalso prop then
        r = *cast(zstring ptr, prop)
        XFree( prop )
    end if
    return r
end function

private function LnxActivateWindowByTitle( byref sTitle as const string ) as boolean
    dim as any ptr d = gdk_x11_get_default_xdisplay()
    if d = 0 then return false
    dim as culong root = XDefaultRootWindow( d )
    dim as culong aList = XInternAtom( d, "_NET_CLIENT_LIST", 0 )
    dim as culong actualType, nItems, bytesAfter
    dim as long actualFormat
    dim as ubyte ptr prop
    if XGetWindowProperty( d, root, aList, 0, 4096, 0, 33, @actualType, @actualFormat, @nItems, @bytesAfter, @prop ) <> 0 orelse prop = 0 then return false
    dim as culong ptr wins = cast(culong ptr, prop)
    dim as culong found = 0
    for i as long = nItems - 1 to 0 step -1          ' newest windows are last
        if instr( LnxXWindowTitle(d, wins[i]), sTitle ) then found = wins[i] : exit for
    next
    XFree( prop )
    if found = 0 then return false
    ' XClientMessageEvent (64 bit layout) inside a 192 byte XEvent
    dim as clong ev(23)
    cast(long ptr, @ev(0))[0] = 33                   ' type = ClientMessage
    cast(long ptr, @ev(2))[0] = 1                    ' send_event
    ev(4) = found                                    ' window
    ev(5) = XInternAtom( d, "_NET_ACTIVE_WINDOW", 0 )' message_type
    cast(long ptr, @ev(6))[0] = 32                   ' format
    ev(7) = 2                                        ' data.l[0]: source indication = pager
    ev(8) = 0                                        ' data.l[1]: timestamp
    XSendEvent( d, root, 0, (1 shl 20) or (1 shl 19), @ev(0) )   ' SubstructureRedirect | SubstructureNotify
    XFlush( d )
    return true
end function

' Runs a compiled program. Console programs run in a terminal window (like Windows opens a
' console for them); bPause keeps the terminal open after the program ends.
private function LnxRunExecutable( byref wszExe as const wstring, byref wszParams as const wstring, _
                                   byval bConsole as boolean, byval bPause as boolean ) as boolean
    dim as DWSTRING exe = NativePath(wszExe)
    dim as DWSTRING folder = AfxStrPathname("PATH", exe)
    dim as string q = chr(34)
    if bConsole = false then
        dim as DWSTRING cmd = q & exe & q & " " & wszParams
        dim as PROCESS_INFORMATION pi
        dim as STARTUPINFO si
        if CreateProcessW( NULL, cmd.vptr, NULL, NULL, 0, 0, NULL, folder.vptr, @si, @pi ) = 0 then return false
        CloseHandle( pi.hProcess )
        return true
    end if
    ' write a small launcher script
    dim as string sScript = *g_get_tmp_dir() & "/_tiko_run_" & getpid() & ".sh"
    dim as long f = freefile
    if open( sScript for output as #f ) <> 0 then return false
    print #f, "#!/bin/sh"
    print #f, "printf '\033]0;%s\007' " & q & "Tiko - " & AfxStrPathname("NAMEX", exe).utf8 & q
    print #f, "cd " & q & folder.utf8 & q
    print #f, q & exe.utf8 & q & " " & WideToUtf8(@wszParams)
    if bPause then
        print #f, "echo"
        print #f, "printf 'Press Enter to continue . . . '"
        print #f, "read dummy"
    end if
    print #f, "rm -f " & q & sScript & q
    close #f
    dim as string cmd = LnxTerminalCommand( sScript )
    if len(cmd) = 0 then
        ' no terminal emulator: run without one
        cmd = "/bin/sh " & q & sScript & q
    end if
    if g_spawn_command_line_async( strptr(cmd), NULL ) = 0 then return false
    ' bring the terminal to the front once it has appeared (like a Windows console)
    dim as string sTitle = "Tiko - " & AfxStrPathname("NAMEX", exe).utf8
    dim as double t0 = timer
    do while timer - t0 < 3
        while gtk_events_pending() : gtk_main_iteration_do( FALSE ) : wend
        if LnxActivateWindowByTitle( sTitle ) then exit do
        sleep 50, 1
    loop
    return true
end function

' FreeBASIC context help (F1). Windows uses the .chm with HTML Help keyword lookup.
' Linux: a CHM viewer when installed (kchmviewer supports index lookup), otherwise the
' online FreeBASIC manual page of the keyword (https://www.freebasic.net/wiki/KeyPg<word>).
' Returns the exact (case sensitive) name of a page stored in a CHM file, e.g. "/KeyPgPrint.html".
' The CHM directory stores page names as plain text, so they are collected once from the file.
private function LnxChmFindPage( byref sChm as const string, byref sPage as const string ) as string
    static as string cachedFile, names
    if cachedFile <> sChm then
        cachedFile = sChm
        names = chr(10)
        dim as string raw
        if ReadFileBytes( DWSTRING(sChm), raw ) = false then return ""
        dim as long i = 1
        do
            i = instr(i, raw, "/KeyPg")
            if i = 0 then exit do
            dim as long j = instr(i, raw, ".html")
            if j = 0 then exit do
            if j - i < 80 then
                dim as string nm = mid(raw, i, j - i + 5)
                if instr(names, chr(10) & nm & chr(10)) = 0 then names &= nm & chr(10)
            end if
            i += 6
        loop
    end if
    if len(sPage) <= len("KeyPg.html") then return ""
    dim as long k = instr(lcase(names), chr(10) & "/" & lcase(sPage) & chr(10))
    if k = 0 then return ""
    return mid(names, k + 1, len(sPage) + 1)
end function

private sub LnxShowFBHelp( byref wszHelpFile as const wstring, byref wszKeyword as const wstring )
    dim as string kw = trim(WideToUtf8(@wszKeyword))
    if left(kw, 1) = "#" then kw = mid(kw, 2)
    dim as string q = chr(34)
    dim as string chm = U8Path(wszHelpFile)
    dim as boolean hasChm = len(chm) > 0 andalso g_file_test( strptr(chm), G_FILE_TEST_EXISTS ) <> 0
    if hasChm then
        dim as zstring ptr p = g_find_program_in_path( "kchmviewer" )
        if p then
            ' Like HtmlHelp keyword lookup: open the keyword's topic page when the CHM has one
            ' (KeyPg<keyword>.html), otherwise search the index.
            dim as string pg = LnxChmFindPage( chm, "KeyPg" & kw & ".html" )
            dim as string opt
            if len(pg) then
                opt = " -showPage " & q & pg & q
            elseif len(kw) then
                opt = " -index " & q & kw & q
            end if
            dim as string cmd = *p & opt & " " & q & chm & q
            g_free( p )
            g_spawn_command_line_async( strptr(cmd), NULL )
            exit sub
        end if
        p = g_find_program_in_path( "xchm" )
        if p then
            dim as string cmd = *p & " " & q & chm & q
            g_free( p )
            g_spawn_command_line_async( strptr(cmd), NULL )
            exit sub
        end if
    end if
    dim as DWSTRING url = "https://www.freebasic.net/wiki/" & iif(len(kw), "KeyPg" & kw, "DocToc")
    ShellExecute( NULL, "open", url.vptr, NULL, NULL, SW_SHOWNORMAL )
end sub

' ----------------------------------------------------------------------------
' Debug console: on Windows gdb gives the debugged program its own console window.
' On Linux a terminal window is opened, it reports its tty and the program is run on it
' (gdb "set inferior-tty"). The terminal closes when the debug session ends.
' ----------------------------------------------------------------------------
dim shared as string gLnxDebugConsolePidFile

private function LnxCreateDebugConsole( byref sTitle as const string = "Tiko Debug Console" ) as string
    dim as string base_ = *g_get_tmp_dir() & "/_tiko_dbgcon_" & getpid()
    dim as string ttyFile = base_ & ".tty", pidFile = base_ & ".pid", script = base_ & ".sh"
    kill ttyFile : kill pidFile
    dim as long f = freefile
    if open( script for output as #f ) <> 0 then return ""
    print #f, "#!/bin/sh"
    print #f, "echo $$ > '" & pidFile & "'"
    print #f, "printf '\033]0;" & sTitle & "\007'"
    print #f, "tty > '" & ttyFile & ".tmp' && mv '" & ttyFile & ".tmp' '" & ttyFile & "'"
    print #f, "trap 'exit 0' TERM HUP"
    print #f, "while :; do sleep 1; done"
    close #f
    dim as string cmd = LnxTerminalCommand( script )
    if len(cmd) = 0 then return ""
    if g_spawn_command_line_async( strptr(cmd), NULL ) = 0 then return ""
    ' wait (max 5 seconds) for the terminal to report its tty
    dim as double t0 = timer
    do while timer - t0 < 5
        if AfxFileExists( DWSTRING(ttyFile) ) then exit do
        while gtk_events_pending() : gtk_main_iteration_do( FALSE ) : wend
        sleep 20, 1
    loop
    dim as string tty
    if AfxFileExists( DWSTRING(ttyFile) ) then
        f = freefile
        if open( ttyFile for input as #f ) = 0 then line input #f, tty : close #f
    end if
    kill ttyFile : kill script
    gLnxDebugConsolePidFile = pidFile
    for k as long = 1 to 20
        if LnxActivateWindowByTitle( sTitle ) then exit for
        sleep 50, 1
    next
    if left(tty, 5) <> "/dev/" then return ""
    return tty
end function

private sub LnxCloseDebugConsole()
    if len(gLnxDebugConsolePidFile) = 0 then exit sub
    dim as long f = freefile, pid
    if open( gLnxDebugConsolePidFile for input as #f ) = 0 then input #f, pid : close #f
    if pid > 0 then lnx_kill( pid, 15 )     ' SIGTERM: the terminal closes with its shell
    kill gLnxDebugConsolePidFile
    gLnxDebugConsolePidFile = ""
end sub
