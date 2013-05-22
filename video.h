/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* LICENSE:                                                                    *
* Copyright (C) 2010, 2013 Hong Jie Tan                                       *
*                                                                             *
* This file is part of GTKMB.                                                 *
*                                                                             *
*   GTKMB is free software: you can redistribute it and/or modify             *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation, either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   GTKMB is distributed in the hope that it will be useful,                  *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with GTKMB.  If not, see <http://www.gnu.org/licenses/>.            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
