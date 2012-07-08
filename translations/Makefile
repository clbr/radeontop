INSTALL ?= install

src = $(wildcard *.po)
obj = $(src:.po=.mo)
inst = $(src:.po=.inst)

.PHONY: all install

all: $(obj)

$(obj): $(src)

%.mo: %.po
	msgfmt -o $@ $<

%.inst: %.mo
	$(INSTALL) -D -m 644 $< $(DESTDIR)/$(PREFIX)/share/locale/$*/LC_MESSAGES/$<

install: all $(inst)
