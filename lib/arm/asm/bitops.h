#ifndef _ASMARM_BITOPS_H_
#define _ASMARM_BITOPS_H_
/*
 * Adapted from
 *   arch/arm/lib/bitops.h
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */

#ifndef _BITOPS_H_
#error only <bitops.h> can be included directly
#endif

#define BITS_PER_LONG	32

#define HAVE_BUILTIN_FLS 1

#define ATOMIC_BITOP(insn, mask, word)				\
({								\
	unsigned long tmp1, tmp2;				\
	asm volatile(						\
	"1:	ldrex	%0, [%2]\n"				\
		insn"	%0, %0, %3\n"				\
	"	strex	%1, %0, [%2]\n"				\
	"	cmp	%1, #0\n"				\
	"	bne	1b\n"					\
	: "=&r" (tmp1), "=&r" (tmp2)				\
	: "r" (word), "r" (mask)				\
	: "cc");						\
})

#define ATOMIC_TESTOP(insn, mask, word, old)			\
({								\
	unsigned long tmp1, tmp2;				\
	asm volatile(						\
	"1:	ldrex	%0, [%3]\n"				\
	"	and	%1, %0, %4\n"				\
		insn"	%0, %0, %4\n"				\
	"	strex	%2, %0, [%3]\n"				\
	"	cmp	%2, #0\n"				\
	"	bne	1b\n"					\
	: "=&r" (tmp1), "=&r" (old), "=&r" (tmp2)		\
	: "r" (word), "r" (mask)				\
	: "cc");						\
})

extern void set_bit(int nr, volatile unsigned long *addr);
extern void clear_bit(int nr, volatile unsigned long *addr);
extern int test_bit(int nr, const volatile unsigned long *addr);
extern int test_and_set_bit(int nr, volatile unsigned long *addr);
extern int test_and_clear_bit(int nr, volatile unsigned long *addr);

#endif /* _ASMARM_BITOPS_H_ */
