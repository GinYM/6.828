#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

struct tx_desc tx_ring[NTXDESC] __attribute__ ((aligned(16)));
struct rx_desc rx_ring[NRXDESC] __attribute__ ((aligned(16)));


char tx_desc_buffers[NTXDESC][TPACK_MAX_SIZE];
char rx_desc_buffers[NRXDESC][RPACK_MAX_SIZE];




volatile uint32_t *attached_e1000;



//default MAC address of 52:54:00:12:34:56
void set_mac_addr(){
	attached_e1000[RAL(0)] = 0x12005452;
	attached_e1000[RAH(0)] = 0x80005634;
	
}

int
pci_attach_82540em(struct pci_func *f){
	pci_func_enable(f);
	attached_e1000 = mmio_map_region(f->reg_base[0], f->reg_size[0]);
	//cprintf("The attached_e1000: %x\n",attached_e1000);
	
	//transmit initialization

	//Allocate a region of memory for the transmit descriptor list. 
	//Software should insure this memory is
	//aligned on a paragraph (16-byte) boundary
	physaddr_t td_addr = PADDR(tx_ring);

	attached_e1000[TDBAL] = td_addr;
	attached_e1000[TDBAH] = 0;

	//Set the Transmit Descriptor Length (TDLEN) register 
	//to the size (in bytes) of the descriptor ring
	attached_e1000[TDLEN] = sizeof(tx_ring);

	attached_e1000[TDH] = 0;
	attached_e1000[TDT] = 0;

	//Initialize the Transmit Control Register (TCTL)
	// *Set the Enable (TCTL.EN) bit to 1b for normal operation.
	attached_e1000[TCTL] = TCTL_EN(1)
		|TCTL_PSP(1)
		|TCTL_CT(0x10)
		|TCTL_COLD(0x40);


	//Program the Transmit IPG (TIPG) register with the following decimal values to get the minimum
	//legal Inter Packet Gap:
	attached_e1000[TIPG] = TIPG_IPGT(0xA)
		|TIPG_IPGR1(0X8)
		|TIPG_IPGR2(0xC);

	// init transmit descriptor ring
	memset(tx_ring, 0, sizeof(tx_ring));
	for(int i = 0;i<NTXDESC;i++){
		tx_ring[i].addr = PADDR(tx_desc_buffers[i]);
		tx_ring[i].status = TDESC_STATUS_DD;
	}

	//receive initialization
	
	//init receive descriptor ring
	memset(rx_ring, 0, sizeof(rx_ring));
	for(int i = 0;i<NRXDESC;i++){
		rx_ring[i].addr = PADDR(rx_desc_buffers[i]);
	}
	
	//set MAC address
	set_mac_addr();
	memset((void*)&attached_e1000[MTA_BASE],0,MTA_SIZE);
	attached_e1000[IMS] = 0;
	attached_e1000[RDBAL] = PADDR(rx_ring);
	attached_e1000[RDBAH] = 0;
	attached_e1000[RDLEN] = sizeof(rx_ring);
	attached_e1000[RDH] = 0;
	attached_e1000[RDT] = NRXDESC;
	attached_e1000[RCTL] = RCTL_EN(1) 
		| RCTL_LPE(0) 
		| RCTL_LBM(0) 
		| RCTL_BAM(1) 
		| RCTL_BSIZE(0) 
		| RCTL_SECRC(1);


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
	//static int real_tail = 0;

	if(addr == NULL || len == 0){
		return 0;
	}
	uint16_t size=0;
	int next=0; 
	void *buffer; 
	int size_send = 0;

	while(len){
		//next = real_tail;
		next = attached_e1000[TDT]%NTXDESC;

		//if(next == attached_e1000[TDH]%NTXDESC){
		//	return -1;
		//}

		size = len>PACKET_MAX_SIZE?PACKET_MAX_SIZE:len;

		//while(!is_td_free(next));
		if(!is_td_free(next)){
			return -1;
		}
		tx_ring[next].status ^= TDESC_STATUS_DD;
		buffer = KADDR(tx_ring[next].addr);
		memmove(buffer,addr+size_send,size);
		size_send+=size;

		tx_ring[next].cso = 0;
		tx_ring[next].special = 0;

		tx_ring[next].length = size;
		len-=size;
		if(len == 0){
			tx_ring[next].cmd |= TDESC_CMD_RS | TDESC_CMD_EOP;
		}
		else{
			tx_ring[next].cmd |= TDESC_CMD_RS;
		}
		attached_e1000[TDT] = (next+1)%NTXDESC;
	}

	return size_send;
}

int is_rd_used(int next){
	if((rx_ring[next].status&RDESC_STATUS_DD)  ){
		return 1;
	}
	else{
		return 0;
	}
}


int is_rd_eop(int next){
	if((rx_ring[next].status&RDESC_STATUS_EOP) ){
		return 1;
	}
	else{
		return 0;
	}
}

int recv_data_at(void *addr){

	if(addr == NULL ){
		return 0;
	}

	static int real_tail = 0;


	int next = real_tail;
	//attached_e1000[RDT]%NRXDESC;
	//if(attached_e1000[RDH] == next){
	//	return -1;
	//}
	//cprintf("Next is %d\n",next);
	void* buffer;
	int size;
	//void*buffer = KADDR(rx_ring[next].addr);

	//int size = len>rx_ring[next].length?rx_ring[next].length:len;

	if(is_rd_used(next)){
		//cprintf("Here?\n");
		buffer = KADDR(rx_ring[next].addr);
		size = rx_ring[next].length;

		//cprintf("next is %d\n",next);
		//cprintf("rx_ring[next].length %d\n",rx_ring[next].length);

		memmove(addr,buffer,size);
		attached_e1000[RDT] = next;//(next+1)%NRXDESC; //fuck!!!!!
		rx_ring[next].status ^= RDESC_STATUS_DD;
		rx_ring[next].length = size;
		real_tail = (real_tail+1)%NRXDESC;
		
		//cprintf("res->nread:%d\n",res->nread);
		//cprintf("is_eop: %d\n",*is_eop);
		return size;
	}
	else{
		return -1;
	}
}
