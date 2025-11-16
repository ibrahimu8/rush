CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lcurl

rush: rush.c
	$(CC) $(CFLAGS) -o rush rush.c $(LDFLAGS)

clean:
	rm -f rush
