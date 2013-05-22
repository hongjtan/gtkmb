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

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gst/gst.h>

#define INITIAL_VALUE 0
#define MIN_VALUE 0
#define MAX_VALUE 100
#define MAX_CHARS 256
#define WIN_WIDTH 394
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
