/*
 * SCLP definitions
 *
 * Based on the file pc-bios/s390-ccw/sclp.h from QEMU
 * Copyright (c) 2013 Alexander Graf <agraf@suse.de>
 *
 * and based on the file include/hw/s390x/sclp.h from QEMU
 * Copyright IBM, Corp. 2012
 * Author: Christian Borntraeger <borntraeger@de.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or (at
 * your option) any later version. See the COPYING file in the top-level
 * directory.
 */

#ifndef SCLP_H
#define SCLP_H

#define SCLP_CMD_CODE_MASK                      0xffff00ff

/* SCLP command codes */
#define SCLP_CMDW_READ_SCP_INFO                 0x00020001
#define SCLP_CMDW_READ_SCP_INFO_FORCED          0x00120001
#define SCLP_READ_STORAGE_ELEMENT_INFO          0x00040001
#define SCLP_ATTACH_STORAGE_ELEMENT             0x00080001
#define SCLP_ASSIGN_STORAGE                     0x000D0001
#define SCLP_CMD_READ_EVENT_DATA                0x00770005
#define SCLP_CMD_WRITE_EVENT_DATA               0x00760005
#define SCLP_CMD_READ_EVENT_DATA                0x00770005
#define SCLP_CMD_WRITE_EVENT_DATA               0x00760005
#define SCLP_CMD_WRITE_EVENT_MASK               0x00780005

/* SCLP Memory hotplug codes */
#define SCLP_FC_ASSIGN_ATTACH_READ_STOR         0xE00000000000ULL
#define SCLP_STARTING_SUBINCREMENT_ID           0x10001
#define SCLP_INCREMENT_UNIT                     0x10000
#define MAX_AVAIL_SLOTS                         32
#define MAX_STORAGE_INCREMENTS                  1020

/* CPU hotplug SCLP codes */
#define SCLP_HAS_CPU_INFO                       0x0C00000000000000ULL
#define SCLP_CMDW_READ_CPU_INFO                 0x00010001
#define SCLP_CMDW_CONFIGURE_CPU                 0x00110001
#define SCLP_CMDW_DECONFIGURE_CPU               0x00100001

/* SCLP PCI codes */
#define SCLP_HAS_IOA_RECONFIG                   0x0000000040000000ULL
#define SCLP_CMDW_CONFIGURE_IOA                 0x001a0001
#define SCLP_CMDW_DECONFIGURE_IOA               0x001b0001
#define SCLP_RECONFIG_PCI_ATYPE                 2

/* SCLP response codes */
#define SCLP_RC_NORMAL_READ_COMPLETION          0x0010
#define SCLP_RC_NORMAL_COMPLETION               0x0020
#define SCLP_RC_SCCB_BOUNDARY_VIOLATION         0x0100
#define SCLP_RC_NO_ACTION_REQUIRED              0x0120
#define SCLP_RC_INVALID_SCLP_COMMAND            0x01f0
#define SCLP_RC_CONTAINED_EQUIPMENT_CHECK       0x0340
#define SCLP_RC_INSUFFICIENT_SCCB_LENGTH        0x0300
#define SCLP_RC_STANDBY_READ_COMPLETION         0x0410
#define SCLP_RC_ADAPTER_IN_RESERVED_STATE       0x05f0
#define SCLP_RC_ADAPTER_TYPE_NOT_RECOGNIZED     0x06f0
#define SCLP_RC_ADAPTER_ID_NOT_RECOGNIZED       0x09f0
#define SCLP_RC_INVALID_FUNCTION                0x40f0
#define SCLP_RC_NO_EVENT_BUFFERS_STORED         0x60f0
#define SCLP_RC_INVALID_SELECTION_MASK          0x70f0
#define SCLP_RC_INCONSISTENT_LENGTHS            0x72f0
#define SCLP_RC_EVENT_BUFFER_SYNTAX_ERROR       0x73f0
#define SCLP_RC_INVALID_MASK_LENGTH             0x74f0

/* SCLP control mask bits */
#define SCLP_CM2_VARIABLE_LENGTH_RESPONSE       0x80

/* SCLP function codes */
#define SCLP_FC_NORMAL_WRITE                    0
#define SCLP_FC_SINGLE_INCREMENT_ASSIGN         0x40
#define SCLP_FC_DUMP_INDICATOR                  0x80

/* SCLP event buffer flags */
#define SCLP_EVENT_BUFFER_ACCEPTED              0x80

/* Service Call Control Block (SCCB) and its elements */
#define SCCB_SIZE 4096

typedef struct SCCBHeader {
    uint16_t length;
    uint8_t function_code;
    uint8_t control_mask[3];
    uint16_t response_code;
} __attribute__((packed)) SCCBHeader;

#define SCCB_DATA_LEN (SCCB_SIZE - sizeof(SCCBHeader))
#define SCCB_CPU_FEATURE_LEN 6

/* CPU information */
typedef struct CPUEntry {
    uint8_t address;
    uint8_t reserved0;
    uint8_t features[SCCB_CPU_FEATURE_LEN];
    uint8_t reserved2[6];
    uint8_t type;
    uint8_t reserved1;
} __attribute__((packed)) CPUEntry;

typedef struct ReadInfo {
    SCCBHeader h;
    uint16_t rnmax;
    uint8_t rnsize;
    uint8_t  _reserved1[16 - 11];       /* 11-15 */
    uint16_t entries_cpu;               /* 16-17 */
    uint16_t offset_cpu;                /* 18-19 */
    uint8_t  _reserved2[24 - 20];       /* 20-23 */
    uint8_t  loadparm[8];               /* 24-31 */
    uint8_t  _reserved3[48 - 32];       /* 32-47 */
    uint64_t facilities;                /* 48-55 */
    uint8_t  _reserved0[76 - 56];       /* 56-75 */
    uint32_t ibc_val;
    uint8_t  conf_char[99 - 80];        /* 80-98 */
    uint8_t mha_pow;
    uint32_t rnsize2;
    uint64_t rnmax2;
    uint8_t  _reserved6[116 - 112];     /* 112-115 */
    uint8_t  conf_char_ext[120 - 116];   /* 116-119 */
    uint16_t highest_cpu;
    uint8_t  _reserved5[124 - 122];     /* 122-123 */
    uint32_t hmfai;
    struct CPUEntry entries[0];
} __attribute__((packed)) ReadInfo;

typedef struct ReadCpuInfo {
    SCCBHeader h;
    uint16_t nr_configured;         /* 8-9 */
    uint16_t offset_configured;     /* 10-11 */
    uint16_t nr_standby;            /* 12-13 */
    uint16_t offset_standby;        /* 14-15 */
    uint8_t reserved0[24-16];       /* 16-23 */
    struct CPUEntry entries[0];
} __attribute__((packed)) ReadCpuInfo;

typedef struct ReadStorageElementInfo {
    SCCBHeader h;
    uint16_t max_id;
    uint16_t assigned;
    uint16_t standby;
    uint8_t _reserved0[16 - 14]; /* 14-15 */
    uint32_t entries[0];
} __attribute__((packed)) ReadStorageElementInfo;

typedef struct AttachStorageElement {
    SCCBHeader h;
    uint8_t _reserved0[10 - 8];  /* 8-9 */
    uint16_t assigned;
    uint8_t _reserved1[16 - 12]; /* 12-15 */
    uint32_t entries[0];
} __attribute__((packed)) AttachStorageElement;

typedef struct AssignStorage {
    SCCBHeader h;
    uint16_t rn;
} __attribute__((packed)) AssignStorage;

typedef struct IoaCfgSccb {
    SCCBHeader header;
    uint8_t atype;
    uint8_t reserved1;
    uint16_t reserved2;
    uint32_t aid;
} __attribute__((packed)) IoaCfgSccb;

typedef struct SCCB {
    SCCBHeader h;
    char data[SCCB_DATA_LEN];
 } __attribute__((packed)) SCCB;

/* SCLP event types */
#define SCLP_EVENT_ASCII_CONSOLE_DATA           0x1a
#define SCLP_EVENT_SIGNAL_QUIESCE               0x1d

/* SCLP event masks */
#define SCLP_EVENT_MASK_SIGNAL_QUIESCE          0x00000008
#define SCLP_EVENT_MASK_MSG_ASCII               0x00000040

#define SCLP_UNCONDITIONAL_READ                 0x00
#define SCLP_SELECTIVE_READ                     0x01

typedef struct WriteEventMask {
    SCCBHeader h;
    uint16_t _reserved;
    uint16_t mask_length;
    uint32_t cp_receive_mask;
    uint32_t cp_send_mask;
    uint32_t send_mask;
    uint32_t receive_mask;
} __attribute__((packed)) WriteEventMask;

typedef struct EventBufferHeader {
    uint16_t length;
    uint8_t  type;
    uint8_t  flags;
    uint16_t _reserved;
} __attribute__((packed)) EventBufferHeader;

typedef struct WriteEventData {
    SCCBHeader h;
    EventBufferHeader ebh;
    char data[0];
} __attribute__((packed)) WriteEventData;

typedef struct ReadEventData {
    SCCBHeader h;
    EventBufferHeader ebh;
    uint32_t mask;
} __attribute__((packed)) ReadEventData;

extern char _sccb[];
void sclp_console_setup(void);
void sclp_print(const char *str);
int sclp_service_call(unsigned int command, void *sccb);
void sclp_memory_setup(void);

#endif /* SCLP_H */
