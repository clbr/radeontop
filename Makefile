PREFIX ?= /usr
INSTALL ?= install

bin = radeontop
src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS ?= -Os
CFLAGS += -Wall -Wextra -pthread
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(shell pkg-config --cflags pciaccess)

# Comment this if you don't want translations
CFLAGS += -DENABLE_NLS=1

ifdef debug
	CFLAGS += -g
else
	CFLAGS += -s
endif

LDFLAGS += -Wl,-O1 -Wl,-gc-sections
LDFLAGS += $(shell pkg-config --libs pciaccess)

# On some distros, you might have to change this to ncursesw
LDFLAGS += -lncurses

.PHONY: all clean install man

all: $(bin)

$(obj): $(wildcard *.h) version.h

$(bin): $(obj)
	$(CC) -o $(bin) $(obj) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o $(bin)

.git:

version.h: .git
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
