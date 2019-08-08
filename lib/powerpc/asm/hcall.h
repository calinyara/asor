#ifndef _ASMPOWERPC_HCALL_H_
#define _ASMPOWERPC_HCALL_H_
/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */

#define SC1			0x44000022
#define SC1_REPLACEMENT		0x7c000268

#define H_SUCCESS		0
#define H_HARDWARE		-1
#define H_FUNCTION		-2
#define H_PRIVILEGE		-3
#define H_PARAMETER		-4

#define H_SET_SPRG0		0x24
#define H_SET_DABR		0x28
#define H_PAGE_INIT		0x2c
#define H_CEDE			0xE0
#define H_GET_TERM_CHAR		0x54
#define H_PUT_TERM_CHAR		0x58
#define H_RANDOM		0x300
#define H_SET_MODE		0x31C

#ifndef __ASSEMBLY__
/*
 * hcall_have_broken_sc1 checks if we're on a host with a broken sc1.
 * Returns 0 if we're not.
 */
extern int hcall_have_broken_sc1(void);

/*
 * hcall is the hypercall wrapper function. unittests may do what
 * they like, but the framework should make all hypercalls through
 * here to ensure they use a working sc1 instruction. @nr is the
 * hypercall number.
 */
extern unsigned long hcall(unsigned long nr, ...);

#endif /* !__ASSEMBLY__ */
#endif /* _ASMPOWERPC_HCALL_H_ */
