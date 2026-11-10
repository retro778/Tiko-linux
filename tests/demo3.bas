#include once "../src/lnx/lnx.bi"
function Dlg_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if uMsg = WM_DESTROY then PostQuitMessage(0) : return 0
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function
LnxInit()
dim pWindow as CWindow ptr = new CWindow
dim as HWND hDlg = pWindow->Create( NULL, "LB test", @Dlg_WndProc, 0, 0, 0, 0, WS_POPUP or WS_CAPTION or WS_SYSMENU )
pWindow->SetClientSize( 300, 200 )
dim as HWND hList = pWindow->AddControl( "LISTBOX", , 100, "", 10, 10, 200, 150, WS_CHILD or WS_VISIBLE or WS_VSCROLL or WS_TABSTOP or LBS_NOTIFY or LBS_NOINTEGRALHEIGHT, WS_EX_CLIENTEDGE )
dim as DWSTRING s1 = "ASCII Chart", s2 = "GUID Generator"
ListBox_AddString( hList, s1.vptr ) : ListBox_AddString( hList, s2.vptr )
ListBox_SetCurSel( hList, 0 )
print "cursel="; ListBox_GetCurSel(hList)
ShowWindow( hDlg, SW_SHOW )
dim as MSG m
for k as long = 1 to 60 : PeekMessage( @m, 0, 0, 0, PM_NOREMOVE ) : sleep 10 : next
print "cursel after show="; ListBox_GetCurSel(hList)
dim as GdkWindow ptr gw = gtk_widget_get_window( hDlg->widget )
gdk_pixbuf_save( gdk_pixbuf_get_from_window( gw, 0, 0, gdk_window_get_width(gw), gdk_window_get_height(gw) ), command(1), "png", NULL, NULL )
