'    tiko-lldb-mi - lets the tiko editor's debugger (frmDebug.inc, written for gdb/MI) debug
'    with LLVM's lldb: it answers the gdb/MI commands the editor sends by driving lldb-dap
'    over the Debug Adapter Protocol.
'    Licensed under the GNU GPLv3 or later.
'
'    Usage (like gdb):  tiko-lldb-mi -q -i=mi2 "program" [program arguments]
'
' Supported MI subset (exactly what frmDebug.inc uses):
'   break "file":N | tbreak "file":N | tbreak main   (CLI breakpoints)
'   set inferior-tty /dev/pts/N                      (the program's terminal)
'   -exec-run -exec-continue -exec-next -exec-step -exec-finish   kill   quit
'   -break-list -break-delete N -break-enable N.. -break-disable N..
'   -stack-list-variables --all-values
' Answers are written like gdb's, each followed by "(gdb)": *stopped events, ^done,variables
' and ^done,BreakpointTable. Nothing else is written: the editor's reader keeps only the
' first message of a burst, so messages are also spaced out (EmitMi).
'
' lldb-dap is found as $TIKO_LLDB_DAP, else lldb-dap on the PATH, else the newest lldb-dap-NN.

#include once "crt.bi"
#ifndef __FB_LINUX__
    #include once "windows.bi"
#endif
#include once "tkjson.bi"
#include once "tkproc.bi"

#ifdef __FB_LINUX__
extern "C"
    declare function dbg_read alias "read"( byval fd as long, byval buf as any ptr, byval n as uinteger ) as integer
    declare function dbg_write alias "write"( byval fd as long, byval buf as const any ptr, byval n as uinteger ) as integer
    declare function dbg_fork alias "fork"() as long
    declare function dbg_dup2 alias "dup2"( byval oldfd as long, byval newfd as long ) as long
    declare function dbg_open alias "open"( byval path as const zstring ptr, byval flags as long, ... ) as long
    declare function dbg_access alias "access"( byval path as const zstring ptr, byval mode as long ) as long
    declare sub dbg_exit alias "_exit"( byval code as long )
    declare function dbg_setsid alias "setsid"() as long
    declare function dbg_chdir alias "chdir"( byval path as const zstring ptr ) as long
    declare function dbg_setenv alias "setenv"( byval name_ as const zstring ptr, byval value as const zstring ptr, byval overwrite as long ) as long
    declare function dbg_execvp alias "execvp"( byval file as const zstring ptr, byval argv as zstring ptr ptr ) as long
    declare function dbg_ioctl alias "ioctl"( byval fd as long, byval request as culong, ... ) as long
end extern
#endif

' ----------------------------------------------------------------------------------------
' The editor's side: commands arrive on stdin, answers go to stdout (both pipes)
' ----------------------------------------------------------------------------------------
' Optional log of everything exchanged: set TIKO_LLDB_MI_LOG to a file name
sub LogLine( byref sTag as string, byref s as string )
    static as string sFile
    static as boolean bChecked
    if bChecked = false then bChecked = true : sFile = environ("TIKO_LLDB_MI_LOG")
    if len(sFile) = 0 then exit sub
    dim as long f = freefile
    if open(sFile for append as #f) <> 0 then exit sub
    print #f, sTag & " " & left(s, 2000)
    close #f
end sub

sub StdoutWrite( byref s as string )
#ifdef __FB_LINUX__
    dbg_write( 1, strptr(s), len(s) )
#else
    dim as DWORD n
    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), strptr(s), len(s), @n, NULL )
#endif
end sub

' Appends what the editor sent to sOut without blocking; returns -1 when stdin is closed
function StdinRead( byref sOut as string ) as long
#ifdef __FB_LINUX__
    dim as TKPROC_POLLFD pfd
    pfd.fd = 0 : pfd.events = 1
    if tkp_poll( @pfd, 1, 0 ) <= 0 then return 0
    dim as string sChunk = space(4096)
    dim as integer n = dbg_read( 0, strptr(sChunk), len(sChunk) )
    if n <= 0 then return -1
    sOut &= left(sChunk, n)
    return n
#else
    dim as HANDLE h = GetStdHandle(STD_INPUT_HANDLE)
    dim as DWORD nAvail
    if PeekNamedPipe( h, NULL, 0, NULL, @nAvail, NULL ) = 0 then return -1
    if nAvail = 0 then return 0
    if nAvail > 4096 then nAvail = 4096
    dim as string sChunk = space(nAvail)
    dim as DWORD n
    if ReadFile( h, strptr(sChunk), nAvail, @n, NULL ) = 0 then return -1
    sOut &= left(sChunk, n)
    return n
#endif
end function

function FileExists( byref sPath as string ) as boolean
#ifdef __FB_LINUX__
    return dbg_access( sPath, 1 ) = 0
#else
    return GetFileAttributesA( sPath ) <> INVALID_FILE_ATTRIBUTES
#endif
end function

type TKBREAK
    num      as long          ' MI number
    file     as string        ' source path, or "" for a function breakpoint
    lineNum  as long          ' 1 based
    funcName as string        ' function breakpoint (tbreak main)
    temp     as boolean       ' tbreak: removed after the next stop
    enabled  as boolean
    lldbId   as long
    where    as string        ' function the location is in (from 'breakpoint list')
    deleted  as boolean
end type

dim shared as TKPROC gDap            ' lldb-dap
dim shared as string gDapBuf, gMiBuf
dim shared as long gSeq
dim shared gEvents() as string
dim shared gBreaks() as TKBREAK
dim shared as long gNextBreak = 1
dim shared as string gProgram, gArgsJson, gTty
dim shared as boolean gLaunched, gExitReported, gQuit
dim shared as long gThreadId, gFrameId
dim shared as string gLastCmd
dim shared as double gLastEmit

' ----------------------------------------------------------------------------------------
' Output to the editor
' ----------------------------------------------------------------------------------------
sub EmitMi( byref sMsg as string )
    ' the editor reads one burst per message and drops what follows it at once
    do while timer - gLastEmit < 0.15
        sleep 20, 1
    loop
    dim as string s = sMsg & chr(10) & "(gdb)" & chr(10)
    StdoutWrite( s )
    LogLine( "MI>", sMsg )
    gLastEmit = timer
end sub

function MiEsc( byref s as string ) as string
    dim as string sOut
    for i as long = 1 to len(s)
        dim as string c = mid(s, i, 1)
        select case c
            case chr(34): sOut &= "\" & chr(34)
            case "\":     sOut &= "\\"
            case chr(10): sOut &= "\n"
            case chr(13): sOut &= "\r"
            case chr(9):  sOut &= "\t"
            case else:    sOut &= c
        end select
    next
    return sOut
end function

' ----------------------------------------------------------------------------------------
' lldb-dap
' ----------------------------------------------------------------------------------------
function FindLldbDap() as string
    dim as string sEnv = environ("TIKO_LLDB_DAP")
    if len(sEnv) then return sEnv
#ifdef __FB_LINUX__
    dim as string sPath = environ("PATH"), sBest
    dim as long nBest = -1, p = 1
    do while p <= len(sPath)
        dim as long q = instr(p, sPath, ":")
        if q = 0 then q = len(sPath) + 1
        dim as string sDir = mid(sPath, p, q - p)
        p = q + 1
        if len(sDir) = 0 then continue do
        if FileExists( sDir & "/lldb-dap" ) then return sDir & "/lldb-dap"
        ' numbered versions: take the newest
        for v as long = 40 to 14 step -1
            if v <= nBest then exit for
            if FileExists( sDir & "/lldb-dap-" & v ) then nBest = v : sBest = sDir & "/lldb-dap-" & v : exit for
        next
    loop
    return sBest
#else
    ' on the PATH, else where the LLVM installer puts it
    dim as string sPath = environ("PATH")
    dim as long p = 1
    do while p <= len(sPath)
        dim as long q = instr(p, sPath, ";")
        if q = 0 then q = len(sPath) + 1
        dim as string sDir = rtrim(trim(mid(sPath, p, q - p), chr(34)), any "\/")
        p = q + 1
        if (len(sDir) > 0) andalso FileExists( sDir & "\lldb-dap.exe" ) then return sDir & "\lldb-dap.exe"
    loop
    dim as string sTry(1) = { environ("ProgramFiles") & "\LLVM\bin\lldb-dap.exe", "C:\Program Files\LLVM\bin\lldb-dap.exe" }
    for i as long = 0 to 1
        if FileExists( sTry(i) ) then return sTry(i)
    next
    return ""
#endif
end function

function StartDap() as boolean
    dim as string sDap = FindLldbDap()
    LogLine( "lldb-dap:", iif(len(sDap), sDap, "not found") )
    if len(sDap) = 0 then return false
    return TkProc_Start( gDap, sDap )
end function

sub DapWrite( byref sJson as string )
    LogLine( "DAP>", sJson )
    TkProc_Write( gDap, "Content-Length: " & len(sJson) & chr(13, 10, 13, 10) & sJson )
end sub

function DapSend( byref sCommand as string, byref sArgs as string ) as long
    gSeq += 1
    DapWrite( "{""seq"":" & gSeq & ",""type"":""request"",""command"":""" & sCommand & """,""arguments"":" & sArgs & "}" )
    return gSeq
end function

' Reads what lldb-dap has written (waits up to nMs); false when it ended
function DapFill( byval nMs as long ) as boolean
    return TkProc_Read( gDap, gDapBuf, nMs ) >= 0
end function

' One complete message from the buffer, or ""
function DapTake() as string
    dim as long h = instr(gDapBuf, chr(13, 10, 13, 10))
    if h = 0 then return ""
    dim as long p = instr(lcase(left(gDapBuf, h)), "content-length:")
    dim as long n = valint(mid(gDapBuf, p + 15))
    if len(gDapBuf) < h + 3 + n then return ""
    dim as string s = mid(gDapBuf, h + 4, n)
    gDapBuf = mid(gDapBuf, h + 4 + n)
    LogLine( "DAP<", s )
    return s
end function

' lldb-dap asks us to start the program (through its launcher) in a terminal: run it on
' the editor's debug console, the way an IDE runs it in its integrated terminal
sub RunInTerminal( byref sJson as string )
    dim as long nRoot = TkJ_Parse(sJson)
    dim as long nSeq = TkJ_Num(TkJ_Get(nRoot, "seq"))
    dim as long nArgs = TkJ_Get(nRoot, "arguments")
    dim as string sCwd = TkJ_Str(TkJ_Get(nArgs, "cwd"))
    dim as string sArg(any), sEnvK(any), sEnvV(any)
    dim as long c = gTkj(TkJ_Get(nArgs, "args")).firstChild, n = 0
    do while c >= 0
        redim preserve sArg(n) : sArg(n) = TkJ_Str(c) : n += 1
        c = gTkj(c).nextSib
    loop
    dim as long nEnv = TkJ_Get(nArgs, "env"), m = 0
    c = iif(nEnv >= 0, gTkj(nEnv).firstChild, -1)
    do while c >= 0
        redim preserve sEnvK(m), sEnvV(m)
        sEnvK(m) = gTkj(c).key : sEnvV(m) = TkJ_Str(c) : m += 1
        c = gTkj(c).nextSib
    loop
    dim as long pid = -1
#ifdef __FB_LINUX__
    dim as zstring ptr argv(n)
    for k as long = 0 to n - 1 : argv(k) = strptr(sArg(k)) : next
    argv(n) = NULL
    if n > 0 then pid = dbg_fork()
    if pid = 0 then
        ' the child: a new session whose controlling terminal is the console
        dbg_setsid()
        dim as long fd = -1
        if len(gTty) then fd = dbg_open( strptr(gTty), 2 )            ' O_RDWR
        if fd >= 0 then
            dbg_ioctl( fd, &h540E, 0 )                                 ' TIOCSCTTY
            dbg_dup2( fd, 0 ) : dbg_dup2( fd, 1 ) : dbg_dup2( fd, 2 )
        end if
        if len(sCwd) then dbg_chdir( strptr(sCwd) )
        for k as long = 0 to m - 1
            dbg_setenv( strptr(sEnvK(k)), strptr(sEnvV(k)), 1 )
        next
        dbg_execvp( argv(0), @argv(0) )
        dbg_exit( 127 )
    end if
#else
    ' Windows: the program gets a console window of its own
    if n > 0 then
        dim as string sCmd
        for k as long = 0 to n - 1
            sCmd &= iif(k, " ", "") & chr(34) & sArg(k) & chr(34)
        next
        for k as long = 0 to m - 1
            SetEnvironmentVariableA( sEnvK(k), sEnvV(k) )
        next
        dim as string wCmd = TkProc_Wide(sCmd), wCwd = TkProc_Wide(sCwd)
        dim as STARTUPINFOW si
        dim as PROCESS_INFORMATION pi
        si.cb = sizeof(si)
        if CreateProcessW( NULL, cast(wstring ptr, strptr(wCmd)), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, _
                           iif(len(sCwd), cast(wstring ptr, strptr(wCwd)), NULL), @si, @pi ) then
            pid = pi.dwProcessId
            CloseHandle( pi.hThread ) : CloseHandle( pi.hProcess )
        end if
    end if
#endif
    if pid > 0 then
        DapWrite( "{""seq"":0,""type"":""response"",""request_seq"":" & nSeq & ",""success"":true," & _
                  """command"":""runInTerminal"",""body"":{""processId"":" & pid & "}}" )
    else
        DapWrite( "{""seq"":0,""type"":""response"",""request_seq"":" & nSeq & ",""success"":false," & _
                  """command"":""runInTerminal"",""message"":""could not start the program""}" )
    end if
end sub

' Sends a request and waits for its response (events that arrive meanwhile are queued)
function DapRequest( byref sCommand as string, byref sArgs as string, byval nTimeoutMs as long = 15000 ) as string
    if gDap.running = false then return ""
    dim as long nSeq = DapSend( sCommand, sArgs )
    dim as double t0 = timer
    do
        dim as string sMsg = DapTake()
        if len(sMsg) = 0 then
            if (timer - t0) * 1000 > nTimeoutMs then return ""
            if DapFill(100) = false then return ""
            continue do
        end if
        dim as long nRoot = TkJ_Parse(sMsg)
        dim as string sType = TkJ_Str(TkJ_Get(nRoot, "type"))
        if (sType = "response") andalso (TkJ_Num(TkJ_Get(nRoot, "request_seq")) = nSeq) then return sMsg
        if (sType = "request") andalso (TkJ_Str(TkJ_Get(nRoot, "command")) = "runInTerminal") then
            RunInTerminal( sMsg )
            continue do
        end if
        if sType = "event" then
            dim as long n = ubound(gEvents) + 1
            redim preserve gEvents(n)
            gEvents(n) = sMsg
        end if
    loop
end function

' ----------------------------------------------------------------------------------------
' Breakpoints
' ----------------------------------------------------------------------------------------
sub SyncFile( byref sFile as string )
    if (gLaunched = false) andalso (gDap.running = false) then exit sub
    dim as string sList
    dim as long idx(any)
    for i as long = 0 to ubound(gBreaks)
        with gBreaks(i)
            .lldbId = iif(.file = sFile, 0, .lldbId)
            if (.deleted = false) andalso .enabled andalso (.file = sFile) then
                sList &= iif(len(sList), ",", "") & "{""line"":" & .lineNum & "}"
                dim as long n = ubound(idx) + 1
                redim preserve idx(n)
                idx(n) = i
            end if
        end with
    next
    dim as string sResp = DapRequest( "setBreakpoints", "{""source"":{""path"":" & TkJ_Esc(sFile) & "},""breakpoints"":[" & sList & "]}" )
    if len(sResp) = 0 then exit sub
    dim as long nRoot = TkJ_Parse(sResp)
    dim as long nArr = TkJ_Get(TkJ_Get(nRoot, "body"), "breakpoints")
    for k as long = 0 to ubound(idx)
        dim as long nBp = TkJ_Item(nArr, k)
        if nBp >= 0 then gBreaks(idx(k)).lldbId = TkJ_Num(TkJ_Get(nBp, "id"))
    next
end sub

sub SyncFunctions()
    if gDap.running = false then exit sub
    dim as string sList
    dim as long idx(any)
    for i as long = 0 to ubound(gBreaks)
        with gBreaks(i)
            if (.deleted = false) andalso .enabled andalso (len(.funcName) > 0) then
                sList &= iif(len(sList), ",", "") & "{""name"":" & TkJ_Esc(.funcName) & "}"
                dim as long n = ubound(idx) + 1
                redim preserve idx(n)
                idx(n) = i
            end if
        end with
    next
    dim as string sResp = DapRequest( "setFunctionBreakpoints", "{""breakpoints"":[" & sList & "]}" )
    if len(sResp) = 0 then exit sub
    dim as long nRoot = TkJ_Parse(sResp)
    dim as long nArr = TkJ_Get(TkJ_Get(nRoot, "body"), "breakpoints")
    for k as long = 0 to ubound(idx)
        dim as long nBp = TkJ_Item(nArr, k)
        if nBp >= 0 then gBreaks(idx(k)).lldbId = TkJ_Num(TkJ_Get(nBp, "id"))
    next
end sub

' Sends every file's breakpoints (and the function ones)
sub SyncAll()
    dim as string sDone = chr(1)
    for i as long = 0 to ubound(gBreaks)
        dim as string f = gBreaks(i).file
        if len(f) andalso (instr(sDone, chr(1) & f & chr(1)) = 0) then
            SyncFile( f )
            sDone &= f & chr(1)
        end if
    next
    SyncFunctions()
end sub

' "break "file":N" / "tbreak main" / "break file:N"
sub AddBreak( byref sSpec as string, byval bTemp as boolean )
    dim as string s = trim(sSpec)
    dim as long n = ubound(gBreaks) + 1
    redim preserve gBreaks(n)
    with gBreaks(n)
        .num = gNextBreak : gNextBreak += 1
        .temp = bTemp
        .enabled = true
        dim as long pColon = instrrev(s, ":")
        if (pColon > 1) andalso (val(mid(s, pColon + 1)) > 0) then
            .file = trim(left(s, pColon - 1), chr(34))
            .lineNum = valint(mid(s, pColon + 1))
        else
            .funcName = trim(s, chr(34))
        end if
    end with
    if gLaunched then
        if len(gBreaks(n).file) then SyncFile( gBreaks(n).file ) else SyncFunctions()
    end if
end sub

' The function of every breakpoint location ('breakpoint list' through the lldb console)
sub UpdateWhere()
    dim as string sResp = DapRequest( "evaluate", "{""expression"":""`breakpoint list"",""context"":""repl""}" )
    if len(sResp) = 0 then exit sub
    dim as long nRoot = TkJ_Parse(sResp)
    dim as string sOut = TkJ_Str(TkJ_Get(TkJ_Get(nRoot, "body"), "result"))
    ' "  1.1: where = prog`Total + 226 at rd.tiko:8, address = ..."
    dim as long p = 1
    do
        p = instr(p, sOut, ": where = ")
        if p = 0 then exit do
        dim as long ls = instrrev(sOut, chr(10), p) + 1
        dim as long nId = valint(trim(mid(sOut, ls, p - ls)))
        dim as long b = instr(p, sOut, "`")
        dim as long e = instr(b + 1, sOut, " ")
        if (b > 0) andalso (e > b) then
            dim as string sFunc = mid(sOut, b + 1, e - b - 1)
            for i as long = 0 to ubound(gBreaks)
                if gBreaks(i).lldbId = nId then gBreaks(i).where = sFunc
            next
        end if
        p += 10
    loop
end sub

' ----------------------------------------------------------------------------------------
' Values, written the way gdb writes them
' ----------------------------------------------------------------------------------------
' 0x0000555555572fa0 u"total" -> 0x555555572fa0 'total' (gdb with "set language pascal")
function PlainValue( byref sValue as string ) as string
    dim as string s = sValue
    if left(s, 2) = "0x" then
        dim as long sp = instr(s, " ")
        dim as string sAddr = iif(sp, left(s, sp - 1), s)
        dim as string sRest = iif(sp, mid(s, sp + 1), "")
        dim as string sHex = mid(sAddr, 3)
        do while (left(sHex, 1) = "0") andalso (len(sHex) > 1)
            sHex = mid(sHex, 2)
        loop
        sAddr = "0x" & lcase(sHex)
        if left(sRest, 6) = "error:" then sRest = ""             ' lldb could not make a summary
        if (left(sRest, 2) = "u""") orelse (left(sRest, 2) = "U""") orelse (left(sRest, 2) = "L""") then sRest = mid(sRest, 2)
        if (left(sRest, 1) = """") andalso (right(sRest, 1) = """") andalso (len(sRest) >= 2) then
            ' C string -> Pascal string, the way gdb writes it: 'a'#10'b'
            dim as string sCStr = mid(sRest, 2, len(sRest) - 2), sPas
            dim as long k = 1
            do while k <= len(sCStr)
                dim as string ch = mid(sCStr, k, 1)
                if (ch = "\") andalso (k < len(sCStr)) then
                    k += 1
                    select case mid(sCStr, k, 1)
                        case "n": sPas &= "'#10'"
                        case "r": sPas &= "'#13'"
                        case "t": sPas &= "'#9'"
                        case "0": sPas &= "'#0'"
                        case else: sPas &= mid(sCStr, k, 1)
                    end select
                elseif ch = "'" then
                    sPas &= "''"
                else
                    sPas &= ch
                end if
                k += 1
            loop
            sRest = "'" & sPas & "'"
        end if
        return sAddr & iif(len(sRest), " " & sRest, "")
    end if
    return s
end function

declare function FormatVar( byref sValue as string, byref sType as string, byval nRef as long, byval nDepth as long ) as string

' {name = value, ...} or {v, v, ...} from the children of a variable
function FormatChildren( byval nRef as long, byval nDepth as long ) as string
    dim as string sResp = DapRequest( "variables", "{""variablesReference"":" & nRef & ",""count"":40}" )
    if len(sResp) = 0 then return "{...}"
    ' copy the children out first: FormatVar parses other messages
    dim as string sName(any), sVal(any), sTyp(any)
    dim as long nRefs(any)
    dim as long nRoot = TkJ_Parse(sResp)
    dim as long nArr = TkJ_Get(TkJ_Get(nRoot, "body"), "variables")
    dim as long c = iif(nArr >= 0, gTkj(nArr).firstChild, -1)
    dim as long n = 0
    do while (c >= 0) andalso (n < 40)
        redim preserve sName(n), sVal(n), sTyp(n), nRefs(n)
        sName(n) = TkJ_Str(TkJ_Get(c, "name"))
        sVal(n) = TkJ_Str(TkJ_Get(c, "value"))
        sTyp(n) = TkJ_Str(TkJ_Get(c, "type"))
        nRefs(n) = TkJ_Num(TkJ_Get(c, "variablesReference"))
        n += 1
        c = gTkj(c).nextSib
    loop
    dim as string sOut
    for k as long = 0 to n - 1
        dim as string v = FormatVar( sVal(k), sTyp(k), nRefs(k), nDepth + 1 )
        if left(sName(k), 1) = "[" then
            sOut &= iif(k, ", ", "") & v
        else
            sOut &= iif(k, ", ", "") & sName(k) & " = " & v
        end if
    next
    return "{" & sOut & "}"
end function

function FormatVar( byref sValue as string, byref sType as string, byval nRef as long, byval nDepth as long ) as string
    ' a string's data pointer has a summary: keep it on one line
    if (nRef = 0) orelse (instr(sValue, " u""") > 0) orelse (instr(sValue, " """) > 0) then return PlainValue(sValue)
    if right(sType, 2) = " &" then
        ' reference (byref parameter): gdb writes @0xADDR: value
        if left(sValue, 2) <> "0x" then return FormatChildren( nRef, nDepth )   ' lldb gave no address
        ' the address only: lldb may add a summary (or an error) after it
        dim as string sAddr = sValue
        if instr(sAddr, " ") then sAddr = left(sAddr, instr(sAddr, " ") - 1)
        if nDepth >= 3 then return "@" & PlainValue(sAddr)
        return "@" & PlainValue(sAddr) & ": " & FormatChildren( nRef, nDepth )
    end if
    if right(sType, 1) = "*" then return PlainValue(sValue)      ' pointer: its address
    if nDepth >= 3 then return "{...}"
    return FormatChildren( nRef, nDepth )
end function

sub ListVariables()
    if gFrameId = 0 then EmitMi( "^done,variables=[]" ) : exit sub
    dim as string sResp = DapRequest( "scopes", "{""frameId"":" & gFrameId & "}" )
    dim as long nRoot = TkJ_Parse(sResp)
    dim as long nScope = TkJ_Item(TkJ_Get(TkJ_Get(nRoot, "body"), "scopes"), 0)    ' Locals
    dim as long nLocals = TkJ_Num(TkJ_Get(nScope, "variablesReference"))
    if nLocals = 0 then EmitMi( "^done,variables=[]" ) : exit sub
    sResp = DapRequest( "variables", "{""variablesReference"":" & nLocals & "}" )
    dim as string sName(any), sVal(any), sTyp(any)
    dim as long nRefs(any)
    nRoot = TkJ_Parse(sResp)
    dim as long nArr = TkJ_Get(TkJ_Get(nRoot, "body"), "variables")
    dim as long c = iif(nArr >= 0, gTkj(nArr).firstChild, -1)
    dim as long n = 0
    do while c >= 0
        redim preserve sName(n), sVal(n), sTyp(n), nRefs(n)
        sName(n) = TkJ_Str(TkJ_Get(c, "name"))
        sVal(n) = TkJ_Str(TkJ_Get(c, "value"))
        sTyp(n) = TkJ_Str(TkJ_Get(c, "type"))
        nRefs(n) = TkJ_Num(TkJ_Get(c, "variablesReference"))
        n += 1
        c = gTkj(c).nextSib
    loop
    dim as string sOut
    for k as long = 0 to n - 1
        sOut &= iif(k, ",", "") & "{name=""" & MiEsc(sName(k)) & """,value=""" & _
                MiEsc(FormatVar(sVal(k), sTyp(k), nRefs(k), 0)) & """}"
    next
    EmitMi( "^done,variables=[" & sOut & "]" )
end sub

sub ListBreakpoints()
    UpdateWhere()
    dim as string sBody
    dim as long nRows = 0
    for i as long = 0 to ubound(gBreaks)
        with gBreaks(i)
            if .deleted then continue for
            dim as string sFile = .file
            dim as string sShort = mid(sFile, instrrev(sFile, "/") + 1)
            sBody &= iif(nRows, ",", "") & "bkpt={number=""" & .num & """,type=""breakpoint"",disp=""" & _
                     iif(.temp, "del", "keep") & """,enabled=""" & iif(.enabled, "y", "n") & """,addr=""0x0"",func=""" & _
                     MiEsc(iif(len(.where), .where, .funcName)) & """,file=""" & MiEsc(sShort) & """,fullname=""" & _
                     MiEsc(sFile) & """,line=""" & .lineNum & """,times=""0""}"
            nRows += 1
        end with
    next
    EmitMi( "^done,BreakpointTable={nr_rows=""" & nRows & """,nr_cols=""6"",hdr=[],body=[" & sBody & "]}" )
end sub

' ----------------------------------------------------------------------------------------
' Running
' ----------------------------------------------------------------------------------------
sub Launch()
    if StartDap() = false then
        EmitMi( "^error,msg=""lldb-dap was not found""" )
        EmitMi( "*stopped,reason=""exited"",exit-code=""127""" )
        exit sub
    end if
    if len(DapRequest( "initialize", "{""clientID"":""tiko-editor"",""adapterID"":""lldb-dap"",""linesStartAt1"":true," & _
                              """columnsStartAt1"":true,""pathFormat"":""path""}", 20000 )) = 0 then
        ' lldb-dap did not answer (on Windows it needs python310.dll): not a normal exit
        TkProc_Stop( gDap )
        gExitReported = true
        EmitMi( "*stopped,reason=""signal-received"",signal-name=""SIGSEGV"",frame={}" )
        exit sub
    end if
    dim as long nSlash = instrrev(gProgram, "/")
    if instrrev(gProgram, "\") > nSlash then nSlash = instrrev(gProgram, "\")
    dim as string sDir = left(gProgram, nSlash)
    ' Linux: with a debug console the program runs there; Windows: in a console of its own
    ' (runInTerminal, see RunInTerminal)
#ifdef __FB_LINUX__
    dim as boolean bTerminal = (len(gTty) > 0)
#else
    dim as boolean bTerminal = true
#endif
    DapRequest( "launch", "{""program"":" & TkJ_Esc(gProgram) & ",""args"":[" & gArgsJson & "],""cwd"":" & TkJ_Esc(sDir) & _
                          ",""stopOnEntry"":false,""runInTerminal"":" & iif(bTerminal, "true", "false") & "}" )
    ' wait for "initialized" before sending the breakpoints
    dim as double t0 = timer
    dim as boolean bInit = false
    do while (bInit = false) andalso (timer - t0 < 10)
        for i as long = 0 to ubound(gEvents)
            if instr(gEvents(i), """event"":""initialized""") then bInit = true
        next
        if bInit then exit do
        dim as string sMsg = DapTake()
        if len(sMsg) then
            dim as long n = ubound(gEvents) + 1
            redim preserve gEvents(n)
            gEvents(n) = sMsg
        elseif DapFill(100) = false then
            exit do
        end if
    loop
    gLaunched = true
    SyncAll()
    DapRequest( "configurationDone", "{}" )
end sub

sub OnStopped( byref sJson as string )
    dim as long nRoot = TkJ_Parse(sJson)
    dim as long nBody = TkJ_Get(nRoot, "body")
    dim as string sReason = TkJ_Str(TkJ_Get(nBody, "reason"))
    dim as string sDesc = TkJ_Str(TkJ_Get(nBody, "description")) & " " & TkJ_Str(TkJ_Get(nBody, "text"))
    if TkJ_Get(nBody, "threadId") >= 0 then gThreadId = TkJ_Num(TkJ_Get(nBody, "threadId"))

    ' temporary breakpoints end with the first stop
    dim as boolean bTemp = false
    for i as long = 0 to ubound(gBreaks)
        if gBreaks(i).temp andalso (gBreaks(i).deleted = false) then gBreaks(i).deleted = true : bTemp = true
    next
    if bTemp then SyncAll()

    dim as string sResp = DapRequest( "stackTrace", "{""threadId"":" & gThreadId & ",""levels"":1}" )
    nRoot = TkJ_Parse(sResp)
    dim as long nFrame = TkJ_Item(TkJ_Get(TkJ_Get(nRoot, "body"), "stackFrames"), 0)
    gFrameId = TkJ_Num(TkJ_Get(nFrame, "id"))
    dim as string sFunc = TkJ_Str(TkJ_Get(nFrame, "name"))
    dim as long nLine = TkJ_Num(TkJ_Get(nFrame, "line"))
    dim as string sPath = TkJ_Str(TkJ_Get(TkJ_Get(nFrame, "source"), "path"))
    dim as string sFile = mid(sPath, instrrev(sPath, "/") + 1)

    dim as string sMiReason
    if (sReason = "exception") orelse (sReason = "signal") orelse (instr(sDesc, "SIG") > 0) then
        dim as string sSig = "SIGSEGV"
        dim as long p = instr(sDesc, "SIG")
        if p then sSig = mid(sDesc, p, iif(instr(p, sDesc, " "), instr(p, sDesc, " ") - p, len(sDesc)))
        sSig = rtrim(sSig, any ": ,")
        EmitMi( "*stopped,reason=""signal-received"",signal-name=""" & sSig & """,frame={func=""" & MiEsc(sFunc) & _
                """,args=[],file=""" & MiEsc(sFile) & """,fullname=""" & MiEsc(sPath) & """,line=""" & nLine & """}" )
        exit sub
    elseif (sReason = "breakpoint") orelse (sReason = "function breakpoint") then
        sMiReason = "breakpoint-hit"
    elseif gLastCmd = "stepOut" then
        sMiReason = "function-finished"
    elseif (gLastCmd = "next") orelse (gLastCmd = "stepIn") then
        sMiReason = "end-stepping-range"
    else
        sMiReason = "breakpoint-hit"
    end if
    EmitMi( "*stopped,reason=""" & sMiReason & """,frame={addr=""0x0"",func=""" & MiEsc(sFunc) & """,args=[],file=""" & _
            MiEsc(sFile) & """,fullname=""" & MiEsc(sPath) & """,line=""" & nLine & """},thread-id=""1"",stopped-threads=""all""" )
end sub

sub OnEvent( byref sJson as string )
    dim as long nRoot = TkJ_Parse(sJson)
    select case TkJ_Str(TkJ_Get(nRoot, "event"))
        case "stopped"
            OnStopped( sJson )
        case "exited"
            dim as long nCode = TkJ_Num(TkJ_Get(TkJ_Get(nRoot, "body"), "exitCode"))
            if gExitReported = false then
                gExitReported = true
                if nCode = 0 then
                    EmitMi( "*stopped,reason=""exited-normally""" )
                else
                    EmitMi( "*stopped,reason=""exited"",exit-code=""" & nCode & """" )
                end if
            end if
        case "terminated"
            if gExitReported = false then
                gExitReported = true
                EmitMi( "*stopped,reason=""exited-normally""" )
            end if
    end select
end sub

sub ResumeProgram( byref sCommand as string )
    if gLaunched = false then exit sub
    gLastCmd = sCommand
    gFrameId = 0
    DapRequest( sCommand, "{""threadId"":" & gThreadId & "}" )
end sub

sub SetEnabled( byref sNums as string, byval bOn as boolean )
    dim as string sList = " " & sNums & " "
    for i as long = 0 to ubound(gBreaks)
        if instr(sList, " " & gBreaks(i).num & " ") then gBreaks(i).enabled = bOn
    next
    if gLaunched then SyncAll()
end sub

sub Shutdown()
    if gDap.running then
        DapRequest( "disconnect", "{""terminateDebuggee"":true}", 3000 )
        TkProc_Stop( gDap )
    end if
    gLaunched = false
end sub

sub OnMiCommand( byref sLine as string )
    dim as string s = trim(sLine, any chr(13, 10, 32, 9))
    if len(s) = 0 then exit sub
    LogLine( "MI<", s )
    dim as string sWord = s, sRest
    dim as long sp = instr(s, " ")
    if sp then sWord = left(s, sp - 1) : sRest = trim(mid(s, sp + 1))
    select case sWord
        case "break", "b":     AddBreak( sRest, false )
        case "tbreak":         AddBreak( sRest, true )
        case "set"
            if left(sRest, 13) = "inferior-tty " then gTty = trim(mid(sRest, 14))
        case "-inferior-tty-set": gTty = sRest
        case "-exec-run":      Launch()
        case "-exec-continue": ResumeProgram( "continue" )
        case "-exec-next":     ResumeProgram( "next" )
        case "-exec-step":     ResumeProgram( "stepIn" )
        case "-exec-finish":   ResumeProgram( "stepOut" )
        case "kill":           Shutdown()
        case "quit", "-gdb-exit"
            Shutdown()
            gQuit = true
        case "-break-list":    ListBreakpoints()
        case "-break-delete"
            dim as string sList = " " & sRest & " "
            for i as long = 0 to ubound(gBreaks)
                if instr(sList, " " & gBreaks(i).num & " ") then gBreaks(i).deleted = true
            next
            if gLaunched then SyncAll()
        case "-break-enable":  SetEnabled( sRest, true )
        case "-break-disable": SetEnabled( sRest, false )
        case "-stack-list-variables": ListVariables()
    end select
end sub

' ----------------------------------------------------------------------------------------
' Main: arguments like gdb's (options, the program, its arguments)
' ----------------------------------------------------------------------------------------
dim as long nArg = 1
do while len(command(nArg))
    dim as string a = command(nArg)
    nArg += 1
    if (len(gProgram) = 0) andalso (left(a, 1) = "-") then continue do
    if len(gProgram) = 0 then
        gProgram = a
    else
        gArgsJson &= iif(len(gArgsJson), ",", "") & TkJ_Esc(a)
    end if
loop
#ifdef __FB_LINUX__
if (len(gProgram) > 0) andalso (left(gProgram, 1) <> "/") then gProgram = curdir() & "/" & gProgram
#else
if (len(gProgram) > 0) andalso (mid(gProgram, 2, 1) <> ":") andalso (left(gProgram, 1) <> "\") then gProgram = curdir() & "\" & gProgram
#endif

do until gQuit
    ' commands from the editor
    dim as long nGot = StdinRead( gMiBuf )
    if nGot < 0 then
        Shutdown()
        exit do
    end if
    ' output of lldb-dap (waits a little when the editor sent nothing)
    if gDap.running then
        if DapFill( iif(nGot > 0, 0, 20) ) = false then
            ' lldb-dap ended
            TkProc_Stop( gDap )
            if gLaunched andalso (gExitReported = false) then
                gExitReported = true
                EmitMi( "*stopped,reason=""exited-normally""" )
            end if
            gLaunched = false
        end if
    elseif nGot = 0 then
        sleep 20, 1
    end if
    ' MI commands, one per line
    do
        dim as long q = instr(gMiBuf, chr(10))
        if q = 0 then exit do
        dim as string sCmd = left(gMiBuf, q - 1)
        gMiBuf = mid(gMiBuf, q + 1)
        OnMiCommand( sCmd )
        if gQuit then exit do
    loop
    ' events from lldb-dap (queued while waiting for responses, or just read)
    do
        dim as string sEv
        if ubound(gEvents) >= 0 then
            sEv = gEvents(0)
            for k as long = 0 to ubound(gEvents) - 1
                gEvents(k) = gEvents(k + 1)
            next
            if ubound(gEvents) = 0 then erase gEvents else redim preserve gEvents(ubound(gEvents) - 1)
        else
            sEv = DapTake()
            if len(sEv) = 0 then exit do
            if instr(sEv, """type"":""event""") = 0 then continue do
        end if
        OnEvent( sEv )
    loop
loop
