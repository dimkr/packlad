#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "log.h"
#include "dep.h"

static tristate_t for_each_dep(const char *deps,
                               tristate_t (*cb)(const char *dep, void *arg),
                               void *arg)
{
	char *copy;
	char *dep;
	char *pos;
	tristate_t ret = TSTATE_FATAL;

	if (0 == strlen(deps)) {
		ret = TSTATE_OK;
		goto end;
	}

	copy = strdup(deps);
	if (NULL == copy)
		goto end;

	dep = strtok_r(copy, " ", &pos);
	while (NULL != dep) {
		ret = cb(dep, arg);
		if (TSTATE_OK != ret)
			goto free_copy;
		dep = strtok_r(NULL, " ", &pos);
	}

	ret = TSTATE_OK;

free_copy:
	free(copy);

end:
	return ret;
}

static tristate_t dep_recurse(const char *dep, void *arg)
{
	struct dep_queue_params *params = (struct dep_queue_params *) arg;

	if (true == dep_queue(params->queue, params->list, dep))
		return TSTATE_OK;

	return TSTATE_FATAL;
}

bool dep_queue(struct pkg_queue *queue,
               struct pkg_list *list,
               const char *name)
{
	struct pkg_entry *entry;
	struct dep_queue_params params;
	bool ret = false;

	if (true == pkg_queue_contains(queue, name)) {
		log_write(LOG_DEBUG, "%s is already queued for installation\n", name);
		ret = true;
		goto end;
	}

	if (UINT_MAX == pkg_queue_length(queue))
		goto end;

	entry = (struct pkg_entry *) malloc(sizeof(struct pkg_entry));
	if (NULL == entry)
		goto end;

	switch (pkg_entry_get(name, entry)) {
		case TSTATE_OK:
			log_write(LOG_WARNING, "%s is already installed\n", name);
			ret = true;
			goto free_entry;

		case TSTATE_FATAL:
			log_write(LOG_ERR,
			          "failed to check whether %s is already installed\n",
			          name);
			goto free_entry;
	}

	if (TSTATE_OK != pkg_list_get(list, entry, name))
		goto free_entry;

	log_write(LOG_DEBUG, "queueing %s for installation\n", entry->name);
	if (false == pkg_queue_push(queue, entry))
		goto empty_queue;

	log_write(LOG_DEBUG, "resolving the dependencies of %s\n", entry->name);
	params.queue = queue;
	params.list = list;
	if (TSTATE_OK == for_each_dep(entry->deps, dep_recurse, (void *) &params)) {
		ret = true;
		goto end;
	}

empty_queue:
	pkg_queue_empty(queue);

free_entry:
	free(entry);

end:
	return ret;
}

static tristate_t dep_cmp(const char *dep, void *arg)
{
	if (0 == strcmp(dep, (const char *) arg))
		return TSTATE_ERROR;

	return TSTATE_OK;
}

static tristate_t depends_on(const struct pkg_entry *entry, void *arg)
{
	tristate_t ret = TSTATE_OK;

	log_write(LOG_DEBUG,
	          "Checking whether %s depends on %s\n",
	          entry->name,
	          (const char *) arg);

	/* skip the package itself */
	if (0 == strcmp(entry->name, (const char *) arg))
		goto end;

	ret = for_each_dep(entry->deps, dep_cmp, arg);
	if (TSTATE_ERROR == ret) {
		log_write(LOG_DEBUG,
		          "%s depends on %s\n",
		          entry->name,
		          (const char *) arg);
	}

end:
	return ret;
}

tristate_t pkg_not_required(const char *name)
{
	return pkg_entry_for_each(depends_on, (void *) name);
}

static tristate_t if_unneeded(const struct pkg_entry *entry, void *arg)
{
	struct unneeded_iter_params *params = (struct unneeded_iter_params *) arg;

	/* ignore all packages but those installed as dependencies */
	if (0 != strcmp(INST_REASON_DEP, entry->reason)) {
		log_write(LOG_DEBUG,
		          "%s cannot be removed; it is a %s package\n",
		          entry->name,
		          entry->reason);
		return TSTATE_OK;
	}

	switch (pkg_not_required(entry->name)) {
		case TSTATE_ERROR:
			log_write(LOG_DEBUG,
			          "cannot remove %s; another package depends on it\n",
			          entry->name);
			return TSTATE_OK;

		case TSTATE_FATAL:
			log_write(
			     LOG_ERR,
			     "cannot determine whether %s is required by another package\n",
			     entry->name);
			return TSTATE_FATAL;
	}

	log_write(LOG_DEBUG, "%s is no longer required\n", entry->name);
	if (false == params->cb(entry, params->arg))
		return TSTATE_ERROR;

	++params->removed;
	return TSTATE_OK;
}

tristate_t for_each_unneeded_pkg(bool (*cb)(const struct pkg_entry *entry,
                                            void *arg),
                                 void *arg)
{
	struct unneeded_iter_params params;
	unsigned int total;
	tristate_t ret;

	log_write(LOG_DEBUG, "looking for unneeded packages\n");

	params.cb = cb;
	params.arg = arg;
	total = 0;

	do {
		params.removed = 0;
		ret = pkg_entry_for_each(if_unneeded, (void *) &params);
		if (TSTATE_OK != ret)
			return ret;

		if (0 == params.removed)
			break;

		total += params.removed;
	} while (1);

	if (0 == total)
		log_write(LOG_DEBUG, "there are no unneeded packages\n");

	return TSTATE_OK;
}
