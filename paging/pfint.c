/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

	
	//kprintf("page fault %x\n",read_cr2());
	unsigned int cr2_var;
	unsigned int which_tab;
	unsigned int which_page;
	unsigned int which_byte;
	virt_addr_t *pgf_addr;
	STATWORD ps;
	disable(ps);

//	kprintf("page fault 2 %x\n",read_cr2());
	cr2_var = read_cr2();

//	kprintf("page fault 3  %x %x\n",read_cr2(), cr2_var);
	int bs_store, bs_pageth;
	
	if(bsm_lookup(currpid, cr2_var, &bs_store, &bs_pageth)!=OK){
	//	kprintf("pfint throwing error\n");
			
	//	while(1){
	//	}
		restore(ps);
		kill(currpid);
		return SYSERR;
	}

	//kprintf("after bsm lookup\n");	
	pgf_addr = (virt_addr_t *)(&cr2_var);

	which_tab = (unsigned int)pgf_addr->pd_offset;
	//kprintf("table in directory %d\n", which_tab);
	//int dir_frame_id = (proctab[currpid].pdbr / 4096) - 1024; 


	pd_t *this_dir = (pd_t*)proctab[currpid].pdbr;
	//kprintf("frame of dir %d\n",(int)this_dir/4096);
	//kprintf("TABLE 256 present or not%d\n",this_dir[which_tab].pd_pres);
	
	pt_t *this_table;
	 
	if(this_dir[which_tab].pd_pres == 0){

	//	kprintf("in the correct if\n");	
		this_table = create_table(currpid);
	//	kprintf("frame of table %d\n",(int)this_table/4096);
		this_dir[which_tab].pd_pres=1;
		this_dir[which_tab].pd_write=1;
		this_dir[which_tab].pd_base = (unsigned int)(this_table) / NBPG; 
		unsigned int frame_number = (unsigned int)(this_table) / NBPG - FRAME0; 
		frm_tab[frame_number].fr_pid = currpid;
		frm_tab[frame_number].fr_status= FRM_MAPPED;
		frm_tab[frame_number].fr_type= FR_TBL;
	}	

	else{

		//this_table =(pt_t*) (this_dir[which_tab].pd_base/NBPG);
		this_table =(pt_t*) (this_dir[which_tab].pd_base * NBPG);

	}


	which_page = (unsigned int)pgf_addr->pt_offset;
	int avail;
	
	fr_map_t *page_frame; 
	get_frm(&avail);

	if(debug_on==1 && replaced_yes==1){
		kprintf("FRAME REPLACED %d\n",avail);
	}	
			if(grpolicy()==SC){
				//kprintf("yes\n");		
	
				insert_scfrm(avail);
			}
	//kprintf("============================\n");
	//kprintf("NEW FRAME %d\n",avail);
	//kprintf("============================\n");

	//if(frm_tab[avail].fr_dirty==1){
			
	//}
//read bs
	read_bs((char *)((FRAME0 + avail)*NBPG), bs_store, bs_pageth);

	page_frame=&frm_tab[avail];
	frm_tab[avail].fr_status = FRM_MAPPED;
	frm_tab[avail].fr_pid = currpid;
	//frm_tab[avail].fr_vpno = which_page;
	frm_tab[avail].fr_vpno = bsm_tab[bs_store].bs_vpno + bs_pageth; 
	frm_tab[avail].fr_type= FR_PAGE;
	frm_tab[avail].fr_dirty= 0;
	frm_tab[avail].fr_refcnt++;
	
//update page table entry
//
	//frm_map_t *table_frame;
	//table_frame = (frm_map_t)(this_table);
	//table_frame->fr_refcnt++; 
	int table_frm_id = (unsigned int)(this_table)/NBPG - FRAME0;
	frm_tab[table_frm_id].fr_refcnt++;
	//kprintf("FRAME ID %d refcnt %d\n",table_frm_id, frm_tab[table_frm_id].fr_refcnt);
	this_table[which_page].pt_pres = 1;
	this_table[which_page].pt_write = 1;
	//this_table[which_tab].pt_base = (FRAME0 + avail)*4096;	
	this_table[which_page].pt_base = (FRAME0 + avail);	
	//enable_paging();	
	//unsigned long tempcr3 = read_cr3();
	//write_cr3(tempcr3);
	//which_byte = (unsigned int)cr2_var->pg_offset;
	restore(ps);
	return OK;	
  //kprintf("To be implemented!\n");
  //return OK;
}


