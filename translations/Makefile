# programs
MSGFMT = msgfmt
INSTALL = install
INSTALL_DATA = $(INSTALL) -D -m 644

src = $(wildcard *.po)
obj = $(src:.po=.mo)
inst = $(src:.po=.inst)

.PHONY: all install

all: $(obj)

$(obj): $(src)

%.mo: %.po
	$(MSGFMT) -o $@ $<

%.inst: %.mo
	$(INSTALL_DATA) $< $(DESTDIR)/$(PREFIX)/share/locale/$*/LC_MESSAGES/$<

install: all $(inst)
