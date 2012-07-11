# The make-provided flags like MAKE and CC aren't set, on purpose.
# This is Linux-specific software, so we can depend on GNU make.

PREFIX ?= /usr
INSTALL ?= install

bin = radeontop
src = $(wildcard *.c)
obj = $(src:.c=.o)
verh = include/version.h

CFLAGS ?= -Os
CFLAGS += -Wall -Wextra -pthread
CFLAGS += -Iinclude
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(shell pkg-config --cflags pciaccess)
CFLAGS += $(shell pkg-config --cflags ncurses 2>/dev/null)

# Comment this if you don't want translations
CFLAGS += -DENABLE_NLS=1

ifdef debug
	CFLAGS += -g
else
	CFLAGS += -s
endif

LDFLAGS += -Wl,-O1 -Wl,-gc-sections
LIBS += $(shell pkg-config --libs pciaccess)

# On some distros, you might have to change this to ncursesw
LIBS += $(shell pkg-config --libs ncurses 2>/dev/null || echo "-lncurses")

.PHONY: all clean install man dist

all: $(bin)

$(obj): $(wildcard include/*.h) $(verh)

$(bin): $(obj)
	$(CC) -o $(bin) $(obj) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o $(bin)

.git:

$(verh): .git
	./getver.sh

trans:
	xgettext -o translations/radeontop.pot -k_ *.c \
	--package-name radeontop

install: all
	$(INSTALL) -D -m755 $(bin) $(DESTDIR)/$(PREFIX)/sbin/$(bin)
	$(INSTALL) -D -m644 radeontop.1 $(DESTDIR)/$(PREFIX)/share/man/man1/radeontop.1
	$(MAKE) -C translations install PREFIX=$(PREFIX)

man:
	a2x -f manpage radeontop.asc

dist: ver = $(shell git describe)
dist: name = $(bin)-$(ver)
dist: clean $(verh)
	sed -i '/\t\.\/getver.sh/d' Makefile
	cd .. && \
	ln -s $(bin) $(name) && \
	tar -h --numeric-owner --exclude-vcs -cvf - $(name) | pigz -9 > /tmp/$(name).tgz && \
	rm $(name)
	advdef -z4 /tmp/$(name).tgz
	git checkout Makefile
	cd /tmp && sha1sum $(name).tgz > $(name).tgz.sha1
