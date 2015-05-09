#ifndef _LOG_H_INCLUDED
#	define _LOG_H_INCLUDED

#	include <syslog.h>

#	define MAX_FMT_LEN (512)

void log_open(void);
void log_write(const int level, const char *format, ...);
void log_close(void);

int log_get_min_level(void);
void log_set_min_level(const int level);

#endif
