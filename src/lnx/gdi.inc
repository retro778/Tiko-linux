'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' GDI subset on top of Cairo + Pango.
' Coordinates are logical pixels (GTK handles HiDPI scaling), the design DPI is 96.

#pragma once

#include once "pango/pangocairo.bi"
#include once "fontconfig/fontconfig.bi"

extern "C"
    declare sub pango_fc_font_map_config_changed( byval fcfontmap as any ptr )
end extern

enum LNX_GDIKIND
    GK_FONT = 1
    GK_PEN
    GK_BRUSH
    GK_BITMAP
end enum

type LNXGDIOBJ
    kind        as LNX_GDIKIND
    stock       as boolean
    ' font
    pfd         as PangoFontDescription ptr
    fontHeight  as long          ' Win32 lfHeight (negative = em height in pixels)
    fontWeight  as long
    fontItalic  as boolean
    fontUnderline as boolean
    fontFace    as DWSTRING
    ' pen / brush
    color       as COLORREF
    penStyle    as long
    penWidth    as long
    ' bitmap
    surface     as cairo_surface_t ptr
    bmWidth     as long
    bmHeight    as long
end type

type LNXDCSTATE
    textColor as COLORREF
    bkColor   as COLORREF
    bkMode    as long
    hFont     as HFONT
    hPen      as HPEN
    hBrush    as HBRUSH
    curX      as long
    curY      as long
end type

type LNXDC
    cr          as cairo_t ptr
    ownsCr      as boolean
    hwnd        as HWND
    hBitmap     as HBITMAP        ' bitmap selected into a memory DC
    st          as LNXDCSTATE
    saved(any)  as LNXDCSTATE
    nSaved      as long
end type

#ifndef TRANSPARENT
#define TRANSPARENT     1
#endif
#ifndef OPAQUE
#define OPAQUE          2
#endif
#ifndef SRCCOPY
#define SRCCOPY         &h00CC0020
#endif
#ifndef LOGPIXELSX
#define LOGPIXELSX      88
#endif
#ifndef LOGPIXELSY
#define LOGPIXELSY      90
#endif
#ifndef DT_WORDBREAK
#define DT_WORDBREAK    &h00000010
#endif
#ifndef DT_BOTTOM
#define DT_BOTTOM       &h00000008
#endif
#ifndef DT_CALCRECT
#define DT_CALCRECT     &h00000400
#endif
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS &h00008000
#endif
#ifndef DT_EXPANDTABS
#define DT_EXPANDTABS   &h00000040
#endif
#ifndef DT_WORD_ELLIPSIS
#define DT_WORD_ELLIPSIS &h00040000
#endif
#ifndef PS_DASH
#define PS_DASH         1
#endif
#ifndef PS_DOT
#define PS_DOT          2
#endif
#ifndef NULL_BRUSH
#define NULL_BRUSH      5
#endif
#ifndef WHITE_BRUSH
#define WHITE_BRUSH     0
#endif
#ifndef BLACK_BRUSH
#define BLACK_BRUSH     4
#endif
#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT 17
#endif
#ifndef FW_DONTCARE
#define FW_DONTCARE     0
#endif
#ifndef FW_SEMIBOLD
#define FW_SEMIBOLD     600
#endif

' Font file loaded with AddFontResourceEx (private to the application)
private function AddFontResourceEx( byval pwszFile as const wstring ptr, byval fl as DWORD, byval pdv as any ptr ) as long
    if pwszFile = 0 then return 0
    dim as string s = U8Path(*pwszFile)
    if g_file_test( LnxZ(s), G_FILE_TEST_EXISTS ) = 0 then return 0
    if FcConfigAppFontAddFile( FcConfigGetCurrent(), cast(const FcChar8 ptr, LnxZ(s)) ) = 0 then return 0
    ' Make Pango aware of the newly added font
    dim as PangoFontMap ptr fm = pango_cairo_font_map_get_default()
    if fm then pango_fc_font_map_config_changed( cast(any ptr, fm) )
    return 1
end function

private function RemoveFontResource( byval pwszFile as const wstring ptr ) as BOOL
    return 1
end function

' Map Windows font names to something that exists on Linux when the font is not installed.
private function LnxMapFontFamily( byref wszFace as const wstring ) as string
    dim as string face = WideToUtf8(@wszFace)
    select case lcase(face)
        case "segoe ui", "ms shell dlg", "ms shell dlg 2", "tahoma", "microsoft sans serif", ""
            return face & ", Noto Sans, Cantarell, DejaVu Sans, Sans"
        case "consolas", "courier new", "lucida console", "cascadia code", "cascadia mono"
            return face & ", DejaVu Sans Mono, Noto Sans Mono, Monospace"
    end select
    return face
end function

private function LnxCreateFontObj( byval nHeight as long, byval nWeight as long, byval bItalic as boolean, _
                                   byval bUnderline as boolean, byref wszFace as const wstring ) as HFONT
    dim as HFONT f = new LNXGDIOBJ
    f->kind = GK_FONT
    f->fontHeight = nHeight
    f->fontWeight = iif(nWeight = 0, 400, nWeight)
    f->fontItalic = bItalic
    f->fontUnderline = bUnderline
    f->fontFace = wszFace
    f->pfd = pango_font_description_new()
    dim as string fam = LnxMapFontFamily(wszFace)
    pango_font_description_set_family( f->pfd, LnxZ(fam) )
    dim as long px = abs(nHeight)
    if nHeight > 0 then px = (nHeight * 8) \ 10      ' cell height -> em height
    if px = 0 then px = 12
    pango_font_description_set_absolute_size( f->pfd, px * PANGO_SCALE )
    pango_font_description_set_weight( f->pfd, f->fontWeight )
    if bItalic then pango_font_description_set_style( f->pfd, PANGO_STYLE_ITALIC )
    return f
end function

' Win32 CreateFont signature
private function CreateFont( byval cHeight as long, byval cWidth as long, byval cEscapement as long, _
            byval cOrientation as long, byval cWeight as long, byval bItalic as DWORD, byval bUnderline as DWORD, _
            byval bStrikeOut as DWORD, byval iCharSet as DWORD, byval iOutPrecision as DWORD, _
            byval iClipPrecision as DWORD, byval iQuality as DWORD, byval iPitchAndFamily as DWORD, _
            byval pszFaceName as LPCWSTR ) as HFONT
    return LnxCreateFontObj( cHeight, cWeight, bItalic <> 0, bUnderline <> 0, iif(pszFaceName, *pszFaceName, wstr("")) )
end function

private function CreateFontIndirect( byval lplf as const LOGFONTW ptr ) as HFONT
    if lplf = 0 then return 0
    return LnxCreateFontObj( lplf->lfHeight, lplf->lfWeight, lplf->lfItalic <> 0, lplf->lfUnderline <> 0, lplf->lfFaceName )
end function

private function CreatePen( byval iStyle as long, byval cWidth as long, byval clr as COLORREF ) as HPEN
    dim as HPEN p = new LNXGDIOBJ
    p->kind = GK_PEN
    p->penStyle = iStyle
    p->penWidth = iif(cWidth < 1, 1, cWidth)
    p->color = clr
    return p
end function

private function CreateSolidBrush( byval clr as COLORREF ) as HBRUSH
    dim as HBRUSH b = new LNXGDIOBJ
    b->kind = GK_BRUSH
    b->color = clr
    return b
end function

private function GetStockObject( byval i as long ) as HGDIOBJ
    static as HGDIOBJ objs(0 to 20)
    if i < 0 orelse i > 20 then return 0
    if objs(i) = 0 then
        select case i
            case WHITE_BRUSH: objs(i) = CreateSolidBrush( BGR(255,255,255) )
            case BLACK_BRUSH: objs(i) = CreateSolidBrush( BGR(0,0,0) )
            case NULL_BRUSH:  objs(i) = CreateSolidBrush( BGR(0,0,0) ) : objs(i)->penStyle = -1
            case DEFAULT_GUI_FONT: objs(i) = LnxCreateFontObj( -12, 400, false, false, "Segoe UI" )
            case else: objs(i) = CreateSolidBrush( BGR(0,0,0) )
        end select
        objs(i)->stock = true
    end if
    return objs(i)
end function

private function DeleteObject( byval ho as HGDIOBJ ) as BOOL
    if ho = 0 then return 0
    if ho->stock then return 1
    if ho->pfd then pango_font_description_free( ho->pfd )
    if ho->surface then cairo_surface_destroy( ho->surface )
    delete ho
    return 1
end function
#define DeleteFont(hf)  DeleteObject(hf)

' Windows default system colors (Windows 10 light)
#ifndef COLOR_SCROLLBAR
#define COLOR_SCROLLBAR      0
#endif
#ifndef COLOR_BACKGROUND
#define COLOR_BACKGROUND     1
#endif
#ifndef COLOR_MENU
#define COLOR_MENU           4
#endif
#ifndef COLOR_WINDOWFRAME
#define COLOR_WINDOWFRAME    6
#endif
#ifndef COLOR_MENUTEXT
#define COLOR_MENUTEXT       7
#endif
#ifndef COLOR_3DFACE
#define COLOR_3DFACE         15
#endif
#ifndef COLOR_BTNSHADOW
#define COLOR_BTNSHADOW      16
#endif
#ifndef COLOR_GRAYTEXT
#define COLOR_GRAYTEXT       17
#endif
#ifndef COLOR_BTNTEXT
#define COLOR_BTNTEXT        18
#endif
#ifndef COLOR_3DHILIGHT
#define COLOR_3DHILIGHT      20
#endif
#ifndef COLOR_INFOTEXT
#define COLOR_INFOTEXT       23
#endif
#ifndef COLOR_INFOBK
#define COLOR_INFOBK         24
#endif
#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT       26
#endif
private function GetSysColor( byval nIndex as long ) as COLORREF
    ' Windows 10 default system colors (index -> COLORREF)
    static as COLORREF tbl(0 to 30) = { _
        BGR(200,200,200), BGR(0,0,0),       BGR(153,180,209), BGR(191,205,219), BGR(240,240,240), _
        BGR(255,255,255), BGR(100,100,100), BGR(0,0,0),       BGR(0,0,0),       BGR(0,0,0), _
        BGR(180,180,180), BGR(244,247,252), BGR(171,171,171), BGR(0,120,215),   BGR(255,255,255), _
        BGR(240,240,240), BGR(160,160,160), BGR(109,109,109), BGR(0,0,0),       BGR(0,0,0), _
        BGR(255,255,255), BGR(105,105,105), BGR(227,227,227), BGR(0,0,0),       BGR(255,255,225), _
        BGR(0,0,0),       BGR(0,102,204),   BGR(185,209,234), BGR(215,228,242), BGR(0,120,215), _
        BGR(240,240,240) }
    if nIndex < 0 orelse nIndex > 30 then return BGR(240,240,240)
    return tbl(nIndex)
end function

private function GetSysColorBrush( byval nIndex as long ) as HBRUSH
    static as HBRUSH brushes(0 to 40)
    if nIndex < 0 orelse nIndex > 40 then return 0
    if brushes(nIndex) = 0 then
        brushes(nIndex) = CreateSolidBrush( GetSysColor(nIndex) )
        brushes(nIndex)->stock = true
    end if
    return brushes(nIndex)
end function

private function MulDiv( byval a as long, byval b as long, byval c as long ) as long
    if c = 0 then return -1
    return cint( (cdbl(a) * b) / c )
end function


' ----------------------------------------------------------------------------
' Device contexts
' ----------------------------------------------------------------------------
private sub LnxDCInit( byval hdc as HDC )
    hdc->st.textColor = BGR(0,0,0)
    hdc->st.bkColor = BGR(255,255,255)
    hdc->st.bkMode = OPAQUE
    hdc->st.hFont = GetStockObject(DEFAULT_GUI_FONT)
end sub

private sub LnxSetColor( byval cr as cairo_t ptr, byval clr as COLORREF )
    cairo_set_source_rgb( cr, GetRValue(clr) / 255.0, GetGValue(clr) / 255.0, GetBValue(clr) / 255.0 )
end sub

' A DC that can only be used for measuring text (GetDC on a window outside WM_PAINT draws
' into a scratch surface; GTK does not allow drawing outside of the draw signal).
private function LnxCreateScratchDC( byval hwnd as HWND ) as HDC
    dim as HDC hdc = new LNXDC
    LnxDCInit( hdc )
    dim as cairo_surface_t ptr surf = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, 1, 1 )
    hdc->cr = cairo_create( surf )
    cairo_surface_destroy( surf )
    hdc->ownsCr = true
    hdc->hwnd = hwnd
    if hwnd andalso hwnd->hFont then hdc->st.hFont = hwnd->hFont
    return hdc
end function

private function GetDC( byval hwnd as HWND ) as HDC
    return LnxCreateScratchDC( hwnd )
end function

private function GetDCEx( byval hwnd as HWND, byval hrgnClip as any ptr, byval flags as DWORD ) as HDC
    return LnxCreateScratchDC( hwnd )
end function

private function ReleaseDC( byval hwnd as HWND, byval hdc as HDC ) as long
    if hdc = 0 then return 0
    if hdc->ownsCr andalso hdc->cr then cairo_destroy( hdc->cr )
    delete hdc
    return 1
end function

private function DeleteDC( byval hdc as HDC ) as BOOL
    return ReleaseDC( 0, hdc )
end function

private function CreateCompatibleDC( byval hdcRef as HDC ) as HDC
    dim as HDC hdc = new LNXDC
    LnxDCInit( hdc )
    if hdcRef then hdc->hwnd = hdcRef->hwnd : hdc->st.hFont = hdcRef->st.hFont
    ' a 1x1 surface until a bitmap gets selected
    dim as cairo_surface_t ptr surf = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, 1, 1 )
    hdc->cr = cairo_create( surf )
    cairo_surface_destroy( surf )
    hdc->ownsCr = true
    return hdc
end function

private function CreateCompatibleBitmap( byval hdc as HDC, byval cx as long, byval cy as long ) as HBITMAP
    dim as HBITMAP b = new LNXGDIOBJ
    b->kind = GK_BITMAP
    b->bmWidth = iif(cx < 1, 1, cx)
    b->bmHeight = iif(cy < 1, 1, cy)
    b->surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, b->bmWidth, b->bmHeight )
    return b
end function

private function SelectObject( byval hdc as HDC, byval h as HGDIOBJ ) as HGDIOBJ
    if hdc = 0 orelse h = 0 then return 0
    dim as HGDIOBJ prev
    select case h->kind
        case GK_FONT:  prev = hdc->st.hFont : hdc->st.hFont = h
        case GK_PEN:   prev = hdc->st.hPen : hdc->st.hPen = h
        case GK_BRUSH: prev = hdc->st.hBrush : hdc->st.hBrush = h
        case GK_BITMAP
            prev = hdc->hBitmap
            hdc->hBitmap = h
            if hdc->ownsCr andalso hdc->cr then cairo_destroy( hdc->cr )
            hdc->cr = cairo_create( h->surface )
            hdc->ownsCr = true
    end select
    return prev
end function

private function SaveDC( byval hdc as HDC ) as long
    if hdc = 0 then return 0
    redim preserve hdc->saved(hdc->nSaved)
    hdc->saved(hdc->nSaved) = hdc->st
    hdc->nSaved += 1
    if hdc->cr then cairo_save( hdc->cr )
    return hdc->nSaved
end function

private function RestoreDC( byval hdc as HDC, byval nSavedDC as long ) as BOOL
    if hdc = 0 orelse hdc->nSaved = 0 then return 0
    hdc->nSaved -= 1
    hdc->st = hdc->saved(hdc->nSaved)
    if hdc->cr then cairo_restore( hdc->cr )
    return 1
end function

private function SetTextColor( byval hdc as HDC, byval clr as COLORREF ) as COLORREF
    if hdc = 0 then return 0
    function = hdc->st.textColor
    hdc->st.textColor = clr
end function

private function SetBkColor( byval hdc as HDC, byval clr as COLORREF ) as COLORREF
    if hdc = 0 then return 0
    function = hdc->st.bkColor
    hdc->st.bkColor = clr
end function

private function SetBkMode( byval hdc as HDC, byval mode as long ) as long
    if hdc = 0 then return 0
    function = hdc->st.bkMode
    hdc->st.bkMode = mode
end function

private function GetDeviceCaps( byval hdc as HDC, byval index as long ) as long
    select case index
        case LOGPIXELSX, LOGPIXELSY: return 96
    end select
    return 0
end function

private function BitBlt( byval hdcDest as HDC, byval x as long, byval y as long, byval cx as long, byval cy as long, _
                         byval hdcSrc as HDC, byval x1 as long, byval y1 as long, byval rop as DWORD ) as BOOL
    if hdcDest = 0 orelse hdcSrc = 0 orelse hdcDest->cr = 0 orelse hdcSrc->hBitmap = 0 then return 0
    dim as cairo_t ptr cr = hdcDest->cr
    cairo_save( cr )
    cairo_rectangle( cr, x, y, cx, cy )
    cairo_clip( cr )
    cairo_set_source_surface( cr, hdcSrc->hBitmap->surface, x - x1, y - y1 )
    cairo_set_operator( cr, CAIRO_OPERATOR_SOURCE )
    cairo_paint( cr )
    cairo_restore( cr )
    return 1
end function

private function FillRect( byval hdc as HDC, byval lprc as const RECT ptr, byval hbr as HBRUSH ) as long
    if hdc = 0 orelse hdc->cr = 0 orelse lprc = 0 orelse hbr = 0 then return 0
    if hbr->penStyle = -1 then return 1     ' NULL_BRUSH
    LnxSetColor( hdc->cr, hbr->color )
    cairo_rectangle( hdc->cr, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top )
    cairo_fill( hdc->cr )
    return 1
end function

private function ExtTextOut( byval hdc as HDC, byval x as long, byval y as long, byval options as UINT, _
                             byval lprc as const RECT ptr, byval lpString as LPCWSTR, byval c as UINT, _
                             byval lpDx as const long ptr ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 then return 0
    if (options and ETO_OPAQUE) andalso lprc then
        LnxSetColor( hdc->cr, hdc->st.bkColor )
        cairo_rectangle( hdc->cr, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top )
        cairo_fill( hdc->cr )
    end if
    if lpString andalso c > 0 then
        dim as string s = WideToUtf8( lpString, c )
        dim as PangoLayout ptr layout = pango_cairo_create_layout( hdc->cr )
        if hdc->st.hFont then pango_layout_set_font_description( layout, hdc->st.hFont->pfd )
        pango_layout_set_text( layout, LnxZ(s), len(s) )
        LnxSetColor( hdc->cr, hdc->st.textColor )
        cairo_move_to( hdc->cr, x, y )
        pango_cairo_show_layout( hdc->cr, layout )
        g_object_unref( layout )
    end if
    return 1
end function

private sub LnxStrokeFillShape( byval hdc as HDC, byval bFill as boolean )
    dim as cairo_t ptr cr = hdc->cr
    if bFill andalso hdc->st.hBrush andalso hdc->st.hBrush->penStyle <> -1 then
        LnxSetColor( cr, hdc->st.hBrush->color )
        cairo_fill_preserve( cr )
    end if
    if hdc->st.hPen andalso hdc->st.hPen->penStyle <> PS_NULL then
        LnxSetColor( cr, hdc->st.hPen->color )
        cairo_set_line_width( cr, hdc->st.hPen->penWidth )
        cairo_stroke( cr )
    else
        cairo_new_path( cr )
    end if
end sub

private function Rectangle( byval hdc as HDC, byval l as long, byval t as long, byval r as long, byval b as long ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 then return 0
    dim as double pw = iif(hdc->st.hPen, hdc->st.hPen->penWidth, 1)
    cairo_rectangle( hdc->cr, l + pw / 2, t + pw / 2, (r - l) - pw, (b - t) - pw )
    LnxStrokeFillShape( hdc, true )
    return 1
end function

private function RoundRect( byval hdc as HDC, byval l as long, byval t as long, byval r as long, byval b as long, _
                            byval ew as long, byval eh as long ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 then return 0
    if ew <= 0 orelse eh <= 0 then return Rectangle( hdc, l, t, r, b )
    dim as cairo_t ptr cr = hdc->cr
    dim as double pw = iif(hdc->st.hPen andalso hdc->st.hPen->penStyle <> PS_NULL, hdc->st.hPen->penWidth, 0)
    dim as double x = l + pw / 2, y = t + pw / 2, w = (r - l) - pw, h = (b - t) - pw
    dim as double rad = iif(ew < eh, ew, eh) / 2.0
    if rad > w / 2 then rad = w / 2
    if rad > h / 2 then rad = h / 2
    const PI = 3.14159265358979
    cairo_new_sub_path( cr )
    cairo_arc( cr, x + w - rad, y + rad, rad, -PI / 2, 0 )
    cairo_arc( cr, x + w - rad, y + h - rad, rad, 0, PI / 2 )
    cairo_arc( cr, x + rad, y + h - rad, rad, PI / 2, PI )
    cairo_arc( cr, x + rad, y + rad, rad, PI, 3 * PI / 2 )
    cairo_close_path( cr )
    LnxStrokeFillShape( hdc, true )
    return 1
end function

private function MoveToEx( byval hdc as HDC, byval x as long, byval y as long, byval lppt as POINT ptr ) as BOOL
    if hdc = 0 then return 0
    if lppt then lppt->x = hdc->st.curX : lppt->y = hdc->st.curY
    hdc->st.curX = x : hdc->st.curY = y
    return 1
end function

private function LineTo( byval hdc as HDC, byval x as long, byval y as long ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 then return 0
    dim as cairo_t ptr cr = hdc->cr
    dim as double pw = iif(hdc->st.hPen, hdc->st.hPen->penWidth, 1)
    dim as double off = iif((cint(pw) mod 2) = 1, 0.5, 0)
    ' Win32 LineTo excludes the end point; for axis aligned lines this matches pixel coverage.
    cairo_move_to( cr, hdc->st.curX + iif(hdc->st.curX = x, off, 0), hdc->st.curY + iif(hdc->st.curY = y, off, 0) )
    cairo_line_to( cr, x + iif(hdc->st.curX = x, off, 0), y + iif(hdc->st.curY = y, off, 0) )
    if hdc->st.hPen then LnxSetColor( cr, hdc->st.hPen->color ) else LnxSetColor( cr, 0 )
    cairo_set_line_width( cr, pw )
    cairo_stroke( cr )
    hdc->st.curX = x : hdc->st.curY = y
    return 1
end function

' Builds a Pango layout for the DC's font.
private function LnxLayout( byval hdc as HDC, byref sUtf8 as const string ) as PangoLayout ptr
    dim as PangoLayout ptr layout = pango_cairo_create_layout( hdc->cr )
    if hdc->st.hFont andalso hdc->st.hFont->pfd then pango_layout_set_font_description( layout, hdc->st.hFont->pfd )
    if hdc->st.hFont andalso hdc->st.hFont->fontUnderline then
        dim as PangoAttrList ptr attrs = pango_attr_list_new()
        pango_attr_list_insert( attrs, pango_attr_underline_new( PANGO_UNDERLINE_SINGLE ) )
        pango_layout_set_attributes( layout, attrs )
        pango_attr_list_unref( attrs )
    end if
    pango_layout_set_text( layout, LnxZ(sUtf8), len(sUtf8) )
    return layout
end function

private function DrawText( byval hdc as HDC, byval lpchText as LPCWSTR, byval cchText as long, _
                           byval lprc as RECT ptr, byval uFormat as UINT ) as long
    if hdc = 0 orelse hdc->cr = 0 orelse lpchText = 0 orelse lprc = 0 then return 0
    dim as string s = WideToUtf8( lpchText, cchText )
    dim as cairo_t ptr cr = hdc->cr
    dim as PangoLayout ptr layout = LnxLayout( hdc, s )
    dim as long rcW = lprc->right - lprc->left, rcH = lprc->bottom - lprc->top

    if (uFormat and DT_SINGLELINE) = 0 andalso (uFormat and DT_WORDBREAK) then
        pango_layout_set_width( layout, rcW * PANGO_SCALE )
        pango_layout_set_wrap( layout, PANGO_WRAP_WORD_CHAR )
    end if
    if (uFormat and DT_CALCRECT) = 0 then
        if (uFormat and DT_PATH_ELLIPSIS) then
            pango_layout_set_width( layout, rcW * PANGO_SCALE )
            pango_layout_set_ellipsize( layout, PANGO_ELLIPSIZE_MIDDLE )
        elseif (uFormat and (DT_END_ELLIPSIS or DT_WORD_ELLIPSIS)) then
            pango_layout_set_width( layout, rcW * PANGO_SCALE )
            pango_layout_set_ellipsize( layout, PANGO_ELLIPSIZE_END )
        end if
    end if

    dim as long tw, th
    pango_layout_get_pixel_size( layout, @tw, @th )

    if (uFormat and DT_CALCRECT) then
        lprc->right = lprc->left + tw
        lprc->bottom = lprc->top + th
        g_object_unref( layout )
        return th
    end if

    dim as double x = lprc->left, y = lprc->top
    if (uFormat and DT_CENTER) then
        x = lprc->left + (rcW - tw) / 2
    elseif (uFormat and DT_RIGHT) then
        x = lprc->right - tw
    end if
    if (uFormat and DT_SINGLELINE) then
        if (uFormat and DT_VCENTER) then
            y = lprc->top + (rcH - th) / 2
        elseif (uFormat and DT_BOTTOM) then
            y = lprc->bottom - th
        end if
    end if

    cairo_save( cr )
    if (uFormat and DT_NOCLIP) = 0 then
        cairo_rectangle( cr, lprc->left, lprc->top, rcW, rcH )
        cairo_clip( cr )
    end if
    if hdc->st.bkMode = OPAQUE then
        LnxSetColor( cr, hdc->st.bkColor )
        cairo_rectangle( cr, x, y, tw, th )
        cairo_fill( cr )
    end if
    LnxSetColor( cr, hdc->st.textColor )
    cairo_move_to( cr, cint(x), cint(y) )
    pango_cairo_show_layout( cr, layout )
    cairo_restore( cr )
    g_object_unref( layout )
    return th
end function

private function GetTextExtentPoint32( byval hdc as HDC, byval lpString as LPCWSTR, byval c as long, byval psizl as SIZE ptr ) as BOOL
    if hdc = 0 orelse hdc->cr = 0 orelse psizl = 0 then return 0
    dim as string s = iif(lpString andalso c > 0, WideToUtf8( lpString, c ), "")
    dim as PangoLayout ptr layout = LnxLayout( hdc, s )
    pango_layout_get_pixel_size( layout, @psizl->cx, @psizl->cy )
    g_object_unref( layout )
    return 1
end function
#define GetTextExtentPoint32W  GetTextExtentPoint32

private function GetTextMetrics( byval hdc as HDC, byval lptm as TEXTMETRIC ptr ) as BOOL
    if hdc = 0 orelse lptm = 0 then return 0
    dim as PangoLayout ptr layout = LnxLayout( hdc, "Xg" )
    dim as PangoFontMetrics ptr fm = pango_context_get_metrics( pango_layout_get_context(layout), _
                        iif(hdc->st.hFont, hdc->st.hFont->pfd, NULL), NULL )
    lptm->tmAscent = pango_font_metrics_get_ascent(fm) \ PANGO_SCALE
    lptm->tmDescent = pango_font_metrics_get_descent(fm) \ PANGO_SCALE
    lptm->tmHeight = lptm->tmAscent + lptm->tmDescent
    lptm->tmAveCharWidth = pango_font_metrics_get_approximate_char_width(fm) \ PANGO_SCALE
    lptm->tmMaxCharWidth = lptm->tmAveCharWidth * 2
    lptm->tmWeight = iif(hdc->st.hFont, hdc->st.hFont->fontWeight, 400)
    pango_font_metrics_unref( fm )
    g_object_unref( layout )
    return 1
end function
#define GetTextMetricsW GetTextMetrics
