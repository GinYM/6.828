#include "ns.h"
#include <inc/lib.h>

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	for(;;){
		int r = ipc_recv(NULL,&nsipcbuf,NULL);
		if(r<0){
			panic("output fails ipc_recv: %e",r);
		}
		else{
			void * data = nsipcbuf.pkt.jp_data;
			int data_len = nsipcbuf.pkt.jp_len;
			//cprintf("data: %c\n",(char*)data);
			//cprintf("data_len: %d\n",data_len);
			r = sys_send_data_at(data,data_len);
			if(r<0){
				panic("output fails sys_send_data_at: %e",r);
			}
		}
	}
}
