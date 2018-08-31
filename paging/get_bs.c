#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

//typedef struct{
////  int bs_status;                        /* MAPPED or UNMAPPED           */
////  int bs_pid;                           /* process id using this slot   */
////  int bs_vpno;                          /* starting virtual page number */
////  int bs_npages;                        /* number of pages in the store */
////  int bs_sem;                           /* semaphore mechanism ?        */
////} bs_map_t;
//


int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    //kprintf("To be implemented!\n");
    //

    struct pentry *  curr_proc;

    STATWORD ps;
    disable(ps);
    curr_proc = &proctab[currpid]; 
    if(npages <= 128 && npages>0 && bs_id<16 && bs_id>=0){
	if(bsm_tab[bs_id].bs_status==BSM_UNMAPPED){
		bsm_tab[bs_id].bs_status = BSM_BOOKED;
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_vpno = 0;
		bsm_tab[bs_id].bs_sem = 1;
		bsm_tab[bs_id].is_private = 0;
		bsm_tab[bs_id].bs_npages = npages;
		curr_proc->bs[bs_id] = 2;
		//kprintf("%d first one to get bs %d\n", currpid, bs_id);
		restore(ps);
		return npages;		
		
	}
	//return npages;
	else if(bsm_tab[bs_id].is_private==0){
		curr_proc->bs[bs_id] = 2;
		
		restore(ps);
		return bsm_tab[bs_id].bs_npages;
	}
	
	else{
		restore(ps);
		return SYSERR;
	}

   }

   restore(ps);
   return SYSERR;

	


	

}


