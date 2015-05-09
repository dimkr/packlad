#ifndef _INSTALL_H_INCLUDED
#	define _INSTALL_H_INCLUDED

#	include <stdbool.h>

bool packlad_install(const char *name,
                     const char *url,
                     const char *reason,
                     const bool check_sig);

#endif