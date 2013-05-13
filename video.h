#ifndef _VIDEO_H
#define _VIDEO_H

#include <gtk/gtk.h>
#include <glib.h>

#ifdef GDK_WINDOWING_X11
	#include <gdk/gdkx.h>
	#include <gdk/gdkkeysyms.h>
	#include <gst/interfaces/xoverlay.h>
	#include <gst/video/video.h>
#endif

// Code specific to environments using the X Window System.
#ifdef GDK_WINDOWING_X11
GstBusSyncReply bus_sync (GstBus *bus,
				 GstMessage *message,
				 gpointer data);
void video_realized (GtkWidget *widget, gpointer data);
gboolean catch_video_button (GtkWidget *widget,
			     GdkEventButton *event,
			     gpointer data);
gboolean catch_window_key (GtkWidget *window,
			      GdkEventKey *event,
			      gpointer data);
gboolean window_fullscreen (GtkWidget *widget,
			    GdkEventWindowState *event,
			    gpointer data);
gboolean window_motion (GtkWidget *widget,
			    GdkEventMotion *event,
			    gpointer data);
#endif

#endif
