'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' CFindFile, RichEdit RTF loading (help viewer) and a few GDI leftovers.

#pragma once

type LPPOINT as POINT ptr

' Dotted focus rectangle
private function DrawFocusRect( byval hdc as HDC, byval lprc as const RECT ptr ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 orelse lprc = 0 then return 0
    dim as cairo_t ptr cr = hdc->cr
    cairo_save( cr )
    dim as double dashes(1) = { 1, 1 }
    cairo_set_dash( cr, @dashes(0), 2, 0 )
    cairo_set_line_width( cr, 1 )
    LnxSetColor( cr, BGR(0,0,0) )
    cairo_rectangle( cr, lprc->left + 0.5, lprc->top + 0.5, lprc->right - lprc->left - 1, lprc->bottom - lprc->top - 1 )
    cairo_stroke( cr )
    cairo_restore( cr )
    return 1
end function

' ----------------------------------------------------------------------------
' CFindFile (AfxNova) on top of FindFirstFileW
' ----------------------------------------------------------------------------
type CFindFile
    private:
        m_hFind  as HANDLE
        m_fd     as WIN32_FIND_DATAW
        m_folder as DWSTRING
    public:
        declare destructor()
        declare sub Close()
        declare function FindFile( byref wszFileSpec as const wstring ) as LONG_PTR
        declare function FindNext() as long
        declare function IsDots() as boolean
        declare function IsFolder() as boolean
        declare function IsFile() as boolean
        declare function FileName() as DWSTRING
        declare function FileNameX() as DWSTRING
        declare function FileExt() as DWSTRING
        declare function FilePath() as DWSTRING
        declare function Root() as DWSTRING
        declare function FileSize() as ulongint
end type

private destructor CFindFile()
    this.Close()
end destructor

private sub CFindFile.Close()
    if m_hFind andalso m_hFind <> INVALID_HANDLE_VALUE then FindClose( m_hFind )
    m_hFind = 0
end sub

private function CFindFile.FindFile( byref wszFileSpec as const wstring ) as LONG_PTR
    this.Close()
    dim as DWSTRING spec = NativePath(wszFileSpec)
    m_folder = left(spec, instrrev(spec, "/"))
    m_hFind = FindFirstFileW( spec.vptr, @m_fd )
    if m_hFind = INVALID_HANDLE_VALUE then m_hFind = 0 : return E_FAIL
    return S_OK
end function

private function CFindFile.FindNext() as long
    if m_hFind = 0 then return 0
    return FindNextFileW( m_hFind, @m_fd )
end function

private function CFindFile.IsDots() as boolean
    return m_fd.cFileName = "." orelse m_fd.cFileName = ".."
end function

private function CFindFile.IsFolder() as boolean
    return (m_fd.dwFileAttributes and FILE_ATTRIBUTE_DIRECTORY) <> 0
end function

private function CFindFile.IsFile() as boolean
    return (m_fd.dwFileAttributes and FILE_ATTRIBUTE_DIRECTORY) = 0
end function

private function CFindFile.FileName() as DWSTRING
    return m_fd.cFileName
end function

private function CFindFile.FileNameX() as DWSTRING
    dim as DWSTRING s = m_fd.cFileName
    dim as long p = instrrev(s, ".")
    if p > 1 then return left(s, p - 1)
    return s
end function

private function CFindFile.FileExt() as DWSTRING
    dim as DWSTRING s = m_fd.cFileName
    dim as long p = instrrev(s, ".")
    if p > 1 then return mid(s, p + 1)
    return DWSTRING()
end function

private function CFindFile.FilePath() as DWSTRING
    return m_folder & m_fd.cFileName
end function

private function CFindFile.Root() as DWSTRING
    return m_folder
end function

private function CFindFile.FileSize() as ulongint
    return (cast(ulongint, m_fd.nFileSizeHigh) shl 32) or m_fd.nFileSizeLow
end function


' ----------------------------------------------------------------------------
' RTF -> GtkTextBuffer (the subset written by WordPad/RichEdit used in the help files)
' ----------------------------------------------------------------------------
type LNXRTFSTATE
    bold      as boolean
    italic    as boolean
    underline as boolean
    fontSize  as long         ' half points
    colorIdx  as long
    skip      as boolean      ' inside an ignorable destination
    mono      as boolean
end type

private function LnxRtfTag( byval buf as GtkTextBuffer ptr, byref st as LNXRTFSTATE, colors() as COLORREF ) as GtkTextTag ptr
    dim as string nm = "rtf" & iif(st.bold, "b", "") & iif(st.italic, "i", "") & iif(st.underline, "u", "") & _
                       "_s" & st.fontSize & "_c" & st.colorIdx
    dim as GtkTextTagTable ptr tbl = gtk_text_buffer_get_tag_table( buf )
    dim as GtkTextTag ptr tag = gtk_text_tag_table_lookup( tbl, strptr(nm) )
    if tag then return tag
    tag = gtk_text_tag_new( strptr(nm) )
    if st.bold then g_object_set( G_OBJECT(tag), "weight", PANGO_WEIGHT_BOLD, NULL )
    if st.italic then g_object_set( G_OBJECT(tag), "style", PANGO_STYLE_ITALIC, NULL )
    if st.underline then g_object_set( G_OBJECT(tag), "underline", PANGO_UNDERLINE_SINGLE, NULL )
    if st.fontSize > 0 then
        ' RTF half points at 96 DPI -> pixels
        g_object_set( G_OBJECT(tag), "size", cint(st.fontSize / 2 * 96 / 72) * PANGO_SCALE, NULL )
    end if
    if st.colorIdx > 0 andalso st.colorIdx <= ubound(colors) then
        dim as GdkRGBA c
        c.red = GetRValue(colors(st.colorIdx)) / 255 : c.green = GetGValue(colors(st.colorIdx)) / 255
        c.blue = GetBValue(colors(st.colorIdx)) / 255 : c.alpha = 1
        g_object_set( G_OBJECT(tag), "foreground-rgba", @c, NULL )
    end if
    gtk_text_tag_table_add( tbl, tag )
    g_object_unref( tag )
    return tag
end function

private sub LnxRtfFlush( byval buf as GtkTextBuffer ptr, byref st as LNXRTFSTATE, colors() as COLORREF, byref pending as string )
    if len(pending) = 0 then exit sub
    dim as GtkTextIter itEnd
    gtk_text_buffer_get_end_iter( buf, @itEnd )
    gtk_text_buffer_insert_with_tags( buf, @itEnd, strptr(pending), len(pending), LnxRtfTag(buf, st, colors()), NULL )
    pending = ""
end sub

private function RichEdit_LoadRtfFromFile( byval hRichEdit as HWND, byref wszFileName as const wstring ) as boolean
    if IsWindow(hRichEdit) = 0 orelse hRichEdit->inner = 0 then return false
    dim as string rtf
    if ReadFileBytes( wszFileName, rtf ) = false then return false
    dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hRichEdit->inner) )
    gtk_text_buffer_set_text( buf, "", 0 )

    dim as LNXRTFSTATE stack(any), st
    dim as long depth = 0
    dim as COLORREF colors(any)
    redim colors(0)
    dim as long ucSkip = 1
    dim as string pending           ' UTF-8 text waiting to be inserted with the current format
    dim as long n = len(rtf), i = 1
    dim as boolean inColorTbl = false
    dim as long cr_, cg, cb


    while i <= n
        dim as string c = mid(rtf, i, 1)
        select case c
            case "{"
                LnxRtfFlush( buf, st, colors(), pending )
                redim preserve stack(depth)
                stack(depth) = st
                depth += 1
                i += 1
            case "}"
                LnxRtfFlush( buf, st, colors(), pending )
                if inColorTbl then inColorTbl = false
                if depth > 0 then depth -= 1 : st = stack(depth)
                i += 1
            case "\"
                i += 1
                dim as string nc = mid(rtf, i, 1)
                if nc = "\" orelse nc = "{" orelse nc = "}" then
                    if st.skip = false then pending &= nc
                    i += 1
                elseif nc = "'" then
                    ' \'hh  (code page 1252 byte)
                    dim as long b = valint("&h" & mid(rtf, i + 1, 2))
                    i += 3
                    if st.skip = false then
                        dim as string one = chr(b)
                        dim as wstring ptr w = Utf8ToWide( one )      ' cp1252 fallback for invalid UTF-8
                        pending &= WideToUtf8( w )
                        deallocate( w )
                    end if
                elseif nc = "*" then
                    st.skip = true
                    i += 1
                elseif nc = "~" then
                    if st.skip = false then pending &= chr(&hC2, &hA0)
                    i += 1
                elseif nc = !"\n" orelse nc = !"\r" then
                    LnxRtfFlush( buf, st, colors(), pending )
                    if st.skip = false then pending &= !"\n"
                    i += 1
                else
                    ' control word
                    dim as long j = i
                    while j <= n andalso ((mid(rtf, j, 1) >= "a" andalso mid(rtf, j, 1) <= "z") orelse (mid(rtf, j, 1) >= "A" andalso mid(rtf, j, 1) <= "Z"))
                        j += 1
                    wend
                    dim as string word = mid(rtf, i, j - i)
                    dim as long k = j
                    if mid(rtf, k, 1) = "-" then k += 1
                    while k <= n andalso mid(rtf, k, 1) >= "0" andalso mid(rtf, k, 1) <= "9" : k += 1 : wend
                    dim as string sParam = mid(rtf, j, k - j)
                    dim as boolean hasParam = len(sParam) > 0
                    dim as long param = valint(sParam)
                    if mid(rtf, k, 1) = " " then k += 1
                    i = k
                    select case word
                        case "fonttbl", "stylesheet", "info", "pict", "generator", "listtable", "listoverridetable", "rsidtbl", "themedata", "fldinst"
                            st.skip = true
                        case "colortbl"
                            inColorTbl = true
                            st.skip = true
                            redim colors(0)
                        case "red": cr_ = param
                        case "green": cg = param
                        case "blue": cb = param
                        case "par", "line"
                            LnxRtfFlush( buf, st, colors(), pending )
                            if st.skip = false then pending = !"\n" : LnxRtfFlush( buf, st, colors(), pending )
                        case "tab"
                            if st.skip = false then pending &= !"\t"
                        case "b": LnxRtfFlush( buf, st, colors(), pending ) : st.bold = (hasParam = false orelse param <> 0)
                        case "i": LnxRtfFlush( buf, st, colors(), pending ) : st.italic = (hasParam = false orelse param <> 0)
                        case "ul": LnxRtfFlush( buf, st, colors(), pending ) : st.underline = (hasParam = false orelse param <> 0)
                        case "ulnone": LnxRtfFlush( buf, st, colors(), pending ) : st.underline = false
                        case "fs": LnxRtfFlush( buf, st, colors(), pending ) : st.fontSize = param
                        case "cf": LnxRtfFlush( buf, st, colors(), pending ) : st.colorIdx = param
                        case "plain": LnxRtfFlush( buf, st, colors(), pending ) : st.bold = false : st.italic = false : st.underline = false : st.colorIdx = 0
                        case "uc": ucSkip = param
                        case "u"
                            if st.skip = false then
                                dim as ulong cp = iif(param < 0, param + 65536, param)
                                dim as ulong w(1) = { cp, 0 }
                                pending &= WideToUtf8( cast(wstring ptr, @w(0)) )
                            end if
                            ' skip the ANSI replacement characters
                            for s as long = 1 to ucSkip
                                if mid(rtf, i, 1) = "\" andalso mid(rtf, i + 1, 1) = "'" then i += 4 else i += 1
                            next
                        case "emdash": if st.skip = false then pending &= chr(&hE2, &h80, &h94)
                        case "endash": if st.skip = false then pending &= chr(&hE2, &h80, &h93)
                        case "bullet": if st.skip = false then pending &= chr(&hE2, &h80, &hA2)
                        case "lquote": if st.skip = false then pending &= chr(&hE2, &h80, &h98)
                        case "rquote": if st.skip = false then pending &= chr(&hE2, &h80, &h99)
                        case "ldblquote": if st.skip = false then pending &= chr(&hE2, &h80, &h9C)
                        case "rdblquote": if st.skip = false then pending &= chr(&hE2, &h80, &h9D)
                    end select
                end if
            case !"\r", !"\n"
                i += 1
            case ";"
                if inColorTbl then
                    redim preserve colors(ubound(colors) + 1)
                    colors(ubound(colors)) = BGR(cr_, cg, cb)
                    cr_ = 0 : cg = 0 : cb = 0
                elseif st.skip = false then
                    pending &= c
                end if
                i += 1
            case else
                if st.skip = false then pending &= c
                i += 1
        end select
    wend
    LnxRtfFlush( buf, st, colors(), pending )
    return true
end function
