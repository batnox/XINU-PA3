/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


pt_t global_pg_tab[4];


void create_global_pagetables()
{
	int i=0;
	
	STATWORD ps;
	disable(ps);

	for(i=0; i<4; i++){
		frm_tab[i+1].fr_type = FR_TBL;		
		frm_tab[i].fr_status = FR_MAPPED;
		global_pg_tab[i] = (pt_t *) ((FRAME0 + i) * NBPG);
	
		int j=0;
		for(j=0; j<1024; j++){
		
		

 			global_pg_tab[i].pt_pres =  1;            /* page is present?             */
 			 global_pg_tab[i].pt_write = 1;            /* page is writable?            */
  			  global_pg_tab[i].pt_user  = 0;            /* is use level protection?     */
  			  global_pg_tab[i].pt_pwt   = 0;            /* write through for this page? */
  			  global_pg_tab[i].pt_pcd  = 0;            /* cache disable for this page? */
  			  global_pg_tab[i].pt_acc = 0;            /* page was accessed?           */
  			  global_pg_tab[i].pt_dirty = 0;            /* page was written?            */
  			  global_pg_tab[i].pt_mbz = 0;            /* must be zero                 */
  			  global_pg_tab[i].pt_global = 0;            /* should be zero in 586        */
  			  global_pg_tab[i].pt_avail = 0;            /* for programmer's use         */
  			  global_pg_tab[i].pt_base  = (i*4096)+j;           /* location of page?            */
 	

		}	
	}

}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{

}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

}



