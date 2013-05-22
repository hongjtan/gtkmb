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

#ifndef _PLAYER_H
#define _PLAYER_H

#include <gtk/gtk.h>
#include <gst/gst.h>

// Function declarations.
void back_button_clicked (GtkWidget *widget, gpointer data);
void play_button_clicked (GtkWidget *widget, gpointer data);
void pause_button_clicked (GtkWidget *widget, gpointer data);
void stop_button_clicked (GtkWidget *widget, gpointer data);
void next_button_clicked (GtkWidget *widget, gpointer data);
void add_button_clicked (GtkWidget *widget, gpointer data);
void remove_button_clicked (GtkWidget *widget, gpointer data);
void open_pl_button_clicked (GtkWidget *widget, gpointer data);
void help_button_clicked (GtkWidget *widget, gpointer data);
void loop_button_clicked (GtkWidget *widget, gpointer data);
void shuffle_button_clicked (GtkWidget *widget, gpointer data);
gboolean pl_delete (GtkWidget *widget, gpointer data);
gboolean bus_message (GstBus *bus, GstMessage *message, gpointer data);
void error_dialog (GtkWidget *parent, gchar *error_text);
gboolean track_bar_callback (GstElement *pipeline);
void volume_changed (GtkWidget *widget, gpointer data);
void seek_changed (GtkWidget *widget, gpointer data);
void quit_program (GtkWidget *widget, gpointer data);

#endif
