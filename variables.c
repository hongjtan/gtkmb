#include "variables.h"

gboolean loop = FALSE, shuffle = FALSE;
GtkTreeIter selected;
GtkTreePath *playing;
GtkTreeIter editing;
gint num_tracks = 0;

gulong video_window_xid = 0;
gint window_state = 1;
