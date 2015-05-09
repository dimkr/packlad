#include <string.h>

#include "../core/pkg_entry.h"
#include "../core/dep.h"
#include "../core/pkg_ops.h"
#include "../core/log.h"

#include "cleanup.h"
#include "remove.h"

bool packlad_remove(const char *name)
{
	struct pkg_entry entry;
	bool ret = false;

	switch (pkg_entry_get(name, &entry)) {
		case TSTATE_ERROR:
			log_write(LOG_WARNING, "%s is not installed\n", name);
			ret = true;
			goto end;

		case TSTATE_FATAL:
			log_write(LOG_ERR,
			          "could not check whether %s is installed\n",
			          name);
			goto end;
	}

	if (0 != strcmp(INST_REASON_USER, entry.reason)) {
		log_write(LOG_ERR,
		          "%s cannot be removed; it is a %s package\n",
		          name,
		          entry.reason);
		goto end;
	}

	if (TSTATE_OK != pkg_not_required(name)) {
		log_write(LOG_ERR, "%s cannot be removed\n", name);
		goto end;
	}

	if (false == pkg_remove(name))
		goto end;

	(void) packlad_cleanup();

	ret = true;

end:
	return ret;
}
