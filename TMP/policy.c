/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
int debug_on;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */

 // kprintf("=================SRPOLICYTo be implemented! %d \n",policy);
	STATWORD ps;
	disable(ps);
	if(policy != SC && policy != AGING){
		restore(ps);
		return SYSERR;
	}
	page_replace_policy = policy;
//	kprintf("RePLACEMENT POLICY CALLED inout %d  output%d\n",policy, page_replace_policy); 
	debug_on=1;
	restore(ps);
	 return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{

	//return AGING;
 	return page_replace_policy;
}
