#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../core/pkg_list.h"
#include "../core/pkg_entry.h"
#include "../core/pkg_queue.h"
#include "../core/repo.h"
#include "../core/dep.h"
#include "../core/pkg_ops.h"
#include "../core/key.h"
#include "../core/log.h"

#include "cleanup.h"
#include "install.h"

bool packlad_install(const char *name,
                     const char *url,
                     const char *reason,
                     const bool check_sig)
{
	char path[PATH_MAX];
	unsigned char pub_key[32];
	struct pkg_list list;
	struct repo repo;
	struct pkg_queue q;
	struct pkg_entry *entry;
	const unsigned char *keyp;
	unsigned int count;
	int len;
	bool ret = false;
	bool error = false;

	if (false == pkg_list_open(&list))
		goto end;

	if (false == key_read(PUB_KEY_PATH, pub_key, sizeof(pub_key))) {
		log_write(LOG_ERR, "Failed to read the public key\n");
		goto end;
	}

	log_write(LOG_INFO, "Building the package queue\n");

	pkg_queue_init(&q);
	if (false == dep_queue(&q, &list, name))
		goto close_list;

	count = pkg_queue_length(&q);
	if (0 == count) {
		ret = true;
		goto close_list;
	}

	if (false == repo_open(&repo, url))
		goto empty_queue;

	switch (count) {
		case 0:
			goto close_repo;

		case 1:
			break;

		default:
			log_write(LOG_INFO,
			          "Processing the package queue (%u packages)\n",
			          count);
	}

	if (true == check_sig)
		keyp = pub_key;
	else
		keyp = NULL;

	do {
		entry = pkg_queue_pop(&q);
		if (NULL == entry) {
			ret = true;
			goto free_entry;
		}

		len = snprintf(path, sizeof(path), PKG_ARCHIVE_DIR"/%s", entry->fname);
		if ((sizeof(path) <= len) || (0 > len)) {
			error = true;
			goto free_entry;
		}

		if (false == repo_fetch(&repo, entry->fname, path)) {
			error = true;
			goto free_entry;
		}

		if (0 == strcmp(entry->name, name))
			entry->reason = (char *) reason;
		else
			entry->reason = (char *) INST_REASON_DEP;
		if (false == pkg_install(path, entry, keyp)) {
			log_write(LOG_ERR, "Cannot install %s\n", name);
			error = true;
			goto free_entry;
		}

free_entry:
		free(entry);
	} while ((false == ret) && (false == error));

	(void) packlad_cleanup();

close_repo:
	repo_close(&repo);

empty_queue:
	pkg_queue_empty(&q);

close_list:
	pkg_list_close(&list);

end:
	return ret;
}
