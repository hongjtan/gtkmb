#include <string.h>
#include <glib/gprintf.h>
#include "playlist.h"
#include "player.h"
#include "variables.h"

GtkTreeIter editing;
gint num_tracks;

// Removes the playlist entries.
void new_pl_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkListStore *pl_entries;

	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");

	gtk_list_store_clear (pl_entries);
}

#ifdef unix
// Parses a playlist, returns FALSE if not a playlist.
TotemPlParserResult parse_pl (gchar *uri, GtkListStore *pl_entries)
{
	TotemPlParser *pl_parser;
	TotemPlParserResult result;

	pl_parser = totem_pl_parser_new ();

	g_object_set_data (G_OBJECT (pl_parser), "pl_entries", pl_entries);
	g_signal_connect (pl_parser, "entry-parsed",
			  G_CALLBACK (parsed_entry), NULL);

	result = totem_pl_parser_parse (pl_parser, uri, FALSE);
	return result;
}

// Parses an entry and adds it to the playlist.
void parsed_entry (TotemPlParser *parser,
			  gchar *uri,
			  GHashTable *metadata,
			  gchar data)
{
	gchar *filename, *f_to_uri;
	tag_info track_info;
	GtkListStore *pl_entries;

	pl_entries = g_object_get_data (G_OBJECT (parser), "pl_entries");

	// Get the filename of the track file.
	filename =  gnome_vfs_get_local_path_from_uri (uri);
	
	// The first if statement is mainly for M3U's, since they don't use
	// URI's. Otherwise, just check if it's an invalid entry.
	if (filename == NULL)
	{
		f_to_uri = gnome_vfs_get_uri_from_local_path (uri);
		filename = gnome_vfs_get_local_path_from_uri (f_to_uri);

		if (filename == NULL)
			return;
	}

	// Parse file of its tag info.
	if (parse_tag (filename, &track_info))
	{
		add_data_at_end (pl_entries,
				 track_info,
				 uri,
				 filename);
	}
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

	g_free (filename);
}

// Loads a playlist and clears the old playlist.
void load_pl_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *pl_window;
	GtkFileFilter *format_playlist;

	pl_window = g_object_get_data (G_OBJECT (widget), "pl_window");

	// Set file filter specifications.
	format_playlist = gtk_file_filter_new ();
	gtk_file_filter_set_name (format_playlist,
				  "Playlist Files (*.m3u;*.pls;*.xspf)");
	gtk_file_filter_add_pattern (format_playlist, "*.m3u");
	gtk_file_filter_add_pattern (format_playlist, "*.pls");
	gtk_file_filter_add_pattern (format_playlist, "*.xspf");

	dialog = gtk_file_chooser_dialog_new ("Open File",
					      GTK_WINDOW (pl_window),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), format_playlist);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  	{
	    	char *uri;
    		uri = gtk_file_chooser_get_uri
					(GTK_FILE_CHOOSER (dialog));

		GtkListStore *pl_entries;

		pl_entries = g_object_get_data
					(G_OBJECT (widget), "pl_entries");

		// Clear the old playlist and load the new one if the file is
		// a valid playlist.
		gtk_list_store_clear (pl_entries);

		if (parse_pl (uri, pl_entries) !=
					TOTEM_PL_PARSER_RESULT_SUCCESS)
			error_dialog (pl_window, "Not a valid playlist.");

		g_free (uri);
  	}

	gtk_widget_destroy (dialog);
}

// Prompts user for a filename and saves a playlist.
void save_pl_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *pl_window, *playlist;
	GtkTreeModel *model;
	GtkTreeIter playlist_iter;
	GtkFileFilter *format_playlist;

	pl_window = g_object_get_data (G_OBJECT (widget), "pl_window");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (playlist));

	// Set file filter specifications.
	format_playlist = gtk_file_filter_new ();
	gtk_file_filter_set_name (format_playlist,
				  "Playlist Files (*.m3u;*.pls;*.xspf)");
	gtk_file_filter_add_pattern (format_playlist, "*.m3u");
	gtk_file_filter_add_pattern (format_playlist, "*.pls");
	gtk_file_filter_add_pattern (format_playlist, "*.xspf");

	dialog = gtk_file_chooser_dialog_new ("Save File",
					      GTK_WINDOW (pl_window),
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);

	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), format_playlist);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
							TRUE);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
					   "Untitled.pls");

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename, *extension;
		GtkListStore *pl_entries;
		TotemPlParser *save_playlist;
		gint type;

		pl_entries = g_object_get_data
					(G_OBJECT (widget), "pl_entries");
		save_playlist = totem_pl_parser_new ();

		// Get the filename and its extension, if not a valid playlist
		// extension, use pls format.
		filename = gtk_file_chooser_get_filename
					(GTK_FILE_CHOOSER (dialog));
		extension = strrchr (filename, '.') + 1;

		if (g_strcmp0 (extension, "pls") == 0)
			type = TOTEM_PL_PARSER_PLS;
		else if (g_strcmp0 (extension, "m3u") == 0)
			type = TOTEM_PL_PARSER_M3U;
		else if (g_strcmp0 (extension, "xspf") == 0)
			type = TOTEM_PL_PARSER_XSPF;
		else if (g_strcmp0 (extension, "pla") == 0)
			type = TOTEM_PL_PARSER_IRIVER_PLA;
		else
			type = TOTEM_PL_PARSER_PLS;

		// Use this code for Totem Playlist Parser 2.28 and down.
		if (totem_pl_parser_write (save_playlist, model, save_parser,
				       	   filename, type, NULL, NULL) != TRUE)
			error_dialog (pl_window,
				      "Error occurred while saving playlist.");
		// End code for 2.28.

		// Use this code for Totem Playlist Parser 2.30+
		/*TotemPlPlaylist *list_to_save;
		TotemPlPlaylistIter pl_iter;
		GFile *file;
		gchar *uri, *title;

		list_to_save = totem_pl_playlist_new ();
		file = g_file_new_for_path (filename);

		if (gtk_tree_model_get_iter_first (model, &playlist_iter))
		{
			// Get and set the first element of the playlist.
			gtk_tree_model_get (model, &playlist_iter,
					    COL_URI, &uri,
					    COL_TRACK_NAME, &title,
					    -1);

			totem_pl_playlist_append (list_to_save, &pl_iter);
			
			totem_pl_playlist_set (list_to_save, &pl_iter,
					       TOTEM_PL_PARSER_FIELD_URI, uri,
					       TOTEM_PL_PARSER_FIELD_TITLE,
					       title, NULL);

			// Now get and set the rest of the playlist.
			while (gtk_tree_model_iter_next
						(model, &playlist_iter))
			{
				gtk_tree_model_get (model, &playlist_iter,
						    COL_URI, &uri,
						    COL_TRACK_NAME, &title,
						    -1);

				totem_pl_playlist_append (list_to_save,
							  &pl_iter);
			
				totem_pl_playlist_set (list_to_save,
						       &pl_iter,
						       TOTEM_PL_PARSER_FIELD_URI,
						       uri,
						       TOTEM_PL_PARSER_FIELD_TITLE,
						       title, NULL);
			}

			if (totem_pl_parser_save (save_playlist, list_to_save,
						  file, NULL,
						  type, NULL) != TRUE)
				error_dialog (pl_window,
				      "Error occurred while saving playlist.");

		}*/
		// End code for 2.30.
		

	    g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

// Parses the tree model entries for saving playlist.
void save_parser (GtkTreeModel *model, GtkTreeIter *iter, gchar **uri,
		  gchar **title, gboolean *custom_title, gpointer data)
{
	gtk_tree_model_get (model, iter, COL_URI, uri,
					 COL_TRACK_NAME, title,
					-1);
	*custom_title = TRUE;
}
#endif

// Used for creating the tree view.
GtkWidget *create_treeview (GtkListStore *list)
{
	GtkWidget *treeview;
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;

	treeview = gtk_tree_view_new ();

	// Create the playing column.
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1,
						     "",
						     renderer,
						     "pixbuf", COL_PLAYING,
						     NULL);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview),
					   COL_PLAYING);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 32);

	// Create the artist column.
	renderer = gtk_cell_renderer_text_new ();
	//g_object_set (G_OBJECT (renderer), "size-points", 5, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1,
						     "Track Name",
						     renderer,
						     "text", COL_TRACK_NAME,
						     NULL);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview),
					   COL_TRACK_NAME);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 305);

	// Create the length column.
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     -1,
						     "Length",
						     renderer,
						     "text", COL_LENGTH,
						     NULL);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview),
					   COL_LENGTH);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 40);
	gtk_tree_view_column_set_alignment (column, 0);

	model = GTK_TREE_MODEL (list);
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);

	g_signal_connect (G_OBJECT (model),
			  "row-deleted",
			  G_CALLBACK (on_row_delete),
			  NULL);

	g_object_unref (model);

	return treeview;
}

// Add data to the end of a tree list.
void add_data_at_end (GtkListStore *list,
		      tag_info tag,
		      gchar* uri,
		      gchar* filename)
{
	GtkTreeIter iter;
	gchar *track_name;

	track_name = calloc (MAX_CHARS * 2, sizeof (gchar));

	// Concatenate artist and title if the artist is not empty.
	if (g_strcmp0 (tag.artist, "") == 0)
		track_name = g_strconcat (tag.title, NULL);
		//g_sprintf (track_name, "%s", tag.title);

	else
		track_name = g_strconcat (tag.artist,
					  " - ",
					  tag.title,
					  NULL);
		//g_sprintf (track_name, "%s - %s", tag.artist, tag.title);
	
	gtk_list_store_append (list, &iter);
	gtk_list_store_set (list, &iter, COL_TRACK_NAME, track_name,
					 COL_LENGTH, tag.length,
					 COL_URI, uri,
					 COL_FILENAME, filename,
			    -1);

	num_tracks++;

	g_free (track_name);
}

// Gets the tag info and puts it into the list after a tag edit.
void reset_data (GtkWidget *treeview,
		 GtkListStore *list,
		 gchar *artist,
		 gchar *title)
{
	//GtkTreeIter iter;
	gchar *track_name;
	//GtkTreePath *path;
	//GtkTreeModel *model;

	//model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	track_name = calloc (MAX_CHARS * 2, sizeof (gchar));

	if (g_strcmp0 (artist, "") == 0 && g_strcmp0 (title, "") != 0)
		track_name = g_strconcat ("Unknown Artist - ", title, NULL);
	else if (g_strcmp0 (title, "") == 0 && g_strcmp0 (artist, "") != 0)
		track_name = g_strconcat (artist, " - Unknown Title", NULL);
	else if (g_strcmp0 (artist, "") == 0 && g_strcmp0 (title, "") == 0)
		track_name = g_strconcat ("Unknown Artist - Unknown Title",
					  NULL);
	else
		track_name = g_strconcat (artist, " - ", title, NULL);

	//gtk_tree_view_get_cursor (GTK_TREE_VIEW (treeview), &path, NULL);
	//gtk_tree_model_get_iter (model, &iter, path);

	if (gtk_list_store_iter_is_valid (list, &editing))
		gtk_list_store_set (list, &editing,
				    COL_TRACK_NAME, track_name, -1);

	g_free (track_name);
}

// Clears the column of pixbufs so not more than one is set.
void clear_pixbuf (GtkListStore *list, GtkTreeView *treeview)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

	if (gtk_tree_model_get_iter_first (model, &iter))
		gtk_list_store_set (list, &iter, COL_PLAYING, NULL, -1);
	else
		return;

	while (gtk_tree_model_iter_next (model, &iter))
		gtk_list_store_set (list, &iter, COL_PLAYING, NULL, -1);
}

// Parses tag information from a track, returns false if failed.
gboolean parse_tag (gchar *filename, tag_info *track_tag)
{
	TagLib_File *open_file;
	TagLib_Tag *tag;
	const TagLib_AudioProperties *properties;
	gint min, sec;

	open_file = taglib_file_new (filename);

	if (open_file == NULL)
		return FALSE;

	tag = taglib_file_tag (open_file);
	properties = taglib_file_audioproperties (open_file);

	if (tag == NULL || properties == NULL)
		return FALSE;

	// If title or artist is just blank, set tag info to unknown.
	track_tag -> title = taglib_tag_title (tag);
	if (g_strcmp0 (track_tag -> title, "") == 0)
		track_tag -> title = "Unknown Title";

	track_tag -> artist = taglib_tag_artist (tag);
	if (g_strcmp0 (track_tag -> artist, "") == 0)
		track_tag -> artist = "Unknown Artist";

	// Grab all of the info and put it into the track tag information.
	track_tag -> album = taglib_tag_album (tag);
	track_tag -> year = taglib_tag_year (tag);
	track_tag -> comment = taglib_tag_comment (tag);
	track_tag -> track = taglib_tag_track (tag);
	track_tag -> genre = taglib_tag_genre (tag);
	track_tag -> bitrate = taglib_audioproperties_bitrate (properties);
	track_tag -> samplerate = taglib_audioproperties_samplerate
				  (properties);
	track_tag -> channels = taglib_audioproperties_channels (properties);

	sec = taglib_audioproperties_length (properties) % 60;
	min = (taglib_audioproperties_length (properties) - sec) / 60;

	g_sprintf (track_tag -> length, "%i:%02i", min, sec);

	taglib_file_free (open_file);

	return TRUE;
}

// Callback function for double clicking on a row.
void double_click_row (GtkTreeView *treeview, GtkTreePath *path,
                       GtkTreeViewColumn *col, gpointer data)
{
	GstElement *bin;
	GstState state;
	GtkListStore *pl_entries;
	GdkPixbuf *playing_icon;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkWidget *track_label;

	track_label = g_object_get_data (G_OBJECT (treeview), "track_label");
	playing_icon = g_object_get_data (G_OBJECT (treeview), "playing_icon");
	pl_entries = g_object_get_data (G_OBJECT (treeview), "pl_entries");
	bin = g_object_get_data (G_OBJECT (treeview), "bin");

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

	gst_element_set_state (bin, GST_STATE_NULL);
	gst_element_set_state (bin, GST_STATE_PLAYING);

	gst_element_get_state (bin, &state, NULL, GST_SECOND * 2);

	// If something is selected and playing, set the pixbuf to playing.
	if (gtk_tree_selection_get_selected (selection, &model, &iter)
	    && (g_strcmp0
	       (gst_element_state_get_name (state), "PLAYING") == 0))
	{
		gchar *label;

		clear_pixbuf (pl_entries, treeview);

		gtk_list_store_set (pl_entries,
				    &iter,
				    COL_PLAYING,
				    playing_icon,
				    -1);

		gtk_tree_model_get (model, &iter, 
				    COL_TRACK_NAME, &label, -1);

		gtk_label_set_text (GTK_LABEL (track_label), label);

		selected = iter;
		playing = gtk_tree_model_get_path (model, &iter);

		// For X Windows, sets the size of the window when video is played.
		#ifdef GDK_WINDOWING_X11
		GtkWidget *window;
		GList *stream_list;	// Stream info list.
		GObject *stream;	// Playing stream.
		GstObject *pad;		// The video pad.
		gint playing_video;	// Whether a video is playing.
		gint width = 0, height = 0;	// Width and height of video.

		window = g_object_get_data (G_OBJECT (treeview), "window");

		g_object_get (G_OBJECT (bin), "stream-info",
			      &stream_list, NULL);
		stream = g_list_nth_data (stream_list, 0);
		g_object_get (stream, "object", &pad, NULL);

		g_object_get (G_OBJECT (bin), "current-video",
			      &playing_video, NULL);

		if (playing_video > -1 &&
		    gst_video_get_size (GST_PAD (pad), &width, &height))
			gtk_window_resize (GTK_WINDOW (window),
					   width,
					   height + WIN_HEIGHT);
		else if (playing_video == -1)
			gtk_window_resize (GTK_WINDOW (window),
					   WIN_WIDTH,
					   WIN_HEIGHT);
		#endif
	}

}

// Callback to set the URI to the focused cell's file.
void focus_change (GtkTreeView *treeview, gpointer data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GstElement *bin;

	bin = g_object_get_data (G_OBJECT (treeview), "bin");

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gchar *uri;

		// Get the URI value associated with the row element and set
		// the pipeline's selected track to that URI value.
		gtk_tree_model_get (model, &iter, COL_URI, &uri, -1);

		g_object_set (G_OBJECT (bin), "uri", uri, NULL);
		selected = iter;

		g_free (uri);
	}

}

// Called when a row is deleted from the list (needed before due to iter
// errors).
void on_row_delete (GtkTreeModel *model,
			   GtkTreePath *path,
			   gpointer data)
{
	num_tracks--;

	/*GtkTreeIter temp;

	if (gtk_tree_model_get_iter_first (model, &temp))
	{
		// Set to previous path so that an iterator error isn't
		// generated.
		if (gtk_tree_path_compare (path, playing) == 0)
		{
			playing = NULL;
			//gtk_tree_path_prev (playing);
		}
	}*/
}

// This function will be called when a user presses a button.
gboolean on_button_press (GtkWidget *treeview,
			  GdkEventButton *event,
			  gpointer data)
{
	// Enter here if there is something selected and a single right
	// click is pressed.
	if (event -> type == GDK_BUTTON_PRESS && event -> button == 3)
	{
		GtkTreeSelection *selection;

		selection = gtk_tree_view_get_selection
					(GTK_TREE_VIEW (treeview));

		//if (gtk_tree_selection_count_selected_rows (selection) != 0)
		//	playlist_popup (treeview, event, data);
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
						   (gint) event -> x,
						   (gint) event -> y,
						   &path,
						   NULL, NULL, NULL))
		{
			gtk_tree_selection_select_path (selection, path);
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview),
						  path,
						  NULL,
						  FALSE);
			playlist_popup (treeview, event, data);
			gtk_tree_path_free (path);
		}

		return TRUE;
	}

	return FALSE;
}

// This is the right click context menu that will pop up when users right
// click on an item in the playlist.
void playlist_popup (GtkWidget *treeview,
			    GdkEventButton *event,
			    gpointer data)
{
	GtkListStore *pl_entries;
	GtkWidget *pl_window;
	GtkWidget *play, *remove;
	GtkWidget *menu,
			*play_item,
			*remove_item,
			*tag_item;

	menu = gtk_menu_new();

	pl_window = g_object_get_data (G_OBJECT (treeview), "pl_window");
	pl_entries = g_object_get_data (G_OBJECT (treeview), "pl_entries");
	play = g_object_get_data (G_OBJECT (treeview), "play");
	remove = g_object_get_data (G_OBJECT (treeview), "remove");

	// Create the menu items.
	play_item = gtk_menu_item_new_with_label ("Play Selected Item");
	remove_item = gtk_menu_item_new_with_label ("Remove Selected Item");
	tag_item = gtk_menu_item_new_with_label ("Edit File Tag...");

	// Connect signal callbacks for menu items.
	g_object_set_data (G_OBJECT (play_item), "play", play);
	g_signal_connect (play_item,
			  "activate",
			  G_CALLBACK (menu_play),
			  NULL);

	g_object_set_data (G_OBJECT (remove_item), "remove", remove);
	g_signal_connect (remove_item,
			  "activate",
			  G_CALLBACK (menu_remove),
			  NULL);

	g_object_set_data (G_OBJECT (tag_item), "pl_window", pl_window);
	g_object_set_data (G_OBJECT (tag_item), "playlist", treeview);
	g_object_set_data (G_OBJECT (tag_item), "pl_entries", pl_entries);
	g_signal_connect (tag_item,
			  "activate",
			  G_CALLBACK (menu_tag),
			  NULL);

	// Add menu items to the menu and show the popup menu.
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), play_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), remove_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), tag_item);

	gtk_widget_show_all (menu);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			(event != NULL) ? event -> button : 0,
			event -> time);
}

// Playlist context menu play.
void menu_play (GtkWidget *widget, gpointer data)
{
	GtkWidget *play;

	play = g_object_get_data (G_OBJECT (widget), "play");

	gtk_button_clicked (GTK_BUTTON (play));
}

// Playlist context menu remove.
void menu_remove (GtkWidget *widget, gpointer data)
{
	GtkWidget *remove;

	remove = g_object_get_data (G_OBJECT (widget), "remove");

	gtk_button_clicked (GTK_BUTTON (remove));
}

// Playlist context menu edit tag.
void menu_tag (GtkWidget *widget, gpointer data)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkWidget *playlist, *pl_window;
	GtkListStore *pl_entries;
	GdkPixbuf *window_icon;

	window_icon = gdk_pixbuf_new_from_file ("./img/icon.png", NULL);

	pl_window = g_object_get_data (G_OBJECT (widget), "pl_window");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (playlist));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (playlist));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gchar *filename;
		TagLib_File *open_file;
		TagLib_Tag *tag;
		const TagLib_AudioProperties *properties;

		// Get the filename associated with the row element.
		gtk_tree_model_get (model, &iter, COL_FILENAME, &filename, -1);

		open_file = taglib_file_new (filename);

		if (open_file == NULL)
		{
			error_dialog (pl_window, "Error reading file tag.");
			return;
		}

		tag = taglib_file_tag (open_file);
		properties = taglib_file_audioproperties (open_file);

		if (tag == NULL || properties == NULL)
		{
			error_dialog (pl_window, "File does not have a valid tag.");
			return;
		}
		
		// Tag dialog box starts here.
		GtkWidget *tag_window;		// Window for the tag dialog.
		GtkWidget *f_metadata, *f_properties;	// Frames.
		GtkWidget *vbox_main,		// Vertical boxes.
			  *vbox_labels,
			  *vbox_entries,
			  *vbox_properties;
		GtkWidget *hbox_main,		// Horizontal boxes.
			  *hbox_buttons;
		GtkWidget *metatable;		// Table for the metadata.
		GtkWidget *save, *cancel;	// Buttons.

		GtkWidget *track_l, *year_l,	// Labels next to textviews.
			  *title_l,
			  *artist_l,
			  *album_l,
			  *genre_l,
			  *comment_l;

		GtkWidget *filename_t,
			  *track_t, *year_t,	// Entry boxes.
			  *title_t,
			  *artist_t,
			  *album_t,
			  *genre_t,
			  *comment_t;		// Comment text view.
		GtkTextBuffer *comment_b;	// Comment text buffer.

		GtkWidget *bitrate,		// Audio information labels.
			  *samplerate,
			  *channels,
			  *length;
		
		gchar *temp;
		temp = calloc (4096, sizeof (gchar));

		editing = selected;

		// Create all of the containers and set their properties.
		tag_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

		gtk_widget_set_size_request (tag_window, 640, 375);
		gtk_window_set_resizable (GTK_WINDOW (tag_window), FALSE);
		gtk_window_set_icon (GTK_WINDOW (tag_window), window_icon);
		gtk_window_set_transient_for (GTK_WINDOW (tag_window),
					      GTK_WINDOW (pl_window));
		gtk_window_set_modal (GTK_WINDOW (tag_window), TRUE);

		f_metadata = gtk_frame_new ("Metadata");
		gtk_widget_set_size_request (f_metadata, 440, 300);

		f_properties = gtk_frame_new ("Audio Properties");
		gtk_widget_set_size_request (f_properties, 200, 300);

		vbox_main = gtk_vbox_new (FALSE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 5);
		vbox_labels = gtk_vbox_new (FALSE, 0);
		vbox_entries = gtk_vbox_new (FALSE, 0);
		vbox_properties = gtk_vbox_new (FALSE, 0);
		gtk_container_set_border_width
					(GTK_CONTAINER (vbox_properties), 5);
		hbox_main = gtk_hbox_new (FALSE, 0);
		hbox_buttons = gtk_hbox_new (FALSE, 0);

		metatable = gtk_table_new (8, 4, FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (metatable), 5);

		// Create buttons.
		save = gtk_button_new_with_label ("Save");
		cancel = gtk_button_new_with_label ("Cancel");

		// Start creation of labels and setting their properties.
		track_l = gtk_label_new ("Track #");
		gtk_misc_set_alignment (GTK_MISC (track_l), 1, 0.5);

		year_l = gtk_label_new ("Year");
		gtk_misc_set_alignment (GTK_MISC (year_l), 1, 0.5);

		title_l = gtk_label_new ("Title");
		gtk_misc_set_alignment (GTK_MISC (title_l), 1, 0.5);

		artist_l = gtk_label_new ("Artist");
		gtk_misc_set_alignment (GTK_MISC (artist_l), 1, 0.5);

		album_l = gtk_label_new ("Album");
		gtk_misc_set_alignment (GTK_MISC (album_l), 1, 0.5);

		genre_l = gtk_label_new ("Genre");
		gtk_misc_set_alignment (GTK_MISC (genre_l), 1, 0.5);

		comment_l = gtk_label_new ("Comment");
		gtk_misc_set_alignment (GTK_MISC (comment_l), 1, 0);

		// Start filling audio properties data.
		bitrate = gtk_label_new ("Bitrate: ");
		gtk_misc_set_alignment (GTK_MISC (bitrate), 0, 0.5);
		gtk_misc_set_padding (GTK_MISC (bitrate), 2, 0);
		g_sprintf (temp, "Bitrate: %i kbps",
			   taglib_audioproperties_bitrate (properties));
		gtk_label_set_text (GTK_LABEL (bitrate), temp);

		samplerate = gtk_label_new ("Sample Rate: ");
		gtk_misc_set_alignment (GTK_MISC (samplerate), 0, 0.5);
		gtk_misc_set_padding (GTK_MISC (samplerate), 2, 0);
		g_sprintf (temp, "Sample Rate: %i kHz",
			   taglib_audioproperties_samplerate (properties));
		gtk_label_set_text (GTK_LABEL (samplerate), temp);

		channels = gtk_label_new ("Channels: ");
		gtk_misc_set_alignment (GTK_MISC (channels), 0, 0.5);
		gtk_misc_set_padding (GTK_MISC (channels), 2, 0);
		g_sprintf (temp, "Channels: %i",
			   taglib_audioproperties_channels (properties));
		gtk_label_set_text (GTK_LABEL (channels), temp);

		length = gtk_label_new ("Length: ");
		gtk_misc_set_alignment (GTK_MISC (length), 0, 0.5);
		gtk_misc_set_padding (GTK_MISC (length), 2, 0);
		g_sprintf (temp, "Length: %i seconds",
			   taglib_audioproperties_length (properties));
		gtk_label_set_text (GTK_LABEL (length), temp);

		// Create the text entries and fill them with tag data.
		filename_t = gtk_entry_new ();
		gtk_entry_set_editable (GTK_ENTRY (filename_t), FALSE);
		gtk_entry_set_text (GTK_ENTRY (filename_t), filename);
		
		track_t = gtk_entry_new ();
		gtk_widget_set_size_request (track_t, 50, -1);
		if (taglib_tag_track (tag) != 0)
		{
			g_sprintf (temp, "%i", taglib_tag_track (tag));
			gtk_entry_set_text (GTK_ENTRY (track_t), temp);
		}

		year_t = gtk_entry_new ();
		gtk_widget_set_size_request (year_t, 50, -1);
		g_sprintf (temp, "%i", taglib_tag_year (tag));
		if (taglib_tag_track (tag) != 0)
		{
			g_sprintf (temp, "%i", taglib_tag_track (tag));
			gtk_entry_set_text (GTK_ENTRY (year_t), temp);
		}

		title_t = gtk_entry_new ();
		gtk_widget_set_size_request (title_t, 300, -1);
		gtk_entry_set_max_length (GTK_ENTRY (title_t), MAX_CHARS);
		temp = taglib_tag_title (tag);
		gtk_entry_set_text (GTK_ENTRY (title_t), temp);

		artist_t = gtk_entry_new ();
		gtk_widget_set_size_request (artist_t, 300, -1);
		gtk_entry_set_max_length (GTK_ENTRY (artist_t), MAX_CHARS);
		temp = taglib_tag_artist (tag);
		gtk_entry_set_text (GTK_ENTRY (artist_t), temp);

		album_t = gtk_entry_new ();
		gtk_widget_set_size_request (album_t, 300, -1);
		gtk_entry_set_max_length (GTK_ENTRY (album_t), MAX_CHARS);
		temp = taglib_tag_album (tag);
		gtk_entry_set_text (GTK_ENTRY (album_t), temp);

		genre_t = gtk_entry_new ();
		gtk_widget_set_size_request (genre_t, 300, -1);
		gtk_entry_set_max_length (GTK_ENTRY (genre_t), MAX_CHARS);
		temp = taglib_tag_genre (tag);
		gtk_entry_set_text (GTK_ENTRY (genre_t), temp);

		comment_t = gtk_text_view_new ();
		gtk_widget_set_size_request (comment_t, 300, 80);
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (comment_t),
					     GTK_WRAP_WORD);
		comment_b = gtk_text_view_get_buffer
					(GTK_TEXT_VIEW (comment_t));
		temp = taglib_tag_comment (tag);
		gtk_text_buffer_set_text (comment_b, temp, -1);

		// Pack all of the widgets into their containers.
		gtk_container_add (GTK_CONTAINER (tag_window), vbox_main);
		gtk_box_pack_start (GTK_BOX (vbox_main),
				    filename_t, FALSE, FALSE, 2);
		gtk_box_pack_start (GTK_BOX (vbox_main),
				    hbox_main, FALSE, FALSE, 2);
		gtk_box_pack_start (GTK_BOX (vbox_main),
				    hbox_buttons, FALSE, FALSE, 2);
		gtk_box_pack_start (GTK_BOX (hbox_main),
				    f_metadata, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (hbox_main),
				    f_properties, FALSE, FALSE, 2);

		gtk_container_add (GTK_CONTAINER (f_metadata), metatable);
		gtk_table_attach (GTK_TABLE (metatable), track_l,
				  0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), track_t,
				  1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), year_l,
				  2, 3, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), year_t,
				  3, 4, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), title_l,
				  0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), title_t,
				  1, 4, 1, 2, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), artist_l,
				  0, 1, 2, 3, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), artist_t,
				  1, 4, 2, 3, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), album_l,
				  0, 1, 3, 4, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), album_t,
				  1, 4, 3, 4, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), genre_l,
				  0, 1, 4, 5, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), genre_t,
				  1, 4, 4, 5, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), comment_l,
				  0, 1, 5, 7, GTK_FILL, GTK_FILL, 2, 2);
		gtk_table_attach (GTK_TABLE (metatable), comment_t,
				  1, 4, 5, 7, GTK_FILL, GTK_FILL, 2, 2);

		gtk_container_add (GTK_CONTAINER (f_properties), vbox_properties);
		gtk_box_pack_start (GTK_BOX (vbox_properties),
				    bitrate, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox_properties),
				    samplerate, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox_properties),
				    channels, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox_properties),
				    length, FALSE, FALSE, 0);

		gtk_box_pack_end (GTK_BOX (hbox_buttons),
				    cancel, FALSE, FALSE, 2);
		gtk_box_pack_end (GTK_BOX (hbox_buttons),
				    save, FALSE, FALSE, 2);

		// Connect signals for the buttons to their callback.
		g_object_set_data (G_OBJECT (save),
				   "tag_window", tag_window);
		g_object_set_data (G_OBJECT (save), "playlist", playlist);
		g_object_set_data (G_OBJECT (save), "pl_entries", pl_entries);
		g_object_set_data (G_OBJECT (save), "open_file", open_file);
		g_object_set_data (G_OBJECT (save), "track_t", track_t);
		g_object_set_data (G_OBJECT (save), "year_t", year_t);
		g_object_set_data (G_OBJECT (save), "title_t", title_t);
		g_object_set_data (G_OBJECT (save), "artist_t", artist_t);
		g_object_set_data (G_OBJECT (save), "album_t", album_t);
		g_object_set_data (G_OBJECT (save), "genre_t", genre_t);
		g_object_set_data (G_OBJECT (save), "comment_b", comment_b);
		g_signal_connect (G_OBJECT (save), "clicked",
				  G_CALLBACK (save_button_clicked), NULL);
		g_object_set_data (G_OBJECT (cancel),
				   "tag_window", tag_window);
		g_object_set_data (G_OBJECT (cancel), "open_file", open_file);
		g_signal_connect (G_OBJECT (cancel), "clicked",
				  G_CALLBACK (cancel_button_clicked), NULL);

		gtk_widget_show_all (tag_window);

		g_free (filename);
		g_free (temp);
	}
}

// Saves the file tags when the save button is clicked.
void save_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *track_t,
		  *year_t,
		  *title_t,
		  *artist_t,
		  *album_t,
		  *genre_t,
		  *tag_window,
		  *playlist;
	GtkListStore *pl_entries;
	GtkTextBuffer *comment_b;
	tag_info track_info;
	GtkTextIter comment_start, comment_end;
	TagLib_File *open_file;
	TagLib_Tag *tag;

	tag_window = g_object_get_data (G_OBJECT (widget), "tag_window");
	playlist = g_object_get_data (G_OBJECT (widget), "playlist");
	pl_entries = g_object_get_data (G_OBJECT (widget), "pl_entries");
	open_file = g_object_get_data (G_OBJECT (widget), "open_file");
	track_t = g_object_get_data (G_OBJECT (widget), "track_t");
	year_t = g_object_get_data (G_OBJECT (widget), "year_t");
	title_t = g_object_get_data (G_OBJECT (widget), "title_t");
	artist_t = g_object_get_data (G_OBJECT (widget), "artist_t");
	album_t = g_object_get_data (G_OBJECT (widget), "album_t");
	genre_t = g_object_get_data (G_OBJECT (widget), "genre_t");
	comment_b = g_object_get_data (G_OBJECT (widget), "comment_b");

	tag = taglib_file_tag (open_file);

	if (tag == NULL)
		return;

	// Initialize all of the variables.
	gtk_text_buffer_get_start_iter (comment_b, &comment_start);
	gtk_text_buffer_get_end_iter (comment_b, &comment_end);
	track_info.title = calloc (MAX_CHARS, sizeof (gchar));
	track_info.artist = calloc (MAX_CHARS, sizeof (gchar));
	track_info.album = calloc (MAX_CHARS, sizeof (gchar));
	track_info.genre = calloc (MAX_CHARS, sizeof (gchar));

	// Set all the necessary info.
	track_info.track =
	(int) g_ascii_strtoll (gtk_entry_get_text (GTK_ENTRY (track_t)),
				       NULL, 10);
	track_info.year =
	(int) g_ascii_strtoll (gtk_entry_get_text (GTK_ENTRY (year_t)),
				       NULL, 10);
	g_sprintf (track_info.title, "%s",
		   gtk_entry_get_text (GTK_ENTRY (title_t)));
	g_sprintf (track_info.artist, "%s",
		   gtk_entry_get_text (GTK_ENTRY (artist_t)));
	g_sprintf (track_info.album, "%s",
		   gtk_entry_get_text (GTK_ENTRY (album_t)));
	g_sprintf (track_info.genre, "%s",
		   gtk_entry_get_text (GTK_ENTRY (genre_t)));
	track_info.comment =
	gtk_text_buffer_get_text (comment_b,
				  &comment_start,
				  &comment_end,
				  FALSE);

	taglib_tag_set_track (tag, track_info.track);
	taglib_tag_set_year (tag, track_info.year);
	taglib_tag_set_title (tag, track_info.title);
	taglib_tag_set_artist (tag, track_info.artist);
	taglib_tag_set_album (tag, track_info.album);
	taglib_tag_set_genre (tag, track_info.genre);
	taglib_tag_set_comment (tag, track_info.comment);

	taglib_file_save (open_file);

	reset_data (playlist, pl_entries, track_info.artist, track_info.title);

	gtk_widget_destroy (tag_window);

	// Free the memory.
	g_free (track_info.title);
	g_free (track_info.artist);
	g_free (track_info.album);
	g_free (track_info.genre);
	taglib_file_free (open_file);
}

// If cancel is clicked while editing tag, just hide the window.
void cancel_button_clicked (GtkWidget *widget, gpointer data)
{
	GtkWidget *tag_window;
	TagLib_File *open_file;

	tag_window = g_object_get_data (G_OBJECT (widget), "tag_window");
	open_file = g_object_get_data (G_OBJECT (widget), "open_file");

	taglib_file_free (open_file);
	gtk_widget_destroy (tag_window);
}
