APP=gtkmb
CC=gcc
CFLAGS=-Wall -g $(shell pkg-config --cflags --libs gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10 gstreamer-video-0.10 gnome-vfs-2.0 totem-plparser taglib)
LIBS=-ltag_c
SRC= main.c playlist.c player.c video.c variables.c

all: $(SRC) $(APP)

$(APP): $(OBJ)
	$(CC) -o $(APP) $(SRC) $(CFLAGS) $(LIBS) 

clean:
	rm $(APP)
