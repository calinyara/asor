#ifndef _ASMPPC64_BARRIER_H_
#define _ASMPPC64_BARRIER_H_

#define mb() asm volatile("sync":::"memory")
#define rmb() asm volatile("sync":::"memory")
#define wmb() asm volatile("sync":::"memory")

#include <asm-generic/barrier.h>
#endif
