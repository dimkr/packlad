#ifndef _TYPES_H_INCLUDED
#	define _TYPES_H_INCLUDED

typedef char tristate_t;

/* the enum starts at 2, to ensure we don't compare bool and tristate_t by
 * mistake */
enum states {
	TSTATE_OK    = 2,
	TSTATE_ERROR = 3,
	TSTATE_FATAL = 4
};

#endif
