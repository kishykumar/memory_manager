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
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[POLICY_MGR_EXEC_POOL];
  size_t              pool_total = pool_ctx->total_pool_mem_policyMgrPoolCtx;
  size_t              pool_used = pool_ctx->used_memory_policyMgrPoolCtx;
  size_t              total_system_memory = pm_stats.total_memory_policyMgrStats;

  /* Sanity checks:
   * Return can't allocate if:
   * 1. Allocation bytes is greater than max. pool allocation size, or
   * 2. Allocation bytes is greater than the system memory
   */
  if (num_bytes > pool_ctx->max_req_size_policyMgrPoolCtx || 
      num_bytes > total_system_memory)
  {
    return false;
  }
  
  /* Pool full: execution pool can go over pool_total, but not
   * total_system_memory.
   */
  if ((total_system_memory - num_bytes) < pool_used)
    return false;

  /* Update pool used bytes */
  pool_ctx->used_memory_policyMgrPoolCtx = pool_used + num_bytes;
  return true;
}

bool policyMgrStoragePoolAlloc(size_t num_bytes)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[POLICY_MGR_STORAGE_POOL];
  size_t              pool_total = pool_ctx->total_pool_mem_policyMgrPoolCtx;
  size_t              pool_used = pool_ctx->used_memory_policyMgrPoolCtx;

  /* sanity checks */
  if (num_bytes > pool_ctx->max_req_size_policyMgrPoolCtx || 
      num_bytes > pool_total)
  {
    return false;
  }
  
  /* Pool full */
  if ((pool_total - num_bytes) < pool_used)
    return false;
  
  /* Update pool used bytes */
  pool_ctx->used_memory_policyMgrPoolCtx = pool_used + num_bytes;
  return true;
}

bool policyMgrAlloc(policyMgrPool pool, size_t num_bytes)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[pool];

  return pool_ctx->allocCbk_policyMgrPoolCtx(num_bytes);
}

void policyMgrAdjustAlloc(policyMgrPool pool, signed long bytes_diff)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[pool];

  /* Update pool used bytes */
  pool_ctx->used_memory_policyMgrPoolCtx = (size_t)
    ((signed long)(pool_ctx->used_memory_policyMgrPoolCtx) + bytes_diff);
}