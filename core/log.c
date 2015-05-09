#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "log.h"

static int g_min_level = LOG_INFO;

void log_open(void)
{
	openlog("packlad", LOG_PID, LOG_USER);
}

void log_write(const int level, const char *format, ...)
{
	char full_format[MAX_FMT_LEN];
	char text_now[26];
	va_list args;
	va_list tmp;
	FILE *stream;
	time_t now;
	int len;

	if (level > g_min_level)
		return;

	(void) time(&now);
	if (NULL == ctime_r(&now, text_now))
		return;
	text_now[strlen(text_now) - 1] = '\0';

	if (LOG_ERR <= level)
		stream = stderr;
	else
		stream = stdout;

	va_start(args, format);

	if (LOG_INFO >= level) {
		va_copy(tmp, args);
		vsyslog(level, format, tmp);
		va_end(tmp);
	}

	len = snprintf(full_format,
	               sizeof(full_format),
	               "[%s]<%d>: %s",
	               text_now,
	               level,
	               format);
	if ((0 < len) && (sizeof(full_format) > len))
		(void) vfprintf(stream, full_format, args);

	va_end(args);
}

void log_close(void)
{
	closelog();
}

int log_get_min_level(void)
{
	return g_min_level;
}

void log_set_min_level(const int level)
{
	g_min_level = level;
}
