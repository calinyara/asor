/*
 * Hypercall helpers
 *
 * broken_sc1 probing/patching inspired by SLOF, see
 *   SLOF:lib/libhvcall/brokensc1.c
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/hcall.h>
#include <libcflat.h>
#include "io.h"

int hcall_have_broken_sc1(void)
{
	register unsigned long r3 asm("r3") = H_SET_DABR;
	register unsigned long r4 asm("r4") = 0;

	asm volatile("sc 1"
	: "=r" (r3)
	: "r" (r3), "r" (r4)
	: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");

	return r3 == (unsigned long)H_PRIVILEGE;
}

void putchar(int c)
{
	unsigned long vty = 0;		/* 0 == default */
	unsigned long nr_chars = 1;
	unsigned long chars = (unsigned long)c << 56;

	hcall(H_PUT_TERM_CHAR, vty, nr_chars, chars);
}

int __getchar(void)
{
	register unsigned long r3 asm("r3") = H_GET_TERM_CHAR;
	register unsigned long r4 asm("r4") = 0; /* 0 == default vty */
	register unsigned long r5 asm("r5");

	asm volatile (" sc 1 "  : "+r"(r3), "+r"(r4), "=r"(r5)
				: "r"(r3),  "r"(r4));

	return r3 == H_SUCCESS && r4 > 0 ? r5 >> 48 : -1;
}
