/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


//typedef struct{
 // int bs_status;                        /* MAPPED or UNMAPPED           */
//  int bs_pid;                           /* process id using this slot   */
//  int bs_vpno;                          /* starting virtual page number */
///  int bs_npages;                        /* number of pages in the store */
//  int bs_sem;                           /* semaphore mechanism ?        */
//  int is_private;
//  int is_first;
//  int is_last;
//  bs_map_t *next_map;
//} bs_map_t;

bs_map_t bsm_tab[16];
/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-/------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	
	STATWORD ps;
	disable(ps);
	bsd_t i=0;
	for(i=0; i< 16; i++){
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = BADPID;
		bsm_tab[i].bs_vpno = 0;
		bsm_tab[i].bs_npages = -1;
		bsm_tab[i].bs_sem=0;	
		bsm_tab[i].is_private = 0;
		bsm_tab[i].is_last = 1;
		bsm_tab[i].is_first = 1;
		bsm_tab[i].next_map = NULL;
		bsm_tab[i].following_count=-1;
	}	
	restore(ps);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;

	disable(ps);
	int i=0;
	for(i=0; i<16; i++){
		if(bsm_tab[i].bs_status==BSM_UNMAPPED){
			bsm_tab[i].bs_status = BSM_BOOKED;
			*avail = i;
			restore(ps);
			return OK;
		}
	}
	
	restore(ps);
	return SYSERR;

}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	

	int base, range, mypid, iter;	
	STATWORD ps;
	disable(ps);

		base = bsm_tab[i].bs_vpno;
		range = bsm_tab[i].bs_npages;
		mypid = bsm_tab[i].bs_pid;	

		for(iter=0; iter<NFRAMES; iter++){

			if(frm_tab[iter].fr_pid==mypid && frm_tab[iter].fr_vpno>=base && frm_tab[iter].fr_vpno < (base+range)){
				free_frm(iter);
				

			}			

		}


		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = BADPID;
		bsm_tab[i].bs_vpno = 0;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem=0;	
		bsm_tab[i].is_private = 0;
		bsm_tab[i].is_last = 1;
		bsm_tab[i].is_first = 1;
		bsm_tab[i].next_map = NULL;
		bsm_tab[i].following_count=0;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	
	STATWORD ps;
	disable(ps);
	//kprintf("lookup input %d %d\n",pid, vaddr );	
	bsd_t i = 0;
	for(i=0; i<16; i++){
	//	kprintf("bsm lookup loop %d\n",i);		
		if(bsm_tab[i].bs_status != BSM_UNMAPPED){

			bs_map_t *temp = &bsm_tab[i];
//			kprintf("bsm %d status %d pid %d\n", i, temp->bs_status, temp->bs_pid);
			while(temp != NULL){
//				kprintf("BSM Lookup while loop pid %d  and bs pid %d\n", pid, temp->bs_pid); 
				if(temp->bs_pid == pid){
//					kprintf("bs lookup pid check of bs %d curr pid %d next pid %d \n", i,temp->bs_pid, temp->next_map->bs_pid);	
					if(((vaddr/NBPG) - temp->bs_vpno) < temp->bs_npages && ((vaddr/NBPG) - temp->bs_vpno) >= 0){
//						kprintf("BSM LOOKUP final if\n");
						*store = i;
						*pageth = (vaddr/NBPG) - temp->bs_vpno;
						
						restore(ps);
						return OK; 

					}

				}	
				temp = temp->next_map;
			}
		}
	}

	restore(ps);
	return SYSERR;


}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)  //basic validity checks left
{

	bs_map_t *self;
	bs_map_t *temp;

	STATWORD ps;
	disable(ps);
	
	if(source < 0 || source > 15 || vpno < 4096 || npages>128 ){
		restore(ps);
		return SYSERR;
	}	

	int ll=0;
//	kprintf("bsm map %d\n", source);
	temp = &bsm_tab[source];
	int l_bs;
	if((l_bs=bsm_local_lookup(source, pid, vpno))!=-1){
//		kprintf("L_BS %d source %d\n",l_bs,source);
		for(ll=0; ll<l_bs; ll++){
			temp = temp->next_map;
		}
		self = temp;
		//self = bsm_local_pointer(temp, l_bs);	
		//kprintf("self address %x\n",(unsigned int)self);
		//if(self->bs_status==BSM_MAPPED){
		//	restore(ps);
		//	return SYSERR;
		//}	

	}

	else if(temp->following_count>=0 && temp->bs_status != BSM_UNMAPPED){
//		kprintf("bsm map here pidi %d temp pid %d\n",pid, temp->bs_pid);
		while(temp->following_count > 0){
		//while(temp!=NULL){
			temp->following_count++;
			temp = temp->next_map;
//			kprintf("inside loop pid %d\n", temp->bs_pid);
		}
//		kprintf("temp pid %d\n", temp->bs_pid);
		temp->following_count++;	
		temp->is_last = 0;
		//temp->next_map = (bs_map_t *) getmem(sizeof(bs_map_t));
		self = (bs_map_t *) getmem(sizeof(bs_map_t));
		temp->next_map = self; 
		//self = temp->next_map;
	}
	else{
	
		self = temp;

	}
	
	//printf("MAPPING self %d prev %d\n",self->bs_pid, temp->pid);
	
	self->bs_status = BSM_MAPPED;                        /* MAPPED or UNMAPPED           */
	self->bs_pid = pid;                           /* process id using this slot   */
	self->bs_vpno = vpno;                          /* starting virtual page number */
	self->bs_npages = npages;                        /* number of pages in the store */
	self->is_last=1;
	self->following_count=0;
	self->is_first=0;                           /* semaphore mechanism ?        */
	self->next_map = NULL;
//	kprintf("temp next pid now %d\n", temp->next_map->bs_pid);
	proctab[pid].bs[source]=1;	
	//kprintf("MAPPING self %d prev %d\n",self->bs_pid, temp->bs_pid);
	
	bs_map_t *temp1 = &bsm_tab[source];
	
//	bs_map_t *temp1 = self;
	//
	//bs_map_t *temp1 = &bsm_tab[source];
	//bs_map_t *temp1 = &bsm_tab[source];
/*	kprintf("bsm address %x self addr %x\n", &bsm_tab[source], self);
	while(temp1 !=NULL){
		
		kprintf("bsm_tab[i].bs_status = %d\n ", temp1->bs_status);
		kprintf("bsm_tab[i].bs_pid  = %d\n ", temp1->bs_pid);
		kprintf("bsm_tab[i].bs_vpno  = %d\n ", temp1->bs_vpno);
		kprintf("bsm_tab[i].bs_npages = %d\n ", temp1->bs_npages);
		kprintf("bsm_tab[i].bs_sem =%d\n ", temp1->bs_sem);
		kprintf("bsm_tab[i].bs_private =%d\n ", temp1->is_private);
		kprintf("bsm_tab[i].bs_last =%d\n ", temp1->is_last);
		kprintf("bsm_tab[i].bs_first =%d\n ", temp1->is_first);
		//kprintf("bsm_tab[i].bs_statu =%d\n ", temp->next_map);
		temp1 = temp1->next_map;	
	}*/
	restore(ps);
	return OK;

}



int bsm_local_lookup(int store, int pid, int vpno){


	
	//bs_map_t *temp = bsm_tab[store];
	int counter = 0;

	STATWORD ps;
	disable(ps);


	if(store>15 || store < 0){
		restore(ps);
		return -1;				
	}
	bs_map_t *temp = &bsm_tab[store];
	
	while(temp != NULL){

		if(temp->bs_pid == pid){
				
		//	if((vaddr/NBPG) - temp->bs_vpno < temp->npages && (vaddr/NBPG) - temp->bs_vpno >= 0){
			//kprintf("iBSM LOOKUP pid %d\n", temp->bs_pid);
				//*store = c;
				//*pageth = (vaddr/NBPG) - temp->bs_vpno;
				
				restore(ps);
				return counter; 

		//	}

		}	
		counter++;
		temp = temp->next_map;
	}

	restore(ps);
	return -1;

}


bs_map_t * bsm_local_pointer(bsd_t store, int offset){
	STATWORD ps;

	disable(ps);
	bs_map_t *temp = &bsm_tab[store];
	int i=0;
	
	for(i=0; i<offset; i++){
	//	//kprintf("local ppointer\n");
		temp = temp->next_map;
	
	}
	//kprintf("temp address %x bsm address %x \n",(unsigned int)temp,(unsigned int)&bsm_tab[store]);
	return temp;
	

}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{

	int store=-1;
	int pageth=-1;
	
	STATWORD ps;
	disable(ps);

	//kprintf("BSM UNMAP enter %d\n",pid);
	
	if(bsm_lookup(pid, vpno*NBPG, &store, &pageth)!=OK){
	
		restore(ps);
		kill(pid);
		return SYSERR;

	}	



	int local_position = bsm_local_lookup(store, pid, vpno);
	//kprintf("UNMAP local position %d pid  %d\n",local_position, pid);
	bs_map_t *root_bs = &bsm_tab[store];	
 	
	int total_following_submap = root_bs->following_count;
//	kprintf("UNMAP following %d pid %d\n", total_following_submap, root_bs->bs_pid);
	
	bs_map_t *temp1 = &bsm_tab[store];
	
/*	while(temp1 !=NULL){
		
		kprintf("bsm_tab[i].bs_status = %d\n ", temp1->bs_status);
		kprintf("bsm_tab[i].bs_pid  = %d\n ", temp1->bs_pid);
		kprintf("bsm_tab[i].bs_vpno  = %d\n ", temp1->bs_vpno);
		kprintf("bsm_tab[i].bs_npages = %d\n ", temp1->bs_npages);
		kprintf("bsm_tab[i].bs_sem =%d\n ", temp1->bs_sem);
		kprintf("bsm_tab[i].bs_private =%d\n ", temp1->is_private);
		kprintf("bsm_tab[i].bs_last =%d\n ", temp1->is_last);
		kprintf("bsm_tab[i].bs_first =%d\n ", temp1->is_first);
		//kprintf("bsm_tab[i].bs_statu =%d\n ", temp->next_map);
		temp1 = temp1->next_map;	
	}
*/

//	kprintf("==============================\n");
	
	if(local_position == 0){
		if(total_following_submap <= 0){
	//		kprintf("bsm unmap came here");
			root_bs->bs_status = BSM_UNMAPPED;
			root_bs->is_private = 0;
			root_bs->bs_vpno = 0;
			root_bs->bs_pid = BADPID;
			root_bs->bs_npages = 0;
			root_bs->following_count = -1;
		}

		else{
//			kprintf("UNMAPPED second condition pid before %d pid of next %d\n", root_bs->bs_pid, root_bs->next_map->bs_pid );	
	//		kprintf("bsm_unmap came in second condition");
			bs_map_t * tempbs = root_bs->next_map;
			freemem((void *)root_bs, sizeof(bs_map_t));
			root_bs = tempbs;
			bsm_tab[store] = *root_bs;
			bsm_tab[store].is_first=1;
//			kprintf("UNMAPPED pid now %d in table %d \n", root_bs->bs_pid, bsm_tab[store].bs_pid);
			
			//root_bs->is_first=1;
		}
	}

	else if(local_position == total_following_submap){
	//	kprintf("%d unmapped here as last\n", pid);
		int i=0;
		bs_map_t *temp = root_bs;
		for(i=1; i<local_position; i++){
			temp->following_count--;
			temp = temp->next_map;
			
		}
		bs_map_t *target = temp->next_map;
		temp->next_map = NULL;
		temp->is_last=1;
		temp->following_count--;
		freemem((void *) target, sizeof(bs_map_t));



	}
	else{
		int i=0;
                bs_map_t *temp = root_bs;
                for(i=1; i<local_position; i++){
                        temp->following_count--;
                        temp = temp->next_map;

                }
		
		bs_map_t *target = temp->next_map;
		
		temp->next_map = target->next_map;		
		freemem((void *) target, sizeof(bs_map_t));
	
	}

	/*int i=0;
	bs_map_t *temp = &bsm_tab[store];
	kprintf("==============after unmap===================\n");
	while(temp !=NULL){
		
		kprintf("bsm_tab[i].bs_status = %d\n ", temp->bs_status);
		kprintf("bsm_tab[i].bs_pid  = %d\n ", temp->bs_pid);
		kprintf("bsm_tab[i].bs_vpno  = %d\n ", temp->bs_vpno);
		kprintf("bsm_tab[i].bs_npages = %d\n ", temp->bs_npages);
		kprintf("bsm_tab[i].bs_sem =%d\n ", temp->bs_sem);
		kprintf("bsm_tab[i].bs_private =%d\n ", temp->is_private);
		kprintf("bsm_tab[i].bs_last =%d\n ", temp->is_last);
		kprintf("bsm_tab[i].bs_first =%d\n ", temp->is_first);
		//kprintf("bsm_tab[i].bs_statu =%d\n ", temp->next_map);
		temp = temp->next_map;	
	}*/
//	kprintf("UNMAPPING DONE\n");
	restore(ps);
	return OK;
	

}


SYSCALL bsm_kill_unmap(int bs_num, int pid)
{
 // kprintf("To be implemented!");
  //return SYSERR;
  	int store, pageth, localstore;
	long base, range, iter;
	bs_map_t *my_bs;
	STATWORD ps;
	disable(ps);
	

	localstore = bsm_local_lookup(bs_num, pid,0);
	if(localstore==-1){
		restore(ps);
		return SYSERR;
	}
	
	my_bs = bsm_local_pointer(bs_num, localstore);

	
	base = my_bs->bs_vpno;
	range = my_bs->bs_npages;
	
	for(iter=0; iter<NFRAMES; iter++){

		 if(frm_tab[iter].fr_pid==pid && frm_tab[iter].fr_vpno>=base && frm_tab[iter].fr_vpno < (base+range)){
                                free_frm(iter);

		}	
	}


	if(bsm_unmap(pid, base, 1)!=OK){
		restore(ps);
		return SYSERR;
	}

	proctab[pid].bs[store]=0;


	//kprintf("BSM KILL DONE\n");
	//enable_paging();
	//
	//
	restore(ps);
	return OK;
}
