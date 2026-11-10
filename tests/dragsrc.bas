' Test helper: a window that drags the file given on the command line (text/uri-list)
#include once "gtk/gtk3.bi"
dim shared as string gUri
sub OnGet cdecl ( byval w as GtkWidget ptr, byval ctx as GdkDragContext ptr, byval sel as GtkSelectionData ptr, byval info as guint, byval t as guint, byval d as gpointer )
    dim as zstring ptr uris(1) = { strptr(gUri), 0 }
    gtk_selection_data_set_uris( sel, @uris(0) )
end sub
gtk_init( 0, 0 )
gUri = "file://" & command(1)
dim as GtkWidget ptr win = gtk_window_new( GTK_WINDOW_TOPLEVEL )
gtk_window_set_title( GTK_WINDOW(win), "DRAGSRC" )
dim as GtkWidget ptr btn = gtk_button_new_with_label( "drag me" )
gtk_container_add( GTK_CONTAINER(win), btn )
gtk_window_set_default_size( GTK_WINDOW(win), 160, 80 )
gtk_window_move( GTK_WINDOW(win), 1700, 900 )
dim as GtkTargetEntry te
te.target = @"text/uri-list"
gtk_drag_source_set( btn, GDK_BUTTON1_MASK, @te, 1, GDK_ACTION_COPY )
g_signal_connect( G_OBJECT(btn), "drag-data-get", G_CALLBACK(@OnGet), NULL )
gtk_widget_show_all( win )
gtk_main()
