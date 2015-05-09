DESTDIR ?=
BIN_DIR ?= /bin
DOC_DIR ?= /usr/share/doc
MAN_DIR ?= /usr/share/man

INSTALL = install -v

include ./Makefile.common

all: dir2pkg/dir2pkg packlad/packlad

genkeys:
	cd keys; $(MAKE)

core/libpacklad-core.a: keys/pub_key
	cd core; $(MAKE)

logic/libpacklad-logic.a: core/libpacklad-core.a
	cd logic; $(MAKE)

packlad/packlad: core/libpacklad-core.a logic/libpacklad-logic.a
	cd packlad; $(MAKE)

pkgsign/pkgsign: core/libpacklad-core.a
	cd pkgsign; $(MAKE)

tar2pkg/tar2pkg: pkgsign/pkgsign
	cd tar2pkg; $(MAKE)

dir2pkg/dir2pkg: tar2pkg/tar2pkg
	cd dir2pkg; $(MAKE)

install: all
	$(INSTALL) -D -m 755 pkgsign/pkgsign $(DESTDIR)/$(BIN_DIR)/pkgsign
	$(INSTALL) -m 755 tar2pkg/tar2pkg $(DESTDIR)/$(BIN_DIR)/tar2pkg
	$(INSTALL) -m 755 dir2pkg/dir2pkg $(DESTDIR)/$(BIN_DIR)/dir2pkg
	$(INSTALL) -m 755 packlad/packlad $(DESTDIR)/$(BIN_DIR)/packlad
	$(INSTALL) -D -m 644 README $(DESTDIR)/$(DOC_DIR)/packlad/README
	$(INSTALL) -m 644 AUTHORS $(DESTDIR)/$(DOC_DIR)/packlad/AUTHORS
	$(INSTALL) -m 644 COPYING $(DESTDIR)/$(DOC_DIR)/packlad/COPYING
	$(INSTALL) -D -m 644 doc/tar2pkg.1 $(DESTDIR)/$(MAN_DIR)/man1/tar2pkg.1
	$(INSTALL) -D -m 644 doc/dir2pkg.1 $(DESTDIR)/$(MAN_DIR)/man1/dir2pkg.1
	$(INSTALL) -D -m 644 doc/packlad.8 $(DESTDIR)/$(MAN_DIR)/man8/packlad.8
	$(INSTALL) -D -d -m 755 $(DESTDIR)/$(VAR_DIR)
	$(INSTALL) -d -m 755 $(DESTDIR)/$(PKG_ARCHIVE_DIR)
	$(INSTALL) -d -m 755 $(DESTDIR)/$(INST_DATA_DIR)
	$(INSTALL) -D -d -m 755 $(DESTDIR)/$(CONF_DIR)
	$(INSTALL) -m 400 keys/pub_key $(DESTDIR)/$(CONF_DIR)/pub_key

clean:
	cd dir2pkg; $(MAKE) clean
	cd tar2pkg; $(MAKE) clean
	cd pkgsign; $(MAKE) clean
	cd logic; $(MAKE) clean
	cd core; $(MAKE) clean
	cd keys; $(MAKE) clean
	cd packlad; $(MAKE) clean
