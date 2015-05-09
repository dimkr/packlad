#ifndef _PKG_H_INCLUDED
#	define _PKG_H_INCLUDED

#	include <stdint.h>
#	include <sys/types.h>
#	include <stdbool.h>

/* convenience - O_RDONLY and O_WRONLY */
#	include <fcntl.h>

#	define PKG_MAGIC ((uint32_t) 0x686a6b6c)

struct pkg_header {
	uint32_t magic;
	unsigned char sig[64];
} __attribute__((packed));

struct pkg {
	void *data;
	struct pkg_header *hdr;
	size_t size;
	size_t tar_size;
	int fd;
};

bool pkg_open(struct pkg *pkg, const char *path);
bool pkg_verify(const struct pkg *pkg,
                const unsigned char *pub_key,
                const bool strict);
void pkg_close(struct pkg *pkg);

#endif
