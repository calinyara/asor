#ifndef _ASMARM64_PGTABLE_H_
#define _ASMARM64_PGTABLE_H_
/*
 * Adapted from arch/arm64/include/asm/pgtable.h
 *              include/asm-generic/pgtable-nopmd.h
 *              include/linux/mm.h
 *
 * Note: some Linux function APIs have been modified. Nothing crazy,
 *       but if a function took, for example, an mm_struct, then
 *       that was either removed or replaced.
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */
#include <alloc.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/pgtable-hwdef.h>

/*
 * We can convert va <=> pa page table addresses with simple casts
 * because we always allocate their pages with alloc_page(), and
 * alloc_page() always returns identity mapped pages.
 */
#define pgtable_va(x)		((void *)(unsigned long)(x))
#define pgtable_pa(x)		((unsigned long)(x))

#define pgd_none(pgd)		(!pgd_val(pgd))
#define pmd_none(pmd)		(!pmd_val(pmd))
#define pte_none(pte)		(!pte_val(pte))

#define pgd_index(addr) \
	(((addr) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_offset(pgtable, addr) ((pgtable) + pgd_index(addr))

#define pgd_free(pgd) free(pgd)
static inline pgd_t *pgd_alloc(void)
{
	pgd_t *pgd = memalign(PAGE_SIZE, PTRS_PER_PGD * sizeof(pgd_t));
	memset(pgd, 0, PTRS_PER_PGD * sizeof(pgd_t));
	return pgd;
}

#define pmd_offset(pgd, addr)	((pmd_t *)pgd)
#define pmd_free(pmd)
#define pmd_alloc(pgd, addr)	pmd_offset(pgd, addr)

static inline pte_t *pmd_page_vaddr(pmd_t pmd)
{
	return pgtable_va(pmd_val(pmd) & PHYS_MASK & (s32)PAGE_MASK);
}

#define pte_index(addr) \
	(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset(pmd, addr) \
	(pmd_page_vaddr(*(pmd)) + pte_index(addr))

#define pte_free(pte) free_page(pte)
static inline pte_t *pte_alloc_one(void)
{
	assert(PTRS_PER_PTE * sizeof(pte_t) == PAGE_SIZE);
	pte_t *pte = alloc_page();
	memset(pte, 0, PTRS_PER_PTE * sizeof(pte_t));
	return pte;
}
static inline pte_t *pte_alloc(pmd_t *pmd, unsigned long addr)
{
	if (pmd_none(*pmd)) {
		pte_t *pte = pte_alloc_one();
		pmd_val(*pmd) = pgtable_pa(pte) | PMD_TYPE_TABLE;
	}
	return pte_offset(pmd, addr);
}

#endif /* _ASMARM64_PGTABLE_H_ */
