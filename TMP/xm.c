/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  //kprintf("xmmap - to be implemented!\n");
  //return SYSERR;
  //
//	kprintf("xmmap vpage %d\n", virtpage);
	struct pentry * curr_proc;

	STATWORD ps;
	disable(ps);

	/*int ll=0;
	for(ll=0; ll<16;ll++){
		kprintf("XMMAP bs %d status %d bs pid %d pid %d\n", ll, bsm_tab[ll].bs_status,bsm_tab[ll].bs_pid, currpid);
	}*/

/*
	bs_map_t *tt = bsm_tab[1].next_map;
	int ll=0;
	for(ll=1; ll<2;ll++){
		kprintf("XMMAP bs %d status %d bs_pid %d pid %d followinf %d next pid %d\n", ll, bsm_tab[ll].bs_status,bsm_tab[ll].bs_pid, currpid, bsm_tab[ll].following_count, tt->bs_pid);
	*/


	if(source <16 && source>=0 && virtpage >=4096){
//		kprintf("xmmap enterd if\n");
		if(bsm_tab[source].bs_status != BSM_UNMAPPED && bsm_tab[source].is_private==0){
//			kprintf("xmmap enterd unmapped if npages %d bs pages %d\n", npages, bsm_tab[source].bs_npages);
			
			if(npages <= bsm_tab[source].bs_npages || (bsm_tab[source].bs_npages==-1 && npages<=128)){
				
				//kprintf("========xmmap enterd npages if %d local id %d\n",currpid,local_lookup);
				int local_lookup =  bsm_local_lookup(source, currpid, virtpage);
	//			kprintf("========xmmap enterd npages if %d local id %d\n",currpid,local_lookup);
			
				bs_map_t * this_map;
	
				if(local_lookup != -1){
					this_map = bsm_local_pointer(source, local_lookup);
					if(this_map->bs_status == BSM_MAPPED){
						restore(ps);
						return SYSERR;
					}
				}

				if(bsm_map(currpid, virtpage,source, npages)!=OK){
				
					restore(ps);
					return SYSERR;

				}
				
				curr_proc = &proctab[currpid];
				curr_proc->bs[source] = 1;
				
				restore(ps);
				return OK;
			
			}
			
		
		}

	}

	restore(ps);
	return SYSERR;
		//if(npages < bsm_tab[source].bs_npages && bsm_tab[source].is_private == 0 && bsm_tab[source].bs_status != BSM_UNMAPPED ){

			

		
		//}	


		

}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
 // kprintf("To be implemented!");
  //return SYSERR;
  	int store, pageth, localstore;
	long base, range, iter;
	bs_map_t *my_bs;
	STATWORD ps;
	disable(ps);
	//kprintf("XMUNMAP called by %d\n",currpid);	
	if(bsm_lookup(currpid, virtpage*NBPG,&store, &pageth )!=OK){
		
		restore(ps);
		kill(currpid);
		return SYSERR;
	}

	localstore = bsm_local_lookup(store, currpid,virtpage);
	if(localstore==-1){
		restore(ps);
		return SYSERR;
	}
	
	my_bs = bsm_local_pointer(store, localstore);

	if(my_bs->is_private){
		restore(ps);
		return SYSERR;
	}
	
	base = my_bs->bs_vpno;
	range = my_bs->bs_npages;
	
	for(iter=0; iter<NFRAMES; iter++){

		 if(frm_tab[iter].fr_pid==currpid && frm_tab[iter].fr_vpno>=base && frm_tab[iter].fr_vpno < (base+range)){
                                free_frm(iter);

		}	
	}


	if(bsm_unmap(currpid, virtpage, 1)!=OK){
		restore(ps);
		return SYSERR;
	}

	proctab[currpid].bs[store]=0;


//	kprintf("XMUNMAP DONE\n");
	//enable_paging();
/*
	int ll=0;
	for(ll=1; ll<2;ll++){
		kprintf("XMUNMAP bs %d status %d bs_pid %d pid %d followinf %d next pid %d\n", ll, bsm_tab[ll].bs_status,bsm_tab[ll].bs_pid, currpid, bsm_tab[ll].following_count, bsm_tab[ll].next_map->bs_pid);
	}*/
	restore(ps);
	return OK;	
}
