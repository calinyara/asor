/*
 * Copyright (C) 2012, 2017, Red Hat Inc.
 *
 * This allocator provides contiguous physical addresses with page
 * granularity.
 */

#include "libcflat.h"
#include "asm/spinlock.h"
#include "asm/page.h"
#include "asm/io.h"
#include "alloc.h"
#include "alloc_phys.h"
#include "alloc_page.h"
#include "vmalloc.h"

static struct spinlock lock;
static void *vfree_top = 0;
static void *page_root;

void *alloc_vpages(ulong nr)
{
	spin_lock(&lock);
	vfree_top -= PAGE_SIZE * nr;
	spin_unlock(&lock);
	return vfree_top;
}

void *alloc_vpage(void)
{
	return alloc_vpages(1);
}

void init_alloc_vpage(void *top)
{
	vfree_top = top;
}

void *vmap(phys_addr_t phys, size_t size)
{
	void *mem, *p;
	unsigned pages;

	size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	pages = size / PAGE_SIZE;
	mem = p = alloc_vpages(pages);

	phys &= ~(unsigned long long)(PAGE_SIZE - 1);
	while (pages--) {
		install_page(page_root, phys, p);
		phys += PAGE_SIZE;
		p += PAGE_SIZE;
	}
	return mem;
}

static void *vm_memalign(size_t alignment, size_t size)
{
	void *mem, *p;
	unsigned pages;

	assert(alignment <= PAGE_SIZE);
	size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	pages = size / PAGE_SIZE;
	mem = p = alloc_vpages(pages);
	while (pages--) {
		phys_addr_t pa = virt_to_phys(alloc_page());
		install_page(page_root, pa, p);
		p += PAGE_SIZE;
	}
	return mem;
}

static void vm_free(void *mem, size_t size)
{
	while (size) {
		free_page(phys_to_virt(virt_to_pte_phys(page_root, mem)));
		mem += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
}

static struct alloc_ops vmalloc_ops = {
	.memalign = vm_memalign,
	.free = vm_free,
	.align_min = PAGE_SIZE,
};

void setup_vm()
{
	phys_addr_t base, top;

	if (alloc_ops == &vmalloc_ops)
		return;

	phys_alloc_get_unused(&base, &top);
	assert(base != top || page_alloc_initialized());
	if (!page_alloc_initialized()) {
		base = (base + PAGE_SIZE - 1) & -PAGE_SIZE;
		top = top & -PAGE_SIZE;
		free_pages(phys_to_virt(base), top - base);
	}
	page_root = setup_mmu(top);
	alloc_ops = &vmalloc_ops;
}
