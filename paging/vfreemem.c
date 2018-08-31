/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{

//	kprintf("To be implemented!\n");
//	return(OK);
//

	STATWORD ps;
	disable(ps);

	if(size <= 0 || size>(proctab[currpid].vhpnpages*NBPG) || (int)block < 4096 * NBPG){

		restore(ps);
		return SYSERR;
	
	}

	struct mblock *prev, *curr, *temp;

	prev = proctab[currpid].vmemlist;
	curr = prev->mnext;
	//kprintf("VFREEMEM curr position %x and length %d\n",(unsigned int)curr, curr->mlen);
	while(curr && (unsigned int)curr < (unsigned int) block){

		prev = curr;
		curr = curr->mnext;

	}

	//kprintf("block position %x, size %d, next point %x\n", (unsigned int)block, size, (unsigned int)block + size);	

	if((unsigned int)block + size == (unsigned int) curr){
	//	kprintf("VFREEMEM right condition\n");
		block->mlen = size + curr->mlen;
		block->mnext = curr->mnext;
		prev->mnext = block;
	//	kprintf("VFREEMEM block address %x, length %d, check from vmemlist %x \n", (unsigned int)block, block->mlen, (unsigned int)proctab[currpid].vmemlist->mnext);
	}

	else if((int)prev + size == (int)block){

		prev->mlen = size + prev->mlen;
	}

	else{

		block->mlen = size;
		block->mnext = curr;
		prev->mnext = block;
	}

	
	
	return OK;

}
