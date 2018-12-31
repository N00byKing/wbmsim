.POSIX:

CC=cc
CFLAGS=-O
LDFLAGS=-s

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
