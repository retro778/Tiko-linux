#include once "../src/lnx/lnx.bi"
function Dlg_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    if uMsg = WM_DESTROY then PostQuitMessage(0) : return 0
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function
function Child_WndProc( byval hwnd as HWND, byval uMsg as UINT, byval wParam as WPARAM, byval lParam as LPARAM ) as LRESULT
    return DefWindowProc( hwnd, uMsg, wParam, lParam )
end function
LnxInit()
dim pWindow as CWindow ptr = new CWindow
dim as HWND hDlg = pWindow->Create( NULL, "Test dialog", @Dlg_WndProc, 0, 0, 0, 0, WS_POPUP or WS_CAPTION or WS_SYSMENU, WS_EX_DLGMODALFRAME )
pWindow->SetClientSize( 625, 460 )
pWindow->AddControl( "LABEL", , 1002, "Header label", 216, 10, 400, 30, WS_CHILD or WS_VISIBLE or SS_LEFT or SS_CENTERIMAGE or SS_NOTIFY or SS_SUNKEN )
dim as HWND hTv = pWindow->AddControl( "TREEVIEW", , 1004, "", 10, 10, 200, 385, WS_CHILD or WS_VISIBLE or WS_TABSTOP or TVS_HASBUTTONS, WS_EX_CLIENTEDGE )
TreeView_AppendItem( hTv, TVI_ROOT, "General Options", 0 )
TreeView_AppendItem( hTv, TVI_ROOT, "Code Editor", 1 )
if command(1) = "child" then
    for k as long = 1 to 3
        dim pc as CWindow ptr = new CWindow
        dim as HWND hc = pc->Create( hDlg, "", @Child_WndProc, 0, 0, 0, 0, WS_CHILD )
        if command(4) = "nobrush" then pc->Brush = 0
        if command(4) = "noraise" then hc->hbrBackground = 0
        pc->AddControl( "CHECKBOX", , 100 + k, "Check " & k, 0, 0, 300, 20, WS_CHILD or WS_VISIBLE or WS_TABSTOP )
        if k = 1 andalso command(3) <> "noshow" then SetWindowPos( hc, HWND_TOP, 221, 50, 420, 350, SWP_SHOWWINDOW )
        if command(3) = "one" then exit for
    next
end if
pWindow->AddControl( "BUTTON", , 1, "OK", 450, 417, 74, 28, WS_CHILD or WS_VISIBLE or WS_TABSTOP or BS_DEFPUSHBUTTON )
ShowWindow( hDlg, SW_SHOW )
dim as MSG m
for k as long = 1 to 60 : PeekMessage( @m, 0, 0, 0, PM_NOREMOVE ) : sleep 10 : next
dim as GdkWindow ptr gw = gtk_widget_get_window( hDlg->widget )
dim as GdkPixbuf ptr pb = gdk_pixbuf_get_from_window( gw, 0, 0, gdk_window_get_width(gw), gdk_window_get_height(gw) )
gdk_pixbuf_save( pb, command(2), "png", NULL, NULL )
