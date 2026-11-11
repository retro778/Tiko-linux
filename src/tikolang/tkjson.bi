'    tiko editor - minimal JSON reader/writer (shared by the tiko-lsp client and tiko-lldb-mi)
'    Licensed under the GNU GPLv3 or later.
'
' One message at a time is parsed into a pool of nodes: TkJ_Parse returns the root node,
' TkJ_Get/TkJ_Item walk objects and arrays, TkJ_Str/TkJ_Num read values, TkJ_Esc writes a
' JSON string literal.

#pragma once

' ----------------------------------------------------------------------------------------
' Minimal JSON reader: one message at a time is parsed into a node pool
' ----------------------------------------------------------------------------------------
enum TKJK_KIND
    TKJK_NULL = 0, TKJK_BOOL, TKJK_NUM, TKJK_STR, TKJK_ARR, TKJK_OBJ
end enum

type TKJNODE
    kind       as TKJK_KIND
    key        as string
    sval       as string        ' strings (UTF-8), also "true"/"false"
    nval       as double
    firstChild as long
    lastChild  as long
    nextSib    as long
end type

dim shared gTkj() as TKJNODE
dim shared as long gTkjCount
dim shared as string gTkjSrc
dim shared as long gTkjPos

private function TkJ_New( byval kind as TKJK_KIND ) as long
    if gTkjCount > ubound(gTkj) then redim preserve gTkj(gTkjCount * 2 + 64)
    dim as long n = gTkjCount
    gTkjCount += 1
    gTkj(n).kind = kind : gTkj(n).key = "" : gTkj(n).sval = "" : gTkj(n).nval = 0
    gTkj(n).firstChild = -1 : gTkj(n).lastChild = -1 : gTkj(n).nextSib = -1
    return n
end function

private sub TkJ_Skip()
    do while gTkjPos <= len(gTkjSrc)
        select case gTkjSrc[gTkjPos - 1]
            case 32, 9, 10, 13: gTkjPos += 1
            case else: exit do
        end select
    loop
end sub

private function TkJ_Utf8( byval cp as ulong ) as string
    if cp < &h80 then return chr(cp)
    if cp < &h800 then return chr(&hC0 or (cp shr 6), &h80 or (cp and &h3F))
    if cp < &h10000 then return chr(&hE0 or (cp shr 12), &h80 or ((cp shr 6) and &h3F), &h80 or (cp and &h3F))
    return chr(&hF0 or (cp shr 18), &h80 or ((cp shr 12) and &h3F), &h80 or ((cp shr 6) and &h3F), &h80 or (cp and &h3F))
end function

private function TkJ_ReadString() as string
    ' gTkjPos is on the opening quote
    gTkjPos += 1
    dim as string sOut
    dim as long nStart = gTkjPos
    do while gTkjPos <= len(gTkjSrc)
        dim as long c = gTkjSrc[gTkjPos - 1]
        if c = 34 then
            sOut &= mid(gTkjSrc, nStart, gTkjPos - nStart)
            gTkjPos += 1
            return sOut
        elseif c = 92 then
            sOut &= mid(gTkjSrc, nStart, gTkjPos - nStart)
            dim as long e = gTkjSrc[gTkjPos]
            gTkjPos += 2
            select case e
                case asc("n"): sOut &= chr(10)
                case asc("t"): sOut &= chr(9)
                case asc("r"): sOut &= chr(13)
                case asc("b"): sOut &= chr(8)
                case asc("f"): sOut &= chr(12)
                case asc("u")
                    dim as ulong cp = valuint("&h" & mid(gTkjSrc, gTkjPos, 4))
                    gTkjPos += 4
                    if (cp >= &hD800) andalso (cp < &hDC00) andalso (mid(gTkjSrc, gTkjPos, 2) = "\u") then
                        dim as ulong lo = valuint("&h" & mid(gTkjSrc, gTkjPos + 2, 4))
                        if (lo >= &hDC00) andalso (lo < &hE000) then
                            cp = &h10000 + ((cp - &hD800) shl 10) + (lo - &hDC00)
                            gTkjPos += 6
                        end if
                    end if
                    sOut &= TkJ_Utf8(cp)
                case else: sOut &= chr(e)
            end select
            nStart = gTkjPos
        else
            gTkjPos += 1
        end if
    loop
    return sOut
end function

private function TkJ_Value() as long
    TkJ_Skip()
    if gTkjPos > len(gTkjSrc) then return TkJ_New(TKJK_NULL)
    dim as long c = gTkjSrc[gTkjPos - 1]
    dim as long n
    select case c
        case 123, 91        ' { [
            n = TkJ_New(iif(c = 123, TKJK_OBJ, TKJK_ARR))
            gTkjPos += 1
            do
                TkJ_Skip()
                if gTkjPos > len(gTkjSrc) then exit do
                dim as long d = gTkjSrc[gTkjPos - 1]
                if (d = 125) orelse (d = 93) then gTkjPos += 1 : exit do
                if d = 44 then gTkjPos += 1 : continue do
                dim as string sKey
                if c = 123 then
                    sKey = TkJ_ReadString()
                    TkJ_Skip()
                    if (gTkjPos <= len(gTkjSrc)) andalso (gTkjSrc[gTkjPos - 1] = 58) then gTkjPos += 1
                end if
                dim as long child = TkJ_Value()
                gTkj(child).key = sKey
                if gTkj(n).lastChild < 0 then gTkj(n).firstChild = child else gTkj(gTkj(n).lastChild).nextSib = child
                gTkj(n).lastChild = child
            loop
        case 34
            n = TkJ_New(TKJK_STR)
            gTkj(n).sval = TkJ_ReadString()
        case asc("t"), asc("f")
            n = TkJ_New(TKJK_BOOL)
            gTkj(n).sval = iif(c = asc("t"), "true", "false")
            gTkjPos += iif(c = asc("t"), 4, 5)
        case asc("n")
            n = TkJ_New(TKJK_NULL)
            gTkjPos += 4
        case else
            n = TkJ_New(TKJK_NUM)
            dim as long nStart = gTkjPos
            do while gTkjPos <= len(gTkjSrc)
                select case gTkjSrc[gTkjPos - 1]
                    case asc("0") to asc("9"), asc("-"), asc("+"), asc("."), asc("e"), asc("E"): gTkjPos += 1
                    case else: exit do
                end select
            loop
            gTkj(n).nval = val(mid(gTkjSrc, nStart, gTkjPos - nStart))
    end select
    return n
end function

private function TkJ_Parse( byref sJson as string ) as long
    gTkjCount = 0
    if ubound(gTkj) < 0 then redim gTkj(255)
    gTkjSrc = sJson
    gTkjPos = 1
    return TkJ_Value()
end function

' child of an object by key (-1 if missing)
private function TkJ_Get( byval n as long, byref sKey as string ) as long
    if (n < 0) orelse (gTkj(n).kind <> TKJK_OBJ) then return -1
    dim as long c = gTkj(n).firstChild
    do while c >= 0
        if gTkj(c).key = sKey then return c
        c = gTkj(c).nextSib
    loop
    return -1
end function

' i-th element of an array (-1 if missing)
private function TkJ_Item( byval n as long, byval i as long ) as long
    if (n < 0) orelse (gTkj(n).kind <> TKJK_ARR) then return -1
    dim as long c = gTkj(n).firstChild
    do while (c >= 0) andalso (i > 0)
        c = gTkj(c).nextSib : i -= 1
    loop
    return c
end function

private function TkJ_Str( byval n as long ) as string
    if n < 0 then return ""
    if gTkj(n).kind = TKJK_NUM then return str(gTkj(n).nval)
    return gTkj(n).sval
end function

private function TkJ_Num( byval n as long ) as long
    if (n < 0) orelse (gTkj(n).kind <> TKJK_NUM) then return 0
    return clng(gTkj(n).nval)
end function

' a JSON string literal for UTF-8 text
private function TkJ_Esc( byref s as string ) as string
    dim as string sOut = chr(34)
    dim as long nStart = 1
    for i as long = 1 to len(s)
        dim as long c = s[i - 1]
        if (c = 34) orelse (c = 92) orelse (c < 32) then
            sOut &= mid(s, nStart, i - nStart)
            select case c
                case 34: sOut &= "\" & chr(34)
                case 92: sOut &= "\\"
                case 10: sOut &= "\n"
                case 13: sOut &= "\r"
                case 9:  sOut &= "\t"
                case else: sOut &= "\u00" & lcase(hex(c, 2))
            end select
            nStart = i + 1
        end if
    next
    return sOut & mid(s, nStart) & chr(34)
end function

