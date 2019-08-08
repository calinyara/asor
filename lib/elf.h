// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#ifndef __ELF_H__
#define __ELF_H__

#include <unistd.h>
#include <linux/elf.h>

#define cacheflush(a1, a2, a3) __builtin___clear_cache(a1, a1 + a2)

#if defined(X64)
	typedef Elf64_Ehdr Elf_Ehdr;
	typedef Elf64_Phdr Elf_Phdr;
	typedef Elf64_Sym Elf_Sym;
	typedef Elf64_Shdr Elf_Shdr;
	typedef Elf64_Rel Elf_Rel;
	typedef Elf64_Word Elf_Word;
	#define ELF_R_SYM(x) ELF64_R_SYM(x)
	#define ELF_R_TYPE(x) ELF64_R_TYPE(x)
#else
	typedef Elf32_Ehdr Elf_Ehdr;
	typedef Elf32_Phdr Elf_Phdr;
	typedef Elf32_Sym Elf_Sym;
	typedef Elf32_Shdr Elf_Shdr;
	typedef Elf32_Rel Elf_Rel;
	typedef Elf32_Word Elf_Word;
	#define ELF_R_SYM(x) ELF32_R_SYM(x)
	#define ELF_R_TYPE(x) ELF32_R_TYPE(x)
#endif

#define STACK_SIZE (8*1024*1024)
#define STACK_STORAGE_SIZE 0x5000
#define STACK_STRING_SIZE 0x5000

#define ROUND_UP(v, s) ((v + s - 1) & -s)
#define ROUND_DOWN(v, s) (v & -s)

/* Not all archs have this one defined */
#ifndef AT_RANDOM
	#define AT_RANDOM 25
#endif

struct ATENTRY {
	size_t id;
	size_t value;
} __attribute__((packed));

void *elf_sym(void *elf_start, char *sym_name);

/*
 * brief Map the ELF into memory.
 */
void elf_load(char *elf_start, void *stack, int stack_size, size_t *base_addr, size_t *entry);

/*
 * brief Map the ELF into memory and run it with the provided arguments.
 */
void elf_run(void *buf, char **argv, char **env);

#endif /* __ELF_H__ */

