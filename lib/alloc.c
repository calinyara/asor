#include "alloc.h"
#include "asm/page.h"

void *malloc(size_t size)
{
	return memalign(sizeof(long), size);
}

void *calloc(size_t nmemb, size_t size)
{
	void *ptr = malloc(nmemb * size);
	if (ptr)
		memset(ptr, 0, nmemb * size);
	return ptr;
}

#define METADATA_EXTRA	(2 * sizeof(uintptr_t))
#define OFS_SLACK	(-2 * sizeof(uintptr_t))
#define OFS_SIZE	(-sizeof(uintptr_t))

static inline void *block_begin(void *mem)
{
	uintptr_t slack = *(uintptr_t *)(mem + OFS_SLACK);
	return mem - slack;
}

static inline uintptr_t block_size(void *mem)
{
	return *(uintptr_t *)(mem + OFS_SIZE);
}

void free(void *ptr)
{
	if (!alloc_ops->free)
		return;

	void *base = block_begin(ptr);
	uintptr_t sz = block_size(ptr);

	alloc_ops->free(base, sz);
}

void *memalign(size_t alignment, size_t size)
{
	void *p;
	uintptr_t blkalign;
	uintptr_t mem;

	assert(alloc_ops && alloc_ops->memalign);
	if (alignment <= sizeof(uintptr_t))
		alignment = sizeof(uintptr_t);
	else
		size += alignment - 1;

	blkalign = MAX(alignment, alloc_ops->align_min);
	size = ALIGN(size + METADATA_EXTRA, alloc_ops->align_min);
	p = alloc_ops->memalign(blkalign, size);

	/* Leave room for metadata before aligning the result.  */
	mem = (uintptr_t)p + METADATA_EXTRA;
	mem = ALIGN(mem, alignment);

	/* Write the metadata */
	*(uintptr_t *)(mem + OFS_SLACK) = mem - (uintptr_t)p;
	*(uintptr_t *)(mem + OFS_SIZE) = size;
	return (void *)mem;
}
