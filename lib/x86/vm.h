#ifndef VM_H
#define VM_H

#include "processor.h"
#include "asm/page.h"
#include "asm/io.h"

void setup_5level_page_table(void);

struct pte_search {
	int level;
	pteval_t *pte;
};

static inline bool found_huge_pte(struct pte_search search)
{
	return (search.level == 2 || search.level == 3) &&
	       (*search.pte & PT_PRESENT_MASK) &&
	       (*search.pte & PT_PAGE_SIZE_MASK);
}

static inline bool found_leaf_pte(struct pte_search search)
{
	return search.level == 1 || found_huge_pte(search);
}

struct pte_search find_pte_level(pgd_t *cr3, void *virt,
				 int lowest_level);
pteval_t *get_pte(pgd_t *cr3, void *virt);
pteval_t *get_pte_level(pgd_t *cr3, void *virt, int pte_level);
pteval_t *install_pte(pgd_t *cr3,
		      int pte_level,
		      void *virt,
		      pteval_t pte,
		      pteval_t *pt_page);

pteval_t *install_large_page(pgd_t *cr3, phys_addr_t phys, void *virt);
void install_pages(pgd_t *cr3, phys_addr_t phys, size_t len, void *virt);
bool any_present_pages(pgd_t *cr3, void *virt, size_t len);

static inline void *current_page_table(void)
{
	return phys_to_virt(read_cr3());
}

void split_large_page(unsigned long *ptep, int level);
void force_4k_page(void *addr);
#endif
