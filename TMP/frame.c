/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


fr_map_t frm_tab[NFRAMES];
//int sc_frames[NFRAMES];
pt_t *global_pg_tab[4];
sc_frames *sc_head;
sc_frames sc_tail;
sc_frames *temp1;
sc_frames *temp2;
int sc_count = 0;
int sc_last=0;
int replaced_yes=0;
//int sc_head =0, sc_tail=0,sc_oldest=0 ;
//typedef struct{
  //int fr_status;			/* MAPPED or UNMAPPED		*/
  //int fr_pid;				/* process id using this frame  */
  //int fr_vpno;				/* corresponding virtual page no*/
  //int fr_refcnt;			/* reference count		*/
  //int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  //int fr_dirty;
//}fr_map_t;


/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  //kprintf("To be implemented!\n");
	int i=0;
	
	STATWORD ps;
	disable(ps);
	
	for(i=0; i<NFRAMES; i++){
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = BADPID;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_NONE;
		frm_tab[i].fr_dirty = 0; 
	
	}
	//kprintf("init address of head and tail %d\n",&sc_head1);


	sc_head->next = &sc_head;
	sc_head->prev = &sc_head;
	sc_head->fr_num = -1;
	//c_tail.next = &sc_head;


	restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{

	int i=0;
	
	  void* inv;
	STATWORD ps;
	disable(ps);
	
	for(i=0; i<NFRAMES; i++){
		
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
			*avail = i;
			//sc_frames[tail]=i;
			//tail = (tail+1)%NFRAMES;
			//if(grpolicy()==SC){
			//	insert_scfrm(i);
			//}i
			replaced_yes=0;
			restore(ps);
			return OK;
		}

		

	}
	
	*avail = evict_frm();
	int temp = *avail;
	replaced_yes=1;
	//kprintf("GET FRM TMP %d\n",temp);
	if(temp == -1){
		restore(ps);
		return SYSERR;
	}
	
//	kprintf("BEFORE RESet temp %d fr pid %d, fr vpno %d\n",temp, frm_tab[temp].fr_pid, frm_tab[temp].fr_vpno);
	
	reset_ptpres(temp);
	pt_t *mytabentry;
	mytabentry = get_pttentry(temp);
	//if(frm_tab[*avail].fr_dirty==1){
	if(mytabentry->pt_dirty==1){

	//	kprintf("FRAME DIRTYYYYY\n");
		int fr_bs, fr_pg;
		if(bsm_lookup(frm_tab[*avail].fr_pid, frm_tab[*avail].fr_vpno*NBPG, &fr_bs, &fr_pg)==OK){
//			kprintf("BSM LOOKUP WORKS FOR DIRTT\n");
			write_bs((char *)((FRAME0 + temp)*NBPG), fr_bs, fr_pg);
		}
	}	
	
	//reset_ptpres(temp);

   	 inv = frm_tab[temp].fr_vpno*4096;
     	asm volatile ( "invlpg (%0)" : : "b"(inv) : "memory" );
/*	
		frm_tab[temp].fr_status = FRM_UNMAPPED;
		frm_tab[temp].fr_pid = BADPID;
		frm_tab[temp].fr_vpno = 0;
		//frm_tab[temp].fr_refcnt = 0;
		frm_tab[temp].fr_type = FR_NONE;
		frm_tab[temp].fr_dirty = 0; 

	
*/

	free_frm(temp);	

	unsigned long tempcr3 = read_cr3();
	write_cr3(tempcr3);
	restore(ps);
	return OK;
  //kprintf("To be implemented!\n");
  //return OK;
}
/*
void insert_scfrm(int frid){
	sc_frames *temp1;
	sc_frames *temp2;
	kprintf("\n");
	if(sc_head->fr_num!=-1){
		kprintf("INSERT if %d head %d\n",frid, sc_head->fr_num);	
		sc_frames newfrm;
		newfrm.fr_num = frid;
		kprintf("INSERT if %d head %d newframe %d tail %d\n",&frid, (unsigned int)sc_head, (unsigned int)newfrm, (unsigned int)sc_tail);	
		sc_tail.next = &newfrm;
		kprintf("INSERT if %d head %d newframe %d tail %d\n",frid, (unsigned int)sc_head, (unsigned int)newfrm, (unsigned int)sc_tail);	
		newfrm.next = &sc_head;
		kprintf("INSERT if %d head %d\n",frid, sc_head->fr_num);	
		&sc_tail = &sc_tail.next;
		kprintf("INSERT if %d head %d\n",frid, sc_head->fr_num);	

		sc_frames *tempfr = &sc_head;
		while(tempfr!=&sc_tail){
			kprintf("%d ",tempfr->fr_num);
			tempfr = tempfr->next;
		}
	}
	else{
		kprintf("INSERT else %d\n",frid);	
		sc_head.fr_num=frid;
	}

	kprintf("\n");
}


*/
/*
void insert_scfrm(int frid){
	sc_frames *temp1;
	sc_frames *temp2;
	temp1 = &sc_head;
	temp2= &sc_tail;
	kprintf("INSERT address of temp1 and temp2i %d %d\n", temp1, temp2);
	if(temp1->fr_num!=-1 && temp2->fr_num!=-1){
		kprintf("INSERT if %d head %d\n",frid,temp1->fr_num);	
		sc_frames newfrm;
		newfrm.fr_num = frid;
		kprintf("INSERT if %d head %d newframe %d tail %d\n",&frid, temp1, &newfrm, &sc_tail);	
		temp2->next = &newfrm;
		//kprintf("INSERT if %d head %d newframe %d tail %d\n",frid, (unsigned int)sc_head, (unsigned int)newfrm, (unsigned int)sc_tail);	
		newfrm.next = temp1;
		//kprintf("INSERT if %d head %d\n",frid, sc_head->fr_num);	
		temp2 = temp2->next;
		//kprintf("INSERT if %d head %d\n",frid, sc_head->fr_num);	

		sc_frames *tempfr = temp1;
	===had comment start	while(tempfr!=temp2){
			kprintf("%d ",tempfr->fr_num);
			tempfr = tempfr->next;
		}======had comment end
	}
	else if(temp1->fr_num==-1){
		kprintf("INSERT else %d\n",frid);	
		temp1->fr_num=frid;
	}
	
	else if(temp2->fr_num==-1){
		kprintf("INSERT else %d\n",frid);	
		temp2->fr_num=frid;
	}

	kprintf("\n");
}
*/

void insert_scfrm(int frid){

	if(sc_count == 0){
	
		sc_head->fr_num = frid;
	}
	else if(sc_count == 1){
		sc_frames *new_sc = (sc_frames *) getmem(sizeof(sc_frames));;
		
		new_sc->fr_num = frid;
		new_sc->next = sc_head;
		new_sc->prev = sc_head;
		sc_head->next = new_sc;
		sc_head->prev = new_sc;
	}
	else{
		sc_frames *new_sc = (sc_frames *) getmem(sizeof(sc_frames));;
		new_sc->fr_num = frid;
		new_sc->prev = sc_head->prev;
		new_sc->next = sc_head;
		sc_frames *temp = sc_head->prev;
		sc_head->prev->next = new_sc;
		sc_head->prev = new_sc;
	}
/*	int i=0;
	sc_frames *temp = sc_head;
	kprintf("INSSERT SC\n");
	for(i=0; i<sc_count+2; i++){
		kprintf(" %d ",temp->fr_num);
		temp = temp->next;
	}*/
	sc_count++;

}

int evict_frm(){
	
	STATWORD ps;
	disable(ps);

	if(grpolicy() == SC){

//		kprintf("right policy\n");
		return sc_policy();	
	}

	else{
//		kprintf("LFU policy\n");
		return LFU_policy();
	}

	restore(ps);
	return -1;
}


int LFU_policy(){
	//kprintf("LFU POLICY HAPPENING\n");
	//int min_ref = frm_tab[0].fr_refcnt;
	unsigned int min_ref = 2147483647;
	int min_pid = 0;
	int i=0;
	for(i=0; i<NFRAMES; i++){
		if(frm_tab[i].fr_type == FR_PAGE){
			if(frm_tab[i].fr_refcnt<min_ref){
				min_ref = frm_tab[i].fr_refcnt;
				min_pid = i;
			}
			else if(frm_tab[i].fr_refcnt==min_ref){
				if(frm_tab[i].fr_vpno > frm_tab[min_pid].fr_vpno){
					min_ref = frm_tab[i].fr_refcnt;
					min_pid = i;
				
				}
			}
		}

	}

	return min_pid;

}

/*

int sc_policy(){
	kprintf("SC POLICY\n");
	int answer;
	sc_frames *temp1;
	sc_frames *temp2;
	while(1==1){
*/
		/*if(get_ptacc(sc_frames[head])==1){
			set_ptacc(sc_frames[head]=0);
		}
		else{
			answer = sc_frames[head];
			head = (head+1)%NFRAMES;
			break;
		}
		if(head!=tail){
			head = (head+1)%NFRAMES;
		}
		else{
			head = oldest;
		}*/
/*		if(get_ptacc(sc_he.fr_num)==1){
			set_ptacc(sc_head.fr_num);
			&sc_head = &sc_head.next;
			&sc_tail = sc_tail.next;
		}
		else{
			answer = sc_head.fr_num;
			&sc_head =&sc_head.next;
			sc_tail.next = &sc_head;
			break;
		}
		
	}

	kprintf("SC frame %d\n",answer);
	return answer;	


}
*/

/*
int sc_policy(){
//	kprintf("SC POLICY\n");
	int answer;
	sc_frames *temp1 = &sc_head;
	sc_frames *temp2 = &sc_tail;
	while(1==1){
		o=====*if(get_ptacc(sc_frames[head])==1){
			set_ptacc(sc_frames[head]=0);
		}
		else{
			answer = sc_frames[head];
			head = (head+1)%NFRAMES;
			break;
		}
		if(head!=tail){
			head = (head+1)%NFRAMES;
		}
		else{
			head = oldest;
		}====================
		if(get_ptacc(temp1->fr_num)==1){
			set_ptacc(temp1->fr_num);
			temp1 = temp1->next;
			temp2 = temp2->next;
		}
		else{
			answer = temp1->fr_num;
			temp1 =temp1->next;
			temp2->next =temp1;
			break;
		}
		
	}

	//kprintf("SC frame %d\n",answer);
	return answer;	


}*/

int sc_policy(){
	int answer;
/*	sc_frames *temp = sc_head->prev;
	answer = temp->fr_num;
	sc_frames *new_tail = temp->prev;
	new_tail->next = sc_head;
	sc_head->prev = new_tail;
		freemem((void *) temp, sizeof(sc_frames));
*/
	while(1==1){
		if(get_ptacc(sc_head->fr_num)==1){
			set_ptacc(sc_head->fr_num);
			sc_head = sc_head->next;
		}
		else{
			answer = sc_head->fr_num;
			sc_frames *temp = sc_head;
			sc_frames *new_tail = sc_head->prev;
			sc_head = sc_head->next;
			sc_head->prev = new_tail;
			new_tail->next = sc_head;
		//	freemem((void *) temp, sizeof(sc_frames));
			break;
			
		}
	}


	sc_count--;

	
	/*int i=0;
	sc_frames *temp1 = sc_head;
	kprintf("REmove SC\n");
	for(i=0; i<sc_count+2; i++){
		kprintf(" %d ",temp1->fr_num);
		temp1 = temp1->next;
	}
	kprintf("\n");*/
	return answer;
}

int sc_policy_temp(){
//	kprintf("SC TEMP\n");
	int answer;
	while(1==1){
		if(frm_tab[sc_last].fr_type == FR_PAGE){
			if(get_ptacc(sc_last)==1){
				set_ptacc(sc_last);
				sc_last = (sc_last+1)%NFRAMES;
			}
			else{
				answer = sc_last;
				
				sc_last = (sc_last+1)%NFRAMES;
				break;
			}
			
		}
		else{
			
				sc_last = (sc_last+1)%NFRAMES;
		}
		

	}
	//kprintf("SC TEMP %d\n",answer);
	return answer;

}


int get_ptacc(int frm_id){
	//kprintf("get_ptacc for frm id %d\n", frm_id);
	//kprintf("GETPTACC frm vpno %d\n",frm_tab[frm_id].fr_vpno);	
	fr_map_t *frm = &frm_tab[frm_id];
	int f_pid = frm->fr_pid;
	int f_vp = frm->fr_vpno;
//	kprintf("getptacc pid %d vpage %d\n",f_pid, f_vp);	
	if(frm->fr_type!=FR_PAGE){
		kprintf("geptacc error shouln't be here\n");
		return -1;
	}

	unsigned int vaddr = f_vp * NBPG;
	virt_addr_t *vaddr_struct = (virt_addr_t *)(&vaddr);
	int tab_entry =(unsigned int) vaddr_struct->pd_offset;
	pd_t *dir = (pd_t *)proctab[f_pid].pdbr;
	
//	kprintf("getptacc dir frame %d\n",(unsigned int)dir/4096 - FRAME0);
	pt_t *tab = (pt_t *)(dir[tab_entry].pd_base * NBPG);
//	kprintf("getptacc table entry  %d\n",tab_entry);
//	kprintf("getptacc table frame %d\n",(unsigned int)tab/4096 - FRAME0);
	unsigned int tab1_entry = (unsigned int)vaddr_struct->pt_offset;

//	kprintf("getptacc page entry  %d\n",tab1_entry);
	//kprintf("getptacc table frame %d\n",(unsigned int)tab/4096 - FRAME0);
	if(tab[tab1_entry].pt_acc == 1){
		return 1;
	}
	else{
		return 0;
	}
	return -1;
}

void reset_ptpres(int frm_id){
//	kprintf("RESET GETS CALLED!!!!!!!!!11\n");	
//	kprintf("RESET frm vpno %d\n",frm_tab[frm_id].fr_vpno);	
	fr_map_t *frm = &frm_tab[frm_id];
	int f_pid = frm->fr_pid;
	int f_vp = frm->fr_vpno;
	
	if(frm->fr_type!=FR_PAGE){
		return -1;
	}

	unsigned int vaddr = f_vp * NBPG;
	virt_addr_t *vaddr_struct = (virt_addr_t *)(&vaddr);
	int tab_entry = (unsigned int)vaddr_struct->pd_offset;
	pd_t *dir = (pd_t *)proctab[f_pid].pdbr;
	pt_t *tab = (pt_t *)(dir[tab_entry].pd_base * NBPG);
	unsigned int tab1_entry = (unsigned int)vaddr_struct->pt_offset;

	
//	kprintf("RESET page entry  %d\n",tab1_entry);
	
	tab[tab1_entry].pt_pres = 0;

}



pt_t * get_pttentry(int frm_id){
	//kprintf("RESET GETS CALLED!!!!!!!!!11\n");	
	//kprintf("RESET frm vpno %d\n",frm_tab[frm_id].fr_vpno);	
	fr_map_t *frm = &frm_tab[frm_id];
	int f_pid = frm->fr_pid;
	int f_vp = frm->fr_vpno;
	
	if(frm->fr_type!=FR_PAGE){
		return -1;
	}

	unsigned int vaddr = f_vp * NBPG;
	virt_addr_t *vaddr_struct = (virt_addr_t *)(&vaddr);
	int tab_entry = (unsigned int)vaddr_struct->pd_offset;
	pd_t *dir = (pd_t *)proctab[f_pid].pdbr;
	pt_t *tab = (pt_t *)(dir[tab_entry].pd_base * NBPG);
	unsigned int tab1_entry = (unsigned int)vaddr_struct->pt_offset;

	
	//kprintf("RESET page entry  %d\n",tab1_entry);
	
	return &tab[tab1_entry];

}


void set_ptacc(int frm_id){

	fr_map_t *frm = &frm_tab[frm_id];
	int f_pid = frm->fr_pid;
	int f_vp = frm->fr_vpno;
	
	if(frm->fr_type!=FR_PAGE){
		return SYSERR;
	}

	unsigned int vaddr = f_vp * NBPG;
	virt_addr_t *vaddr_struct = (virt_addr_t *)(&vaddr);
	int tab_entry = (unsigned int)vaddr_struct->pd_offset;
	pd_t *dir = (pd_t *)proctab[f_pid].pdbr;
	pt_t *tab = (pt_t *)(dir[tab_entry].pd_base * NBPG);
	unsigned int tab1_entry = (unsigned int)vaddr_struct->pt_offset;

	tab[tab1_entry].pt_acc = 0;
	return OK;
}


SYSCALL get_td_frm(int* avail)
{

	int i=0;

	STATWORD ps;
	disable(ps);
	
	for(i=0; i<(NFRAMES/2); i++){
		
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
			*avail = i;

			restore(ps);
			return OK;
		}

	}


	restore(ps);
	return SYSERR;
  //kprintf("To be implemented!\n");
  //return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

 // kprintf("To be implemented!\n");
  //return OK;
  //
	int vpage;

	unsigned int vaddr;	
	unsigned int table_number, page_number, table_frm;
	pd_t *dir;
	pt_t *table;
	//unsigned int page;
	virt_addr_t *vaddr_struct;
	int pid;
	STATWORD ps;
	disable(ps);
	 if(frm_tab[i].fr_type == FR_PAGE){
		pid = frm_tab[i].fr_pid;
		vpage = frm_tab[i].fr_vpno;
		vaddr = vpage * NBPG;
		vaddr_struct = (virt_addr_t *)(&vaddr);

		table_number = (unsigned int)vaddr_struct->pd_offset;
		page_number = (unsigned int)vaddr_struct->pt_offset;
	
		dir = (pd_t *)proctab[pid].pdbr;
		table = (pt_t *)(dir[table_number].pd_base * NBPG);
		table[page_number].pt_pres = 0;
		//table[page_number].pt_pres = -1;
		table_frm = (unsigned int)table/NBPG - FRAME0;
		frm_tab[table_frm].fr_refcnt--;
//		kprintf("TABLE FRM %d REFCNT %d\n",table_frm, frm_tab[table_frm].fr_refcnt);
		if(frm_tab[table_frm].fr_refcnt==0){
			frm_tab[table_frm].fr_status = FRM_UNMAPPED; 
			dir[table_number].pd_pres = 0;	
		}
			
	}

	frm_tab[i].fr_status = FRM_UNMAPPED;
//	kprintf("FRAME FREED %d\n",i);
	restore(ps);
	return OK;
}



void create_global_pagetables()
{
	int i=0;
	
		int j=0;
	STATWORD ps;
	disable(ps);

	for(i=0; i<4; i++){
		frm_tab[i+1].fr_type = FR_TBL;		
		frm_tab[i+1].fr_status = FRM_MAPPED;
		//&global_pg_tab[i] = (pt_t *) ((FRAME0 + i + 1) * NBPG);
		global_pg_tab[i] = (FRAME0 + i + 1) * NBPG;
		pt_t *temp = global_pg_tab[i];
		//int j=0;

		for(j=0; j<1024; j++){
		
		

 			temp[j].pt_pres =  1;            /* page is present?             */
 			 temp[j].pt_write = 1;            /* page is writable?            */
  			  temp[j].pt_user  = 0;            /* is use level protection?     */
  			  temp[j].pt_pwt   = 0;            /* write through for this page? */
  			  temp[j].pt_pcd  = 0;            /* cache disable for this page? */
  			  temp[j].pt_acc = 0;            /* page was accessed?           */
  			  temp[j].pt_dirty = 0;            /* page was written?            */
  			  temp[j].pt_mbz = 0;            /* must be zero                 */
  			  temp[j].pt_global = 0;            /* should be zero in 586        */
  			  temp[j].pt_avail = 0;            /* for programmer's use         */
  			  temp[j].pt_base  = (i*1024)+j;           /* location of page?            */
 	
			//kprintf("global pagetable %d entru %d\n",i,temp[j].pt_base);
		}
//		kprintf("global page address and frame %d %ld %d %d %d\n",NBPG,temp[0].pt_base, i,(int)global_pg_tab[i]/NBPG,(FRAME0 + i + 1) * NBPG);	
	}

}


void create_pagedir(int procid){
	
	int frame_id;
	int j=0;
	STATWORD ps;
	disable(ps);


	//int frame_id;
	 //get_td_frm(&frame_id);
	 get_frm(&frame_id);
	
	if(procid==NULLPROC){
		frame_id = 0;
	}

	pd_t *temp_dir = (pd_t *)((frame_id+1024)*NBPG);
//	kprintf("size of dir entry %d\n",sizeof(pd_t));
	bzero(temp_dir,sizeof(pd_t)*1024);
	frm_tab[frame_id].fr_type = FR_DIR; 
	frm_tab[frame_id].fr_status = FRM_MAPPED;
	frm_tab[frame_id].fr_pid = procid;
	
	proctab[procid].pdbr = (frame_id+1024)*NBPG;

//	int j=0;
	for(j=0; j<4; j++){
		 temp_dir[j].pd_pres  = 1;            /* page table present?          */
		  temp_dir[j].pd_write = 1;            /* page is writable?            */
  		 temp_dir[j].pd_user = 0;            /* is use level protection?     */
 		 temp_dir[j].pd_pwt  = 0;            /* write through cachine for pt?*/
 		temp_dir[j].pd_pcd  = 0;            /* cache disable for this pt?   */
  		temp_dir[j].pd_acc  = 0;            /* page table was accessed?     */
 		temp_dir[j].pd_mbz  = 0;            /* must be zero                 */
		temp_dir[j].pd_fmb  = 0;            /* four MB pages?               */
  		temp_dir[j].pd_global = 0;            /* global (ignored)             */
 	temp_dir[j].pd_avail = 0;            /* for programmer's use         */
 		//temp_dir[j].pd_base  = (1024+j+1) * NBPG;           /* location of page table?      */
 		temp_dir[j].pd_base  = (1024+j+1);           /* location of page table?      */
	
	}

	
	for(j=4; j<1024; j++){
		 temp_dir[j].pd_pres  = 0;            /* page table present?          */
		  temp_dir[j].pd_write = 0;            /* page is writable?            */
  		 temp_dir[j].pd_user = 0;            /* is use level protection?     */
 		 temp_dir[j].pd_pwt  = 0;            /* write through cachine for pt?*/
 		temp_dir[j].pd_pcd  = 0;            /* cache disable for this pt?   */
  		temp_dir[j].pd_acc  = 0;            /* page table was accessed?     */
 		temp_dir[j].pd_mbz  = 0;            /* must be zero                 */
		temp_dir[j].pd_fmb  = 0;            /* four MB pages?               */
  		temp_dir[j].pd_global = 0;            /* global (ignored)             */
 	temp_dir[j].pd_avail = 0;            /* for programmer's use         */
 		//temp_dir[j].pd_base  = (1024+j+1) * NBPG;           /* location of page table?      */
 		temp_dir[j].pd_base  = 0;           /* location of page table?      */
	
	}

//	kprintf("CREATE PAGE DIR procid %d, frame %d\n first four table %d %d %d %d ",procid, frame_id, temp_dir[0].pd_base, temp_dir[1].pd_base, temp_dir[2].pd_base, temp_dir[3].pd_base);
	restore(ps);
	return OK;


}

pt_t * create_table(int procid){

	int frame_id;
		int j=0;
	STATWORD ps;
	disable(ps);
	
//	int frame_id;
	//get_td_frm(&frame_id);
	get_frm(&frame_id);
	
	pt_t *temp = (pt_t *)((frame_id+1024)*NBPG);
	
	
	frm_tab[frame_id].fr_type = FR_TBL; 
	frm_tab[frame_id].fr_status = FRM_MAPPED;
	frm_tab[frame_id].fr_pid = procid;


//		int j=0;
		for(j=0; j<1024; j++){
		
		

 			temp[j].pt_pres =  0;            /* page is present?             */
 			 temp[j].pt_write = 0;            /* page is writable?            */
  			  temp[j].pt_user  = 0;            /* is use level protection?     */
  			  temp[j].pt_pwt   = 0;            /* write through for this page? */
  			  temp[j].pt_pcd  = 0;            /* cache disable for this page? */
  			  temp[j].pt_acc = 0;            /* page was accessed?           */
  			  temp[j].pt_dirty = 0;            /* page was written?            */
  			  temp[j].pt_mbz = 0;            /* must be zero                 */
  			  temp[j].pt_global = 0;            /* should be zero in 586        */
  			  temp[j].pt_avail = 0;            /* for programmer's use         */
  			  temp[j].pt_base  = 0;           /* location of page?            */
 	

		}	

	restore(ps);
	return temp;
}
