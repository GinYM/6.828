#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>
#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/string.h>


#define NTXDESC 32
#define PACKET_MAX_SIZE 1528

typedef char packet_buffer[PACKET_MAX_SIZE];

// Transmit Descriptor
#define TCTL   (0x00400 / 4)
#define TDBAL  (0x03800 / 4)
#define TDBAH  (0x03804 / 4)
#define TDLEN  (0x03808 / 4)
#define TDH    (0x03810 / 4)
#define TDT    (0x03818 / 4)
#define TIPG   (0x00410 / 4)

// transmit control register
#define TCTL_EN(x) ((x)<<1)
#define TCTL_PSP(x) ((x)<<3)
#define TCTL_CT(x) ((x)<<4)
#define TCTL_COLD(x) ((x)<<12)
#define TCTL_SWXOFF(x) ((x)<<22)

//transmit IPG register
#define TIPG_IPGT(x) (x)
#define TIPG_IPGR1(x) ((x)<<10)
#define TIPG_IPGR2(x) ((x)<<20)

#define TDESC_CMD_RS (1<<3)

#define TDESC_STATUS_DD 1
#define TDESC_CMD_EOP 1

int pci_attach_82540em(struct pci_func *f);

struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

int send_data_at(void *addr, uint16_t len);

#endif	// JOS_KERN_E1000_H
