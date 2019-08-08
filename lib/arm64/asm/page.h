#ifndef _ASMARM64_PAGE_H_
#define _ASMARM64_PAGE_H_
/*
 * Adapted from
 *   arch/arm64/include/asm/pgtable-types.h
 *   include/asm-generic/pgtable-nopmd.h
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */

#include <linux/const.h>

#define PGTABLE_LEVELS		2
#define VA_BITS			42

#define PAGE_SHIFT		16
#define PAGE_SIZE		(_AC(1,UL) << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1))

#ifndef __ASSEMBLY__

#define PAGE_ALIGN(addr)	ALIGN(addr, PAGE_SIZE)

typedef u64 pteval_t;
typedef u64 pmdval_t;
typedef u64 pgdval_t;
typedef struct { pteval_t pte; } pte_t;
typedef struct { pgdval_t pgd; } pgd_t;
typedef struct { pteval_t pgprot; } pgprot_t;

#define pte_val(x)		((x).pte)
#define pgd_val(x)		((x).pgd)
#define pgprot_val(x)		((x).pgprot)

#define __pte(x)		((pte_t) { (x) } )
#define __pgd(x)		((pgd_t) { (x) } )
#define __pgprot(x)		((pgprot_t) { (x) } )

typedef struct { pgd_t pgd; } pmd_t;
#define pmd_val(x)		(pgd_val((x).pgd))
#define __pmd(x)		((pmd_t) { __pgd(x) } )

#define __va(x)			((void *)__phys_to_virt((phys_addr_t)(x)))
#define __pa(x)			__virt_to_phys((unsigned long)(x))

#define virt_to_pfn(kaddr)	(__pa(kaddr) >> PAGE_SHIFT)
#define pfn_to_virt(pfn)	__va((pfn) << PAGE_SHIFT)

extern phys_addr_t __virt_to_phys(unsigned long addr);
extern unsigned long __phys_to_virt(phys_addr_t addr);

#endif /* !__ASSEMBLY__ */
#endif /* _ASMARM64_PAGE_H_ */
