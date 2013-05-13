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
