#ifndef __ASMARM_MMU_H_
#define __ASMARM_MMU_H_
/*
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/barrier.h>

#define PTE_USER		L_PTE_USER
#define PTE_RDONLY		PTE_AP2
#define PTE_SHARED		L_PTE_SHARED
#define PTE_AF			PTE_EXT_AF
#define PTE_WBWA		L_PTE_MT_WRITEALLOC

/* See B3.18.7 TLB maintenance operations */

static inline void local_flush_tlb_all(void)
{
	/* TLBIALL */
	asm volatile("mcr p15, 0, %0, c8, c7, 0" :: "r" (0));
	dsb();
	isb();
}

static inline void flush_tlb_all(void)
{
	//TODO
	local_flush_tlb_all();
}

static inline void flush_tlb_page(unsigned long vaddr)
{
	/* TLBIMVAA */
	asm volatile("mcr p15, 0, %0, c8, c7, 3" :: "r" (vaddr));
	dsb();
	isb();
}

static inline void flush_dcache_addr(unsigned long vaddr)
{
	asm volatile("mcr p15, 0, %0, c7, c14, 1" :: "r" (vaddr));
}

#include <asm/mmu-api.h>

#endif /* __ASMARM_MMU_H_ */
