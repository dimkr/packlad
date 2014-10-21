DESTDIR ?=
BIN_DIR ?= /bin
DOC_DIR ?= /usr/share/doc
MAN_DIR ?= /usr/share/man

INSTALL = install -v

include ./Makefile.common

all: dir2pkg/dir2pkg packlad/packlad

ed25519/libed25519.a:
	cd ed25519; $(MAKE)

keygen/keygen: ed25519/libed25519.a
	cd keygen; $(MAKE)

core/pub_key core/pub_key.h core/priv_key core/priv_key.h: keygen/keygen
	./keygen/keygen core/pub_key \
	                core/pub_key.h \
	                core/priv_key \
	                core/priv_key.h

core/libpacklad-core.a: ed25519/libed25519.a core/pub_key.h
	cd core; $(MAKE)

logic/libpacklad-logic.a: core/libpacklad-core.a
	cd logic; $(MAKE)

packlad/packlad: core/libpacklad-core.a logic/libpacklad-logic.a
	cd packlad; $(MAKE)

pkgsign/pkgsign: core/libpacklad-core.a
	cd pkgsign; $(MAKE)

dir2pkg/dir2pkg: pkgsign/pkgsign
	cd dir2pkg; $(MAKE)

install: all
	$(INSTALL) -D -m 755 pkgsign/pkgsign $(DESTDIR)/$(BIN_DIR)/pkgsign
	$(INSTALL) -m 755 dir2pkg/dir2pkg $(DESTDIR)/$(BIN_DIR)/dir2pkg
	$(INSTALL) -m 755 packlad/packlad $(DESTDIR)/$(BIN_DIR)/packlad
	$(INSTALL) -D -m 644 README $(DESTDIR)/$(DOC_DIR)/packlad/README
	$(INSTALL) -m 644 AUTHORS $(DESTDIR)/$(DOC_DIR)/packlad/AUTHORS
	$(INSTALL) -m 644 COPYING $(DESTDIR)/$(DOC_DIR)/packlad/COPYING
	$(INSTALL) -D -m 644 doc/dir2pkg.1 $(DESTDIR)/$(MAN_DIR)/man1/dir2pkg.1
	$(INSTALL) -D -m 644 doc/packlad.8 $(DESTDIR)/$(MAN_DIR)/man8/packlad.8
	$(INSTALL) -D -d -m 755 $(DESTDIR)/$(VAR_DIR)
	$(INSTALL) -d -m 755 $(DESTDIR)/$(PKG_ARCHIVE_DIR)
	$(INSTALL) -d -m 755 $(DESTDIR)/$(INST_DATA_DIR)

clean:
	cd dir2pkg; $(MAKE) clean
	cd pkgsign; $(MAKE) clean
	cd logic; $(MAKE) clean
	cd core; $(MAKE) clean
	rm -f core/priv_key.h core/priv_key core/pub_key.h core/pub_key
	cd keygen; $(MAKE) clean
	cd ed25519; $(MAKE) clean
	cd packlad; $(MAKE) clean
