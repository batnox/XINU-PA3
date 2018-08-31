/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/
int vgetmem_first[NPROC];
LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{

//	kprintf("To be implemented!\n");
//	return OK;
//
	STATWORD ps;
	disable(ps);

	int bs_number;
	

	if(hsize<=0 || hsize>BACKING_STORE_UNIT_SIZE){
		restore(ps);
		return SYSERR;
	}
	
	if(get_bsm(&bs_number)!=OK){
		restore(ps);
		return SYSERR;
	}
	
	int pid = create(procaddr, ssize, priority, name, nargs, args);

	//int bs_number = get_bsm(&bs_number);

	bs_map_t *bsm;

 	if(bsm_map(pid, 4096, bs_number, hsize)!=OK){
		//kprintf("VCREATE problem with bsm_map\n");
		restore(ps);
		return SYSERR;
	}
	/*else{
		kprintf("VCREATE bsm_mapped\n");
	}*/



	bsm_tab[bs_number].is_private = 1;	
	
	//	kprintf("VCREATE bsm_mapped 1\n");
	proctab[pid].store = bs_number;
	//	kprintf("VCREATE bsm_mapped 2\n");
	proctab[pid].vhpno = 4096;
	//	kprintf("VCREATE bsm_mapped 3\n");
	proctab[pid].vhpnpages = hsize;
	//	kprintf("VCREATE bsm_mapped 4\n");
	proctab[pid].vmemlist->mnext = 4096 * NBPG;
	//	kprintf("VCREATE bsm_mapped 5\n");
	proctab[pid].vmemlist->mlen = hsize*NBPG;
	vgetmem_first[pid]=1;

	//proctab[pid].vmemlist->mnext->mlen = hsize*NBPG;
/*
	struct mblock *temp =  proctab[pid].vmemlist->mnext;
		kprintf("VCREATE bsm_mapped 6 addr %x\n", (unsigned int)temp);
	temp->mlen = NBPG*hsize;
		kprintf("VCREATE bsm_mapped 7\n");
	temp->mnext = NULL;
		kprintf("VCREATE bsm_mapped 8\n");
	proctab[pid].vmemlist->mlen = -1;
	
*/
	/*
	proctab[pid].vmemnext =  proctab[pid].vmemlist->mnext;
	proctab[pid].vmemnext->mnext = 4096 * NBPG;
	//proctab[pid].vmemlist->mnext = proctab[pid].vmemnext;
		kprintf("VCREATE bsm_mapped 6 addr \n");
	proctab[pid].vmemnext->mlen = NBPG*hsize;
		kprintf("VCREATE bsm_mapped 7\n");
	proctab[pid].vmemnext->mnext = NULL;
		kprintf("VCREATE bsm_mapped 8\n");
	proctab[pid].vmemlist->mlen = -1;
*/

//	kprintf("PROC %d VMEMLIST starts at %x\n",pid, proctab[pid].vmemlist->mnext);
	
	restore(ps);
	return pid;

}


/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
