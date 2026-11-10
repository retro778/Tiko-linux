'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Remaining Win32 / AfxNova procedures used by tiko:
'   FindFirstFile, keyboard layout queries, system time, directories, layered windows,
'   AfxNova control helpers (TreeView_*, ListView_*, RichEdit_*) and CWinHttpRequest.

#pragma once

' ----------------------------------------------------------------------------
' FindFirstFile / FindNextFile (wildcards in the file name part only)
' ----------------------------------------------------------------------------
type WIN32_FIND_DATAW
    dwFileAttributes   as DWORD
    ftCreationTime     as FILETIME
    ftLastAccessTime   as FILETIME
    ftLastWriteTime    as FILETIME
    nFileSizeHigh      as DWORD
    nFileSizeLow       as DWORD
    dwReserved0        as DWORD
    dwReserved1        as DWORD
    cFileName          as wstring * MAX_PATH
    cAlternateFileName as wstring * 14
end type
type WIN32_FIND_DATA as WIN32_FIND_DATAW

type LNXFINDHANDLE
    magic   as ulong          ' &h46494E44 "FIND"
    folder  as DWSTRING
    names(any) as DWSTRING
    count   as long
    pos_    as long
end type

private sub LnxFillFindData( byval h as LNXFINDHANDLE ptr, byval idx as long, byval lpFindFileData as WIN32_FIND_DATAW ptr )
    dim as DWSTRING full = h->folder & h->names(idx)
    lpFindFileData->cFileName = h->names(idx)
    lpFindFileData->dwFileAttributes = iif(AfxPathIsDirectory(full), FILE_ATTRIBUTE_DIRECTORY, FILE_ATTRIBUTE_NORMAL)
    dim as ulongint sz = AfxGetFileSize(full)
    lpFindFileData->nFileSizeLow = sz and &hFFFFFFFFull
    lpFindFileData->nFileSizeHigh = sz shr 32
    lpFindFileData->ftLastWriteTime = AfxGetFileLastWriteTime(full)
end sub

private function FindFirstFileW( byval lpFileName as LPCWSTR, byval lpFindFileData as WIN32_FIND_DATAW ptr ) as HANDLE
    if lpFileName = 0 orelse lpFindFileData = 0 then return INVALID_HANDLE_VALUE
    dim as DWSTRING spec = NativePath(*lpFileName)
    dim as long p = instrrev(spec, "/")
    dim as DWSTRING folder = iif(p, left(spec, p), wstr("./"))
    dim as DWSTRING pattern = mid(spec, p + 1)
    if len(pattern) = 0 then pattern = "*"
    dim as string sFolder = folder.utf8
    dim as GDir ptr d = g_dir_open( LnxZ(sFolder), 0, NULL )
    if d = 0 then return INVALID_HANDLE_VALUE
    dim as LNXFINDHANDLE ptr h = new LNXFINDHANDLE
    h->magic = &h46494E44
    h->folder = folder
    dim as string pat = lcase(pattern.utf8)
    if pat = "*.*" then pat = "*"
    dim as GPatternSpec ptr ps = g_pattern_spec_new( LnxZ(pat) )
    ' Windows lists "." and ".." for directory searches
    if pat = "*" then
        redim h->names(1) : h->names(0) = "." : h->names(1) = ".." : h->count = 2
    end if
    do
        dim as const zstring ptr nm = g_dir_read_name( d )
        if nm = 0 then exit do
        dim as string lnm = lcase(*nm)
        ' case insensitive matching like Windows
        if g_pattern_match_string( ps, LnxZ(lnm) ) then
            redim preserve h->names(h->count)
            h->names(h->count) = DWSTRING(*nm)
            h->count += 1
        end if
    loop
    g_pattern_spec_free( ps )
    g_dir_close( d )
    if h->count = 0 then delete h : return INVALID_HANDLE_VALUE
    LnxFillFindData( h, 0, lpFindFileData )
    h->pos_ = 1
    return h
end function
#define FindFirstFile FindFirstFileW

private function FindNextFileW( byval hFindFile as HANDLE, byval lpFindFileData as WIN32_FIND_DATAW ptr ) as BOOL
    dim as LNXFINDHANDLE ptr h = hFindFile
    if h = 0 orelse h = INVALID_HANDLE_VALUE orelse h->magic <> &h46494E44 then return 0
    if h->pos_ >= h->count then return 0
    LnxFillFindData( h, h->pos_, lpFindFileData )
    h->pos_ += 1
    return 1
end function
#define FindNextFile FindNextFileW

private function FindClose( byval hFindFile as HANDLE ) as BOOL
    dim as LNXFINDHANDLE ptr h = hFindFile
    if h = 0 orelse h = INVALID_HANDLE_VALUE orelse h->magic <> &h46494E44 then return 0
    h->magic = 0
    delete h
    return 1
end function

private function SHCreateDirectory( byval hwnd as HWND, byval pszPath as LPCWSTR ) as long
    if pszPath = 0 then return E_INVALIDARG
    dim as string s = U8Path(*pszPath)
    return iif( g_mkdir_with_parents( LnxZ(s), &o755 ) = 0, 0, E_FAIL )
end function
#define SHCreateDirectoryEx(hwnd, pszPath, psa) SHCreateDirectory(hwnd, pszPath)

extern "C"
    declare function lnx_mkdir alias "mkdir" ( byval path as const zstring ptr, byval mode as ulong ) as long
end extern

private function CreateDirectory( byval lpPathName as LPCWSTR, byval lpSecurityAttributes as any ptr ) as BOOL
    if lpPathName = 0 then return 0
    dim as string s = U8Path(*lpPathName)
    return iif( lnx_mkdir( LnxZ(s), &o755 ) = 0, 1, 0 )
end function
#define CreateDirectoryW CreateDirectory

' ----------------------------------------------------------------------------
' Characters / keyboard
' ----------------------------------------------------------------------------
private function IsCharAlphaNumeric( byval ch as ulong ) as BOOL
    return iif( g_unichar_isalnum( ch ), 1, 0 )
end function
#define IsCharAlphaNumericW IsCharAlphaNumeric

private function IsCharAlpha( byval ch as ulong ) as BOOL
    return iif( g_unichar_isalpha( ch ), 1, 0 )
end function

private function GetKeyboardLayout( byval idThread as DWORD ) as HKL
    return 0
end function

' Returns the VK code (low byte) and shift state (high byte) needed to type a character.
private function VkKeyScanEx( byval ch as ulong, byval dwhkl as HKL ) as short
    dim as GdkKeymap ptr km = gdk_keymap_get_for_display( gdk_display_get_default() )
    dim as guint kv = gdk_unicode_to_keyval( ch )
    dim as GdkKeymapKey ptr keys
    dim as gint n
    if gdk_keymap_get_entries_for_keyval( km, kv, @keys, @n ) andalso n > 0 then
        dim as guint baseKv
        gdk_keymap_translate_keyboard_state( km, keys[0].keycode, 0, keys[0].group, @baseKv, NULL, NULL, NULL )
        dim as long vk = LnxKeyvalToVK( baseKv )
        dim as long shiftState = iif(keys[0].level = 1, 1, 0)
        g_free( keys )
        if vk then return (shiftState shl 8) or vk
    end if
    return -1
end function
#define VkKeyScanExW VkKeyScanEx

' ----------------------------------------------------------------------------
' System information / time
' ----------------------------------------------------------------------------
private function AfxWindowsBitness() as long
    return 64
end function

private sub GetLocalTime( byval lpSystemTime as SYSTEMTIME ptr )
    if lpSystemTime = 0 then exit sub
    dim as GDateTime ptr dt = g_date_time_new_now_local()
    with *lpSystemTime
        .wYear = g_date_time_get_year(dt) : .wMonth = g_date_time_get_month(dt) : .wDay = g_date_time_get_day_of_month(dt)
        .wDayOfWeek = g_date_time_get_day_of_week(dt) mod 7
        .wHour = g_date_time_get_hour(dt) : .wMinute = g_date_time_get_minute(dt) : .wSecond = g_date_time_get_second(dt)
        .wMilliseconds = g_date_time_get_microsecond(dt) \ 1000
    end with
    g_date_time_unref( dt )
end sub

private function AfxSystemSystemTime() as SYSTEMTIME
    dim as SYSTEMTIME st
    GetLocalTime( @st )
    return st
end function

' Windows date/time picture strings: yyyy MM dd hh HH mm ss tt
private function LnxFormatSystemTime( byref st as SYSTEMTIME, byref wszMask as const wstring ) as DWSTRING
    dim as DWSTRING m = wszMask, r
    dim as long i = 1
    while i <= len(m)
        dim as DWSTRING c = mid(m, i, 1)
        dim as long n = 1
        while i + n <= len(m) andalso mid(m, i + n, 1) = c : n += 1 : wend
        select case c
            case "y": r &= iif(n >= 4, right("0000" & st.wYear, 4), right("00" & (st.wYear mod 100), 2))
            case "M"
                if n >= 3 then r &= format( dateserial(st.wYear, st.wMonth, 1), iif(n = 3, "mmm", "mmmm") ) _
                else r &= iif(n = 2, right("0" & st.wMonth, 2), str(st.wMonth))
            case "d"
                if n >= 3 then r &= format( dateserial(st.wYear, st.wMonth, st.wDay), iif(n = 3, "ddd", "dddd") ) _
                else r &= iif(n = 2, right("0" & st.wDay, 2), str(st.wDay))
            case "H": r &= iif(n = 2, right("0" & st.wHour, 2), str(st.wHour))
            case "h"
                dim as long h12 = st.wHour mod 12 : if h12 = 0 then h12 = 12
                r &= iif(n = 2, right("0" & h12, 2), str(h12))
            case "m": r &= iif(n = 2, right("0" & st.wMinute, 2), str(st.wMinute))
            case "s": r &= iif(n = 2, right("0" & st.wSecond, 2), str(st.wSecond))
            case "t": r &= iif(n = 1, iif(st.wHour < 12, "A", "P"), iif(st.wHour < 12, "AM", "PM"))
            case "'"
                ' quoted literal
                dim as long q = instr(i + 1, m, "'")
                if q = 0 then q = len(m) + 1
                r &= mid(m, i + 1, q - i - 1)
                n = q - i + 1
            case else: r &= wstring(n, c[0])
        end select
        i += n
    wend
    return r
end function

private function AfxSystemTimeToDateStr( byref st as SYSTEMTIME, byref wszMask as const wstring, byval lcid as long = 0 ) as DWSTRING
    return LnxFormatSystemTime( st, wszMask )
end function

private function AfxSystemTimeToTimeStr( byref st as SYSTEMTIME, byref wszMask as const wstring, byval lcid as long = 0 ) as DWSTRING
    return LnxFormatSystemTime( st, wszMask )
end function

' ----------------------------------------------------------------------------
' Bitmaps / layered windows (popup menu drop shadow)
' ----------------------------------------------------------------------------
type BITMAP
    bmType       as long
    bmWidth      as long
    bmHeight     as long
    bmWidthBytes as long
    bmPlanes     as WORD
    bmBitsPixel  as WORD
    bmBits       as any ptr
end type

type BITMAPINFOHEADER
    biSize          as DWORD
    biWidth         as long
    biHeight        as long
    biPlanes        as WORD
    biBitCount      as WORD
    biCompression   as DWORD
    biSizeImage     as DWORD
    biXPelsPerMeter as long
    biYPelsPerMeter as long
    biClrUsed       as DWORD
    biClrImportant  as DWORD
end type

type RGBQUAD
    rgbBlue     as ubyte
    rgbGreen    as ubyte
    rgbRed      as ubyte
    rgbReserved as ubyte
end type

type BITMAPINFO
    bmiHeader    as BITMAPINFOHEADER
    bmiColors(0) as RGBQUAD
end type

type BLENDFUNCTION
    BlendOp             as ubyte
    BlendFlags          as ubyte
    SourceConstantAlpha as ubyte
    AlphaFormat         as ubyte
end type

private function CreateDIBSection( byval hdc as HDC, byval pbmi as const BITMAPINFO ptr, byval usage as UINT, _
                                   byval ppvBits as any ptr ptr, byval hSection as HANDLE, byval offset as DWORD ) as HBITMAP
    if pbmi = 0 then return 0
    dim as HBITMAP b = new LNXGDIOBJ
    b->kind = GK_BITMAP
    b->bmWidth = iif(pbmi->bmiHeader.biWidth < 1, 1, pbmi->bmiHeader.biWidth)
    b->bmHeight = iif(abs(pbmi->bmiHeader.biHeight) < 1, 1, abs(pbmi->bmiHeader.biHeight))
    b->surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, b->bmWidth, b->bmHeight )
    if ppvBits then *ppvBits = cairo_image_surface_get_data( b->surface )
    return b
end function

private function GetObject( byval h as HGDIOBJ, byval c as long, byval pv as any ptr ) as long
    if h = 0 orelse pv = 0 then return 0
    if h->kind = GK_BITMAP andalso c >= sizeof(BITMAP) then
        dim as BITMAP ptr bm = pv
        cairo_surface_flush( h->surface )
        bm->bmWidth = h->bmWidth
        bm->bmHeight = h->bmHeight
        bm->bmWidthBytes = cairo_image_surface_get_stride( h->surface )
        bm->bmPlanes = 1
        bm->bmBitsPixel = 32
        bm->bmBits = cairo_image_surface_get_data( h->surface )
        return sizeof(BITMAP)
    end if
    return 0
end function
#define GetObjectW GetObject

private function LnxLayeredDraw cdecl ( byval w as GtkWidget ptr, byval cr as cairo_t ptr, byval hwnd as HWND ) as gboolean
    if IsWindow(hwnd) = 0 then return FALSE
    dim as cairo_surface_t ptr surf = g_object_get_data( G_OBJECT(hwnd->widget), "lnxlayered" )
    if surf then
        cairo_surface_mark_dirty( surf )
        cairo_set_operator( cr, CAIRO_OPERATOR_SOURCE )
        cairo_set_source_surface( cr, surf, 0, 0 )
        cairo_paint_with_alpha( cr, hwnd->layeredAlpha / 255.0 )
        return TRUE
    end if
    return FALSE
end function

' Layered windows: the window shows the bitmap selected into hdcSrc with the constant alpha.
private function UpdateLayeredWindow( byval hwnd as HWND, byval hdcDst as HDC, byval pptDst as POINT ptr, byval psize as SIZE ptr, _
            byval hdcSrc as HDC, byval pptSrc as POINT ptr, byval crKey as COLORREF, byval pblend as BLENDFUNCTION ptr, byval dwFlags as DWORD ) as BOOL
    if IsWindow(hwnd) = 0 orelse hwnd->kind <> LK_TOPLEVEL then return 0
    if hdcSrc = 0 orelse hdcSrc->hBitmap = 0 then return 0
    dim as GtkWidget ptr w = hwnd->widget
    if g_object_get_data( G_OBJECT(w), "lnxlayeredinit" ) = 0 then
        dim as GdkVisual ptr vis = gdk_screen_get_rgba_visual( gtk_widget_get_screen(w) )
        if vis andalso gtk_widget_get_realized(w) = 0 then gtk_widget_set_visual( w, vis )
        gtk_widget_set_app_paintable( w, TRUE )
        gtk_widget_set_app_paintable( hwnd->client, TRUE )
        g_signal_connect( G_OBJECT(hwnd->client), "draw", G_CALLBACK(@LnxLayeredDraw), hwnd )
        g_object_set_data( G_OBJECT(w), "lnxlayeredinit", cast(gpointer, 1) )
    end if
    ' keep our own copy of the bitmap
    dim as cairo_surface_t ptr old = g_object_get_data( G_OBJECT(w), "lnxlayered" )
    if old then cairo_surface_destroy( old )
    cairo_surface_reference( hdcSrc->hBitmap->surface )
    g_object_set_data( G_OBJECT(w), "lnxlayered", hdcSrc->hBitmap->surface )
    hwnd->layeredAlpha = iif(pblend, pblend->SourceConstantAlpha, 255)
    if pptDst then gtk_window_move( GTK_WINDOW(w), pptDst->x, pptDst->y )
    if psize then gtk_window_resize( GTK_WINDOW(w), psize->cx, psize->cy )
    gtk_widget_queue_draw( hwnd->client )
    return 1
end function

' ----------------------------------------------------------------------------
' AfxNova control helpers
' ----------------------------------------------------------------------------
private function TreeView_AppendItem( byval hwndTV as HWND, byval hParent as HTREEITEM, byval pwszText as wstring ptr, _
                                      byval lParam as LPARAM = 0, byval iImage as long = 0, byval iSelectedImage as long = 0 ) as HTREEITEM
    dim as TVINSERTSTRUCTW tvis
    tvis.hParent = hParent
    tvis.hInsertAfter = TVI_LAST
    tvis.item.mask = TVIF_TEXT or TVIF_PARAM
    tvis.item.pszText = pwszText
    tvis.item.lParam = lParam
    return cast(HTREEITEM, SendMessage( hwndTV, TVM_INSERTITEM, 0, cast(LPARAM, @tvis) ))
end function

private function TreeView_GetlParam( byval hwndTV as HWND, byval hItem as HTREEITEM ) as LPARAM
    dim as TVITEMW tvi
    tvi.mask = TVIF_PARAM
    tvi.hItem = hItem
    SendMessage( hwndTV, TVM_GETITEM, 0, cast(LPARAM, @tvi) )
    return tvi.lParam
end function

#define TreeView_SelectItem(hwnd, hitem)   cast(BOOL, SendMessage((hwnd), TVM_SELECTITEM, TVGN_CARET, cast(LPARAM, (hitem))))
#define TreeView_Expand(hwnd, hitem, code) cast(BOOL, SendMessage((hwnd), TVM_EXPAND, cast(WPARAM, (code)), cast(LPARAM, (hitem))))
#define TreeView_GetSelection(hwnd)        cast(HTREEITEM, SendMessage((hwnd), TVM_GETNEXTITEM, TVGN_CARET, 0))
#define TreeView_DeleteAllItems(hwnd)      cast(BOOL, SendMessage((hwnd), TVM_DELETEITEM, 0, cast(LPARAM, TVI_ROOT)))

private function ListView_AddColumn( byval hwndLV as HWND, byval nCol as long, byval pwszText as wstring ptr, _
                                     byval nWidth as long, byval nFormat as long = 0 ) as long
    dim as LVCOLUMNW lvc
    lvc.mask = LVCF_FMT or LVCF_WIDTH or LVCF_TEXT
    lvc.fmt = nFormat
    lvc.cx = nWidth
    lvc.pszText = pwszText
    lvc.iSubItem = nCol
    return SendMessage( hwndLV, LVM_INSERTCOLUMN, nCol, cast(LPARAM, @lvc) )
end function

private function ListView_GetSelection( byval hwndLV as HWND ) as long
    return SendMessage( hwndLV, LVM_GETNEXTITEM, cast(WPARAM, -1), LVNI_SELECTED )
end function

private sub ListView_SelectItem( byval hwndLV as HWND, byval nIndex as long )
    dim as LVITEMW lvi
    lvi.stateMask = LVIS_SELECTED or LVIS_FOCUSED
    lvi.state = LVIS_SELECTED or LVIS_FOCUSED
    SendMessage( hwndLV, LVM_SETITEMSTATE, nIndex, cast(LPARAM, @lvi) )
    SendMessage( hwndLV, LVM_ENSUREVISIBLE, nIndex, 0 )
end sub

private sub ListView_UnselectAllItems( byval hwndLV as HWND )
    dim as LVITEMW lvi
    lvi.stateMask = LVIS_SELECTED
    lvi.state = 0
    SendMessage( hwndLV, LVM_SETITEMSTATE, cast(WPARAM, -1), cast(LPARAM, @lvi) )
end sub

private sub ListView_MakeHeaderFlat( byval hwndLV as HWND )
end sub

#define ListView_GetItemCount(hwnd)                    clng(SendMessage((hwnd), LVM_GETITEMCOUNT, 0, 0))
#define ListView_DeleteAllItems(hwnd)                  cast(BOOL, SendMessage((hwnd), LVM_DELETEALLITEMS, 0, 0))
#define ListView_SetExtendedListViewStyle(hwnd, dw)    SendMessage((hwnd), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (dw))
#define ListView_GetExtendedListViewStyle(hwnd)        cast(DWORD, SendMessage((hwnd), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0))
#define ListView_EnsureVisible(hwnd, i, fPartialOK)    cast(BOOL, SendMessage((hwnd), LVM_ENSUREVISIBLE, cast(WPARAM, (i)), cast(LPARAM, (fPartialOK))))

private sub RichEdit_SetMargins( byval hRichEdit as HWND, byval nMargins as long, byval nWidth as long )
    if IsWindow(hRichEdit) = 0 orelse hRichEdit->inner = 0 then exit sub
    if (nMargins and EC_LEFTMARGIN) then gtk_text_view_set_left_margin( GTK_TEXT_VIEW(hRichEdit->inner), LOWORD(nWidth) )
    if (nMargins and EC_RIGHTMARGIN) then gtk_text_view_set_right_margin( GTK_TEXT_VIEW(hRichEdit->inner), HIWORD(nWidth) )
end sub

private function RichEdit_GetSelText( byval hRichEdit as HWND ) as DWSTRING
    if IsWindow(hRichEdit) = 0 orelse hRichEdit->inner = 0 then return DWSTRING()
    if hRichEdit->kind = LK_TEXTBOX then
        dim as long a, b
        gtk_editable_get_selection_bounds( GTK_EDITABLE(hRichEdit->inner), @a, @b )
        return mid( LnxEditGetText(hRichEdit), a + 1, b - a )
    end if
    dim as GtkTextBuffer ptr buf = gtk_text_view_get_buffer( GTK_TEXT_VIEW(hRichEdit->inner) )
    dim as GtkTextIter a, b
    if gtk_text_buffer_get_selection_bounds( buf, @a, @b ) = 0 then return DWSTRING()
    dim as zstring ptr p = gtk_text_buffer_get_text( buf, @a, @b, FALSE )
    dim as DWSTRING r = *p
    g_free( p )
    return r
end function

private function RichEdit_CanPaste( byval hRichEdit as HWND, byval clipformat as long ) as long
    return SendMessage( hRichEdit, EM_CANPASTE, clipformat, 0 )
end function
private function RichEdit_SetZoom( byval hRichEdit as HWND, byval zNum as DWORD, byval zDen as DWORD ) as long
    return 1
end function
private function RichEdit_SetTypographyOptions( byval hRichEdit as HWND, byval pto as long, byval fMask as long ) as long
    return 1
end function
private sub RichEdit_SetSel( byval hRichEdit as HWND, byval nStart as long, byval nEnd as long )
    SendMessage( hRichEdit, EM_SETSEL, nStart, nEnd )
end sub
private function RichEdit_SetReadOnly( byval hRichEdit as HWND, byval fReadOnly as long ) as long
    return SendMessage( hRichEdit, EM_SETREADONLY, fReadOnly, 0 )
end function
private sub RichEdit_ScrollCaret( byval hRichEdit as HWND )
    SendMessage( hRichEdit, EM_SCROLLCARET, 0, 0 )
end sub

' ----------------------------------------------------------------------------
' CWinHttpRequest (used only for the "check for updates" download) via curl
' ----------------------------------------------------------------------------
private function AfxWinHttpCheckPlatform() as boolean
    dim as zstring ptr p = g_find_program_in_path( "curl" )
    if p = 0 then return false
    g_free( p )
    return true
end function

type CWinHttpRequest
    m_url    as string
    m_body   as string
    m_status as long
    declare function Open( byref wszVerb as const wstring, byref wszUrl as const wstring, byval bAsync as boolean = false ) as HRESULT
    declare function Send( byref wszBody as const wstring = "" ) as HRESULT
    declare function WaitForResponse( byval nTimeout as long = -1 ) as boolean
    declare function GetResponseBody() as string
    declare function GetStatus() as long
end type

private function CWinHttpRequest.Open( byref wszVerb as const wstring, byref wszUrl as const wstring, byval bAsync as boolean = false ) as HRESULT
    m_url = WideToUtf8(@wszUrl)
    return S_OK
end function

private function CWinHttpRequest.Send( byref wszBody as const wstring = "" ) as HRESULT
    dim as string cmd = "curl -s -L --max-time 10 " & chr(34) & m_url & chr(34)
    dim as zstring ptr outp, errp
    dim as long exitStatus
    m_body = ""
    if g_spawn_command_line_sync( LnxZ(cmd), @outp, @errp, @exitStatus, NULL ) then
        if outp then m_body = *outp : g_free( outp )
        if errp then g_free( errp )
        m_status = iif(exitStatus = 0, 200, 0)
        return S_OK
    end if
    return E_FAIL
end function

private function CWinHttpRequest.WaitForResponse( byval nTimeout as long = -1 ) as boolean
    return m_status = 200
end function

private function CWinHttpRequest.GetResponseBody() as string
    return m_body
end function

private function CWinHttpRequest.GetStatus() as long
    return m_status
end function

' ----------------------------------------------------------------------------
' CImageCtx (AfxNova image control) - only what the About dialog needs.
' ----------------------------------------------------------------------------
private function LnxImageCtx_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if uMsg = WM_PAINT then
        dim as GdkPixbuf ptr pb = g_object_get_data( G_OBJECT(hwnd->client), "lnxpixbuf" )
        if pb andalso hwnd->paintCr then
            dim as GdkPixbuf ptr scaled = gdk_pixbuf_scale_simple( pb, hwnd->w, hwnd->h, GDK_INTERP_BILINEAR )
            gdk_cairo_set_source_pixbuf( hwnd->paintCr, scaled, 0, 0 )
            cairo_paint( hwnd->paintCr )
            g_object_unref( scaled )
        end if
        return 0
    end if
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function

type CImageCtx
    m_hCtl as HWND
    declare constructor( byval pWindow as CWindow ptr, byval cID as LONG_PTR, byref wszFile as const wstring = "", _
                         byval x as long = 0, byval y as long = 0, byval nWidth as long = 0, byval nHeight as long = 0 )
    declare function LoadImageFromResource( byval hInstance as HINSTANCE, byref wszImageName as const wstring ) as boolean
    declare function LoadImageFromFile( byref wszFileName as const wstring ) as boolean
    declare property hWindow() as HWND
end type

private constructor CImageCtx( byval pWindow as CWindow ptr, byval cID as LONG_PTR, byref wszFile as const wstring = "", _
                               byval x as long = 0, byval y as long = 0, byval nWidth as long = 0, byval nHeight as long = 0 )
    if pWindow = 0 then exit constructor
    m_hCtl = LnxCreateWindow( LK_CUSTOM, "CImageCtx", "", WS_CHILD or WS_VISIBLE, 0, pWindow->ScaleX(x), pWindow->ScaleY(y), _
                              pWindow->ScaleX(nWidth), pWindow->ScaleY(nHeight), pWindow->hWindow, cID, @LnxImageCtx_WndProc, NULL )
    if len(wszFile) then this.LoadImageFromFile( wszFile )
end constructor

private function CImageCtx.LoadImageFromFile( byref wszFileName as const wstring ) as boolean
    if IsWindow(m_hCtl) = 0 then return false
    dim as string s = U8Path(wszFileName)
    dim as GdkPixbuf ptr pb = gdk_pixbuf_new_from_file( LnxZ(s), NULL )
    if pb = 0 then return false
    g_object_set_data_full( G_OBJECT(m_hCtl->client), "lnxpixbuf", pb, cast(GDestroyNotify, @g_object_unref) )
    gtk_widget_queue_draw( m_hCtl->client )
    return true
end function

' Resources are not linked into the Linux executable; the application icon lives in bin/tiko.png
private function CImageCtx.LoadImageFromResource( byval hInstance as HINSTANCE, byref wszImageName as const wstring ) as boolean
    return this.LoadImageFromFile( AfxGetExePathName & "bin/tiko.png" )
end function

private property CImageCtx.hWindow() as HWND
    return m_hCtl
end property

' ----------------------------------------------------------------------------
' WM_SYSCOMMAND codes
' ----------------------------------------------------------------------------
#ifndef SC_CLOSE
const SC_SIZE     = &hF000
const SC_MOVE     = &hF010
const SC_MINIMIZE = &hF020
const SC_MAXIMIZE = &hF030
const SC_CLOSE    = &hF060
const SC_RESTORE  = &hF120
#endif

private function AfxSetWindowText overload ( byval hwnd as HWND, byval pwszText as const wstring ptr ) as BOOL
    return SetWindowText( hwnd, pwszText )
end function

' The "tiko" lexer comes from src/lexer/LexTiko.cxx (the Windows build's lexer is not
' published); other names are created by the linked Lexilla.
private function LnxCreateTikoLexer cdecl ( byval name_ as const zstring ptr ) as any ptr
    if name_ andalso lcase(*name_) = "tiko" then return TikoCreateLexer()
    dim as any ptr p = LnxLexCreateLexer( name_ )
    if p = 0 then p = LnxLexCreateLexer( "freebasic" )
    return p
end function
