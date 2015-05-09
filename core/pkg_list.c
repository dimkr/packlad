#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "pkg_list.h"

tristate_t pkg_list_open(struct pkg_list *list)
{
	list->fh = fopen(PKG_LIST_PATH, "r");
	if (NULL != list->fh)
		return TSTATE_OK;

	if (ENOENT == errno) {
		log_write(LOG_WARNING, "the package list does not exist\n");
		return TSTATE_ERROR;
	}

	log_write(LOG_ERR, "failed to open the package list\n");
	return TSTATE_FATAL;
}

void pkg_list_close(struct pkg_list *list)
{
	(void) fclose(list->fh);
}

tristate_t pkg_list_get(struct pkg_list *list,
                        struct pkg_entry *entry,
                        const char *name)
{
	tristate_t ret = TSTATE_FATAL;

	log_write(LOG_DEBUG, "searching the package list for %s\n", name);

	rewind(list->fh);

	do {
		if (NULL == fgets(entry->buf, sizeof(entry->buf), list->fh)) {
			if (0 != feof(list->fh)) {
				ret = TSTATE_ERROR;
				break;
			}
			else {
				log_write(LOG_DEBUG, "failed to read the package list\n");
				break;
			}
		}

		if (false == pkg_entry_parse(entry)) {
			log_write(LOG_DEBUG, "failed to parse the package list\n");
			break;
		}

		if (0 == strcmp(name, entry->name)) {
			ret = TSTATE_OK;
			break;
		}
	} while (1);

	switch (ret) {
		case TSTATE_OK:
			log_write(LOG_DEBUG, "found %s, version %s\n", name, entry->ver);
			break;

		case TSTATE_FATAL:
			log_write(LOG_DEBUG, "failed to read the package list\n", name);
			break;

		case TSTATE_ERROR:
			log_write(LOG_ERR,
			          "could not find %s in the package list\n",
			          name);
	}

	return ret;
}

tristate_t pkg_list_for_each(struct pkg_list *list,
                             bool (*cb)(const struct pkg_entry *entry))
{
	struct pkg_entry entry;

	rewind(list->fh);

	do {
		if (NULL == fgets(entry.buf, sizeof(entry.buf), list->fh)) {
			if (0 != feof(list->fh))
				break;
			else {
				log_write(LOG_ERR, "failed to read the package list\n");
				return TSTATE_FATAL;
			}
		}

		if (false == pkg_entry_parse(&entry)) {
			log_write(LOG_ERR, "failed to parse the package list\n");
			return TSTATE_FATAL;
		}

		if (false == cb(&entry))
			return TSTATE_ERROR;
	} while (1);

	return TSTATE_OK;
}
