LDFLAGS=-lpulse -lpulse-simple

pamiec: pamiec.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
