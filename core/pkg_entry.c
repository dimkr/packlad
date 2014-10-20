#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#ifndef _BSD_SOURCE
#	define _BSD_SOURCE /* for struct dirent.d_type */
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "log.h"
#include "pkg_entry.h"

bool pkg_entry_register(const struct pkg_entry *entry, const char *root)
{
	char path[PATH_MAX];
	FILE *fh;
	bool ret = false;

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, entry->name);

	fh = fopen(path, "w");
	if (NULL == fh)
		goto end;

	if (0 < fprintf(fh,
	                "%s|%s|%s|%s|%s|%s|%s\n",
	                entry->name,
	                entry->ver,
	                entry->desc,
	                entry->fname,
	                entry->arch,
	                entry->deps,
	                entry->reason))
		ret = true;

	(void) fclose(fh);

end:
	return ret;
}

bool pkg_entry_unregister(const char *name, const char *root)
{
	char path[PATH_MAX];

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, name);
	if (-1 == unlink(path))
		return false;

	return true;
}

bool pkg_entry_parse(struct pkg_entry *entry)
{
	char *last;
	ssize_t len;

	entry->name = entry->buf;

	entry->ver = strchr(entry->name, '|');
	if (NULL == entry->ver) {
		log_write(LOG_ERR, "The package entry lacks a version field\n");
		goto invalid;
	}
	entry->ver[0] = '\0';
	++entry->ver;

	entry->desc = strchr(entry->ver, '|');
	if (NULL == entry->desc) {
		log_write(LOG_ERR, "The package entry lacks a description field\n");
		goto invalid;
	}
	entry->desc[0] = '\0';
	++entry->desc;

	entry->fname = strchr(entry->desc, '|');
	if (NULL == entry->fname) {
		log_write(LOG_ERR, "The package entry lacks a file name field\n");
		goto invalid;
	}
	entry->fname[0] = '\0';
	++entry->fname;

	entry->arch = strchr(entry->fname, '|');
	if (NULL == entry->arch) {
		log_write(LOG_ERR, "The package entry lacks an architecture field\n");
		goto invalid;
	}
	entry->arch[0] = '\0';
	++entry->arch;

	entry->deps = strchr(entry->arch, '|');
	if (NULL == entry->deps) {
		log_write(LOG_ERR, "The package entry lacks a dependencies field\n");
		goto invalid;
	}
	entry->deps[0] = '\0';
	++entry->deps;

	entry->reason = strchr(entry->deps, '|');
	if (NULL == entry->reason)
		last = entry->deps;
	else {
		entry->reason[0] = '\0';
		++entry->reason;
		last = entry->reason;
	}

	len = (ssize_t) strlen(last) - 1;
	if ('\n' == last[len])
		last[len] = '\0';

	return true;

invalid:
	return false;
}

tristate_t pkg_entry_get(const char *name,
                         struct pkg_entry *entry,
                         const char *root)
{
	char path[PATH_MAX];
	ssize_t len;
	int fd;
	tristate_t ret = TSTATE_FATAL;

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, name);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		if (ENOENT == errno)
			ret = TSTATE_ERROR;
		goto end;
	}

	len = read(fd, (void *) entry->buf, sizeof(entry->buf) - 1);
	if (0 >= len)
		goto close_file;

	entry->buf[len] = '\0';
	if (true == pkg_entry_parse(entry))
		ret = TSTATE_OK;

close_file:
	(void) close(fd);

end:
	return ret;
}

tristate_t pkg_entry_for_each(
	                 const char *root,
                     tristate_t (*cb)(const struct pkg_entry *entry, void *arg),
                     void *arg)
{
	char path[PATH_MAX];
	struct dirent name;
	struct pkg_entry entry;
	struct dirent *namep;
	DIR *dir;
	tristate_t ret = TSTATE_FATAL;
	tristate_t cb_ret;

	(void) sprintf(path, "%s"INST_DATA_DIR, root);

	dir = opendir(path);
	if (NULL == dir)
		goto end;

	do {
		if (0 != readdir_r(dir, &name, &namep))
			break;
		if (NULL == namep) {
			ret = TSTATE_OK;
			break;
		}

		/* ignore directories, relative paths and hidden files */
		if (DT_DIR != namep->d_type)
			continue;
		if ('.' == namep->d_name[0])
			continue;

		switch (pkg_entry_get(namep->d_name, &entry, root)) {
			/* if the package entry is missing, skip it */
			case TSTATE_ERROR:
				continue;

			case TSTATE_FATAL:
				goto close_dir;
		}

		cb_ret = cb(&entry, arg);
		if (TSTATE_OK != cb_ret) {
			ret = cb_ret;
			break;
		}
	} while (1);

close_dir:
	(void) closedir(dir);

end:
	return ret;
}
