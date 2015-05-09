#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "pkg.h"
#include "tar.h"
#include "flist.h"
#include "log.h"
#include "pkg_ops.h"

static bool delete_file(const char *path, void *arg)
{
	log_write(LOG_DEBUG, "removing %s\n", path);

	if (0 == unlink(path))
		return true;

	switch (errno) {
		case ENOENT:
			log_write(LOG_WARNING, "%s is missing\n", path);
			return true;

		case EISDIR:
			if (0 == rmdir(path))
				return true;
			if ((ENOTEMPTY == errno) || (EBUSY == errno) || (EROFS == errno))
				return true;
	}

	log_write(LOG_ERR, "failed to remove %s\n", path);
	return false;
}

static bool list_file(void *arg, const char *path)
{
	log_write(LOG_DEBUG, "extracting %s\n", path);
	return flist_add((struct flist *) arg, path);
}

bool pkg_install(const char *path,
                 const struct pkg_entry *entry,
                 const unsigned char *pub_key,
                 const bool strict)
{
	char dir[PATH_MAX];
	struct flist list;
	struct pkg pkg;
	int len;
	bool ret = false;

	log_write(LOG_DEBUG, "opening %s\n", entry->fname);
	if (false == pkg_open(&pkg, path))
		goto end;

	log_write(LOG_INFO, "verifying %s\n", entry->fname);
	if (false == pkg_verify(&pkg, pub_key, strict))
		goto close_pkg;

	len = snprintf(dir, sizeof(dir), INST_DATA_DIR"/%s", entry->name);
	if ((sizeof(dir) <= len) || (0 > len))
		goto close_pkg;

	log_write(LOG_DEBUG, "creating %s\n", dir);
	if (-1 == mkdir(dir, S_IRUSR | S_IWUSR)) {
		if (EEXIST != errno)
			goto close_pkg;
		log_write(LOG_WARNING, "%s already exists\n", dir);
	}

	log_write(LOG_DEBUG, "creating a file list for %s\n", entry->name);
	if (false == flist_open(&list, "w+", entry->name))
		goto close_pkg;

	log_write(LOG_DEBUG, "extracting %s\n", entry->fname);
	if (false == tar_extract(pkg.data,
	                         pkg.tar_size,
	                         list_file,
	                         (void *) &list))
		goto close_flist;

	/* upon failure to register the package, delete all the extracted files */
	log_write(LOG_INFO,
	          "registering %s as a %s package\n",
	          entry->name,
	          entry->reason);
	if (false == pkg_entry_register(entry)) {
		(void) flist_for_each_reverse(&list, delete_file, NULL);
		flist_close(&list);
		if (true == flist_delete(&list))
			(void) rmdir(dir);
		goto close_pkg;
	}

	log_write(LOG_INFO, "successfully installed %s\n", entry->name);
	ret = true;

close_flist:
	flist_close(&list);

close_pkg:
	pkg_close(&pkg);

end:
	return ret;
}

bool pkg_remove(const char *name)
{
	char dir[PATH_MAX];
	struct flist list;
	int len;
	bool ret = false;

	log_write(LOG_INFO, "removing %s\n", name);

	log_write(LOG_DEBUG, "opening the file list of %s\n", name);
	if (false == flist_open(&list, "r", name))
		goto end;

	log_write(LOG_DEBUG, "removing files installed by %s\n", name);
	if (TSTATE_OK != flist_for_each_reverse(&list, delete_file, NULL))
		goto close_list;

	log_write(LOG_DEBUG, "removing the file list of %s\n", name);
	if (false == flist_delete(&list))
		goto close_list;

	log_write(LOG_INFO, "unregistering %s\n", name);
	if (false == pkg_entry_unregister(name))
		goto close_list;

	len = snprintf(dir, sizeof(dir), INST_DATA_DIR"/%s", name);
	if ((sizeof(dir) <= len) || (0 > len))
		goto close_list;

	/* the file list must be closed before the directory is removed */
	flist_close(&list);
	log_write(LOG_DEBUG, "removing %s\n", dir);
	if (0 == rmdir(dir)) {
		log_write(LOG_INFO, "successfully removed %s\n", name);
		ret = true;
	}
	goto end;

close_list:
	flist_close(&list);

end:
	return ret;
}
