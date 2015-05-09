#ifndef _PKG_ENTRY_H_INCLUDED
#	define _PKG_ENTRY_H_INCLUDED

#	include <stdbool.h>

#	include "types.h"

#	define INST_REASON_DEP "dependency"
#	define INST_REASON_CORE "core"
#	define INST_REASON_USER "user"

struct pkg_entry {
	char buf[512];
	char *name;
	char *ver;
	char *desc;
	char *fname;
	char *deps;
	char *reason;
};

bool pkg_entry_register(const struct pkg_entry *entry);
bool pkg_entry_unregister(const char *name);

bool pkg_entry_parse(struct pkg_entry *entry);

tristate_t pkg_entry_get(const char *name, struct pkg_entry *entry);

tristate_t pkg_entry_for_each(
                     tristate_t (*cb)(const struct pkg_entry *entry, void *arg),
                     void *arg);

#endif
