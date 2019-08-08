// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "vmx.h"

static void asor_guest_main(void)
{
	printf("Hello Guest\n");
}

static int asor_guest_exit_handler(void)
{
	print_vmexit_info();
	return VMX_EXIT;
}

struct asor_guest asor_guests[] = {
	{ "default guest", NULL, asor_guest_main, asor_guest_exit_handler, NULL, {0} },
	{ NULL, NULL, NULL, NULL, NULL, {0} },
};

