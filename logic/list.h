#ifndef _LIST_H_INCLUDED
#	define _LIST_H_INCLUDED

#	include <stdbool.h>

bool packlad_list_avail(const char *url);
bool packlad_list_inst(void);
bool packlad_list_removable(void);

bool packlad_list_files(const char *name);

#endif
