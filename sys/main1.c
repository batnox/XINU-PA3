/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	kprintf("CR3 main %d\n", read_cr3());
	pd_t *nullpd = (pd_t*)(proctab[currpid].pdbr);
	kprintf("MAIN pdbr %d first 4 entries %d %d %d %d\n", proctab[currpid].pdbr,nullpd[0].pd_base, nullpd[1].pd_base, nullpd[2].pd_base, nullpd[3].pd_base);
//	int a;
//	get_bsm(&a);
//	int page;
//	bsm_map(10, 5000, 2, 100);
////	bsm_map(20, 4900, 2, 100);
//	bsm_map(30, 4900, 2, 100);
//	bsm_unmap(20, 4900,0);
	//kprintf("AFTER UNMAPPING IN MAIN\n");
//	bsm_lookup(30, 4901*4096, &a, &page);
//	kprintf("MAIN bsm %d page %d\n",a, page);
	//bsm_map(20, 4900, 2, 100);
//	bsm_lookup(10, 5010*4096, &a, &page);
//	kprintf("MAIN bsm %d page %d\n",a, page);
        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call terminates the QEMU process.
         */
        shutdown();
}
