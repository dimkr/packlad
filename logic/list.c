#include <stdio.h>
#include <string.h>

#include "../core/pkg_list.h"
#include "../core/pkg_entry.h"
#include "../core/dep.h"
#include "../core/flist.h"
#include "../core/log.h"

#include "update.h"
#include "list.h"

static bool list_entry(const struct pkg_entry *entry)
{
	if (0 > printf("%s|%s|%s\n", entry->name, entry->ver, entry->desc))
		return false;

	return true;
}

static bool list_if_not_installed(const struct pkg_entry *entry)
{
	struct pkg_entry temp;

	switch (pkg_entry_get(entry->name, &temp)) {
		case TSTATE_OK:
			return true;

		case TSTATE_ERROR:
			return list_entry(entry);
	}

	return false;
}

bool packlad_list_avail(const char *url)
{
	struct pkg_list list;
	int level;
	bool ret = false;

	level = log_get_min_level();
	log_set_min_level(LOG_ERR);

	switch (pkg_list_open(&list)) {
		case TSTATE_ERROR:
			if (true == packlad_update(url)) {
				if (TSTATE_OK == pkg_list_open(&list))
					break;
			}

		case TSTATE_FATAL:
			goto end;
	}

	log_set_min_level(level);

	if (TSTATE_OK == pkg_list_for_each(&list, list_if_not_installed))
		ret = true;

	pkg_list_close(&list);

end:
	return ret;
}

static tristate_t list_inst_entry(const struct pkg_entry *entry, void *arg)
{
	if (false == list_entry(entry))
		return TSTATE_FATAL;

	return TSTATE_OK;
}

bool packlad_list_inst(void)
{
	return pkg_entry_for_each(list_inst_entry, NULL);
}

static tristate_t list_removable(const struct pkg_entry *entry, void *arg)
{
	if (0 != strcmp(INST_REASON_USER, entry->reason))
		return TSTATE_OK;

	if (TSTATE_OK != pkg_not_required(entry->name))
		return TSTATE_OK;

	if (true == list_entry(entry))
		return TSTATE_OK;

	return TSTATE_FATAL;
}

bool packlad_list_removable(void)
{
	return pkg_entry_for_each(list_removable, NULL);
}

static bool list_file(const char *path, void *arg)
{
	if (0 > printf("%s\n", path))
		return false;

	return true;
}

bool packlad_list_files(const char *name)
{
	struct flist list;
	bool ret = false;

	if (false == flist_open(&list, "r", name)) {
		log_write(LOG_ERR, "%s is not installed\n", name);
		goto end;
	}

	ret = flist_for_each(&list, list_file, NULL);

	flist_close(&list);

end:
	return ret;
}