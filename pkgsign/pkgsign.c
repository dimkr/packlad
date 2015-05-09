#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <ed25519.h>

static int read_key(const char *path, unsigned char *key, const size_t len)
{
	struct stat stbuf;
	int fd;
	int ret = 1;

	fd = open(path, O_RDONLY);
	if (-1 == fd)
		goto end;

	if (-1 == fstat(fd, &stbuf))
		goto close_key;
	if ((off_t) len != stbuf.st_size)
		goto close_key;

	if ((ssize_t) len == read(fd, (void *) key, len))
		ret = 0;

close_key:
	(void) close(fd);

end:
	return ret;
}

int main(int argc, char *argv[])
{
	struct stat stbuf;
	unsigned char priv_key[64];
	unsigned char sig[64];
	void *data;
	size_t size;
	unsigned char pub_key[32];
	int fd;
	int ret = EXIT_FAILURE;

	if (4 != argc) {
		(void) printf("Usage: %s PRIVATE PUBLIC FILE\n", argv[0]);
		goto end;
	}

	if (0 != read_key(argv[1], priv_key, sizeof(priv_key)))
		goto end;

	if (0 != read_key(argv[2], pub_key, sizeof(pub_key)))
		goto end;

	fd = open(argv[3], O_RDONLY);
	if (-1 == fd)
		goto end;
	if (-1 == fstat(fd, &stbuf))
		goto close_file;

	size = (size_t) stbuf.st_size;
	data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (MAP_FAILED == data)
		goto close_file;

	ed25519_sign(sig, data, size, pub_key, priv_key);

	if (sizeof(sig) == write(STDOUT_FILENO, (void *) sig, sizeof(sig)))
		ret = EXIT_SUCCESS;

	(void) munmap(data, size);

close_file:
	(void) close(fd);

end:
	return ret;
}