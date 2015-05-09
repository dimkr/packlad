#include <string.h>
#include <stddef.h>
#include <limits.h>

#include <archive.h>

#include "log.h"
#include "tar.h"

#define EXTRACTION_OPTIONS (ARCHIVE_EXTRACT_OWNER | \
                            ARCHIVE_EXTRACT_PERM | \
                            ARCHIVE_EXTRACT_TIME | \
                            ARCHIVE_EXTRACT_UNLINK | \
                            ARCHIVE_EXTRACT_ACL | \
                            ARCHIVE_EXTRACT_FFLAGS | \
                            ARCHIVE_EXTRACT_XATTR)

static bool extract_file(struct archive *in, struct archive *out)
{
	__LA_INT64_T off;
	size_t size;
	const void *blk;

	do {
		switch (archive_read_data_block(in, &blk, &size, &off)) {
			case ARCHIVE_OK:
				if (ARCHIVE_OK != archive_write_data_block(out, blk, size, off))
					return false;
				break;

			case ARCHIVE_EOF:
				return true;

			default:
				log_write(LOG_ERR, "the archive extraction failed\n");
				return false;
		}
	} while (1);

	return false;
}

bool tar_extract(unsigned char *data,
                 const size_t len,
                 const tar_cb_t cb,
                 void *arg)
{
	struct archive *in;
	struct archive *out;
	struct archive_entry *entry;
	const char *path;
	bool ret = false;

	in = archive_read_new();
	if (NULL == in)
		goto end;

	out = archive_write_disk_new();
	if (NULL == out)
		goto close_input;

	archive_read_support_filter_lzip(in);
	archive_read_support_filter_xz(in);
	archive_read_support_filter_gzip(in);
	archive_read_support_format_tar(in);

	archive_write_disk_set_options(out, EXTRACTION_OPTIONS);
	archive_write_disk_set_standard_lookup(out);

	if (0 != archive_read_open_memory(in, data, len))
		goto close_output;

	do {
		switch (archive_read_next_header(in, &entry)) {
			case ARCHIVE_OK:
				break;

			case ARCHIVE_EOF:
				ret = true;
				goto close_output;

			default:
				log_write(LOG_ERR, "failed to read the archive\n");
				goto close_output;
		}

		path = archive_entry_pathname(entry);
		if (NULL == path)
			break;

		/* make sure all paths begin with "./" */
		if (0 != strncmp("./", path, 2))
			break;

		/* ignore the root directory */
		if (0 == strcmp("./", path))
			continue;

		if (false == cb(arg, path))
			break;
		if (ARCHIVE_OK != archive_write_header(out, entry)) {
			log_write(LOG_ERR, "failed to read a file header\n");
			break;
		}
		if (false == extract_file(in, out))
			break;
	} while (1);

close_output:
	(void) archive_write_close(out);
	archive_write_free(out);

close_input:
	archive_read_free(in);

end:
	return ret;
}
