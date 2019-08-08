#include "auxinfo.h"

#ifndef PROGNAME
#define PROGNAME ((void *)0)
#endif
#ifndef AUXFLAGS
#define AUXFLAGS 0
#endif

struct auxinfo auxinfo = {
	.progname = PROGNAME,
	.flags = AUXFLAGS,
};
