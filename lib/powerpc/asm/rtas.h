#ifndef _ASMPOWERPC_RTAS_H_
#define _ASMPOWERPC_RTAS_H_
/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */

#ifndef __ASSEMBLY__

#include <libcflat.h>

#define RTAS_UNKNOWN_SERVICE	(-1)

struct rtas_args {
	u32 token;
	u32 nargs;
	u32 nret;
	u32 args[16];
	u32 *rets;
};

extern void rtas_init(void);
extern int rtas_token(const char *service, uint32_t *token);
extern int rtas_call(int token, int nargs, int nret, int *outputs, ...);

extern void rtas_power_off(void);
#endif /* __ASSEMBLY__ */

#define RTAS_MSR_MASK 0xfffffffffffffffe

#endif /* _ASMPOWERPC_RTAS_H_ */
