/*
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  Thomas Huth <thuth@redhat.com>
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#ifndef _ASMS390X_PAGE_H_
#define _ASMS390X_PAGE_H_

#include <asm-generic/page.h>

typedef uint64_t pgdval_t;		/* Region-1 table entry */
typedef uint64_t p4dval_t;		/* Region-2 table entry*/
typedef uint64_t pudval_t;		/* Region-3 table entry */
typedef uint64_t pmdval_t;		/* Segment table entry */
typedef uint64_t pteval_t;		/* Page table entry */

typedef struct { pgdval_t pgd; } pgd_t;
typedef struct { p4dval_t p4d; } p4d_t;
typedef struct { pudval_t pud; } pud_t;
typedef struct { pmdval_t pmd; } pmd_t;
typedef struct { pteval_t pte; } pte_t;

#define pgd_val(x)	((x).pgd)
#define p4d_val(x)	((x).p4d)
#define pud_val(x)	((x).pud)
#define pmd_val(x)	((x).pmd)
#define pte_val(x)	((x).pte)

#define __pgd(x)	((pgd_t) { (x) } )
#define __p4d(x)	((p4d_t) { (x) } )
#define __pud(x)	((pud_t) { (x) } )
#define __pmd(x)	((pmd_t) { (x) } )
#define __pte(x)	((pte_t) { (x) } )

#endif
