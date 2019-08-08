#include "delay.h"

void delay(u64 count)
{
	while (count--)
		asm volatile("pause");
}

