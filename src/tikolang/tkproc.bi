'    tiko editor - a child process with pipes on its stdin and stdout (Linux and Windows)
'    Licensed under the GNU GPLv3 or later.
'
' Used by the tiko-lsp client (modTikoLsp.inc) and by tiko-lldb-mi to drive lldb-dap.
' Needs only the C run time (Linux) or windows.bi (Windows), so the stand-alone tiko-lldb-mi
' program can include it. Paths and arguments are UTF-8. Reading never blocks: poll
' TkProc_Read from a timer or a loop.

#pragma once

type TKPROC
    running as boolean
#ifdef __FB_LINUX__
    pid     as long
    fdIn    as long = -1        ' the child's stdin (we write)
    fdOut   as long = -1        ' the child's stdout (we read)
#else
    hProcess as HANDLE
    hIn      as HANDLE
    hOut     as HANDLE
#endif
end type

#ifdef __FB_LINUX__

type TKPROC_POLLFD
    fd      as long
    events  as short
    revents as short
end type

extern "C"
    declare function tkp_read alias "read"( byval fd as long, byval buf as any ptr, byval n as uinteger ) as integer
    declare function tkp_write alias "write"( byval fd as long, byval buf as const any ptr, byval n as uinteger ) as integer
    declare function tkp_close alias "close"( byval fd as long ) as long
    declare function tkp_pipe alias "pipe"( byval fds as long ptr ) as long
    declare function tkp_fork alias "fork"() as long
    declare function tkp_dup2 alias "dup2"( byval oldfd as long, byval newfd as long ) as long
    declare function tkp_execv alias "execv"( byval path as const zstring ptr, byval argv as zstring ptr ptr ) as long
    declare function tkp_poll alias "poll"( byval fds as TKPROC_POLLFD ptr, byval n as culong, byval timeout as long ) as long
    declare function tkp_open alias "open"( byval path as const zstring ptr, byval flags as long, ... ) as long
    declare function tkp_kill alias "kill"( byval pid as long, byval sig as long ) as long
    declare function tkp_waitpid alias "waitpid"( byval pid as long, byval status as long ptr, byval options as long ) as long
    declare sub tkp_exit alias "_exit"( byval code as long )
end extern

' Starts sExe with the arguments in sArgs (one per chr(10)). The child's stderr is discarded.
private function TkProc_Start( byref p as TKPROC, byref sExe as string, byref sArgs as string = "" ) as boolean
    dim as long pIn(1), pOut(1)
    if tkp_pipe( @pIn(0) ) <> 0 then return false
    if tkp_pipe( @pOut(0) ) <> 0 then tkp_close(pIn(0)) : tkp_close(pIn(1)) : return false
    ' argv: built before fork (no allocation in the child)
    dim as string sArg(any)
    dim as long n = 0, a = 1
    redim sArg(0) : sArg(0) = sExe : n = 1
    do while a <= len(sArgs)
        dim as long q = instr(a, sArgs, chr(10))
        if q = 0 then q = len(sArgs) + 1
        redim preserve sArg(n) : sArg(n) = mid(sArgs, a, q - a) : n += 1
        a = q + 1
    loop
    dim as zstring ptr argv(n)
    for k as long = 0 to n - 1 : argv(k) = strptr(sArg(k)) : next
    argv(n) = 0
    p.pid = tkp_fork()
    if p.pid < 0 then return false
    if p.pid = 0 then
        tkp_dup2( pIn(0), 0 )
        tkp_dup2( pOut(1), 1 )
        dim as long fdNull = tkp_open( "/dev/null", 1 )
        if fdNull >= 0 then tkp_dup2( fdNull, 2 )
        tkp_close( pIn(1) ) : tkp_close( pOut(0) )
        tkp_execv( argv(0), @argv(0) )
        tkp_exit( 127 )
    end if
    tkp_close( pIn(0) ) : tkp_close( pOut(1) )
    p.fdIn = pIn(1)
    p.fdOut = pOut(0)
    p.running = true
    return true
end function

private function TkProc_Write( byref p as TKPROC, byref s as string ) as boolean
    if p.running = false then return false
    dim as long nDone = 0
    do while nDone < len(s)
        dim as integer n = tkp_write( p.fdIn, strptr(s) + nDone, len(s) - nDone )
        if n <= 0 then return false
        nDone += n
    loop
    return true
end function

' Appends what the child wrote to sOut. Waits up to nWaitMs for data.
' Returns the number of bytes read, 0 when there is nothing, -1 when the child closed its end.
private function TkProc_Read( byref p as TKPROC, byref sOut as string, byval nWaitMs as long = 0 ) as long
    if p.running = false then return -1
    dim as TKPROC_POLLFD pfd
    pfd.fd = p.fdOut : pfd.events = 1          ' POLLIN
    if tkp_poll( @pfd, 1, nWaitMs ) <= 0 then return 0
    dim as string sChunk = space(65536)
    dim as integer n = tkp_read( p.fdOut, strptr(sChunk), len(sChunk) )
    if n <= 0 then return -1
    sOut &= left(sChunk, n)
    return n
end function

private sub TkProc_Stop( byref p as TKPROC )
    if p.running = false then exit sub
    p.running = false
    tkp_close( p.fdIn ) : tkp_close( p.fdOut )
    p.fdIn = -1 : p.fdOut = -1
    if p.pid > 0 then
        tkp_kill( p.pid, 15 )
        tkp_waitpid( p.pid, 0, 1 )             ' WNOHANG: reap it if it already ended
    end if
    p.pid = 0
end sub

#else   ' ------------------------------------------------------------------ Windows

' UTF-8 -> UTF-16 (zero terminated, as a string of bytes)
private function TkProc_Wide( byref sUtf8 as string ) as string
    dim as long n = MultiByteToWideChar( CP_UTF8, 0, strptr(sUtf8), len(sUtf8), NULL, 0 )
    dim as string w = string((n + 1) * 2, 0)
    if n > 0 then MultiByteToWideChar( CP_UTF8, 0, strptr(sUtf8), len(sUtf8), cast(wstring ptr, strptr(w)), n )
    return w
end function

' Starts sExe with the arguments in sArgs (one per chr(10)). No console window; stderr is discarded.
private function TkProc_Start( byref p as TKPROC, byref sExe as string, byref sArgs as string = "" ) as boolean
    dim as SECURITY_ATTRIBUTES sa
    sa.nLength = sizeof(sa) : sa.bInheritHandle = TRUE
    dim as HANDLE hInRead, hInWrite, hOutRead, hOutWrite
    if CreatePipe( @hInRead, @hInWrite, @sa, 0 ) = 0 then return false
    if CreatePipe( @hOutRead, @hOutWrite, @sa, 0 ) = 0 then CloseHandle(hInRead) : CloseHandle(hInWrite) : return false
    SetHandleInformation( hInWrite, HANDLE_FLAG_INHERIT, 0 )
    SetHandleInformation( hOutRead, HANDLE_FLAG_INHERIT, 0 )
    dim as HANDLE hNull = CreateFileW( wstr("NUL"), GENERIC_WRITE, FILE_SHARE_WRITE or FILE_SHARE_READ, @sa, OPEN_EXISTING, 0, NULL )

    ' command line: every argument quoted
    dim as string sCmd = chr(34) & sExe & chr(34)
    dim as long a = 1
    do while a <= len(sArgs)
        dim as long q = instr(a, sArgs, chr(10))
        if q = 0 then q = len(sArgs) + 1
        sCmd &= " " & chr(34) & mid(sArgs, a, q - a) & chr(34)
        a = q + 1
    loop
    dim as string wCmd = TkProc_Wide(sCmd)

    dim as STARTUPINFOW si
    dim as PROCESS_INFORMATION pi
    si.cb = sizeof(si)
    si.dwFlags = STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW
    si.wShowWindow = SW_HIDE
    si.hStdInput = hInRead
    si.hStdOutput = hOutWrite
    si.hStdError = hNull
    dim as BOOL bCreated = CreateProcessW( NULL, cast(wstring ptr, strptr(wCmd)), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, @si, @pi )
    CloseHandle( hInRead ) : CloseHandle( hOutWrite )
    if hNull <> INVALID_HANDLE_VALUE then CloseHandle( hNull )
    if bCreated = 0 then CloseHandle( hInWrite ) : CloseHandle( hOutRead ) : return false
    CloseHandle( pi.hThread )
    p.hProcess = pi.hProcess
    p.hIn = hInWrite
    p.hOut = hOutRead
    p.running = true
    return true
end function

private function TkProc_Write( byref p as TKPROC, byref s as string ) as boolean
    if p.running = false then return false
    dim as long nDone = 0
    do while nDone < len(s)
        dim as DWORD n
        if WriteFile( p.hIn, strptr(s) + nDone, len(s) - nDone, @n, NULL ) = 0 then return false
        if n = 0 then return false
        nDone += n
    loop
    return true
end function

' Appends what the child wrote to sOut. Waits up to nWaitMs for data.
' Returns the number of bytes read, 0 when there is nothing, -1 when the child closed its end.
private function TkProc_Read( byref p as TKPROC, byref sOut as string, byval nWaitMs as long = 0 ) as long
    if p.running = false then return -1
    dim as double t0 = timer
    do
        dim as DWORD nAvail
        if PeekNamedPipe( p.hOut, NULL, 0, NULL, @nAvail, NULL ) = 0 then return -1
        if nAvail > 0 then
            if nAvail > 65536 then nAvail = 65536
            dim as string sChunk = space(nAvail)
            dim as DWORD n
            if ReadFile( p.hOut, strptr(sChunk), nAvail, @n, NULL ) = 0 then return -1
            sOut &= left(sChunk, n)
            return n
        end if
        if (timer - t0) * 1000 >= nWaitMs then return 0
        Sleep_( 5 )
    loop
end function

private sub TkProc_Stop( byref p as TKPROC )
    if p.running = false then exit sub
    p.running = false
    CloseHandle( p.hIn ) : CloseHandle( p.hOut )
    if WaitForSingleObject( p.hProcess, 300 ) = WAIT_TIMEOUT then TerminateProcess( p.hProcess, 0 )
    CloseHandle( p.hProcess )
    p.hProcess = 0
end sub

#endif
