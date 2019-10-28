# The make-provided flags like MAKE and CC aren't set, on purpose.
# This is Linux-specific software, so we can depend on GNU make.

# Options:
#	nls	enable translations, default on
#	debug	enable debug symbols, default off
#	nostrip	disable stripping, default off
#	plain	apply neither -g nor -s.
#	xcb	enable libxcb to run unprivileged in Xorg, default on
#	amdgpu	enable amdgpu usage reporting, default auto
#		it requires libdrm >= 2.4.63

PREFIX ?= /usr
MANDIR ?= $(PREFIX)/share/man
LOCALEDIR ?= $(PREFIX)/share/locale
INSTALL ?= install
PKG_CONFIG ?= pkg-config
TARGET_OS ?= $(shell uname)

nls ?= 1
xcb ?= 1

bin = radeontop
src = $(filter-out amdgpu.c, $(wildcard *.c))
verh = include/version.h

CFLAGS_SECTIONED = -ffunction-sections -fdata-sections
LDFLAGS_SECTIONED = -Wl,-gc-sections

CFLAGS ?= -Os $(CFLAGS_SECTIONED)
CFLAGS += -Wall -Wextra -pthread
CFLAGS += -Iinclude
CFLAGS += $(shell $(PKG_CONFIG) --cflags pciaccess)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libdrm)
ifeq ($(xcb), 1)
	CFLAGS += $(shell $(PKG_CONFIG) --cflags xcb xcb-dri2)
	CFLAGS += -DENABLE_XCB=1
endif

ifneq ($(filter $(TARGET_OS), GNU/kFreeBSD Linux),)
	libdl = -ldl
else ifneq ($(filter $(TARGET_OS), DragonFly FreeBSD OpenBSD),)
	libintl = -lintl
endif

ifeq ($(shell $(PKG_CONFIG) ncursesw && echo 1), 1)
	ncurses = ncursesw
else
	ncurses = ncurses
endif

CFLAGS += $(shell $(PKG_CONFIG) --cflags $(ncurses))

# Comment this if you don't want translations
ifeq ($(nls), 1)
	CFLAGS += -DENABLE_NLS=1 -DLOCALEDIR=\"$(LOCALEDIR)\"
	LIBS += $(libintl)
endif

# autodetect libdrm features
ifeq ($(shell $(PKG_CONFIG) 'libdrm >= 2.4.66' && echo 1), 1)
	CFLAGS += -DHAS_DRMGETDEVICE=1
endif

ifeq ($(shell $(PKG_CONFIG) libdrm_amdgpu && echo 1), 1)
	amdgpu ?= 1
else
	amdgpu ?= 0
endif

ifeq ($(amdgpu), 1)
	src += amdgpu.c
	CFLAGS += -DENABLE_AMDGPU=1
	LIBS += $(shell $(PKG_CONFIG) --libs libdrm_amdgpu)

	ifeq ($(shell $(PKG_CONFIG) 'libdrm_amdgpu >= 2.4.79' && echo 1), 1)
		CFLAGS += -DHAS_AMDGPU_QUERY_SENSOR_INFO=1
	endif
endif

ifndef plain
ifdef debug
	CFLAGS += -g
else ifndef nostrip
	CFLAGS += -s
endif
endif

obj = $(src:.c=.o)
LDFLAGS ?= -Wl,-O1 $(LDFLAGS_SECTIONED)
LIBS += $(shell $(PKG_CONFIG) --libs pciaccess)
LIBS += $(shell $(PKG_CONFIG) --libs libdrm)
ifeq ($(xcb), 1)
	LIBS += $(shell $(PKG_CONFIG) --libs xcb xcb-dri2)
	LIBS += $(libdl)
endif

# On some distros, you might have to change this to ncursesw
LIBS += $(shell $(PKG_CONFIG) --libs $(ncurses))

export PREFIX LOCALEDIR INSTALL

.PHONY: all clean install man dist

all: $(bin)

$(obj): $(wildcard include/*.h) $(verh)

$(bin): $(obj)
	$(CC) -o $(bin) $(obj) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o $(bin)

.git:
	mkdir .git

$(verh): .git
	./getver.sh

trans:
	xgettext -o translations/radeontop.pot -k_ *.c \
	--package-name radeontop

install: all
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/sbin
	$(INSTALL) $(bin) $(DESTDIR)$(PREFIX)/sbin
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -m644 radeontop.1 $(DESTDIR)$(MANDIR)/man1
ifeq ($(nls), 1)
	$(MAKE) -C translations install
endif

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
