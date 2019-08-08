#ifndef _ALLOC_PHYS_H_
#define _ALLOC_PHYS_H_
/*
 * phys_alloc is a very simple allocator which allows physical memory
 * to be partitioned into regions until all memory is allocated.
 *
 * Note: This is such a simple allocator that there is no way to free
 * a region. For more complicated memory management a single region
 * can be allocated, but then have its memory managed by a more
 * sophisticated allocator, e.g. a page allocator.
 *
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include "libcflat.h"

#define DEFAULT_MINIMUM_ALIGNMENT 32

/*
 * phys_alloc_init creates the initial free memory region of size @size
 * at @base. The minimum alignment is set to DEFAULT_MINIMUM_ALIGNMENT.
 */
extern void phys_alloc_init(phys_addr_t base, phys_addr_t size);

/*
 * phys_alloc_set_minimum_alignment sets the minimum alignment to
 * @align.
 */
extern void phys_alloc_set_minimum_alignment(phys_addr_t align);

/*
 * phys_alloc_show outputs all currently allocated regions with the
 * following format
 *   <start_addr>-<end_addr> [<USED|FREE>]
 */
extern void phys_alloc_show(void);

/*
 * phys_alloc_get_unused allocates all remaining memory from the region
 * passed to phys_alloc_init, returning the newly allocated memory's base
 * and top addresses. phys_allo_get_unused will still return base and top
 * when no free memory is remaining, but base will equal top.
 */
extern void phys_alloc_get_unused(phys_addr_t *p_base, phys_addr_t *p_top);

#endif /* _ALLOC_PHYS_H_ */
