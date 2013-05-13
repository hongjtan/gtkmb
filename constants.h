#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define INITIAL_VALUE 0
#define MIN_VALUE 0
#define MAX_VALUE 100
#define MAX_CHARS 256
#define WIN_WIDTH 360
#define WIN_HEIGHT 110

// Global variables.
static gboolean loop = FALSE, shuffle = FALSE;
static GtkTreeIter selected;	// Iterators which point to the track
				// that is currently selected. Needed for
				// setting "playing" pixbuf.
static GtkTreePath *playing;	// Points to the currently playing track.
static GtkTreeIter editing;	// Points to the track being edited.
static gint num_tracks = 0;	// Total number of tracks in the playlist.

#endif
