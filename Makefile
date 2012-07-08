PREFIX ?= /usr
VERSION = $(shell git describe 2>/dev/null || echo "unknown")

# programs
STRIP = strip
MAKE = make
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -D -m755
XGETTEXT = xgettext
RM = rm

bin = radeontop
src = $(wildcard *.c)
obj = $(src:.c=.o)

CC ?= gcc

CFLAGS ?= -Os
CFLAGS += -Wall -Wextra -pthread
CFLAGS += $(shell pkg-config --cflags pciaccess) \
	  $(shell pkg-config --cflags ncurses)

# Comment this if you don't want translations
ifdef nls
CPPFLAGS += -DENABLE_NLS=1
endif

ifdef debug
CFLAGS += -g
endif

LIBS += \
	$(shell pkg-config --libs pciaccess) \
	$(shell pkg-config --libs ncurses)

.PHONY: all clean install

all: $(bin)

$(obj): $(wildcard *.h) version.h

$(bin): $(obj)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(bin) $(obj) $(LDFLAGS) $(LIBS)

clean:
	$(RM) -f *.o $(bin)

version.h:
	@echo "#ifndef VER_H" > version.h
	@echo "#define VER_H" >> version.h
	@echo >> version.h
	@echo "#define VERSION \"$(VERSION)\"" >> version.h
	@echo >> version.h
	@echo "#endif" >> version.h

trans:
	$(XGETTEXT) -o translations/radeontop.pot -k_ *.c \
	--package-name radeontop

strip:
	$(STRIP) --strip-unneeded -R .comment -R .GCC.command.line $(bin)

install:
	$(INSTALL_PROGRAM) $(bin) $(DESTDIR)/$(PREFIX)/sbin/$(bin)
	$(MAKE) -C translations install PREFIX=$(PREFIX)
