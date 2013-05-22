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

#include <string.h>
#include "player.h"
#include "playlist.h"
#include "variables.h"

GtkTreeIter selected;
gboolean loop, shuffle;
gint num_tracks;

// Callback function when the back button is clicked.
void back_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkListStore *pl_entries;
	GtkWidget *playlist, *play;
	GstState state;
	GstElement *bin;
	GtkTreeModel *model;
	GtkTreeIter temp;
	GtkTreePath *cursor;

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	gst_element_get_state (bin, &state, NULL, GST_SECOND);

	play = g_object_get_data (G_OBJECT (widget), "play");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (playlist));

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (playlist), &cursor, NULL);

	// Test for an empty list or if nothing is selected.
	if (cursor == NULL ||
	    gtk_tree_model_get_iter_first (model, &temp) == FALSE)
		return;

	// If the track is already playing, go back and play the track before,
	// otherwise just change the cursor.
	if (g_strcmp0
	   (gst_element_state_get_name (state), "NULL") == 0 ||
	    g_strcmp0
	   (gst_element_state_get_name (state), "READY") == 0)
	{
		GtkTreePath *current;
		
		current = gtk_tree_model_get_path (model, &selected);

		if (gtk_tree_path_prev (current))
			gtk_tree_view_set_cursor
					(GTK_TREE_VIEW (playlist),
					 current,
					 NULL,
					 FALSE);
	}
	else
	{
		if (gtk_tree_path_prev (playing))
			gtk_tree_view_set_cursor
					(GTK_TREE_VIEW (playlist),
					 playing,
					 NULL,
					 FALSE);

		gtk_button_clicked (GTK_BUTTON (play));
	}
}

// Callback function when the play button is clicked.
void play_button_clicked (GtkWidget *widget, gpointer data)
{
	GstElement *bin;
	GstState state;
	GtkWidget *playlist, *track_label;
	GtkListStore *pl_entries;
	GtkTreePath *path;
	GdkPixbuf *playing_icon;
	GtkTreeIter iter, validity_test;
	GtkTreeModel *model;

	track_label = g_object_get_data (G_OBJECT (widget), "track_label");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");
	playing_icon = g_object_get_data (G_OBJECT (widget), "playing_icon");
	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin; 

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (playlist));
	gtk_tree_view_get_cursor (GTK_TREE_VIEW (playlist), &path, NULL);

	gst_element_get_state (bin, &state, NULL, GST_SECOND);

	if (g_strcmp0
	   (gst_element_state_get_name (state), "PAUSED") == 0)
	{
		gst_element_set_state (bin, GST_STATE_PLAYING);
		return;
	}

	if (gtk_tree_model_get_iter_first (model, &validity_test) == FALSE)
		return;

	// If nothing is selected, play the first track.
	if (path == NULL)
	{
		if (gtk_tree_model_get_iter_first (model, &iter))
		{
			path = gtk_tree_model_get_path (model, &iter);
			selected = iter;
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
						  path,
						  NULL,
						  FALSE);
		}
	}

	// Play and set pixbuf if something is selected.
	if (path != NULL)
	{
		gchar *label;

		clear_pixbuf (pl_entries, GTK_TREE_VIEW (playlist));

		gtk_list_store_set (pl_entries,
				    &selected,
				    COL_PLAYING, playing_icon,
				    -1);
		gtk_tree_model_get (model, &selected,
				    COL_TRACK_NAME, &label, -1);
		gtk_label_set_text (GTK_LABEL (track_label), label);

		gst_element_set_state (bin, GST_STATE_NULL);
		gst_element_set_state (bin, GST_STATE_PLAYING);

		gtk_tree_model_get_iter (model, &selected, path);
		playing = path;

		g_free (label);
	}

	gst_element_get_state (bin, &state, NULL, GST_SECOND * 2);

	// For X Windows, sets the size of the window when video is played.
	#ifdef GDK_WINDOWING_X11
		GtkWidget *window;
		GList *stream_list;	// Stream info list.
		GObject *stream;	// Playing stream.
		GstObject *pad;		// The video pad.
		gint playing_video;	// Whether a video is playing.
		gint width = 0, height = 0;	// Width and height of video.

		window = g_object_get_data (G_OBJECT (widget), "window");

		g_object_get (G_OBJECT (bin), "stream-info",
			      &stream_list, NULL);
		stream = g_list_nth_data (stream_list, 0);
		g_object_get (G_OBJECT (stream), "object", &pad, NULL);

		g_object_get (G_OBJECT (bin), "current-video",
			      &playing_video, NULL);

		if (playing_video > -1 &&
		    gst_video_get_size (GST_PAD (pad), &width, &height))
			gtk_window_resize (GTK_WINDOW (data),
					   width,
					   height + WIN_HEIGHT);
		else if (playing_video == -1)
			gtk_window_resize (GTK_WINDOW (data),
					   WIN_WIDTH,
					   WIN_HEIGHT);
	#endif
}

// Callback function when the pause button is clicked.
void pause_button_clicked (GtkWidget *widget, gpointer data)
{
	GstElement *bin;
	GstState state;
	GtkListStore *pl_entries;
	GdkPixbuf *playing_icon, *paused_icon;

	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");
	playing_icon = g_object_get_data (G_OBJECT (widget), "playing_icon");
	paused_icon = g_object_get_data (G_OBJECT (widget), "paused_icon");

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	gst_element_get_state (bin, &state, NULL, GST_SECOND);

	// If already playing, pause, if paused, start playing.
	if (g_strcmp0
	   (gst_element_state_get_name (state), "PAUSED") == 0)
	{
		gst_element_set_state (bin, GST_STATE_PLAYING);
	}
	else if (g_strcmp0
		(gst_element_state_get_name (state), "PLAYING") == 0)
	{
		gst_element_set_state (bin, GST_STATE_PAUSED);
	}
}

// Callback function for the stop button.
void stop_button_clicked (GtkWidget *widget, gpointer data)
{
	GstElement *bin;
	GstState state;
	GtkWidget *playlist, *seek_bar, *seek_label, *track_label;
	GtkListStore *pl_entries;

	track_label = g_object_get_data (G_OBJECT (widget), "track_label");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");
	seek_bar = g_object_get_data (G_OBJECT (widget), "seek_bar");
	seek_label = g_object_get_data (G_OBJECT (widget), "seek_label");

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	gst_element_set_state (bin, GST_STATE_NULL);

	gst_element_get_state (bin, &state, NULL, GST_SECOND);

	// Remove pixbufs from first column and set seek bar and label to 0.
	if (g_strcmp0
	    (gst_element_state_get_name (state), "NULL") == 0)
	{
		clear_pixbuf (pl_entries, GTK_TREE_VIEW (playlist));

		gtk_range_set_value (GTK_RANGE (seek_bar), 0);
		gtk_label_set_text (GTK_LABEL (seek_label), "--:--:-- / --:--:--");
		gtk_label_set_text (GTK_LABEL (track_label), "No Track Selected");
		playing = NULL;

		#ifdef GDK_WINDOWING_X11
		GtkWidget *window;

		window = g_object_get_data (G_OBJECT (widget), "window");
		gtk_window_unfullscreen (GTK_WINDOW (window));
		gtk_window_unmaximize (GTK_WINDOW (window));
		gtk_window_resize (GTK_WINDOW (window),
				   WIN_WIDTH,
				   WIN_HEIGHT);
		#endif
	}
}

// Callback function for the next button.
void next_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkListStore *pl_entries;
	GtkWidget *playlist, *play;
	GstState state;
	GstElement *bin;
	GtkTreeModel *model;
	GtkTreeIter temp, validity_test;
	GtkTreePath *cursor;
	GtkTreeSelection *selection;

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	gst_element_get_state (bin, &state, NULL, GST_SECOND);

	play = g_object_get_data (G_OBJECT (widget), "play");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (playlist));

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (playlist), &cursor, NULL);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (playlist));

	// Test for an empty list or if nothing is selected.
	if (cursor == NULL ||
	    gtk_tree_model_get_iter_first (model, &temp) == FALSE)
		return;

	// Play the next song if something is already playing, otherwise just
	// select the next song.
	if (g_strcmp0
	    (gst_element_state_get_name (state), "NULL") == 0 ||
	    g_strcmp0
	    (gst_element_state_get_name (state), "READY") == 0)
	{
		GtkTreePath *current;

		// Select something at random if shuffle is true;
		if (shuffle == TRUE)
		{
			if (!gtk_tree_model_get_iter_first (model,
							    &validity_test))
				return;

			if (gtk_tree_model_iter_nth_child (model,
							   &validity_test,
							   NULL,
							   rand () % num_tracks))
				current = gtk_tree_model_get_path (model,
								   &validity_test);
				
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
				 		  current,
						  NULL,
						  FALSE);
			return;
		}
	
		current = gtk_tree_model_get_path (model, &selected);

		gtk_tree_model_get_iter (model, &validity_test, cursor);
		// If next iterator is not valid, go to beginning if loop is
		// TRUE. Otherwise, return.
		if (gtk_tree_model_iter_next (model, &validity_test) == FALSE)
		{ // start if 1
			if (!gtk_list_store_iter_is_valid (pl_entries,
							   &validity_test) &&
			    loop == TRUE)
			{ // start if 2
				if (!gtk_tree_model_get_iter_first (model,
								    &validity_test))
					return;

				current = gtk_tree_model_get_path (model,
								   &validity_test);					
			} // end if 2
			else
				return;
		} // end if 1
		else
			gtk_tree_path_next (current);

		gtk_tree_view_set_cursor
				(GTK_TREE_VIEW (playlist),
				 current,
				 NULL,
				 FALSE);
	}
	else
	{
		// Play something at random if shuffle is true;
		if (shuffle == TRUE)
		{
			GtkTreePath *current;

			if (!gtk_tree_model_get_iter_first (model,
							    &validity_test))
				return;

			if (gtk_tree_model_iter_nth_child (model,
							   &validity_test,
							   NULL,
							   rand () % num_tracks))
				current = gtk_tree_model_get_path (model,
								   &validity_test);
				
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
				 		  current,
						  NULL,
						  FALSE);

			gtk_tree_view_row_activated (GTK_TREE_VIEW (playlist),
						     current,
						     NULL);
			return;
		}

		// If there is a next song, play it.
		gtk_tree_model_get_iter (model, &temp, playing);

		if (gtk_list_store_iter_is_valid (pl_entries, &temp) &&
		    gtk_tree_model_iter_next (model, &temp))
		{
			gtk_tree_path_next (playing);
			gtk_tree_view_set_cursor
				(GTK_TREE_VIEW (playlist),
				 playing,
				 NULL,
				 FALSE);

			gtk_tree_view_row_activated
				(GTK_TREE_VIEW (playlist),
			 	 playing,
				 NULL);
		} // end if

		// If there is no next song but loop is on, loop around.
		else if (loop == TRUE &&
			 !gtk_list_store_iter_is_valid (pl_entries, &temp))
		{
			if (gtk_tree_model_get_iter_first (model,
							   &temp))
			{
				//gtk_tree_selection_select_iter (selection,
				//				&temp);

				playing = gtk_tree_model_get_path (model,
								   &temp);

				gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
				 			  playing,
							  NULL,
							  FALSE);

				gtk_tree_view_row_activated
					(GTK_TREE_VIEW (playlist),
					 playing,
					 NULL);
			}
		}
	} // end else
}

// Callback function when the open/add button is clicked.
void add_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *window;

	window = g_object_get_data (G_OBJECT (widget), "window");

	dialog = gtk_file_chooser_dialog_new ("Open File",
					      GTK_WINDOW (window),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN,
					      GTK_RESPONSE_ACCEPT,
					      NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		tag_info track_info;
		GtkListStore *pl_entries;
		gchar *uri, *filename;

		pl_entries = g_object_get_data (G_OBJECT (widget),
						"pl_entries");

		// Get the URI of the file.
		uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));

		// Get the file path.
		filename = gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER (dialog));

		#ifdef WIN32
		filename = g_locale_from_utf8 (filename, -1, NULL, NULL, NULL);
		#endif

		#ifdef unix
		// Parse playlist if a playlist file is added.
		if (parse_pl (uri, pl_entries) ==
					TOTEM_PL_PARSER_RESULT_SUCCESS)
		{
			g_free (uri);
			g_free (filename);
			gtk_widget_destroy (dialog);
			return;
		}
		#endif

		// Parse file of its tag info.
		if (parse_tag (filename, &track_info))
			add_data_at_end (pl_entries,
					 track_info,
					 uri,
					 filename);
		else
		{
			// Using this causes Windows to crash, so it will set
			// it to the full path on Windows machines.
			track_info.title = strrchr (filename, '/') + 1;

			#ifdef WIN32
			track_info.title = filename;
			#endif

			track_info.artist = "";
			g_sprintf (track_info.length, "??:??");
			add_data_at_end (pl_entries,
					 track_info,
					 uri,
					 filename);
		}
		
		g_free (uri);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}

// Callback for remove button to remove currently selected item from list.
void remove_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *playlist;
	GtkListStore *pl_entries;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (playlist));
	// Remove the selected element.
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_list_store_remove (pl_entries, &iter);
	}

}

// Callback function when the open playlist button is clicked.
void open_pl_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *pl_window;

	pl_window = g_object_get_data (G_OBJECT (widget), "pl_window");

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		gtk_widget_show_all (pl_window);
	else
		gtk_widget_hide_all (pl_window);
}

void loop_button_clicked (GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		loop = TRUE;
	else
		loop = FALSE;

	printf ("Loop: %i\n", loop);
}

void shuffle_button_clicked (GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		shuffle = TRUE;
	else
		shuffle = FALSE;

	printf ("Shuffle: %i\n", shuffle);
}

// Hide the playlist when destroyed rather than delete it.
gboolean pl_delete (GtkWidget *widget, gpointer data)
{
	GtkWidget *open_pl;
	
	open_pl = g_object_get_data (G_OBJECT (widget), "open_pl");

	gtk_widget_hide_all (widget);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (open_pl), FALSE);

	return TRUE;
}

// Callback function for when the pipeline gets a message.
gboolean bus_message (GstBus *bus, GstMessage *message, gpointer data)
{
	GtkWidget *window, *playlist, *stop;
	GtkListStore *pl_entries;
	GtkTreeSelection *selection;

	stop = g_object_get_data (G_OBJECT (bus), "stop");
	window = g_object_get_data (G_OBJECT (bus), "window");
	playlist = g_object_get_data (G_OBJECT (bus), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (bus), "pl_entries");

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (playlist));

	switch (GST_MESSAGE_TYPE (message))
	{
		// If an error has occurred, output the error on the console
		// and quit.
		case GST_MESSAGE_ERROR:
		{
			GError *err;

			gst_message_parse_error (message, &err, NULL);
			error_dialog (window, err -> message);
			g_error_free (err);
			break;

		}

		// Otherwise, in case of end of stream, go to the next song.
		case GST_MESSAGE_EOS:
		{
			GtkTreeIter temp, validity_test;
			GtkTreeModel *model;

			model = gtk_tree_view_get_model
					(GTK_TREE_VIEW (playlist));
			gtk_tree_model_get_iter (model, &temp, playing);

			// Play something at random if shuffle is true;
			if (shuffle == TRUE)
			{
				GtkTreePath *current;

				if (!gtk_tree_model_get_iter_first (model,
								    &validity_test))
					return TRUE;

				if (gtk_tree_model_iter_nth_child (model,
								   &validity_test,
								   NULL,
								   rand () % num_tracks))
					current = gtk_tree_model_get_path (model,
									   &validity_test);
				
				gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
					 		  current,
							  NULL,
							  FALSE);

				gtk_tree_view_row_activated (GTK_TREE_VIEW (playlist),
							     current,
							     NULL);
				playing = current;

				return TRUE;
			}

			// If there is a next song, play it.
			if (gtk_list_store_iter_is_valid (pl_entries, &temp) &&
			    gtk_tree_model_iter_next (model, &temp))
			{
				gtk_tree_path_next (playing);
				gtk_tree_view_set_cursor
					(GTK_TREE_VIEW (playlist),
					 playing,
					 NULL,
					 FALSE);

				gtk_tree_view_row_activated
					(GTK_TREE_VIEW (playlist),
				 	 playing,
					 NULL);
			}
			// If there is no next song but loop is on, loop around.
			else if (loop == TRUE &&
				 !gtk_list_store_iter_is_valid (pl_entries, &temp))
			{
				if (gtk_tree_model_get_iter_first (model,
							   &temp))
				{
					//gtk_tree_selection_select_iter (selection,
					//				&temp);

					playing = gtk_tree_model_get_path (model,
									   &temp);

					gtk_tree_view_set_cursor (GTK_TREE_VIEW (playlist),
					 			  playing,
								  NULL,
								  FALSE);

					gtk_tree_view_row_activated
						(GTK_TREE_VIEW (playlist),
						 playing,
						 NULL);
				}
				else
					gtk_button_clicked (GTK_BUTTON (stop)); 
			}
			else
				gtk_button_clicked (GTK_BUTTON (stop));

			break;
		}

		case GST_MESSAGE_TAG:
		{
			//GstTagList *tag, *temp;

			//gst_message_parse_tag (message, &temp);
			//printf ("A TAG WAS FOUND.\n");
			break;
		}

		default:
			break;
	}

	return TRUE;
}

// Function called on an error.
void error_dialog (GtkWidget *parent, gchar *error_text)
{
	GtkWidget *error, *label, *image, *hBox;

	// Create a new dialog box with one button.
	error = gtk_dialog_new_with_buttons ("Error",
					     GTK_WINDOW (parent),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	// Remove the separator above the buttons.
	gtk_dialog_set_has_separator (GTK_DIALOG (error), FALSE);

	// Create the label, image, and the horizontal box and set properties.
	label = gtk_label_new (error_text);

	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR,
					  GTK_ICON_SIZE_DIALOG);
	hBox = gtk_hbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (hBox), 15);

	// Pack the two widgets into the horizontal box.
	gtk_box_pack_start_defaults (GTK_BOX (hBox), image);
	gtk_box_pack_start_defaults (GTK_BOX (hBox), label);
	
	// Pack the horizontal box into the dialog box.
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (error)->vbox), hBox);
	
	// Show the widgets, and exit on clicking the OK button.
	gtk_widget_show_all (error);

	gtk_dialog_run (GTK_DIALOG (error));
	gtk_widget_destroy (error);
}

// This callback function gets called every set amount of time to move the
// trackbar according to the track position.
gboolean track_bar_callback (GstElement *pipeline)
{
	GtkWidget *seek_bar, *seek_label;

	seek_bar = g_object_get_data (G_OBJECT (pipeline), "seek_bar");
	seek_label = g_object_get_data (G_OBJECT (pipeline), "seek_label");

	GstFormat time = GST_FORMAT_TIME;
	gint64 cur_time, total_time;
	gdouble cur_time_float, total_time_float;

	// Query the position and set the seek bar to the percentage of the
	// track played.
	if (gst_element_query_position (pipeline, &time, &cur_time) &&
	    gst_element_query_duration (pipeline, &time, &total_time))
	{
		gchar *label_text;
	
		// This assumes time will never go over 32 characters.
		label_text = calloc (64, sizeof (gchar));

		g_sprintf (label_text,
			   "%02lld:%02lld:%02lld / %02lld:%02lld:%02lld",
			   (cur_time / 1000000000) / 3600,
			   ((cur_time / 1000000000) / 60) % 60,
			   (cur_time / 1000000000) % 60,
			   (total_time / 1000000000) / 3600,
			   ((total_time / 1000000000) / 60) % 60,
			   (total_time / 1000000000) % 60);


		cur_time_float = cur_time;
		total_time_float = total_time;

		// Set the seek bar and track time display to the current
		// position.
		g_signal_handlers_block_by_func (seek_bar, seek_changed, NULL);
		gtk_range_set_value (GTK_RANGE (seek_bar),
				     (cur_time_float / total_time_float) * 100);
		g_signal_handlers_unblock_by_func (seek_bar,
						   seek_changed,
						   NULL);
		gtk_label_set_text (GTK_LABEL (seek_label), label_text);
	}

  return TRUE;
}

// Changes the volume when the slider values have been changed for volume.
void volume_changed (GtkWidget *widget, gpointer data)
{
	GstElement *bin;
	gdouble new_volume;

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	new_volume = gtk_range_get_value (GTK_RANGE (widget));
	g_object_set (G_OBJECT (bin), "volume", new_volume, NULL);	
}

// Changes the location of the track when the sliders have been dragged for the
// seek bar.
void seek_changed (GtkWidget *widget, gpointer data)
{
	GtkWidget *window;
	GstElement *bin;
	gdouble percent, total_time_float;

	GstFormat time = GST_FORMAT_TIME;
	gint64 total_time;

	window = g_object_get_data (G_OBJECT (widget), "window");

	bin = g_object_get_data (G_OBJECT (widget), "bin");
	bin = (GstElement *) bin;

	percent = gtk_range_get_value (GTK_RANGE (widget));

	// Since we have the percentage, calculate the actual time that the
	// track is seeking to.
	if (gst_element_query_duration (bin, &time, &total_time))
	{
		total_time_float = total_time;

		percent = percent / 100;
		total_time_float = total_time_float * percent;
	}

	if (!gst_element_seek_simple (bin,
				      GST_FORMAT_TIME,
				      GST_SEEK_FLAG_FLUSH,
				      total_time_float))
		error_dialog (window, "Failed to Seek!");

	//printf ("Seek to: %f%s",
	//	  total_time_float / 1000000000, " seconds.\n");
}

// Called when the program is closed to ensure video playback is stopped so an
// X Window error isn't thrown.
void quit_program (GtkWidget *widget, gpointer data)
{
	GtkWidget *stop;

	stop = g_object_get_data (G_OBJECT (widget), "stop");

	gtk_button_clicked (GTK_BUTTON (stop));
	gtk_main_quit ();
}
