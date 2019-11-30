CFLAGS = -std=c89 -Wall -W
PREFIX = /usr/local

shesc: shesc.o

check: shesc
	./test.sh
	./testnul.sh

clean:
	$(RM) shesc shesc.o

install: shesc
	install -D -m755 shesc $(DESTDIR)$(PREFIX)/bin/shesc

.PHONY: check clean install
