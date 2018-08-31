/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	//kprintf("To be implemented!\n");
	//return( SYSERR );
	//
	 
	STATWORD ps;
	disable(ps);

	//kprintf("vgetmem called\n");
	struct mblock *prev, *curr, *temp;

	prev = proctab[currpid].vmemlist;
	curr = prev->mnext;
	if(vgetmem_first[currpid]==1){
		curr->mlen = prev->mlen;
		vgetmem_first[currpid]=0;
	}
	//kprintf("vgetmem prev %x nbytes %d curr address %x and current length%d\n",(unsigned int)prev,nbytes, (unsigned int)curr, curr->mlen);
	if(nbytes<=0 || nbytes > (proctab[currpid].vhpnpages*NBPG) || curr == NULL){
		restore(ps);
		return SYSERR;
	}	

	while(curr!=NULL){


		if(curr->mlen == nbytes){

			prev->mnext = curr->mnext;

			return(WORD *)curr;
		}

		else if(curr->mlen > nbytes){
//		kprintf("VGETMEM right if condition\n");

			temp = (struct mblock *)((unsigned int)curr + nbytes);
			temp->mlen = curr->mlen - nbytes;
			temp->mnext = curr->mnext;
			prev->mnext = temp;
	//		kprintf("VGETMEM nbytes %d, curr position %x, curr plus nbytes %x, temp address %x\n",nbytes, (unsigned int)curr, (unsigned int)curr + nbytes, (unsigned int)temp);
			restore(ps);
			return (WORD *)curr;
		}

		prev = curr;
		curr = curr->mnext;

	}

	restore(ps);
	return (WORD *)SYSERR;

	
	

}


