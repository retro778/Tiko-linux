' Demo of the Linux window layer: custom painted windows, owner drawn listbox,
' native controls, Scintilla and the Win32 style message loop.
#include once "../src/lnx/lnx.bi"

#define IDC_PANEL   100
#define IDC_LIST    101
#define IDC_BUTTON  102
#define IDC_TEXT    103
#define IDC_SCI     104
#define IDC_CHECK   105
#define IDC_COMBO   106

dim shared as HWND hMain, hPanel, hList, hSci, hText
dim shared as long gClicks, gHotPanel

function Panel_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    select case uMsg
        case WM_PAINT
            dim as PAINTSTRUCT ps
            dim as HDC hdc = BeginPaint( hwnd, @ps )
            dim as RECT rc : GetClientRect( hwnd, @rc )
            dim as HBRUSH hbr = CreateSolidBrush( iif(gHotPanel, BGR(60,66,80), BGR(40,44,52)) )
            FillRect( hdc, @rc, hbr ) : DeleteObject( hbr )
            dim as HFONT hf = CreateFont( -14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Segoe UI" )
            SelectObject( hdc, hf )
            SetTextColor( hdc, BGR(204,204,204) ) : SetBkMode( hdc, TRANSPARENT )
            dim as DWSTRING s = "Custom window - clicks: " & gClicks
            DrawText( hdc, s.vptr, -1, @rc, DT_CENTER or DT_VCENTER or DT_SINGLELINE )
            dim as HPEN hp = CreatePen( PS_SOLID, 1, BGR(0,120,215) )
            SelectObject( hdc, hp )
            MoveToEx( hdc, 0, rc.bottom - 1, NULL ) : LineTo( hdc, rc.right, rc.bottom - 1 )
            DeleteObject( hp ) : DeleteObject( hf )
            EndPaint( hwnd, @ps )
            return 0
        case WM_MOUSEMOVE
            dim as TRACKMOUSEEVENT tme
            tme.cbSize = sizeof(tme) : tme.dwFlags = TME_LEAVE : tme.hwndTrack = hwnd
            TrackMouseEvent( @tme )
            if gHotPanel = 0 then gHotPanel = 1 : AfxRedrawWindow( hwnd )
        case WM_MOUSELEAVE
            gHotPanel = 0 : AfxRedrawWindow( hwnd )
        case WM_LBUTTONDOWN
            gClicks += 1 : AfxRedrawWindow( hwnd )
    end select
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function

function Main_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    select case uMsg
        case WM_SIZE
            dim as long cx = LOWORD(lParam), cy = HIWORD(lParam)
            SetWindowPos( hPanel, 0, 0, 0, cx, 40, SWP_NOZORDER )
            SetWindowPos( hList, 0, 0, 40, 200, cy - 40, SWP_NOZORDER )
            SetWindowPos( hSci, 0, 200, 110, cx - 200, cy - 110, SWP_NOZORDER )
            return 0
        case WM_MEASUREITEM
            dim as MEASUREITEMSTRUCT ptr mis = cast(MEASUREITEMSTRUCT ptr, lParam)
            mis->itemHeight = 24
            return 1
        case WM_DRAWITEM
            dim as DRAWITEMSTRUCT ptr dis = cast(DRAWITEMSTRUCT ptr, lParam)
            dim as HBRUSH hbr = CreateSolidBrush( iif(dis->itemState and ODS_SELECTED, BGR(62,68,81), BGR(33,37,43)) )
            FillRect( dis->hDC, @dis->rcItem, hbr ) : DeleteObject( hbr )
            SetTextColor( dis->hDC, BGR(215,218,224) ) : SetBkMode( dis->hDC, TRANSPARENT )
            dim as DWSTRING s = AfxGetListBoxText( dis->hwndItem, dis->itemID )
            dim as RECT rc = dis->rcItem : rc.left += 8
            DrawText( dis->hDC, s.vptr, -1, @rc, DT_LEFT or DT_VCENTER or DT_SINGLELINE )
            return 1
        case WM_COMMAND
            select case LOWORD(wParam)
                case IDC_BUTTON
                    if HIWORD(wParam) = BN_CLICKED then
                        ListBox_AddString( hList, AfxGetWindowText(hText).vptr )
                        print "button: added "; AfxGetWindowText(hText).utf8
                    end if
                case IDC_LIST
                    if HIWORD(wParam) = LBN_SELCHANGE then print "listbox sel ="; ListBox_GetCurSel(hList)
                case IDC_TEXT
                    if HIWORD(wParam) = EN_CHANGE then print "text changed: "; AfxGetWindowText(hText).utf8
                case IDC_CHECK
                    print "checkbox ="; Button_GetCheck(GetDlgItem(hwnd, IDC_CHECK))
                case IDC_COMBO
                    if HIWORD(wParam) = CBN_SELCHANGE then print "combo sel ="; ComboBox_GetCurSel(cast(HWND, lParam))
                case 9001
                    print "accelerator Ctrl+K -> WM_COMMAND 9001"
            end select
        case WM_TIMER
            static as long ticks
            ticks += 1
            if ticks = 1 then print "timer works"
        case WM_DESTROY
            PostQuitMessage(0)
            return 0
    end select
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function

LnxInit()
dim pWindow as CWindow ptr = new CWindow("demo")
hMain = pWindow->Create( NULL, "Tiko Linux layer demo", @Main_WndProc, 100, 100, 900, 600 )
pWindow->Brush = CreateSolidBrush( BGR(33,37,43) )

dim pPanel as CWindow ptr = new CWindow
hPanel = pPanel->Create( hMain, "", @Panel_WndProc, 0, 0, 900, 40, WS_CHILD or WS_VISIBLE )
hList = pWindow->AddControl( "LISTBOX", hMain, IDC_LIST, "", 0, 40, 200, 560, _
          WS_CHILD or WS_VISIBLE or WS_TABSTOP or LBS_NOTIFY or LBS_OWNERDRAWFIXED or LBS_HASSTRINGS or LBS_NOINTEGRALHEIGHT )
for i as long = 1 to 40 : dim as DWSTRING s = "Item number " & i : ListBox_AddString( hList, s.vptr ) : next
pWindow->AddControl( "BUTTON", hMain, IDC_BUTTON, "&Add item", 210, 50, 90, 24, WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_PUSHBUTTON )
hText = pWindow->AddControl( "TEXTBOX", hMain, IDC_TEXT, "new item", 310, 50, 200, 22, WS_CHILD or WS_VISIBLE or WS_TABSTOP, WS_EX_CLIENTEDGE )
pWindow->AddControl( "CHECKBOX", hMain, IDC_CHECK, "Check me", 520, 52, 110, 20, WS_CHILD or WS_VISIBLE or WS_TABSTOP )
dim as HWND hCombo = pWindow->AddControl( "COMBOBOX", hMain, IDC_COMBO, "", 640, 50, 150, 24, WS_CHILD or WS_VISIBLE or WS_TABSTOP or CBS_DROPDOWNLIST )
ComboBox_AddString( hCombo, @wstr("Win64 Console") ) : ComboBox_AddString( hCombo, @wstr("Win64 GUI") ) : ComboBox_SetCurSel( hCombo, 0 )
pWindow->AddControl( "LABEL", hMain, 0, "Label: the quick brown fox", 210, 82, 300, 20, WS_CHILD or WS_VISIBLE )
hSci = CreateWindowEx( 0, "Scintilla", "", WS_CHILD or WS_VISIBLE or WS_TABSTOP, 200, 110, 700, 490, hMain, cast(HMENU, IDC_SCI), NULL, NULL )
dim as any ptr lx = LnxLexCreateLexer("freebasic")
SendMessage( hSci, 4033, 0, cast(LPARAM, lx) )   ' SCI_SETILEXER
dim as string code = !"' Scintilla inside the window layer\ndim as long x = 10\nprint x\n"
SendMessage( hSci, 2181, 0, cast(LPARAM, strptr(code)) )   ' SCI_SETTEXT
SetTimer( hMain, 1, 200, NULL )

dim as ACCEL acc(0)
acc(0).fVirt = FVIRTKEY or FCONTROL : acc(0).key = VK_K : acc(0).cmd = 9001
dim as HACCEL hAcc = CreateAcceleratorTable( @acc(0), 1 )

ShowWindow( hMain, SW_SHOW )
SetFocus( hSci )

' --- automated self test (when run with "auto") ---
if command(1) = "auto" then
    dim as MSG m
    for k as long = 1 to 50 : PeekMessage( @m, 0, 0, 0, PM_NOREMOVE ) : while gtk_events_pending() : gtk_main_iteration() : wend : next
    print "list count ="; ListBox_GetCount(hList); "  top ="; ListBox_GetTopIndex(hList)
    ListBox_SetCurSel( hList, 5 )
    print "cursel ="; ListBox_GetCurSel(hList); "  text = "; AfxGetListBoxText(hList, 5).utf8
    SendMessage( GetDlgItem(hMain, IDC_BUTTON), BM_CLICK, 0, 0 )
    print "list count after click ="; ListBox_GetCount(hList)
    AfxSetWindowText( hText, "changed by code" )
    Button_SetCheck( GetDlgItem(hMain, IDC_CHECK), BST_CHECKED )
    print "check ="; Button_GetCheck(GetDlgItem(hMain, IDC_CHECK)); "  combo text = "; AfxGetWindowText(hCombo).utf8
    dim as RECT rc : GetClientRect( hMain, @rc ) : print "client ="; rc.right; rc.bottom
    GetWindowRect( hList, @rc ) : print "list screen rect ="; rc.left; rc.top; rc.right; rc.bottom
    print "sci length ="; SendMessage( hSci, 2006, 0, 0 )
    ' screenshot of the window for visual check
    dim as GdkWindow ptr gw = gtk_widget_get_window( hMain->widget )
    dim as long w = gdk_window_get_width(gw), h = gdk_window_get_height(gw)
    dim as GdkPixbuf ptr pb = gdk_pixbuf_get_from_window( gw, 0, 0, w, h )
    if pb then gdk_pixbuf_save( pb, command(2), "png", NULL, NULL ) : print "screenshot saved"
    DestroyWindow( hMain )
end if

dim as MSG uMsg
do while GetMessage( @uMsg, NULL, 0, 0 )
    if TranslateAccelerator( hMain, hAcc, @uMsg ) = 0 then
        TranslateMessage( @uMsg )
        DispatchMessage( @uMsg )
    end if
loop
print "message loop ended, exit code"; uMsg.wParam
