/*
 * MMU enable and page table manipulation functions
 *
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/setup.h>
#include <asm/thread_info.h>
#include <asm/cpumask.h>
#include <asm/mmu.h>
#include <asm/setup.h>
#include <asm/page.h>

#include "alloc_page.h"
#include "vmalloc.h"
#include <asm/pgtable-hwdef.h>
#include <asm/pgtable.h>

extern unsigned long etext;

pgd_t *mmu_idmap;

/* CPU 0 starts with disabled MMU */
static cpumask_t mmu_disabled_cpumask = { {1} };
unsigned int mmu_disabled_cpu_count = 1;

bool __mmu_enabled(void)
{
	int cpu = current_thread_info()->cpu;

	/*
	 * mmu_enabled is called from places that are guarding the
	 * use of exclusive ops (which require the mmu to be enabled).
	 * That means we CANNOT call anything from here that may use a
	 * spinlock, atomic bitop, etc., otherwise we'll recurse.
	 * [cpumask_]test_bit is safe though.
	 */
	return !cpumask_test_cpu(cpu, &mmu_disabled_cpumask);
}

void mmu_mark_enabled(int cpu)
{
	if (cpumask_test_and_clear_cpu(cpu, &mmu_disabled_cpumask))
		--mmu_disabled_cpu_count;
}

void mmu_mark_disabled(int cpu)
{
	if (!cpumask_test_and_set_cpu(cpu, &mmu_disabled_cpumask))
		++mmu_disabled_cpu_count;
}

extern void asm_mmu_enable(phys_addr_t pgtable);
void mmu_enable(pgd_t *pgtable)
{
	struct thread_info *info = current_thread_info();

	asm_mmu_enable(__pa(pgtable));
	flush_tlb_all();

	info->pgtable = pgtable;
	mmu_mark_enabled(info->cpu);
}

extern void asm_mmu_disable(void);
void mmu_disable(void)
{
	int cpu = current_thread_info()->cpu;

	mmu_mark_disabled(cpu);

	asm_mmu_disable();
}

static void flush_entry(pgd_t *pgtable, uintptr_t vaddr)
{
	pgd_t *pgd = pgd_offset(pgtable, vaddr);
	pmd_t *pmd = pmd_offset(pgd, vaddr);

	flush_dcache_addr((ulong)pgd);
	flush_dcache_addr((ulong)pmd);
	flush_dcache_addr((ulong)pte_offset(pmd, vaddr));
	flush_tlb_page(vaddr);
}

static pteval_t *get_pte(pgd_t *pgtable, uintptr_t vaddr)
{
	pgd_t *pgd = pgd_offset(pgtable, vaddr);
	pmd_t *pmd = pmd_alloc(pgd, vaddr);
	pte_t *pte = pte_alloc(pmd, vaddr);

	return &pte_val(*pte);
}

static pteval_t *install_pte(pgd_t *pgtable, uintptr_t vaddr, pteval_t pte)
{
	pteval_t *p_pte = get_pte(pgtable, vaddr);

	*p_pte = pte;
	flush_entry(pgtable, vaddr);
	return p_pte;
}

static pteval_t *install_page_prot(pgd_t *pgtable, phys_addr_t phys,
				   uintptr_t vaddr, pgprot_t prot)
{
	pteval_t pte = phys;
	pte |= PTE_TYPE_PAGE | PTE_AF | PTE_SHARED;
	pte |= pgprot_val(prot);
	return install_pte(pgtable, vaddr, pte);
}

pteval_t *install_page(pgd_t *pgtable, phys_addr_t phys, void *virt)
{
	return install_page_prot(pgtable, phys, (uintptr_t)virt,
				 __pgprot(PTE_WBWA | PTE_USER));
}

phys_addr_t virt_to_pte_phys(pgd_t *pgtable, void *mem)
{
	return (*get_pte(pgtable, (uintptr_t)mem) & PHYS_MASK & -PAGE_SIZE)
		+ ((ulong)mem & (PAGE_SIZE - 1));
}

void mmu_set_range_ptes(pgd_t *pgtable, uintptr_t virt_offset,
			phys_addr_t phys_start, phys_addr_t phys_end,
			pgprot_t prot)
{
	phys_addr_t paddr = phys_start & PAGE_MASK;
	uintptr_t vaddr = virt_offset & PAGE_MASK;
	uintptr_t virt_end = phys_end - paddr + vaddr;

	for (; vaddr < virt_end; vaddr += PAGE_SIZE, paddr += PAGE_SIZE)
		install_page_prot(pgtable, paddr, vaddr, prot);
}

void mmu_set_range_sect(pgd_t *pgtable, uintptr_t virt_offset,
			phys_addr_t phys_start, phys_addr_t phys_end,
			pgprot_t prot)
{
	phys_addr_t paddr = phys_start & PGDIR_MASK;
	uintptr_t vaddr = virt_offset & PGDIR_MASK;
	uintptr_t virt_end = phys_end - paddr + vaddr;

	for (; vaddr < virt_end; vaddr += PGDIR_SIZE, paddr += PGDIR_SIZE) {
		pgd_t *pgd = pgd_offset(pgtable, vaddr);
		pgd_val(*pgd) = paddr;
		pgd_val(*pgd) |= PMD_TYPE_SECT | PMD_SECT_AF | PMD_SECT_S;
		pgd_val(*pgd) |= pgprot_val(prot);
		flush_dcache_addr((ulong)pgd);
		flush_tlb_page(vaddr);
	}
}

void *setup_mmu(phys_addr_t phys_end)
{
	uintptr_t code_end = (uintptr_t)&etext;

	/* 0G-1G = I/O, 1G-3G = identity, 3G-4G = vmalloc */
	if (phys_end > (3ul << 30))
		phys_end = 3ul << 30;

#ifdef __aarch64__
	init_alloc_vpage((void*)(4ul << 30));
#endif

	mmu_idmap = alloc_page();
	memset(mmu_idmap, 0, PAGE_SIZE);

	/*
	 * mach-virt I/O regions:
	 *   - The first 1G (arm/arm64)
	 *   - 512M at 256G (arm64, arm uses highmem=off)
	 *   - 512G at 512G (arm64, arm uses highmem=off)
	 */
	mmu_set_range_sect(mmu_idmap,
		0, 0, (1ul << 30),
		__pgprot(PMD_SECT_UNCACHED | PMD_SECT_USER));
#ifdef __aarch64__
	mmu_set_range_sect(mmu_idmap,
		(1ul << 38), (1ul << 38), (1ul << 38) | (1ul << 29),
		__pgprot(PMD_SECT_UNCACHED | PMD_SECT_USER));
	mmu_set_range_sect(mmu_idmap,
		(1ul << 39), (1ul << 39), (1ul << 40),
		__pgprot(PMD_SECT_UNCACHED | PMD_SECT_USER));
#endif

	/* armv8 requires code shared between EL1 and EL0 to be read-only */
	mmu_set_range_ptes(mmu_idmap, PHYS_OFFSET,
		PHYS_OFFSET, code_end,
		__pgprot(PTE_WBWA | PTE_RDONLY | PTE_USER));

	mmu_set_range_ptes(mmu_idmap, code_end,
		code_end, phys_end,
		__pgprot(PTE_WBWA | PTE_USER));

	mmu_enable(mmu_idmap);
	return mmu_idmap;
}

phys_addr_t __virt_to_phys(unsigned long addr)
{
	if (mmu_enabled()) {
		pgd_t *pgtable = current_thread_info()->pgtable;
		return virt_to_pte_phys(pgtable, (void *)addr);
	}
	return addr;
}

unsigned long __phys_to_virt(phys_addr_t addr)
{
	/*
	 * We don't guarantee that phys_to_virt(virt_to_phys(vaddr)) == vaddr, but
	 * the default page tables do identity map all physical addresses, which
	 * means phys_to_virt(virt_to_phys((void *)paddr)) == paddr.
	 */
	assert(!mmu_enabled() || __virt_to_phys(addr) == addr);
	return addr;
}
