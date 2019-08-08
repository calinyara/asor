#include "libcflat.h"
#include "asm/barrier.h"

int getchar(void)
{
	int c;

	while ((c = __getchar()) == -1)
		cpu_relax();
	return c;
}
