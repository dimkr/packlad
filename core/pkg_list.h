#ifndef _PKG_LIST_H_INCLUDED
#	define _PKG_LIST_H_INCLUDED

#	include <stdio.h>
#	include <stdbool.h>

#	include "types.h"
#	include "pkg_entry.h"

struct pkg_list {
	FILE *fh;
};

tristate_t pkg_list_open(struct pkg_list *list);
void pkg_list_close(struct pkg_list *list);

tristate_t pkg_list_get(struct pkg_list *list,
                        struct pkg_entry *entry,
                        const char *name);

tristate_t pkg_list_for_each(struct pkg_list *list,
                             bool (*cb)(const struct pkg_entry *entry));

#endif
