/* Policy Manager exposes internal interfaces to the allocator */

#ifndef POLICY_MGR_H_
#define POLICY_MGR_H_

#include <stdbool.h>
#include <stdlib.h>

typedef enum policyMgrPool
{
    POLICY_MGR_EXEC_POOL = 0,
    POLICY_MGR_STORAGE_POOL,
    POLICY_MGR_POOL_MAX
} policyMgrPool;

#define _1_Ki (1024)
#define _1_Mi (1024 * _1_Ki)
#define _1_Gi (1024 * _1_Mi)

#define POLICY_MGR_EXEC_POOL_QUOTA    50

#define PM_EXEC_POOL_MIN_REQ_SIZE     1             // 1 byte
#define PM_EXEC_POOL_MAX_REQ_SIZE     (8 * _1_Mi)   // 8 MB

#define PM_STORAGE_POOL_MIN_REQ_SIZE  (4 * _1_Ki)   // 4 KB
#define PM_STORAGE_POOL_MAX_REQ_SIZE  (64 * _1_Mi)  // 64 MB

typedef bool (*pool_alloc_cbk)(size_t);
typedef size_t (*pool_free_cbk)(size_t);

/* All the memory stats are in bytes */
typedef struct policyMgrPoolCtx
{
  size_t          used_memory_policyMgrPoolCtx;
  size_t          min_req_size_policyMgrPoolCtx;
  size_t          max_req_size_policyMgrPoolCtx;
  size_t          total_pool_mem_policyMgrPoolCtx;
  pool_alloc_cbk  allocCbk_policyMgrPoolCtx;
  pool_free_cbk   freeCbk_policyMgrPoolCtx;
} policyMgrPoolCtx;

typedef struct policyMgrStats
{
  int                 total_memory_policyMgrStats;
  policyMgrPoolCtx    poolStats_policyMgrStats[POLICY_MGR_POOL_MAX];
} policyMgrStats;

policyMgrStats pm_stats;

bool initPolicyMgr(size_t total_memory_bytes);

bool policyMgrExecPoolAlloc(size_t num_bytes);
bool policyMgrStoragePoolAlloc(size_t num_bytes);

size_t policyMgrExecPoolFree(size_t num_bytes_freed);
size_t policyMgrStoragePoolFree(size_t num_bytes_freed);

/* This routine is called by the memory allocator to enforce memory 
 * policies. The policy manager checks and update its statistics to make
 * a decision.
 * If the allocation can be made, return TRUE. 
 * Otherwise, FALSE is returned.
 */
bool policyMgrAllocRequest(policyMgrPool pool, size_t num_bytes);

/* This routine is called by the memory allocator at memory free time 
 * to update memory statistics of the policy manager.
 */
void policyMgrFreeRequest(policyMgrPool pool, size_t num_bytes_freed);

#endif /* POLICY_MGR_H_ */