#include "video.h"
#include "variables.h"

gint window_state;

// Code specific to environments using the X Window System.
#ifdef GDK_WINDOWING_X11
// Most of this code from the next two functions have been adapted from the
// gstreamer documentation.

// Tells the application to render video to an existing window.
GstBusSyncReply bus_sync (GstBus *bus,
				 GstMessage *message,
				 gpointer data)
{
	// Watch the bus for "prepare-xwindow-id" messages.
	if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
		return GST_BUS_PASS;
	if (!gst_structure_has_name (message -> structure,
				     "prepare-xwindow-id"))
		return GST_BUS_PASS;

	// Assign the xvindow id to the video sink.
	if (video_window_xid != 0)
	{
		//GstXOverlay *xoverlay;
		GstElement *video_sink;
		video_sink = g_object_get_data (G_OBJECT (bus), "video_sink");
		// Grab the video sink and set the XID of the window.
		//xoverlay = GST_X_OVERLAY (GST_MESSAGE_SRC (message));
		//gst_x_overlay_set_xwindow_id (xoverlay, video_window_xid);
		gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (video_sink),
					      video_window_xid);
	}
	else
		g_warning ("Still haven't obtained window XID...");
	
	gst_message_unref (message);
	return GST_BUS_DROP;	
}

// Called to set resources ahead of time for the video window so we can get
// the XID of the video window.
void video_realized (GtkWidget *widget, gpointer data)
{
	#if GTK_CHECK_VERSION (2, 18, 0)
		if (!gdk_window_ensure_native (widget -> window))
			g_error ("Could not create native window...");
	#endif

	video_window_xid = GDK_WINDOW_XID (widget -> window);
        //printf("video_window_xid = %i\n", video_window_xid); 
}

gboolean catch_video_button (GtkWidget *widget,
			     GdkEventButton *event,
			     gpointer data)
{
	if (event -> type == GDK_2BUTTON_PRESS)
	{
		GtkWidget *window,
			  *hbox_seek,
			  *hbox_buttons,
			  *track_label,
			  *open_pl;

		window = g_object_get_data (G_OBJECT (widget), "window");
		hbox_seek = g_object_get_data (G_OBJECT (widget), "hbox_seek");
		hbox_buttons = g_object_get_data (G_OBJECT (widget),
						  "hbox_buttons");
		track_label = g_object_get_data (G_OBJECT (widget),
						 "track_label");
		open_pl = g_object_get_data (G_OBJECT (window), "open_pl");

		// If the window is in it's normal state, set it to FULLSCREEN.
		if (window_state == 4 || window_state == 0)
		{
			gtk_window_fullscreen (GTK_WINDOW (window));

			gtk_widget_hide (hbox_seek);
			gtk_widget_hide (hbox_buttons);
			gtk_widget_hide (track_label);

			gtk_widget_set_sensitive (open_pl, FALSE);
		}
		// If it's FULLSCREEN and MAXIMIZED or just FULLSCREEN'ed,
		// revert it back.
		else if (window_state == 16 || window_state == 20)
		{
			gtk_window_unfullscreen (GTK_WINDOW (window));

			gtk_widget_show (hbox_seek);
			gtk_widget_show (hbox_buttons);
			gtk_widget_show (track_label);

			gtk_widget_set_sensitive (open_pl, TRUE);
		}

		return TRUE;
	}

	return FALSE;
}

// Catches Alt+Enter.
gboolean catch_window_key (GtkWidget *window,
			   GdkEventKey *event,
			   gpointer data)
{
	guint modifier;

	modifier = gtk_accelerator_get_default_mod_mask ();

	if (event -> keyval == GDK_Return
	    && (event -> state & modifier) == GDK_MOD1_MASK)
	{
		GtkWidget *hbox_seek, *hbox_buttons, *track_label, *open_pl;

		hbox_seek = g_object_get_data (G_OBJECT (window), "hbox_seek");
		hbox_buttons = g_object_get_data (G_OBJECT (window),
						  "hbox_buttons");
		track_label = g_object_get_data (G_OBJECT (window),
						 "track_label");
		open_pl = g_object_get_data (G_OBJECT (window), "open_pl");

		// If the window is in it's normal state, set it to FULLSCREEN.
		if (window_state == 4 || window_state == 0)
		{
			gtk_window_fullscreen (GTK_WINDOW (window));

			gtk_widget_hide (hbox_seek);
			gtk_widget_hide (hbox_buttons);
			gtk_widget_hide (track_label);

			gtk_widget_set_sensitive (open_pl, FALSE);
		}
		// If it's FULLSCREEN and MAXIMIZED or just FULLSCREEN'ed,
		// revert it back.
		else if (window_state == 16 || window_state == 20)
		{
			gtk_window_unfullscreen (GTK_WINDOW (window));

			gtk_widget_show (hbox_seek);
			gtk_widget_show (hbox_buttons);
			gtk_widget_show (track_label);

			gtk_widget_set_sensitive (open_pl, TRUE);
		}

		return TRUE;
	}

	else if (event -> keyval == GDK_space)
	{
		GtkWidget *pause;
		pause = g_object_get_data (G_OBJECT (window), "pause");

		gtk_button_clicked (GTK_BUTTON (pause));

		return TRUE;
	}

	return FALSE;
}

// Gets the current window state.
gboolean window_fullscreen (GtkWidget *widget,
			    GdkEventWindowState *event,
			    gpointer data)
{
	window_state = event -> new_window_state;

	return FALSE;
}

// Shows the widgets on the bottom of the screen if user's cursor is on the
// bottom of the screen. Hides them otherwise.
gboolean window_motion (GtkWidget *widget,
			    GdkEventMotion *event,
			    gpointer data)
{
/*
	GdkScreen *screen;
	GtkWidget *hbox_seek, *hbox_buttons, *track_label;
	gint x_resolution, y_resolution;

	screen = gdk_screen_get_default ();
	hbox_seek = g_object_get_data (G_OBJECT (widget), "hbox_seek");
	hbox_buttons = g_object_get_data (G_OBJECT (widget),
					  "hbox_buttons");
	track_label = g_object_get_data (G_OBJECT (widget),
					 "track_label");

	if (screen == NULL)
		return FALSE;

	x_resolution = gdk_screen_get_width (screen);
	y_resolution = gdk_screen_get_height (screen);

	// Show widgets if mouse is on the bottom of the screen during full-
	// screen mode.
	if (window_state == 16 || window_state == 20)
	{
		if (event -> y < y_resolution &&
		    event -> y > y_resolution - 150)
		{
			gtk_widget_show (hbox_seek);
			gtk_widget_show (hbox_buttons);
			gtk_widget_show (track_label);

			return FALSE;
		}

		else if (event -> y < y_resolution - 150
			 && event -> y >= 0)
		{
			gtk_widget_hide (hbox_seek);
			gtk_widget_hide (hbox_buttons);
			gtk_widget_hide (track_label);

			return FALSE;
		}
	}
*/
	return FALSE;
}
#endif
