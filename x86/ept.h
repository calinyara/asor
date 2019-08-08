// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#ifndef __EPT_H__
#define __EPT_H__

#include "libcflat.h"

int setup_ept(bool enable_ad);

#endif /* __EPT_H__ */

