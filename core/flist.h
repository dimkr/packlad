#ifndef _FLIST_H_INCLUDED
#	define _FLIST_H_INCLUDED

#	include <limits.h>
#	include <stdio.h>
#	include <stdbool.h>

#	include "types.h"

struct flist {
	char path[PATH_MAX];
	FILE *fh;
};

struct flines {
	char **lines;
	unsigned int count;
};

typedef tristate_t (*flist_iter_t)(struct flist *list,
                                   bool (*cb)(const char *path, void *arg),
                                   void *arg);

bool flist_open(struct flist *list,
                const char *mode,
                const char *name);
void flist_close(struct flist *list);

bool flist_add(struct flist *list, const char *path);

bool flist_delete(struct flist *list);

tristate_t flist_for_each(struct flist *list,
                          bool (*cb)(const char *path, void *arg),
                          void *arg);

tristate_t flist_for_each_reverse(struct flist *list,
                                  bool (*cb)(const char *path, void *arg),
                                  void *arg);

#endif