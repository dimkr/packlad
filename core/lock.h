#ifndef _LOCK_H_INCLUDED
#	define _LOCK_H_INCLUDED

#	include <stdbool.h>

struct lock_file {
	int fd;
};

bool lock_file(struct lock_file *lock);
void unlock_file(struct lock_file *lock);

#endif