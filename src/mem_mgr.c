#include <mem_mgr.h>
#include <policy_mgr.h>
#include <stdlib.h>
#include <stdbool.h>

bool init(size_t total_memory_bytes)
{
  return initPolicyMgr(total_memory_bytes);
}

void destroy(void)
{
  // Check for leaks
}

void *allocStorageMemory(size_t num_bytes)
{
  if (policyMgrAllocRequest(POLICY_MGR_STORAGE_POOL, num_bytes))
  {
    return malloc(num_bytes);
  }
  
  return NULL;
}

void freeStorageMemory(void *p, size_t num_bytes)
{
  size_t  num_bytes_freed = 0;

  // TODO fix this to -> num_bytes_freed = free(p);
  free(p);
  policyMgrFreeRequest(POLICY_MGR_STORAGE_POOL, num_bytes_freed);
}

void *allocExecMemory(size_t num_bytes)
{
  if (policyMgrAllocRequest(POLICY_MGR_EXEC_POOL, num_bytes))
  {
    return malloc(num_bytes);
  }

  return NULL;
}

void freeExecMemory(void *p)
{
  size_t  num_bytes_freed = 0;

  // TODO fix this to -> num_bytes_freed = free(p);
  free(p);
  policyMgrFreeRequest(POLICY_MGR_EXEC_POOL, num_bytes_freed);
}

int main()
{
  return 0;
}