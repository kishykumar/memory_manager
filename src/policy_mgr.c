#include <policy_mgr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

bool initPolicyMgr(size_t total_memory_bytes)
{
  policyMgrPoolCtx   *pool_ctx;
  size_t              exec_pool_total_mem;

  pm_stats.total_memory_policyMgrStats = total_memory_bytes;

  pthread_mutex_init(&pm_stats.mutex_policyMgrStats, NULL);

  /* Initialize execution pool stats */
  pool_ctx = &pm_stats.poolStats_policyMgrStats[POLICY_MGR_EXEC_POOL];
  
  exec_pool_total_mem = 
    (POLICY_MGR_EXEC_POOL_QUOTA * total_memory_bytes) / 100;
  pool_ctx->total_pool_mem_policyMgrPoolCtx = exec_pool_total_mem;

  pool_ctx->min_req_size_policyMgrPoolCtx = PM_EXEC_POOL_MIN_REQ_SIZE;
  pool_ctx->max_req_size_policyMgrPoolCtx = PM_EXEC_POOL_MAX_REQ_SIZE;
  pool_ctx->used_memory_policyMgrPoolCtx = 0;

  /* Initialize storage pool stats */
  pool_ctx = &pm_stats.poolStats_policyMgrStats[POLICY_MGR_STORAGE_POOL];

  pool_ctx->total_pool_mem_policyMgrPoolCtx = total_memory_bytes - 
    exec_pool_total_mem;

  pool_ctx->min_req_size_policyMgrPoolCtx = PM_STORAGE_POOL_MIN_REQ_SIZE;
  pool_ctx->max_req_size_policyMgrPoolCtx = PM_STORAGE_POOL_MAX_REQ_SIZE;
  pool_ctx->used_memory_policyMgrPoolCtx = 0;

  return true;
}

void destroyPolicyMgr()
{
  pthread_mutex_destroy(&pm_stats.mutex_policyMgrStats);
}

bool policyMgrAlloc(policyMgrPool alloc_pool_type, size_t num_bytes)
{
  bool                ret = false;
  policyMgrPoolCtx   *exec_pool_ctx = 
                  &pm_stats.poolStats_policyMgrStats[POLICY_MGR_EXEC_POOL];
  policyMgrPoolCtx   *storage_pool_ctx = 
                &pm_stats.poolStats_policyMgrStats[POLICY_MGR_STORAGE_POOL];

  pthread_mutex_lock(&pm_stats.mutex_policyMgrStats);

  size_t  exec_pool_used = exec_pool_ctx->used_memory_policyMgrPoolCtx;

  size_t  storage_pool_total = 
            storage_pool_ctx->total_pool_mem_policyMgrPoolCtx;

  size_t  storage_pool_used = storage_pool_ctx->used_memory_policyMgrPoolCtx;

  size_t  total_system_memory = pm_stats.total_memory_policyMgrStats;
  size_t  total_used = exec_pool_used + storage_pool_used;

  switch(alloc_pool_type)
  {
    case POLICY_MGR_EXEC_POOL:
    
     /* Sanity checks:
      * Return can't allocate if:
      * 1. Allocation bytes is greater than max. pool allocation size, or
      * 2. Allocation bytes is smaller than min. pool allocation size, or
      * 3. Allocation bytes is greater than the system memory
      */
      if (num_bytes > exec_pool_ctx->max_req_size_policyMgrPoolCtx || 
          num_bytes < exec_pool_ctx->min_req_size_policyMgrPoolCtx ||
          num_bytes > total_system_memory)
      {
        goto done;
      }

      /* can't go over total_system_memory */
      if ((total_system_memory - num_bytes) < exec_pool_used)
        goto done;

      break;

    case POLICY_MGR_STORAGE_POOL:

     /* Sanity checks.
      * Return can't allocate if:
      * 1. Allocation bytes is greater than max. pool allocation size, or
      * 2. Allocation bytes is smaller than min. pool allocation size, or
      * 3. Allocation bytes is greater than pool max.
      */
      if (num_bytes > storage_pool_ctx->max_req_size_policyMgrPoolCtx || 
          num_bytes < storage_pool_ctx->min_req_size_policyMgrPoolCtx ||
          num_bytes > storage_pool_total)
      {
        goto done;
      }

      /* can't go over storage_pool_total */
      if ((storage_pool_total - num_bytes) < storage_pool_used)
        goto done;

      break;

    default:
      assert(false);
  }

  /* Check if hit system max */
  if ((total_system_memory - num_bytes) < total_used)
    goto done;

  /* Update pool used bytes */
  (alloc_pool_type == POLICY_MGR_EXEC_POOL) ? 
      (exec_pool_ctx->used_memory_policyMgrPoolCtx = 
        exec_pool_used + num_bytes) :
      (storage_pool_ctx->used_memory_policyMgrPoolCtx = 
        storage_pool_used + num_bytes);

  ret = true;

done:
  pthread_mutex_unlock(&pm_stats.mutex_policyMgrStats);

  return ret;
}

void policyMgrAdjustAlloc(policyMgrPool pool, signed long bytes_diff)
{
  policyMgrPoolCtx   *pool_ctx = 
    &pm_stats.poolStats_policyMgrStats[pool];

  pthread_mutex_lock(&pm_stats.mutex_policyMgrStats);

  /* Update pool used bytes */
  pool_ctx->used_memory_policyMgrPoolCtx = (size_t)
    ((signed long)(pool_ctx->used_memory_policyMgrPoolCtx) + bytes_diff);

  pthread_mutex_unlock(&pm_stats.mutex_policyMgrStats);
}