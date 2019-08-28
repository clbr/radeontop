PREFIX ?= /usr
LOCALEDIR ?= $(PREFIX)/share/locale
INSTALL ?= install
MSGFMT ?= msgfmt

src = $(wildcard *.po)
obj = $(src:.po=.mo)
inst = $(src:.po=.inst)

.PHONY: all install

all: $(obj)

$(obj): $(src)

%.mo: %.po
	$(MSGFMT) -o $@ $<

%.inst: %.mo
	$(INSTALL) -d $(DESTDIR)$(LOCALEDIR)/$*/LC_MESSAGES
	$(INSTALL) -m 644 $< $(DESTDIR)$(LOCALEDIR)/$*/LC_MESSAGES/radeontop.mo

install: all $(inst)
