// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	//cprintf("err is %x\n",err);
	//cprintf("uvpt is %x\n",uvpt[PGNUM(addr)]);
	//cprintf("addr is %x\n",addr);
	if((err&FEC_WR) ==0 || ((uvpt[PGNUM(addr)]&PTE_COW) ==0)){
		panic("panic pgfault!");
	}
	//cprintf("Here???\n");
	

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	envid_t envid =	0;//sys_getenvid();

	//cprintf("envid is %d\n",envid);

	addr = ROUNDDOWN(addr,PGSIZE);

	if((r = sys_page_alloc(envid, PFTEMP, PTE_W|PTE_P|PTE_U)) <0 ){
		panic("sys_page_alloc panic %e",r);
	}
	memcpy((void*)PFTEMP,addr,PGSIZE);

	if((r = sys_page_map(envid, PFTEMP,envid, addr, PTE_W|PTE_P|PTE_U)) < 0){
		panic("sys_page_map %e",r);
	}

	if((r = sys_page_unmap(envid, PFTEMP)) < 0){
		panic("sys_page_map %e",r);
	}


	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	envid_t curenvid = sys_getenvid();
	if(curenvid<0){
		return curenvid;
	}

	// LAB 4: Your code here.
	//panic("duppage not implemented");
	//cprintf("Here!\n");
	if(uvpt[pn]&PTE_SHARE){
		//cprintf("perm is %d\n",uvpt[PGNUM(pn)] );
		if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), uvpt[pn] & PTE_SYSCALL)) < 0)
			panic("sys_page_map fail: %e",r);
		return 0;
	}
	
	int perm;
	if((uvpt[pn]&PTE_COW) !=0 || (uvpt[pn]&PTE_W) !=0 ){
		
		perm = PTE_P|PTE_U|PTE_COW;
		
		
		if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), perm)) < 0)
			panic("sys_page_map fail: %e",r);
		if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), thisenv->env_id, (void*)(pn*PGSIZE), perm)) < 0)
			panic("sys_page_map fail: %e",r);
		
	}
	else{
		//cprintf("Here??\n");
		
		perm = PTE_P|PTE_U;
		
		//perm = PTE_P|PTE_U;
		if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), perm)) < 0)
			panic("sys_page_map fail: %e",r);
		
	}

	//cprintf("parent envid %d child envid %d\n",curenvid,envid);

	//if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), perm)) < 0)
	//	panic("sys_page_map fail: %e",r);
	//if ((r = sys_page_map(thisenv->env_id, (void*)(pn*PGSIZE), thisenv->env_id, (void*)(pn*PGSIZE), perm)) < 0)
	//	panic("sys_page_map fail: %e",r);
	

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
	uint32_t addr;
	int r;
	set_pgfault_handler(pgfault);
	envid_t child = sys_exofork();
	//cprintf("Child is %d\n",child);
	if(child<0){
		panic("sys_exofork fails: %e",child);
	}
	if(child == 0){
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}


	//uint32_t uvpt_addr;
	//uint32_t uvpd_addr;
	for (addr = 0; addr < UTOP-PGSIZE; addr += PGSIZE){
		//uvpt_addr = uvpt[PGNUM(addr)];
		//uvpd_addr = uvpd[PDX(addr)];

		if( ( (uvpd[PDX(addr)]&PTE_P) && (uvpt[PGNUM(addr)]&PTE_P)  )){
			//cprintf("uvpt_addr is %x\n",uvpt_addr);
			//thisenv->env_pgdir = (pde_t *)uvpd[PDX(addr)];

			//cprintf("uvpt_addr is %x\n",uvpt_addr);
			//cprintf("uvpd_addr is %x\n",uvpd_addr);

			//cprintf("pgnum is %x\n",PGNUM(addr));

			if((r=duppage(child, PGNUM(addr))) < 0){
				//cprintf("pgnum is %x\n",PGNUM(addr));
				panic("duppage fail: %e",r);
			}
		}
		
	}
	//cprintf("Finished\n");

	//duppage(child);

	if((r=sys_page_alloc(child, (void*)(UTOP-PGSIZE), PTE_P|PTE_U|PTE_W))<0){
		panic("sys_page_alloc: %e",r);
	}


	if((r = sys_env_set_pgfault_upcall(child, thisenv->env_pgfault_upcall))<0){
		panic("sys_env_set_pgfault_upcall: %e",r);
	}

	if ((r = sys_env_set_status(child, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);


	return child;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
