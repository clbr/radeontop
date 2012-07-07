bin = radeontop
src = radeontop.c detect.c
obj = $(src:.c=.o)

CFLAGS ?= -Os
CFLAGS += -Wall -Wextra -s
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(shell pkg-config --cflags pciaccess)

LDFLAGS += -Wl,-O1 -Wl,-gc-sections
LDFLAGS += $(shell pkg-config --libs pciaccess)

.PHONY: all clean

all: $(bin)

$(bin): $(obj)
	gcc -o $(bin) $(CFLAGS) $(LDFLAGS) $(obj)

clean:
	rm -f *.o $(bin)
