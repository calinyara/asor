#ifndef __ASMARM64_MMU_H_
#define __ASMARM64_MMU_H_
/*
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/barrier.h>

#define PMD_SECT_UNCACHED	PMD_ATTRINDX(MT_DEVICE_nGnRE)
#define PTE_WBWA		PTE_ATTRINDX(MT_NORMAL)

static inline void flush_tlb_all(void)
{
	dsb(ishst);
	asm("tlbi	vmalle1is");
	dsb(ish);
	isb();
}

static inline void flush_tlb_page(unsigned long vaddr)
{
	unsigned long page = vaddr >> 12;
	dsb(ishst);
	asm("tlbi	vaae1is, %0" :: "r" (page));
	dsb(ish);
}

static inline void flush_dcache_addr(unsigned long vaddr)
{
	asm volatile("dc civac, %0" :: "r" (vaddr));
}

#include <asm/mmu-api.h>

#endif /* __ASMARM64_MMU_H_ */
