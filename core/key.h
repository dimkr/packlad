#ifndef _KEY_H_INCLUDED
#	define _KEY_H_INCLUDED

#	include <stdbool.h>
#	include <sys/types.h>

bool key_read(const char *path, unsigned char *key, const size_t len);

#endif