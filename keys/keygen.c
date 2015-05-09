#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ed25519.h>

static bool write_key(const char *path,
                      const unsigned char *key,
                      const size_t len)
{
	int fd;
	bool ret = false;

	fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (-1 == fd)
		goto end;

	if ((ssize_t) len == write(fd, (void *) key, len))
		ret = true;

	(void) close(fd);

end:
	return ret;
}

int main(int argc, char *argv[])
{
	unsigned char seed[32];
	unsigned char pub_key[32];
	unsigned char priv_key[64];

	if (3 != argc)
		goto error;

	if (0 != ed25519_create_seed(seed))
		goto error;

	ed25519_create_keypair(pub_key, priv_key, seed);

	if (false == write_key(argv[1], pub_key, sizeof(pub_key)))
		goto error;
	if (false == write_key(argv[2], priv_key, sizeof(priv_key)))
		goto error;

	return EXIT_SUCCESS;

error:
	return EXIT_FAILURE;
}