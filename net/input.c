#include "ns.h"



extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{

	//cprintf("Here\n");

	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int r;
	int perm = PTE_P | PTE_W | PTE_U;
	//char buffer[RPACK_MAX_SIZE];
	
	for(;;){

		if((r = sys_page_alloc(0, &nsipcbuf, PTE_U | PTE_P | PTE_W)) != 0){
			panic("sys_page_alloc: %e", r);
		}
		

		while((r = (sys_recv_data_at(nsipcbuf.pkt.jp_data))) < 0){
			sys_yield();
		}
		
		//memmove(nsipcbuf.pkt.jp_data,buffer,r);
		nsipcbuf.pkt.jp_len = r;
		ipc_send(ns_envid,NSREQ_INPUT,&nsipcbuf,perm);
				
	
	}
	
}
