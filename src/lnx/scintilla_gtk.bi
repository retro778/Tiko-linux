'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' GTK3 Scintilla widget and Lexilla bindings. Scintilla and Lexilla are linked statically
' from deps/ (see Makefile), so there is no LoadLibrary/GetProcAddress step on Linux.

#pragma once

#inclib "stdc++"

extern "C"
    declare function scintilla_new() as GtkWidget ptr
    declare sub scintilla_set_id( byval sci as any ptr, byval id as uinteger )
    declare function scintilla_send_message( byval sci as any ptr, byval iMessage as ulong, _
                                             byval wParam as uinteger, byval lParam as integer ) as integer
    declare sub scintilla_release_resources()

    ' Lexilla
    declare function LnxLexCreateLexer alias "CreateLexer" ( byval name as const zstring ptr ) as any ptr
    declare function LnxLexGetLexerCount alias "GetLexerCount" () as long
    ' The "tiko" lexer (src/lexer/LexTiko.cxx)
    declare function TikoCreateLexer() as any ptr
    declare sub LnxLexGetLexerName alias "GetLexerName" ( byval index as ulong, byval name as zstring ptr, byval buflength as long )
end extern

' Notification structure passed by the "sci-notify" GTK signal (same layout as Windows SCNotification).
type SCNotification_ as any
