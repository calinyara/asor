/*
 * Physical memory management related functions and definitions.
 *
 * Copyright IBM Corp. 2018
 * Author(s): Janosch Frank <frankja@de.ibm.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#ifndef _ASM_S390_MEM_H
#define _ASM_S390_MEM_H

#define SKEY_ACC	0xf0
#define SKEY_FP		0x08
#define SKEY_RF		0x04
#define SKEY_CH		0x02

union skey {
	struct {
		uint8_t acc : 4;
		uint8_t fp : 1;
		uint8_t rf : 1;
		uint8_t ch : 1;
		uint8_t pad : 1;
	} str;
	uint8_t val;
};

static inline void set_storage_key(unsigned long addr,
				   unsigned char skey,
				   int nq)
{
	if (nq)
		asm volatile(".insn rrf,0xb22b0000,%0,%1,8,0"
			     : : "d" (skey), "a" (addr));
	else
		asm volatile("sske %0,%1" : : "d" (skey), "a" (addr));
}

static inline unsigned long set_storage_key_mb(unsigned long addr,
					       unsigned char skey)
{
	assert(test_facility(8));

	asm volatile(".insn rrf,0xb22b0000,%[skey],%[addr],1,0"
		     : [addr] "+a" (addr) : [skey] "d" (skey));
	return addr;
}

static inline unsigned char get_storage_key(unsigned long addr)
{
	unsigned char skey;

	asm volatile("iske %0,%1" : "=d" (skey) : "a" (addr));
	return skey;
}
#endif
