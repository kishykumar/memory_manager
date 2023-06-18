#include <mem_mgr.h>
#include <policy_mgr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <jemalloc/jemalloc.h>
#include <assert.h>

/* Private macros */

/* Private Functions */
static void *allocMem(policyMgrPool pool, size_t num_bytes);
static void freeMem(policyMgrPool pool, void *p);

/* Public Functions */
bool initMemoryManager(size_t total_memory_bytes)
{
  return initPolicyMgr(total_memory_bytes);
}

void destroyMemoryManager(void)
{
  // Potentially check for leaks

  destroyPolicyMgr();
}

static void *allocMem(policyMgrPool pool, size_t num_bytes)
{
  size_t  real_alloc_size;
  void   *alloc_ptr;

  if (policyMgrAlloc(pool, num_bytes))
  {
    alloc_ptr = je_malloc(num_bytes);

    if (alloc_ptr)
    {
      real_alloc_size = je_sallocx(alloc_ptr, 0);
      assert(real_alloc_size >= num_bytes);

      /* Adjust the policy manager stats.
       * We do not have to do this if jemalloc provided us a function
       * that retrieves the real allocation size based on num_bytes without 
       * making an allocation.
       */
      if (real_alloc_size > num_bytes)
        policyMgrAdjustAlloc(pool, real_alloc_size - num_bytes);

      return alloc_ptr;
    }
    else /* je_malloc returned NULL */
    {
      policyMgrAdjustAlloc(pool, -num_bytes);
      return NULL;
    }
  }

  return NULL;
}

void *allocStorageMemory(size_t num_bytes)
{
  return allocMem(POLICY_MGR_STORAGE_POOL, num_bytes);
}

void *allocExecMemory(size_t num_bytes)
{
  return allocMem(POLICY_MGR_EXEC_POOL, num_bytes);
}

static void freeMem(policyMgrPool pool, void *p)
{
  size_t  num_bytes_freed = 0;

  num_bytes_freed = je_sallocx(p, 0);

  if (num_bytes_freed <= 0)
    return;

  je_free(p);
  policyMgrAdjustAlloc(pool, -num_bytes_freed);
}

void freeStorageMemory(void *p)
{
  freeMem(POLICY_MGR_STORAGE_POOL, p);
}

void freeExecMemory(void *p)
{
  freeMem(POLICY_MGR_EXEC_POOL, p);
}