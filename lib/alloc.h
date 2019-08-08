#ifndef _ALLOC_H_
#define _ALLOC_H_
/*
 * alloc supplies three ingredients to the test framework that are all
 * related to the support of dynamic memory allocation.
 *
 * The first is a set of alloc function wrappers for malloc and its
 * friends. Using wrappers allows test code and common code to use the
 * same interface for memory allocation at all stages, even though the
 * implementations may change with the stage, e.g. pre/post paging.
 *
 * The second is a set of implementations for the alloc function
 * interfaces. These implementations are named early_*, as they can be
 * used almost immediately by the test framework.
 *
 * The third is a very simple physical memory allocator, which the
 * early_* alloc functions build on.
 *
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include "libcflat.h"

struct alloc_ops {
	void *(*memalign)(size_t alignment, size_t size);
	void (*free)(void *ptr, size_t size);
	size_t align_min;
};

extern struct alloc_ops *alloc_ops;

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *memalign(size_t alignment, size_t size);

#endif /* _ALLOC_H_ */
