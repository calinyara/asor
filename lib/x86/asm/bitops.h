#ifndef _ASMX86_BITOPS_H_
#define _ASMX86_BITOPS_H_

#ifndef _BITOPS_H_
#error only <bitops.h> can be included directly
#endif

#ifdef __x86_64__
#define BITS_PER_LONG	64
#else
#define BITS_PER_LONG	32
#endif

#define HAVE_BUILTIN_FLS 1

#endif
