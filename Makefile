bin = radeontop
src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS ?= -Os
CFLAGS += -Wall -Wextra
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(shell pkg-config --cflags pciaccess)

ifdef debug
	CFLAGS += -g
else
	CFLAGS += -s
endif

LDFLAGS += -Wl,-O1 -Wl,-gc-sections
LDFLAGS += $(shell pkg-config --libs pciaccess)

.PHONY: all clean

all: $(bin)

$(obj): $(wildcard *.h)

$(bin): $(obj)
	gcc -o $(bin) $(CFLAGS) $(LDFLAGS) $(obj)

clean:
	rm -f *.o $(bin)

version.h: .git
	./getver.sh
