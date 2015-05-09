#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "repo.h"

static size_t append_to_file(const void *ptr,
                             size_t size,
                             size_t nmemb,
                             void *stream)
{
	return fwrite(ptr, size, nmemb, (FILE *) stream);
}

bool repo_open(struct repo *repo, const char *url)
{
	bool ret = false;

	if (0 != curl_global_init(CURL_GLOBAL_NOTHING))
		goto end;

	repo->sess = curl_easy_init();
	if (NULL == repo->sess)
		goto cleanup;

	if (CURLE_OK != curl_easy_setopt(repo->sess,
	                                 CURLOPT_USERAGENT,
	                                 USER_AGENT))
		goto cleanup;
	if (CURLE_OK != curl_easy_setopt(repo->sess,
	                                 CURLOPT_TCP_NODELAY,
	                                 1))
		goto cleanup;
	if (CURLE_OK != curl_easy_setopt(repo->sess,
	                                 CURLOPT_FAILONERROR,
	                                 1))
		goto cleanup;

	if (CURLE_OK != curl_easy_setopt(repo->sess,
	                                 CURLOPT_WRITEFUNCTION,
	                                 append_to_file))
		goto cleanup;

	repo->url = url;
	ret = true;

cleanup:
	curl_global_cleanup();

end:
	return ret;
}

void repo_close(struct repo *repo)
{
	curl_easy_cleanup(repo->sess);
	curl_global_cleanup();
}

bool repo_fetch(struct repo *repo, const char *url, const char *path)
{
	char abs_url[1024];
	int len;
	FILE *fh;
	int ret = false;

	len = snprintf(abs_url, sizeof(abs_url), "%s/%s", repo->url, url);
	if ((sizeof(abs_url) <= len) || (0 > len))
		goto end;

	log_write(LOG_INFO, "fetching %s\n", abs_url);

	fh = fopen(path, "wb");
	if (NULL == fh)
		goto end;

	if (CURLE_OK != curl_easy_setopt(repo->sess, CURLOPT_URL, abs_url))
		goto delete_file;
	if (CURLE_OK != curl_easy_setopt(repo->sess, CURLOPT_WRITEDATA, fh))
		goto delete_file;

	if (CURLE_OK == curl_easy_perform(repo->sess)) {
		ret = true;
		goto close_file;
	}

delete_file:
	(void) unlink(path);

close_file:
	(void) fclose(fh);

end:
	if (false == ret)
		log_write(LOG_ERR, "failed to fetch %s\n", abs_url);

	return ret;
}
