#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "key.h"

bool key_read(const char *path, unsigned char *key, const size_t len)
{
	struct stat stbuf;
	int fd;
	bool ret = false;

	fd = open(path, O_RDONLY);
	if (-1 == fd)
		goto end;

	if (-1 == fstat(fd, &stbuf))
		goto close_key;
	if ((off_t) len != stbuf.st_size)
		goto close_key;

	if ((ssize_t) len == read(fd, (void *) key, len))
		ret = true;

close_key:
	(void) close(fd);

end:
	return ret;
}
