/* Policy Manager exposes internal interfaces to the allocator */

#ifndef POLICY_MGR_H_
#define POLICY_MGR_H_

#include <stdbool.h>
#include <stdlib.h>

typedef enum policyMgrPool
{
    POLICY_MGR_EXEC_POOL = 0,
    POLICY_MGR_STORAGE_POOL
} policyMgrPool;

/* This routine is called by the memory allocator to enforce memory 
* policies. The policy manager checks and update its statistics to make
* a decision.
* If the allocation can be made, return TRUE. 
* Otherwise, FALSE is returned.
*/
bool allocRequest(policyMgrPool pool, size_t num_bytes);

/* This routine is called by the memory allocator at memory free time 
* to update memory statistics of the policy manager.
*/
void freeRequest(policyMgrPool pool, size_t num_bytes);


#endif /* POLICY_MGR_H_ */