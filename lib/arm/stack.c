/*
 * backtrace support (this is a modified lib/x86/stack.c)
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <stack.h>

int backtrace_frame(const void *frame, const void **return_addrs,
		    int max_depth)
{
	static int walking;
	int depth;
	const unsigned long *fp = (unsigned long *)frame;

	if (walking) {
		printf("RECURSIVE STACK WALK!!!\n");
		return 0;
	}
	walking = 1;

	for (depth = 0; depth < max_depth; depth++) {
		if (!fp)
			break;
		return_addrs[depth] = (void *)fp[0];
		if (return_addrs[depth] == 0)
			break;
		fp = (unsigned long *)fp[-1];
	}

	walking = 0;
	return depth;
}

int backtrace(const void **return_addrs, int max_depth)
{
	return backtrace_frame(__builtin_frame_address(0),
			       return_addrs, max_depth);
}
