#ifndef _ASMPPC64_IO_H_
#define _ASMPPC64_IO_H_

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define __cpu_is_be() (0)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define __cpu_is_be() (1)
#else
#error Undefined byte order
#endif

#define __iomem

#include <asm-generic/io.h>
#endif
