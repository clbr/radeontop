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

.PHONY: all clean

all: $(bin)

$(obj): $(wildcard *.h)

$(bin): $(obj)
	gcc -o $(bin) $(CFLAGS) $(LDFLAGS) $(obj)

clean:
	rm -f *.o $(bin)

version.h: .git
	./getver.sh

trans:
	xgettext -o translations/radeontop.pot -k_ *.c \
	--package-name radeontop
