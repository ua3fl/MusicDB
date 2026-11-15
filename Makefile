CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
ID4    = 0067

all: musicdb_report

musicdb_report: main.c musicdb.c musicdb.h
	$(CC) $(CFLAGS) -o musicdb_report main.c musicdb.c

check: main.c musicdb.c musicdb.h
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -o musicdb_check main.c musicdb.c
	./musicdb_check data $(ID4)

clean:
	rm -f musicdb_report musicdb_check

.PHONY: all check clean