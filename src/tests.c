#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <jemalloc/jemalloc.h>

/* File to test */
#include <mem_mgr.h>
#include <policy_mgr.h>
#include <unity.h>

#define _1_Ki (1024)
#define _1_Mi (1024 * _1_Ki)
#define _1_Gi (1024 * _1_Mi)

#define TEST_FUNC_TEST_MAX_SIZE   (128 * _1_Mi)
#define TEST_UNIT_TEST_MAX_SIZE   _1_Gi
#define TEST_CONC_TEST_MAX_SIZE   (500 * _1_Mi)

size_t mem_mgr_test_size = 0;

/*
  Testing Requirements:

  1. Unit testing: 
    - Write test cases for all the mem_mgr.h public routines.
    - Write Positive and negative test cases for those routines.
      * Allocate 0, negative, and more than MAX bytes from both pools.
    - Test different return values of those routines.

  2. Functional testing:

    - Positive test cases:
      * Multiple serial memory allocs and frees
      * Test if exec pool can go over its quota
      * Test the max for both pools

    - Negative test cases:
      * free without alloc
      * alloc and no free (checks if destroy catches memory leak)
        This can be tested through a JEMALLOC config:
        (https://technology.blog.gov.uk/2015/12/11/using-jemalloc-to-get-to-the-bottom-of-a-memory-leak/)
        It has not been done in the interest of time. 
      * double free (freeing an already freed memory)

  3. Concurrency testing:
    - This is kind of a stress test. Random allocations and frees from 
      multiple threads. Confirm no crash.
 */

void testNegativeCases()
{
  TEST_ASSERT_EQUAL_MESSAGE(NULL, allocExecMemory(0), "testNegativeCases: 1");
  TEST_ASSERT_EQUAL_MESSAGE(NULL, allocExecMemory(-1), "testNegativeCases: 2");
  TEST_ASSERT_EQUAL_MESSAGE(NULL, allocStorageMemory(0), "testNegativeCases: 3");
  TEST_ASSERT_EQUAL_MESSAGE(NULL, allocStorageMemory(-1), "testNegativeCases: 4");
}

void testExec1ByteAlloc()
{
  void *p = allocExecMemory(PM_EXEC_POOL_MIN_REQ_SIZE);
  TEST_ASSERT_NOT_EQUAL(NULL, p);
  TEST_ASSERT_EQUAL(8, je_sallocx(p, 0));
  freeExecMemory(p);
}

void testExec4BytesAlloc()
{
  void *p = allocExecMemory(4);
  TEST_ASSERT_NOT_EQUAL(NULL, p);
  TEST_ASSERT_EQUAL(8, je_sallocx(p, 0));
  freeExecMemory(p);
}

void testExec8BytesAlloc()
{
  void *p = allocExecMemory(8);
  TEST_ASSERT_NOT_EQUAL(NULL, p);
  TEST_ASSERT_EQUAL(8, je_sallocx(p, 0));
  freeExecMemory(p);
}

void testExecMaxSizeAlloc()
{
  void *p = allocExecMemory(PM_EXEC_POOL_MAX_REQ_SIZE + 1);
  TEST_ASSERT_EQUAL(NULL, p);
}

void testStorage1ByteAlloc()
{
  void *p = allocStorageMemory(1);
  TEST_ASSERT_EQUAL(NULL, p);
}

void testStorage4BytesAlloc()
{
  void *p = allocStorageMemory(4);
  TEST_ASSERT_EQUAL(NULL, p);
}

void testStorage8BytesAlloc()
{
  void *p = allocStorageMemory(8);
  TEST_ASSERT_EQUAL(NULL, p);
}

void testStorage4KBAlloc()
{
  void *p = allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE);
  TEST_ASSERT_EQUAL(4 * _1_Ki, je_sallocx(p, 0));
  freeStorageMemory(p);
}

void testStorageMaxSizeAlloc()
{
  void *p = allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE + 1);
  TEST_ASSERT_EQUAL(NULL, p);
}

void testMultipleSerialAllocs()
{
#define SERIAL_ALLOCS_MAX 1000

  void *pe, *ps, *pe_arr[SERIAL_ALLOCS_MAX], *ps_arr[SERIAL_ALLOCS_MAX];

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    pe = allocExecMemory((8 * _1_Ki));
    TEST_ASSERT_NOT_EQUAL(NULL, pe);
    freeExecMemory(pe);

    pe_arr[i] = allocExecMemory(128);
    TEST_ASSERT_NOT_EQUAL(NULL, pe_arr[i]);

    ps = allocStorageMemory((16 * _1_Mi));
    TEST_ASSERT_NOT_EQUAL(NULL, ps);
    freeStorageMemory(ps);

    ps_arr[i] = allocStorageMemory((16 * _1_Ki));
    TEST_ASSERT_NOT_EQUAL(NULL, ps_arr[i]);
  }

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    freeExecMemory(pe_arr[i]);
    freeStorageMemory(ps_arr[i]);
  }

#undef SERIAL_ALLOCS_MAX
}

void testExecPoolQuota()
{
#define SERIAL_ALLOCS_MAX (TEST_FUNC_TEST_MAX_SIZE / PM_EXEC_POOL_MAX_REQ_SIZE)

  void *pe_arr[SERIAL_ALLOCS_MAX];

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    pe_arr[i] = allocExecMemory(PM_EXEC_POOL_MAX_REQ_SIZE);
    TEST_ASSERT_NOT_EQUAL(NULL, pe_arr[i]);
  }

  /* try to allocate 1 more byte and it should not work */
  TEST_ASSERT_EQUAL(NULL, allocExecMemory(PM_EXEC_POOL_MIN_REQ_SIZE));
  TEST_ASSERT_EQUAL(NULL, allocExecMemory(PM_EXEC_POOL_MAX_REQ_SIZE));

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    freeExecMemory(pe_arr[i]);
  }

#undef SERIAL_ALLOCS_MAX
}

void testStoragePoolQuota()
{
#define SERIAL_ALLOCS_MAX                                                    \
  ((TEST_FUNC_TEST_MAX_SIZE / PM_STORAGE_POOL_MAX_REQ_SIZE) - 1)

  void *pe_arr[SERIAL_ALLOCS_MAX];

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    pe_arr[i] = allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, pe_arr[i], "Repeat allocations");
  }

  /* try to allocate 1 more byte and it should not work */
  TEST_ASSERT_EQUAL_MESSAGE(NULL, 
                            allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE), 
                            "min not work");
  TEST_ASSERT_EQUAL_MESSAGE(NULL, 
                            allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE), 
                            "max not work");

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    freeStorageMemory(pe_arr[i]);
  }

#undef SERIAL_ALLOCS_MAX
}

void testMixedWorkload()
{
  /* Test assumes TEST_FUNC_TEST_MAX_SIZE == 128 MB */
#define MAX_ALLOCS  64

  void *exec1, *storage1, *p_arr[MAX_ALLOCS];

  /* Testing steps: 
   * 1. Allocate 1 MB of exec memory
   * 2. Verify storage memory can be allocated and max can be hit
   * 3. Allocate exec memory until max and verify system max can be hit
   * 4. Deallocate storage memory and reallocate the same to verify system
   *    max can be hit
   */
  exec1 = allocExecMemory(_1_Mi);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, exec1, "testMixedWorkload: 1");

  storage1 = allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, storage1, "testMixedWorkload: 2");

  /* Verify storage max hit */
  TEST_ASSERT_EQUAL_MESSAGE(NULL,
                            allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE),
                            "testMixedWorkload: 3");

  for (int i = 0; i < 63; i++)
  {
    p_arr[i] = allocExecMemory(_1_Mi);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, p_arr[i], "testMixedWorkload: 4");
  }

  /* Verify system max hit with exec allocation */
  TEST_ASSERT_EQUAL_MESSAGE(NULL,
                            allocExecMemory(PM_EXEC_POOL_MIN_REQ_SIZE),
                            "testMixedWorkload: 5");

  /* Verify system max hit with storage allocation */
  TEST_ASSERT_EQUAL_MESSAGE(NULL,
                            allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE),
                            "testMixedWorkload: 6");

  freeStorageMemory(storage1);

  storage1 = allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, storage1, "testMixedWorkload: 7");

  /* Verify system max hit with storage allocation */
  TEST_ASSERT_EQUAL_MESSAGE(NULL,
                            allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE),
                            "testMixedWorkload: 8");

  /* Verify system max hit with exec allocation */
  TEST_ASSERT_EQUAL_MESSAGE(NULL,
                            allocExecMemory(PM_EXEC_POOL_MIN_REQ_SIZE),
                            "testMixedWorkload: 9");

  for (int i = 0; i < 63; i++)
  {
    freeExecMemory(p_arr[i]);
  }

  freeExecMemory(exec1);
  freeStorageMemory(storage1);
}

void testFree()
{
  /* don't run this test because unity does not support crash testing */
  return;

  void *exec1, *storage1;
  void *garbage_p = (void *)0xdeadbeef;

  /* free without alloc */
  freeExecMemory(NULL);
  freeStorageMemory(NULL);

  /* free at garbage address */
  freeExecMemory(garbage_p);
  freeStorageMemory(garbage_p);

  /* double free */
  exec1 = allocExecMemory(PM_STORAGE_POOL_MIN_REQ_SIZE);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, exec1, "testFree: 1");
  
  freeExecMemory(exec1);
  freeExecMemory(exec1);

  storage1 = allocStorageMemory(PM_STORAGE_POOL_MIN_REQ_SIZE);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, storage1, "testFree: 2");

  freeStorageMemory(storage1);
  freeStorageMemory(storage1);
}

void runFunctionalTests()
{
  RUN_TEST(testMultipleSerialAllocs);
  RUN_TEST(testExecPoolQuota);
  RUN_TEST(testStoragePoolQuota);
  RUN_TEST(testMixedWorkload);
  RUN_TEST(testFree);
}

void runUnitTests()
{
  /* positive test cases */
  RUN_TEST(testExec1ByteAlloc);
  RUN_TEST(testExec4BytesAlloc);
  RUN_TEST(testExec8BytesAlloc);
  RUN_TEST(testExecMaxSizeAlloc);

  RUN_TEST(testStorage4KBAlloc);
  RUN_TEST(testStorageMaxSizeAlloc);

  /* negative test cases */  
  RUN_TEST(testStorage1ByteAlloc);
  RUN_TEST(testStorage4BytesAlloc);
  RUN_TEST(testStorage8BytesAlloc);
  RUN_TEST(testNegativeCases);
}

void *testThreadMemAllocs(void *vargp)
{
#define SERIAL_ALLOCS_MAX 1000
  void *pe = NULL, *ps = NULL;
  void *pe_arr[SERIAL_ALLOCS_MAX], *ps_arr[SERIAL_ALLOCS_MAX];
  int rand_num;

  memset(pe_arr, 0, SERIAL_ALLOCS_MAX);
  memset(ps_arr, 0, SERIAL_ALLOCS_MAX);

  usleep(rand() % 3);
  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    pe = allocExecMemory(rand() % PM_EXEC_POOL_MAX_REQ_SIZE);
    if (pe) freeExecMemory(pe);

    pe_arr[i] = allocExecMemory(rand() % PM_EXEC_POOL_MAX_REQ_SIZE);

    ps = allocStorageMemory(rand() % PM_STORAGE_POOL_MAX_REQ_SIZE);
    if (ps) freeStorageMemory(ps);

    ps_arr[i] = allocStorageMemory(rand() % PM_STORAGE_POOL_MAX_REQ_SIZE);
  }
  pe = ps = NULL;

  for (int i = 0; i < SERIAL_ALLOCS_MAX; i++)
  {
    if (pe_arr[i])
    {
      freeExecMemory(pe_arr[i]);
      pe_arr[i] = NULL;
    }

    if (ps_arr[i])
    {
      freeStorageMemory(ps_arr[i]);
      ps_arr[i] = NULL;
    }
  }

  rand_num = rand() % SERIAL_ALLOCS_MAX;

  for (int i = 0; i < rand_num; i++)
    pe_arr[i] = allocExecMemory(_1_Ki);

  for (int i = 0; i < rand_num; i++)
  {
    if (pe_arr[i])
    {
      freeExecMemory(pe_arr[i]);
      pe_arr[i] = NULL;
    }
  }

  rand_num = rand() % SERIAL_ALLOCS_MAX;

  for (int i = 0; i < rand_num; i++)
    ps_arr[i] = allocStorageMemory(PM_STORAGE_POOL_MAX_REQ_SIZE);

  for (int i = 0; i < rand_num; i++)
  {
    if (ps_arr[i])
    {
      freeStorageMemory(ps_arr[i]);
      ps_arr[i] = NULL;
    }
  }

  rand_num = rand() % SERIAL_ALLOCS_MAX;

  for (int i = 0; i < rand_num; i++)
  {
    pe = allocExecMemory(rand() % PM_EXEC_POOL_MAX_REQ_SIZE);
    usleep(rand() % 3);
    if (pe)
    {
      freeExecMemory(pe);
      pe = NULL;
    }

    ps = allocStorageMemory(rand() % PM_STORAGE_POOL_MAX_REQ_SIZE);
    if (ps)
    {
      freeStorageMemory(ps);
      ps = NULL;
    }
  }

#undef SERIAL_ALLOCS_MAX
  return NULL;
}

void testMultiThreadAccess()
{
  /* Create 3 threads and have them do random size and number of allocs and
   * frees
   */
  pthread_t thr1, thr2, thr3;

  pthread_create(&thr1, NULL, testThreadMemAllocs, NULL);
  pthread_create(&thr2, NULL, testThreadMemAllocs, NULL);
  pthread_create(&thr3, NULL, testThreadMemAllocs, NULL);

  pthread_join(thr1, NULL);
  pthread_join(thr2, NULL);
  pthread_join(thr3, NULL);

  TEST_ASSERT_EQUAL(1,1);
}

void runConcurrencyTest()
{
  RUN_TEST(testMultiThreadAccess);
}

void setUp(void)
{
  /* Initialize Memory Manager */
  initMemoryManager(mem_mgr_test_size);
}

void tearDown(void)
{
  /* TODO: Test for memory leak */

  /* Destroy Memory Manager */
  destroyMemoryManager();
}

int main()
{
  mem_mgr_test_size = TEST_UNIT_TEST_MAX_SIZE;
  UNITY_BEGIN();
  runUnitTests();
  UNITY_END();

  mem_mgr_test_size = TEST_FUNC_TEST_MAX_SIZE;
  UNITY_BEGIN();
  runFunctionalTests();
  UNITY_END();

  mem_mgr_test_size = TEST_CONC_TEST_MAX_SIZE;
  UNITY_BEGIN();
  runConcurrencyTest();
  return UNITY_END();
}