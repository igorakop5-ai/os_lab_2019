CC=gcc
CFLAGS=-pthread -I.

all: mutex_without mutex_with factorial deadlock

mutex_without: mutex.c
	$(CC) -DWITHOUT_MUTEX -o mutex_without mutex.c $(CFLAGS)

mutex_with: mutex.c
	$(CC) -o mutex_with mutex.c $(CFLAGS)

factorial: factorial.c
	$(CC) -o factorial factorial.c $(CFLAGS)

deadlock: deadlock.c
	$(CC) -o deadlock deadlock.c $(CFLAGS)

clean:
	rm -f mutex_without mutex_with factorial deadlock
