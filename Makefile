.POSIX:

CC=cc
CFLAGS=-O -I/usr/local/include -I/usr/X11R6/include
LDLIBS=-lm -lglfw -lGLESv2
LDFLAGS=-s -L/usr/local/lib -L /usr/X11R6/lib $(LDLIBS)

SRCOBJ=src/main.o
LIBOBJ=lib/r.o lib/batch.o lib/mat.o
OBJ=$(SRCOBJ) $(LIBOBJ)
DST=wbmsim

$(DST): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

cruncher: src/cruncher.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/cruncher.c

$(OBJ): lib/lib.h
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) cruncher

distclean:
	rm -f $(OBJ) $(DST) cruncher
