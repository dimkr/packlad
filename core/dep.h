#ifndef _DEP_H_INCLUDED
#	define _DEP_H_INCLUDED

#	include <stdbool.h>

#	include "types.h"
#	include "pkg_queue.h"
#	include "pkg_list.h"

struct dep_queue_params {
	struct pkg_queue *queue;
	struct pkg_list *list;
};

struct unneeded_iter_params {
	bool (*cb)(const struct pkg_entry *entry, void *arg);
	void *arg;
	unsigned int removed;
};

bool dep_queue(struct pkg_queue *queue,
               struct pkg_list *list,
               const char *name);

tristate_t pkg_not_required(const char *name);

tristate_t for_each_unneeded_pkg(bool (*cb)(const struct pkg_entry *entry,
                                            void *arg),
                                 void *arg);

#endif