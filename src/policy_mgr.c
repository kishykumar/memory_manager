#include <policy_mgr.h>
#include <stdlib.h>
#include <stdbool.h>

bool initPolicyMgr(size_t total_memory_bytes)
{
  policyMgrPoolCtx   *pool_ctx;
  size_t              exec_pool_total_mem;

  pm_stats.total_memory_policyMgrStats = total_memory_bytes;

  /* Initialize execution pool stats */
  pool_ctx = &pm_stats.poolStats_policyMgrStats[POLICY_MGR_EXEC_POOL];
  
  exec_pool_total_mem = 
    (POLICY_MGR_EXEC_POOL_QUOTA * total_memory_bytes) / 100;
  pool_ctx->total_pool_mem_policyMgrPoolCtx = exec_pool_total_mem;

  pool_ctx->min_req_size_policyMgrPoolCtx = PM_EXEC_POOL_MIN_REQ_SIZE;
  pool_ctx->max_req_size_policyMgrPoolCtx = PM_EXEC_POOL_MAX_REQ_SIZE;
  pool_ctx->used_memory_policyMgrPoolCtx = 0;
  pool_ctx->allocCbk_policyMgrPoolCtx = policyMgrExecPoolAlloc;

  /* Initialize storage pool stats */
  pool_ctx = &pm_stats.poolStats_policyMgrStats[POLICY_MGR_STORAGE_POOL];

  pool_ctx->total_pool_mem_policyMgrPoolCtx = total_memory_bytes - 
    exec_pool_total_mem;

  pool_ctx->min_req_size_policyMgrPoolCtx = PM_STORAGE_POOL_MIN_REQ_SIZE;
  pool_ctx->max_req_size_policyMgrPoolCtx = PM_STORAGE_POOL_MAX_REQ_SIZE;
  pool_ctx->used_memory_policyMgrPoolCtx = 0;
  pool_ctx->allocCbk_policyMgrPoolCtx = policyMgrStoragePoolAlloc;

  return true;
}

bool policyMgrExecPoolAlloc(size_t num_bytes)
{
  return true;
}

bool policyMgrStoragePoolAlloc(size_t num_bytes)
{
  return true;
}

size_t policyMgrExecPoolFree(size_t num_bytes_freed)
{
  return 0;
}

size_t policyMgrStoragePoolFree(size_t num_bytes_freed)
{
  return 0;
}

bool policyMgrAllocRequest(policyMgrPool pool, size_t num_bytes)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[pool];

  pool_ctx->allocCbk_policyMgrPoolCtx(num_bytes);
  return true;
}

void policyMgrFreeRequest(policyMgrPool pool, size_t num_bytes_freed)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[pool];

  pool_ctx->freeCbk_policyMgrPoolCtx(num_bytes_freed);
}