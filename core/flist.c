#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "pkg_entry.h"
#include "flist.h"

bool flist_open(struct flist *list,
                const char *mode,
                const char *name)
{
	int len;

	len = snprintf(list->path,
	               sizeof(list->path),
	               INST_DATA_DIR"/%s/files",
	               name);
	if ((sizeof(list->path) <= len) || (0 > len))
		return false;

	list->fh = fopen(list->path, mode);
	if (NULL == list->fh)
		return false;

	return true;
}

void flist_close(struct flist *list)
{
	(void) fclose(list->fh);
}

bool flist_add(struct flist *list, const char *path)
{
	if (0 > fprintf(list->fh, "%s\n", path))
		return false;

	return true;
}

bool flist_delete(struct flist *list)
{
	if (-1 == unlink(list->path))
		return false;

	return true;
}

tristate_t flist_for_each(struct flist *list,
                          bool (*cb)(const char *path, void *arg),
                          void *arg)
{
	char path[PATH_MAX];
	size_t len;
	tristate_t ret = TSTATE_FATAL;

	rewind(list->fh);

	do {
		if (NULL == fgets(path, sizeof(path), list->fh)) {
			if (0 != feof(list->fh))
				ret = TSTATE_OK;
			break;
		}

		len = strlen(path) - 1;
		if ('\n' == path[len])
			path[len] = '\0';

		if (false == cb(path, arg)) {
			ret = TSTATE_ERROR;
			break;
		}
	} while (1);

	return ret;
}

static bool append_line(const char *path, void *arg)
{
	struct flines *lines = (struct flines *) arg;
	struct flines nlines;

	nlines.count = 1 + lines->count;
	nlines.lines = realloc(lines->lines, sizeof(char *) * nlines.count);
	if (NULL == nlines.lines) {
		if (NULL != lines->lines)
			free(lines->lines);
		return false;
	}

	nlines.lines[lines->count] = strdup(path);
	lines->lines = nlines.lines;
	if (NULL == nlines.lines[lines->count])
		return false;
	lines->count = nlines.count;

	return true;
}

tristate_t flist_for_each_reverse(struct flist *list,
                                  bool (*cb)(const char *path, void *arg),
                                  void *arg)
{
	struct flines lines;
	int i;
	tristate_t ret = TSTATE_FATAL;

	lines.lines = NULL;
	lines.count = 0;

	if (TSTATE_OK != flist_for_each(list, append_line, (void *) &lines))
		goto end;

	for (i = lines.count - 1; 0 <= i; --i) {
		if (false == cb(lines.lines[i], arg)) {
			ret = TSTATE_ERROR;
			goto free_lines;
		}
	}

	ret = TSTATE_OK;

free_lines:
	for (i = 0; lines.count > i; ++i)
		free(lines.lines[i]);
	if (NULL != lines.lines)
		free(lines.lines);

end:
	return ret;
}
