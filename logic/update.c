#include "../core/repo.h"
#include "../core/pkg_list.h"
#include "../core/log.h"

bool packlad_update(const char *url)
{
	struct repo repo;
	bool ret = false;

	log_write(LOG_INFO, "updating the package list\n");

	if (false == repo_open(&repo, url))
		goto end;

	ret = repo_fetch(&repo, PKG_LIST_FILE_NAME, PKG_LIST_PATH);
	if (true == ret)
		log_write(LOG_INFO, "successfully updated the package list\n");

	repo_close(&repo);

end:
	return ret;
}
