/*
 * SCLP ASCII access driver
 *
 * Copyright (c) 2013 Alexander Graf <agraf@suse.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or (at
 * your option) any later version. See the COPYING file in the top-level
 * directory.
 */

#include <libcflat.h>
#include <string.h>
#include <asm/page.h>
#include "sclp.h"

static void sclp_set_write_mask(void)
{
	WriteEventMask *sccb = (void *)_sccb;

	sccb->h.length = sizeof(WriteEventMask);
	sccb->mask_length = sizeof(unsigned int);
	sccb->receive_mask = SCLP_EVENT_MASK_MSG_ASCII;
	sccb->cp_receive_mask = SCLP_EVENT_MASK_MSG_ASCII;
	sccb->send_mask = SCLP_EVENT_MASK_MSG_ASCII;
	sccb->cp_send_mask = SCLP_EVENT_MASK_MSG_ASCII;

	sclp_service_call(SCLP_CMD_WRITE_EVENT_MASK, sccb);
}

void sclp_console_setup(void)
{
	sclp_set_write_mask();
}

void sclp_print(const char *str)
{
	int len = strlen(str);
	WriteEventData *sccb = (void *)_sccb;

	sccb->h.length = sizeof(WriteEventData) + len;
	sccb->h.function_code = SCLP_FC_NORMAL_WRITE;
	sccb->ebh.length = sizeof(EventBufferHeader) + len;
	sccb->ebh.type = SCLP_EVENT_ASCII_CONSOLE_DATA;
	sccb->ebh.flags = 0;
	memcpy(sccb->data, str, len);

	sclp_service_call(SCLP_CMD_WRITE_EVENT_DATA, sccb);
}
