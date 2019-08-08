/*
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#ifndef _ASM_S390X_FACILITY_H_
#define _ASM_S390X_FACILITY_H_

#include <libcflat.h>
#include <asm/facility.h>
#include <asm/arch_def.h>

#define NR_STFL_BYTES 256
extern uint8_t stfl_bytes[];

static inline bool test_facility(int nr)
{
	return stfl_bytes[nr / 8] & (0x80U >> (nr % 8));
}

static inline void stfl(void)
{
	asm volatile("	stfl	0(0)\n");
}

static inline void stfle(uint8_t *fac, unsigned int len)
{
	register unsigned long r0 asm("0") = len - 1;

	asm volatile("	.insn	s,0xb2b00000,0(%1)\n"
		     : "+d" (r0) : "a" (fac) : "memory", "cc");
}

static inline void setup_facilities(void)
{
	struct lowcore *lc = NULL;

	stfl();
	memcpy(stfl_bytes, &lc->stfl, sizeof(lc->stfl));
	if (test_facility(7))
		stfle(stfl_bytes, NR_STFL_BYTES);
}

#endif
