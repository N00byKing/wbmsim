.POSIX:

CC=cc
CFLAGS=-O -I/usr/local/include -I/usr/X11R6/include
LDLIBS=-lm -lglfw -lGLESv2
LDFLAGS=-s -L/usr/local/lib -L /usr/X11R6/lib

SRCOBJ=src/main.o src/wop.o
LIBOBJ=lib/r.o lib/batch.o lib/mat.o
OBJ=$(SRCOBJ) $(LIBOBJ)
DST=wbmsim

$(DST): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(OBJ): lib/lib.h
$(SRCOBJ): src/wop.h
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ)

distclean:
	rm -f $(OBJ) $(DST)
