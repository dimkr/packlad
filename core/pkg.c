#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include "../ed25519/ed25519.h"

#include "log.h"
#include "pkg.h"

bool pkg_open(struct pkg *pkg, const char *path)
{
	struct stat stbuf;

	if (-1 == stat(path, &stbuf))
		goto error;

	pkg->fd = open(path, O_RDONLY);
	if (-1 == pkg->fd)
		goto error;

	pkg->size = (size_t) stbuf.st_size;
	if (sizeof(struct pkg_header) >= pkg->size)
		goto close_pkg;

	pkg->data = mmap(NULL, pkg->size, PROT_READ, MAP_PRIVATE, pkg->fd, 0);
	if (MAP_FAILED == pkg->data)
		goto close_pkg;

	pkg->tar_size = pkg->size - sizeof(struct pkg_header);
	pkg->hdr = (struct pkg_header *) ((unsigned char *) pkg->data +
	                                  pkg->tar_size);

	return true;

	(void) munmap(pkg->data, pkg->size);

close_pkg:
	(void) close(pkg->fd);

error:
	return false;
}

bool pkg_verify(struct pkg *pkg, const unsigned char *pub_key)
{
	if (PKG_MAGIC != ntohl(pkg->hdr->magic)) {
		log_write(LOG_ERR, "The package is invalid\n");
		return false;
	}

	if (NULL == pub_key) {
		log_write(LOG_WARN, "Skipping the digital signature verification\n");
		return true;
	}

	if (1 != ed25519_verify(pkg->hdr->sig, pkg->data, pkg->tar_size, pub_key)) {
		log_write(LOG_ERR, "The package digital signature is invalid\n");
		return false;
	}

	return true;
}

void pkg_close(struct pkg *pkg)
{
	(void) munmap(pkg->data, pkg->size);
	(void) close(pkg->fd);
}