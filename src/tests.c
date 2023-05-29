#include <mem_mgr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define _1_Ki (1024)
#define _1_Mi (1024 * _1_Ki)
#define _1_Gi (1024 * _1_Mi)

void runTests()
{
  
}

int main()
{
  /* Initialize Memory Manager */
  initMemoryManager(_1_Gi);

  runTests();

  /* Destroy Memory Manager */
  destroyMemoryManager();
  return 0;
}