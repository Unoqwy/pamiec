DESTDIR=/usr/local/bin
LDFLAGS=-lpulse -lpulse-simple

pamiec: pamiec.c export.c record.c daemon.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

install: pamiec
	cp $< ${DESTDIR}
