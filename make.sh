gcc -g -o final_hjtan final_hjtan.c `pkg-config --cflags --libs gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10 gstreamer-video-0.10 gnome-vfs-2.0 totem-plparser taglib` -ltag_c
