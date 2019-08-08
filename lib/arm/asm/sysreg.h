#ifndef _ASMARM_SYSREG_H_
#define _ASMARM_SYSREG_H_
/*
 * From the Linux kernel arch/arm/include/asm/cp15.h
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)
#define CR_HA	(1 << 17)	/* Hardware management of Access Flag	*/
#define CR_IT	(1 << 18)
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

#ifndef __ASSEMBLY__
#include <libcflat.h>

#define __ACCESS_CP15(CRn, Op1, CRm, Op2)			\
	"mrc", "mcr", xstr(p15, Op1, %0, CRn, CRm, Op2), u32
#define __ACCESS_CP15_64(Op1, CRm)					\
	"mrrc", "mcrr", xstr(p15, Op1, %Q0, %R0, CRm), u64

#define __ACCESS_CP14(CRn, Op1, CRm, Op2)	\
	"mrc", "mcr", xstr(p14, Op1, %0, CRn, CRm, Op2), u32
#define __ACCESS_CP14_64(Op1, CRm)		\
	"mrrc", "mcrr", xstr(p14, Op1, %Q0, %R0, CRm), u64

#define __read_sysreg(r, w, c, t) ({				\
			t __val;				\
			asm volatile(r " " c : "=r" (__val));	\
			__val;					\
		})
#define read_sysreg(...)                 __read_sysreg(__VA_ARGS__)

#define __write_sysreg(v, r, w, c, t)   asm volatile(w " " c : : "r" ((t)(v)))
#define write_sysreg(v, ...)            __write_sysreg(v, __VA_ARGS__)
#endif /* !__ASSEMBLY__ */

#endif /* _ASMARM_SYSREG_H_ */
