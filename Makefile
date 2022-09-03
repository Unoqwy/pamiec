LDFLAGS=-lpulse -lpulse-simple

pamiec: pamiec.c export.c record.c daemon.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
