#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

#include "../core/pkg_entry.h"
#include "../core/lock.h"
#include "../core/log.h"

#include "../logic/install.h"
#include "../logic/remove.h"
#include "../logic/update.h"
#include "../logic/list.h"

#define USAGE "Usage: %s [-d] [-u] [-s] [-n] [-p PREFIX] [-u URL] " \
              "-l|-q|-c|-f|-i|-r PACKAGE\n"
#define REPO_ENVIRONMENT_VARIABLE "REPO"

enum actions {
	ACTION_INSTALL        = 0,
	ACTION_REMOVE         = 1,
	ACTION_LIST_INSTALLED = 2,
	ACTION_LIST_AVAILABLE = 3,
	ACTION_LIST_REMOVABLE = 4,
	ACTION_LIST_FILES     = 5,
	ACTION_UPDATE         = 6,
	ACTION_INVALID        = 7
};

__attribute__((noreturn)) static void show_help(const char *prog)
{
	(void) fprintf(stderr, USAGE, prog);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	struct lock_file lock;
	struct passwd *user;
	const char *root = "/";
	char *reason = INST_REASON_USER;
	const char *package = NULL;
	const char *url = NULL;
	int option;
	int action = ACTION_INVALID;
	bool check_sig = true;
	bool status = false;

	do {
		option = getopt(argc, argv, "dusnlqcf:u:i:r:p:");
		switch (option) {
			case 'd':
				log_set_min_level(LOG_DEBUG);
				break;

			case 'p':
				root = optarg;
				break;

			case 'u':
				action = ACTION_UPDATE;
				break;

			case 's':
				check_sig = false;
				break;

			case 'r':
				action = ACTION_REMOVE;
				package = optarg;
				break;

			case 'i':
				action = ACTION_INSTALL;
				package = optarg;
				break;

			case 'n':
				reason = INST_REASON_CORE;
				break;

			case 'q':
				action = ACTION_LIST_INSTALLED;
				break;

			case 'l':
				action = ACTION_LIST_AVAILABLE;
				break;

			case 'c':
				action = ACTION_LIST_REMOVABLE;
				break;

			case 'f':
				action = ACTION_LIST_FILES;
				package = optarg;
				break;

			case (-1):
				goto check_sanity;

			default:
				show_help(argv[0]);
		}
	} while (1);

check_sanity:
	switch (action) {
		case ACTION_REMOVE:
		case ACTION_LIST_FILES:
			if (NULL == package)
				show_help(argv[0]);
			break;

		case ACTION_INSTALL:
			if (NULL == package)
				show_help(argv[0]);

			/* fall-through */

		case ACTION_UPDATE:
		case ACTION_LIST_AVAILABLE:
			url = getenv(REPO_ENVIRONMENT_VARIABLE);
			if (NULL == url) {
				(void) fprintf(stderr,
				               "%s: REPO is not set.\n",
				               argv[0]);
				goto end;
			}
			break;

		case ACTION_INVALID:
			show_help(argv[0]);
	}

	if (0 != geteuid()) {
		user = getpwuid(0);
		if (NULL != user) {
			(void) fprintf(stderr,
			               "%s: must run as %s.\n",
			               argv[0],
			               user->pw_name);
		}
		goto end;
	}

	if (-1 == chdir(root)) {
		(void) fprintf(stderr,
		               "%s: cannot change the working directory to %s.\n",
		               argv[0],
		               root);
		goto end;
	}

	log_open();

	if (false == lock_file(&lock))
		goto close_log;

	switch (action) {
		case ACTION_INSTALL:
			status = packlad_install(package, url, reason, check_sig);
			break;

		case ACTION_REMOVE:
			status = packlad_remove(package);
			break;

		case ACTION_LIST_INSTALLED:
			status = packlad_list_inst();
			break;

		case ACTION_LIST_AVAILABLE:
			status = packlad_list_avail(url);
			break;

		case ACTION_LIST_REMOVABLE:
			status = packlad_list_removable();
			break;

		case ACTION_LIST_FILES:
			status = packlad_list_files(package);
			break;

		case ACTION_UPDATE:
			status = packlad_update(url);
			break;
	}

	unlock_file(&lock);

close_log:
	log_close();

end:
	if (false == status)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
