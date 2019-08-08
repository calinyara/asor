#ifndef _ASMPOWERPC_PPC_ASM_H
#define _ASMPOWERPC_PPC_ASM_H

#include <asm/asm-offsets.h>

#define SAVE_GPR(n, base)	std	n,GPR0+8*(n)(base)
#define REST_GPR(n, base)	ld	n,GPR0+8*(n)(base)

#define LOAD_REG_IMMEDIATE(reg,expr)		\
	lis	reg,(expr)@highest;		\
	ori	reg,reg,(expr)@higher;		\
	rldicr	reg,reg,32,31;			\
	oris	reg,reg,(expr)@h;		\
	ori	reg,reg,(expr)@l;

#define LOAD_REG_ADDR(reg,name)			\
	ld	reg,name@got(r2)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define FIXUP_ENDIAN

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define FIXUP_ENDIAN				\
	.long 0x05000048; /* bl . + 4        */ \
	.long 0xa602487d; /* mflr r10        */	\
	.long 0x20004a39; /* addi r10,r10,32 */	\
	.long 0xa600607d; /* mfmsr r11       */	\
	.long 0x01006b69; /* xori r11,r11,1  */	\
	.long 0xa6035a7d; /* mtsrr0 r10	     */	\
	.long 0xa6037b7d; /* mtsrr1 r11      */	\
	.long 0x2400004c; /* rfid            */ \
	.long 0x00000048; /* b .             */ \

#endif /* __BYTE_ORDER__ */

#endif /* _ASMPOWERPC_PPC_ASM_H */
