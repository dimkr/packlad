#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "lock.h"

bool lock_file(struct lock_file *lock)
{
	lock->fd = open(LOCK_FILE_PATH,
	                O_WRONLY | O_CREAT,
	                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (-1 == lock->fd) {
		log_write(LOG_ERR, "failed to open the lock file\n");
		goto error;
	}

	if (-1 == lockf(lock->fd, F_TEST, 0)) {
		switch (errno) {
			case EAGAIN:
			case EACCES:
				log_write(LOG_WARNING,
				          "Another instance is running; waiting\n");
				break;

			default:
				goto close_lock;
		}
	}

	if (-1 == lockf(lock->fd, F_LOCK, 0)) {
		log_write(LOG_ERR, "failed to lock the lock file\n");
		goto close_lock;
	}

	return true;

close_lock:
	(void) close(lock->fd);

error:
	return false;
}

void unlock_file(struct lock_file *lock)
{
	(void) lockf(lock->fd, F_LOCK, 0);
	(void) close(lock->fd);
	(void) unlink(LOCK_FILE_PATH);
}
