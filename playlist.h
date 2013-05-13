#ifndef _PLAYLIST_H
#define _PLAYLIST_H

#include <gtk/gtk.h>
#include <taglib/tag_c.h>
#ifdef unix
	#include <totem-pl-parser.h> 
	#include <libgnomevfs/gnome-vfs.h>
#endif

// Enumeration of playlist tree view columns.
enum
{
	COL_PLAYING = 0,
	COL_TRACK_NAME,
	COL_LENGTH,
	COL_URI,
	COL_FILENAME,
	NUM_COLS
};

// Tag information.
typedef struct
{
	gchar *title;
	gchar *artist;
	gchar *album;
	gint year;
	gchar *comment;
	gint track;
	gchar *genre;
	gint bitrate;
	gint samplerate;
	gint channels;
	gchar length[8];
} tag_info;

void new_pl_button_clicked (GtkWidget *widget, gpointer data);
#ifdef unix
TotemPlParserResult parse_pl (gchar *uri, GtkListStore *pl_entries);
void parsed_entry (TotemPlParser *parser,
			  gchar *uri,
			  GHashTable *metadata,
			  gchar data);
void load_pl_button_clicked (GtkWidget *widget, gpointer data);
void save_pl_button_clicked (GtkWidget *widget, gpointer data);
void save_parser (GtkTreeModel *model, GtkTreeIter *iter, gchar **uri,
		  gchar **title, gboolean *custom_title, gpointer data);
#endif
GtkWidget *create_treeview (GtkListStore *list);
void add_data_at_end (GtkListStore *list,
		      tag_info tag,
		      gchar* uri,
		      gchar* filename);
void reset_data (GtkWidget *treeview,
		 GtkListStore *list,
		 gchar *artist,
		 gchar *title);
void clear_pixbuf (GtkListStore *list, GtkTreeView *treeview);
gboolean parse_tag (gchar *filename, tag_info *track_tag);
void double_click_row (GtkTreeView *treeview, GtkTreePath *path,
                       GtkTreeViewColumn *col, gpointer data);
void focus_change (GtkTreeView *treeview, gpointer data);
void on_row_delete (GtkTreeModel *model,
			   GtkTreePath *path,
			   gpointer data);
gboolean on_button_press (GtkWidget *treeview,
			  GdkEventButton *event,
			  gpointer data);
void playlist_popup (GtkWidget *treeview,
			    GdkEventButton *event,
			    gpointer data);
	void menu_play (GtkWidget *widget, gpointer data);
	void menu_remove (GtkWidget *widget, gpointer data);
	void menu_tag (GtkWidget *widget, gpointer data);
		void save_button_clicked (GtkWidget *widget,
						 gpointer data);
		void cancel_button_clicked (GtkWidget *widget,
						   gpointer data);
#endif
