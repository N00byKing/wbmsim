.POSIX:

CC=cc
CFLAGS=-O -I/usr/local/include -I/usr/X11R6/include
LDLIBS=-lglfw -lGLESv2
LDFLAGS=-s -L/usr/local/lib -L /usr/X11R6/lib $(LDLIBS)

OBJ=src/main.o
DST=wbmsim

$(DST): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ)

distclean:
	rm -f $(OBJ) $(DST)
