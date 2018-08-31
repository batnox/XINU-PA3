#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
    
	STATWORD ps;
	disable(ps);

	if(bs_id < 0  || bs_id>15){
		
		restore(ps);
		return SYSERR;
	} 
	
	if(bsm_tab[bs_id].following_count>0){

		restore(ps);
		return SYSERR;
	}

	if(bsm_tab[bs_id].bs_status == BSM_MAPPED || bsm_tab[bs_id].bs_status == BSM_BOOKED){
		//kprintf("RELEASE BS %d npages %d\n",bs_id, bsm_tab[bs_id].bs_npages);
		if(xmunmap(bsm_tab[bs_id].bs_vpno)!=OK){
			restore(ps);
			return SYSERR;
	
		}
	}

	
		//kprintf("RELEASE BS %d npages %d\n",bs_id, bsm_tab[bs_id].bs_npages);
	if(free_bsm(bs_id)!=OK){
		restore(ps);
		return SYSERR;
	}

	else{
		proctab[currpid].bs[bs_id]=0;
		restore(ps);
		return OK;
	}
	
	//kprintf("To be implemented!\n");
   	//return OK;

}

