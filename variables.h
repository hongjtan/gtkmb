#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gst/gst.h>

#define INITIAL_VALUE 0
#define MIN_VALUE 0
#define MAX_VALUE 100
#define MAX_CHARS 256
#define WIN_WIDTH 360
#define WIN_HEIGHT 110

// Global variables.
extern gboolean loop, shuffle;
extern GtkTreeIter selected;	// Iterators which point to the track
				// that is currently selected. Needed for
				// setting "playing" pixbuf.
extern GtkTreePath *playing;	// Points to the currently playing track.
extern GtkTreeIter editing;	// Points to the track being edited.
extern gint num_tracks;	// Total number of tracks in the playlist.

extern gulong video_window_xid;
extern gint window_state;

#endif
