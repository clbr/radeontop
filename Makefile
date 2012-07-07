bin = radeontop
src = radeontop.c
obj = $(src:.c=.o)

$(bin): $(obj)
	gcc -o $(bin) $(CFLAGS) $(LDFLAGS) $(obj)
