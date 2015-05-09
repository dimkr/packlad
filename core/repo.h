#ifndef _REPO_H_INCLUDED
#	define _REPO_H_INCLUDED

#	include <stdbool.h>

#	include <curl/curl.h>

#	define USER_AGENT "packlad/2"

struct repo {
	CURL *sess;
	const char *url;
};

bool repo_open(struct repo *repo, const char *url);
void repo_close(struct repo *repo);

bool repo_fetch(struct repo *repo, const char *url, const char *path);

#endif