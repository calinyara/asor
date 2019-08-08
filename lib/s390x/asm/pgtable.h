/*
 * s390x page table definitions and functions
 *
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#ifndef _ASMS390X_PGTABLE_H_
#define _ASMS390X_PGTABLE_H_

#include <asm/page.h>
#include <alloc_page.h>

#define ASCE_ORIGIN			0xfffffffffffff000UL
#define ASCE_G				0x0000000000000200UL
#define ASCE_P				0x0000000000000100UL
#define ASCE_S				0x0000000000000080UL
#define ASCE_X				0x0000000000000040UL
#define ASCE_R				0x0000000000000020UL
#define ASCE_DT				0x000000000000000cUL
#define ASCE_TL				0x0000000000000003UL

#define ASCE_DT_REGION1			0x000000000000000cUL
#define ASCE_DT_REGION2			0x0000000000000008UL
#define ASCE_DT_REGION3			0x0000000000000004UL
#define ASCE_DT_SEGMENT			0x0000000000000000UL

#define REGION_TABLE_ORDER		2
#define REGION_TABLE_ENTRIES		2048
#define REGION_TABLE_LENGTH		3

#define REGION1_SHIFT			53
#define REGION2_SHIFT			42
#define REGION3_SHIFT			31

#define REGION_ENTRY_ORIGIN		0xfffffffffffff000UL
#define REGION_ENTRY_P			0x0000000000000200UL
#define REGION_ENTRY_TF			0x00000000000000c0UL
#define REGION_ENTRY_I			0x0000000000000020UL
#define REGION_ENTRY_TT			0x000000000000000cUL
#define REGION_ENTRY_TL			0x0000000000000003UL

#define REGION_ENTRY_TT_REGION1		0x000000000000000cUL
#define REGION_ENTRY_TT_REGION2		0x0000000000000008UL
#define REGION_ENTRY_TT_REGION3		0x0000000000000004UL

#define REGION3_ENTRY_RFAA		0xffffffff80000000UL
#define REGION3_ENTRY_AV		0x0000000000010000UL
#define REGION3_ENTRY_ACC		0x000000000000f000UL
#define REGION3_ENTRY_F			0x0000000000000800UL
#define REGION3_ENTRY_FC		0x0000000000000400UL
#define REGION3_ENTRY_IEP		0x0000000000000100UL
#define REGION3_ENTRY_CR		0x0000000000000010UL

#define SEGMENT_TABLE_ORDER		2
#define SEGMENT_TABLE_ENTRIES		2048
#define SEGMENT_TABLE_LENGTH		3
#define SEGMENT_SHIFT			20

#define SEGMENT_ENTRY_ORIGIN		0xfffffffffffff800UL
#define SEGMENT_ENTRY_SFAA		0xfffffffffff80000UL
#define SEGMENT_ENTRY_AV		0x0000000000010000UL
#define SEGMENT_ENTRY_ACC		0x000000000000f000UL
#define SEGMENT_ENTRY_F			0x0000000000000800UL
#define SEGMENT_ENTRY_FC		0x0000000000000400UL
#define SEGMENT_ENTRY_P			0x0000000000000200UL
#define SEGMENT_ENTRY_IEP		0x0000000000000100UL
#define SEGMENT_ENTRY_I			0x0000000000000020UL
#define SEGMENT_ENTRY_CS		0x0000000000000010UL
#define SEGMENT_ENTRY_TT		0x000000000000000cUL

#define SEGMENT_ENTRY_TT_REGION1	0x000000000000000cUL
#define SEGMENT_ENTRY_TT_REGION2	0x0000000000000008UL
#define SEGMENT_ENTRY_TT_REGION3	0x0000000000000004UL
#define SEGMENT_ENTRY_TT_SEGMENT	0x0000000000000000UL

#define PAGE_TABLE_ORDER		0
#define PAGE_TABLE_ENTRIES		256

#define PAGE_ENTRY_I			0x0000000000000400UL
#define PAGE_ENTRY_P			0x0000000000000200UL
#define PAGE_ENTRY_IEP			0x0000000000000100UL

#define PTRS_PER_PGD			REGION_TABLE_ENTRIES
#define PTRS_PER_P4D			REGION_TABLE_ENTRIES
#define PTRS_PER_PUD			REGION_TABLE_ENTRIES
#define PTRS_PER_PMD			SEGMENT_TABLE_ENTRIES
#define PTRS_PER_PTE			PAGE_TABLE_ENTRIES

#define PGDIR_SHIFT			REGION1_SHIFT
#define P4D_SHIFT			REGION2_SHIFT
#define PUD_SHIFT			REGION3_SHIFT
#define PMD_SHIFT			SEGMENT_SHIFT

#define pgd_none(entry) (pgd_val(entry) & REGION_ENTRY_I)
#define p4d_none(entry) (p4d_val(entry) & REGION_ENTRY_I)
#define pud_none(entry) (pud_val(entry) & REGION_ENTRY_I)
#define pmd_none(entry) (pmd_val(entry) & SEGMENT_ENTRY_I)
#define pte_none(entry) (pte_val(entry) & PAGE_ENTRY_I)

#define pgd_addr(entry) __va(pgd_val(entry) & REGION_ENTRY_ORIGIN)
#define p4d_addr(entry) __va(p4d_val(entry) & REGION_ENTRY_ORIGIN)
#define pud_addr(entry) __va(pud_val(entry) & REGION_ENTRY_ORIGIN)
#define pmd_addr(entry) __va(pmd_val(entry) & SEGMENT_ENTRY_ORIGIN)
#define pte_addr(entry) __va(pte_val(entry) & PAGE_MASK)

#define pgd_index(addr) (((addr) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define p4d_index(addr) (((addr) >> P4D_SHIFT) & (PTRS_PER_P4D - 1))
#define pud_index(addr) (((addr) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))
#define pmd_index(addr) (((addr) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))
#define pte_index(addr) (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

#define pgd_offset(table, addr) ((pgd_t *)(table) + pgd_index(addr))
#define p4d_offset(pgd, addr) ((p4d_t *)pgd_addr(*(pgd)) + p4d_index(addr))
#define pud_offset(p4d, addr) ((pud_t *)p4d_addr(*(p4d)) + pud_index(addr))
#define pmd_offset(pud, addr) ((pmd_t *)pud_addr(*(pud)) + pmd_index(addr))
#define pte_offset(pmd, addr) ((pte_t *)pmd_addr(*(pmd)) + pte_index(addr))

static inline pgd_t *pgd_alloc_one(void)
{
	pgd_t *pgd = alloc_pages(REGION_TABLE_ORDER);
	int i;

	for (i = 0; i < REGION_TABLE_ENTRIES; i++)
		pgd_val(pgd[i]) = REGION_ENTRY_TT_REGION1 | REGION_ENTRY_I;
	return pgd;
}

static inline p4d_t *p4d_alloc_one(void)
{
	p4d_t *p4d = alloc_pages(REGION_TABLE_ORDER);
	int i;

	for (i = 0; i < REGION_TABLE_ENTRIES; i++)
		p4d_val(p4d[i]) = REGION_ENTRY_TT_REGION2 | REGION_ENTRY_I;
	return p4d;
}

static inline p4d_t *p4d_alloc(pgd_t *pgd, unsigned long addr)
{
	if (pgd_none(*pgd)) {
		p4d_t *p4d = p4d_alloc_one();
		pgd_val(*pgd) = __pa(p4d) | REGION_ENTRY_TT_REGION1 |
				REGION_TABLE_LENGTH;
	}
	return p4d_offset(pgd, addr);
}

static inline pud_t *pud_alloc_one(void)
{
	pud_t *pud = alloc_pages(REGION_TABLE_ORDER);
	int i;

	for (i = 0; i < REGION_TABLE_ENTRIES; i++)
		pud_val(pud[i]) = REGION_ENTRY_TT_REGION3 | REGION_ENTRY_I;
	return pud;
}

static inline pud_t *pud_alloc(p4d_t *p4d, unsigned long addr)
{
	if (p4d_none(*p4d)) {
		pud_t *pud = pud_alloc_one();
		p4d_val(*p4d) = __pa(pud) | REGION_ENTRY_TT_REGION2 |
				REGION_TABLE_LENGTH;
	}
	return pud_offset(p4d, addr);
}

static inline pmd_t *pmd_alloc_one(void)
{
	pmd_t *pmd = alloc_pages(SEGMENT_TABLE_ORDER);
	int i;

	for (i = 0; i < SEGMENT_TABLE_ENTRIES; i++)
		pmd_val(pmd[i]) = SEGMENT_ENTRY_TT_SEGMENT | SEGMENT_ENTRY_I;
	return pmd;
}

static inline pmd_t *pmd_alloc(pud_t *pud, unsigned long addr)
{
	if (pud_none(*pud)) {
		pmd_t *pmd = pmd_alloc_one();
		pud_val(*pud) = __pa(pmd) | REGION_ENTRY_TT_REGION3 |
				REGION_TABLE_LENGTH;
	}
	return pmd_offset(pud, addr);
}

static inline pte_t *pte_alloc_one(void)
{
	pte_t *pte = alloc_pages(PAGE_TABLE_ORDER);
	int i;

	for (i = 0; i < PAGE_TABLE_ENTRIES; i++)
		pte_val(pte[i]) = PAGE_ENTRY_I;
	return pte;
}

static inline pte_t *pte_alloc(pmd_t *pmd, unsigned long addr)
{
	if (pmd_none(*pmd)) {
		pte_t *pte = pte_alloc_one();
		pmd_val(*pmd) = __pa(pte) | SEGMENT_ENTRY_TT_SEGMENT |
				SEGMENT_TABLE_LENGTH;
	}
	return pte_offset(pmd, addr);
}

static inline void ipte(unsigned long vaddr, pteval_t *p_pte)
{
	unsigned long table_origin = (unsigned long)p_pte & PAGE_MASK;

	asm volatile(
		"	ipte %0,%1\n"
		: : "a" (table_origin), "a" (vaddr) : "memory");
}

void configure_dat(int enable);

#endif /* _ASMS390X_PGTABLE_H_ */
