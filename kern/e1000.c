#include <kern/e1000.h>

// LAB 6: Your driver code here

struct tx_desc tx_ring[NTXDESC] __attribute__ ((aligned(16)));

packet_buffer tx_desc_buffers[NTXDESC];


volatile uint32_t *attached_e1000;

int
pci_attach_82540em(struct pci_func *f){
	pci_func_enable(f);
	attached_e1000 = mmio_map_region(f->reg_base[0], f->reg_size[0]);
	//cprintf("The attached_e1000: %x\n",attached_e1000);
	
	//Allocate a region of memory for the transmit descriptor list. 
	//Software should insure this memory is
	//aligned on a paragraph (16-byte) boundary
	physaddr_t td_addr = PADDR(tx_ring);

	attached_e1000[TDBAL] = td_addr;
	attached_e1000[TDBAH] = 0;

	//Set the Transmit Descriptor Length (TDLEN) register 
	//to the size (in bytes) of the descriptor ring
	attached_e1000[TDLEN] = sizeof(tx_ring);

	attached_e1000[TDH] = 0x0b;
	attached_e1000[TDT] = 0x0b;

	//Initialize the Transmit Control Register (TCTL)
	// *Set the Enable (TCTL.EN) bit to 1b for normal operation.
	attached_e1000[TCTL] = TCTL_EN(1)|TCTL_PSP(1)|TCTL_CT(0x10)|TCTL_COLD(0x40);


	//Program the Transmit IPG (TIPG) register with the following decimal values to get the minimum
	//legal Inter Packet Gap:
	attached_e1000[TIPG] = TIPG_IPGT(0xA)|TIPG_IPGR1(0X8)|TIPG_IPGR2(0xC);

	// init transmit descriptor ring
	memset(tx_ring, 0, sizeof(tx_ring));
	for(int i = 0;i<NTXDESC;i++){
		tx_ring[i].addr = PADDR(tx_desc_buffers[i]);
		tx_ring[i].status = TDESC_STATUS_DD;
	}

	//send_data_at("hello",5);

	return 0;
}

int is_td_free(int next){
	if((tx_ring[next].status&TDESC_STATUS_DD)!=0){
		return 1;
	}
	else{
		return -1;
	}
}

int send_data_at(void *addr, uint16_t len) {

	//cprintf("addr is %x\n",addr);

	if(addr == NULL || len == 0){
		return 0;
	}

	uint16_t size = len>PACKET_MAX_SIZE?PACKET_MAX_SIZE:len;
	
	int next = attached_e1000[TDT];
	if(is_td_free(next)){
		//cprintf("addr for tx_ring: %x\n",tx_ring[next].addr);
		//cprintf("size is: %d\n",size);
		void *buffer = KADDR(tx_ring[next].addr);
		memmove(buffer,addr,size);

		tx_ring[next].status ^= TDESC_STATUS_DD;

		tx_ring[next].cmd |= TDESC_CMD_RS | TDESC_CMD_EOP;
		
		tx_ring[next].length = size;
		attached_e1000[TDT] = (next+1)%NTXDESC;
		return size;

	}
	else{
		return -1;
	}

}
