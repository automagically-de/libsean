#include <stdlib.h>

#include <gtk/gtk.h>

#include <sean.h>

gboolean gui_init(SeanContext *context);

int main(int argc, char *argv[])
{
	SeanContext *context;
	int retval = EXIT_FAILURE;

	context = sean_init(SEAN_USE_GSTREAMER, &argc, &argv);

	gtk_init(&argc, &argv);

	if(gui_init(context))
	{
		sean_video_play(context);
		gtk_main();
	}

	sean_close(context);

	return retval;
}

void gui_preview_cb(GdkPixbuf *pixbuf, gpointer user_data)
{
	GtkWidget *da = GTK_WIDGET(user_data);

	gdk_draw_pixbuf(da->window, da->style->fg_gc[GTK_WIDGET_STATE(da)],
		pixbuf, 0, 0, 0, 0,
		gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
		GDK_RGB_DITHER_NORMAL, 0, 0);
}

gboolean gui_init(SeanContext *context)
{
	GtkWidget *mainwin, *vbox, *widget;

	/* main window */
	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(mainwin), "destroy", G_CALLBACK(gtk_main_quit),
		NULL);

	/* main vertical box */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox);

	/* preview drawing area */
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 320, 240);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	sean_register_preview_callback(context, gui_preview_cb, widget);

	/* show window */
	gtk_widget_show_all(mainwin);

	return TRUE;
}
