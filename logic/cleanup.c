#include "../core/pkg_ops.h"
#include "../core/dep.h"

#include "cleanup.h"

static bool remove_unneeded(const struct pkg_entry *entry, void *arg)
{
	return pkg_remove(entry->name);
}

bool packlad_cleanup(void)
{
	return for_each_unneeded_pkg(remove_unneeded, NULL);
}
