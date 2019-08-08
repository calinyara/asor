/*
 * Adapted from u-boot's arch/arm/lib/eabi_compat.c
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */
#include <libcflat.h>

/* Needed to compile with -Wmissing-prototypes */
int raise(int signum);
void __aeabi_unwind_cpp_pr0(void);
void __aeabi_unwind_cpp_pr1(void);

int raise(int signum __unused)
{
	printf("Divide by zero!\n");
	abort();
	return 0;
}

/* Dummy functions to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}
