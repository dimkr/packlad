#ifndef _TAR_H_INCLUDED
#	define _TAR_H_INCLUDED

#	include <stdbool.h>
#	include <sys/types.h>

#	include <archive_entry.h>

typedef bool (*tar_cb_t)(void *arg, const char *path);

bool tar_extract(unsigned char *data,
                 const size_t len,
                 const tar_cb_t cb,
                 void *arg);

#endif
