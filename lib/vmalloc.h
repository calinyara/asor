#ifndef VMALLOC_H
#define VMALLOC_H 1

#include <asm/page.h>

extern void *alloc_vpages(ulong nr);
extern void *alloc_vpage(void);
extern void init_alloc_vpage(void *top);
extern void setup_vm(void);

extern void *setup_mmu(phys_addr_t top);
extern phys_addr_t virt_to_pte_phys(pgd_t *pgtable, void *virt);
extern pteval_t *install_page(pgd_t *pgtable, phys_addr_t phys, void *virt);

void *vmap(phys_addr_t phys, size_t size);

#endif
