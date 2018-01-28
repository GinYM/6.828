#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>
#include <kern/pci.h>
//#include <kern/pmap.h>
#include <inc/string.h>


#define NTXDESC 32
#define NRXDESC 256
#define PACKET_MAX_SIZE 1528

#define TPACK_MAX_SIZE 1528
#define RPACK_MAX_SIZE 2048

typedef char packet_buffer[PACKET_MAX_SIZE];

// Transmit Descriptor
#define TCTL   (0x00400 / 4)
#define TDBAL  (0x03800 / 4)
#define TDBAH  (0x03804 / 4)
#define TDLEN  (0x03808 / 4)
#define TDH    (0x03810 / 4)
#define TDT    (0x03818 / 4)
#define TIPG   (0x00410 / 4)

#define RAL_BASE (0x5400 / 4)
#define RAH_BASE (0x5404 / 4)
#define MTA_BASE (0x5200 / 4)
#define IMS (0x0D0 / 4)

//receiver descriptor

#define RDBAL (0x2800 / 4)
#define RDBAH (0x2804 / 4)
#define RDLEN (0x2808 / 4)
#define RDH (0x2810 / 4)
#define RDT (0x2818 / 4)
#define RCTL (0x0100 / 4)


#define MTA_SIZE (128 * 4)



#define RAL(i) (RAL_BASE + 8 * (i))
#define RAH(i) (RAH_BASE + 8 * (i))


// transmit control register
#define TCTL_EN(x) ((x)<<1)
#define TCTL_PSP(x) ((x)<<3)
#define TCTL_CT(x) ((x)<<4)
#define TCTL_COLD(x) ((x)<<12)
#define TCTL_SWXOFF(x) ((x)<<22)

// receive control register PAGE 317
#define RCTL_EN(x) ((x)<<1)
#define RCTL_LPE(x) ((x)<<5)
#define RCTL_LBM(x) ((x)<<6)
#define RCTL_BSIZE(x) ((x)<<16)
#define RCTL_BAM(x) ((x)<<15)
#define RCTL_SECRC(x) ((x)<<26)


//transmit IPG register
#define TIPG_IPGT(x) (x)
#define TIPG_IPGR1(x) ((x)<<10)
#define TIPG_IPGR2(x) ((x)<<20)

#define TDESC_CMD_RS (1<<3)

#define TDESC_STATUS_DD 1
#define TDESC_CMD_EOP 1

#define RDESC_STATUS_DD  1
#define RDESC_STATUS_EOP (1<<1)

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
} __attribute__ ((packed));

struct rx_desc
{
	uint64_t addr;
	uint16_t length;
	uint16_t checksum;
	uint8_t status;
	uint8_t errors;
	uint16_t special;
} __attribute__ ((packed));

struct rec_res{
	int is_eop;
	uint32_t nread;

};

int send_data_at(void *addr, uint16_t len);
int recv_data_at(void *addr);

#endif	// JOS_KERN_E1000_H
