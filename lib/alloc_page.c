/*
 * This work is licensed under the terms of the GNU LGPL, version 2.
 *
 * This is a simple allocator that provides contiguous physical addresses
 * with page granularity.
 */
#include "libcflat.h"
#include "alloc.h"
#include "alloc_phys.h"
#include "alloc_page.h"
#include "bitops.h"
#include <asm/page.h>
#include <asm/io.h>
#include <asm/spinlock.h>

static struct spinlock lock;
static void *freelist = 0;

bool page_alloc_initialized(void)
{
	return freelist != 0;
}

void free_pages(void *mem, unsigned long size)
{
	void *old_freelist;
	void *end;

	assert_msg((unsigned long) mem % PAGE_SIZE == 0,
		   "mem not page aligned: %p", mem);

	assert_msg(size % PAGE_SIZE == 0, "size not page aligned: %#lx", size);

	assert_msg(size == 0 || (uintptr_t)mem == -size ||
		   (uintptr_t)mem + size > (uintptr_t)mem,
		   "mem + size overflow: %p + %#lx", mem, size);

	if (size == 0) {
		freelist = NULL;
		return;
	}

	spin_lock(&lock);
	old_freelist = freelist;
	freelist = mem;
	end = mem + size;
	while (mem + PAGE_SIZE != end) {
		*(void **)mem = (mem + PAGE_SIZE);
		mem += PAGE_SIZE;
	}

	*(void **)mem = old_freelist;
	spin_unlock(&lock);
}

void *alloc_page()
{
	void *p;

	if (!freelist)
		return 0;

	spin_lock(&lock);
	p = freelist;
	freelist = *(void **)freelist;
	spin_unlock(&lock);

	return p;
}

/*
 * Allocates (1 << order) physically contiguous and naturally aligned pages.
 * Returns NULL if there's no memory left.
 */
void *alloc_pages(unsigned long order)
{
	/* Generic list traversal. */
	void *prev;
	void *curr = NULL;
	void *next = freelist;

	/* Looking for a run of length (1 << order). */
	unsigned long run = 0;
	const unsigned long n = 1ul << order;
	const unsigned long align_mask = (n << PAGE_SHIFT) - 1;
	void *run_start = NULL;
	void *run_prev = NULL;
	unsigned long run_next_pa = 0;
	unsigned long pa;

	assert(order < sizeof(unsigned long) * 8);

	spin_lock(&lock);
	for (;;) {
		prev = curr;
		curr = next;

		if (!curr) {
			run_start = NULL;
			break;
		}

		next = *((void **) curr);
		pa = virt_to_phys(curr);

		if (run == 0) {
			if (!(pa & align_mask)) {
				run_start = curr;
				run_prev = prev;
				run_next_pa = pa + PAGE_SIZE;
				run = 1;
			}
		} else if (pa == run_next_pa) {
			run_next_pa += PAGE_SIZE;
			run += 1;
		} else {
			run = 0;
		}

		if (run == n) {
			if (run_prev)
				*((void **) run_prev) = next;
			else
				freelist = next;
			break;
		}
	}
	spin_unlock(&lock);
	return run_start;
}


void free_page(void *page)
{
	spin_lock(&lock);
	*(void **)page = freelist;
	freelist = page;
	spin_unlock(&lock);
}

static void *page_memalign(size_t alignment, size_t size)
{
	unsigned long n = ALIGN(size, PAGE_SIZE) >> PAGE_SHIFT;
	unsigned long order;

	if (!size)
		return NULL;

	order = is_power_of_2(n) ? fls(n) : fls(n) + 1;

	return alloc_pages(order);
}

static void page_free(void *mem, size_t size)
{
	free_pages(mem, size);
}

static struct alloc_ops page_alloc_ops = {
	.memalign = page_memalign,
	.free = page_free,
	.align_min = PAGE_SIZE,
};

void page_alloc_ops_enable(void)
{
	alloc_ops = &page_alloc_ops;
}
