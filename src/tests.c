#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <jemalloc/jemalloc.h>

/* File to test */
#include <mem_mgr.h>
#include <unity.h>

#define _1_Ki (1024)
#define _1_Mi (1024 * _1_Ki)
#define _1_Gi (1024 * _1_Mi)

/*
  Testing Requirements:

  1. Unit testing: 
    - Write test cases for all the mem_mgr.h public routines.
    - Write Positive and negative test cases for those routines.
    - Test different return values of those routines.

  2. Functional testing:

    - Positive test cases:
      * Multiple serial memory allocs and frees
      * Test if exec pool can go over its quota
      * Test the max for both pools

    - Negative test cases:
      * Allocate 0, negative, and more than MAX bytes from both pools.
      * free without alloc
      * alloc and no free (checks if destroy catches memory leak)
      * double free (freeing an already freed memory)

  3. Concurrency testing:
    - Positive test cases:
      * Thread 2 can free memory allocated by Thread 1
      * 2 threads allocate and free memory at the same time

    - Negative test cases:
      * 

 */

void testExec1ByteAlloc()
{
  void *p = allocExecMemory(1);
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
  void *p = allocExecMemory((8 * _1_Mi) + 1);
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
  void *p = allocStorageMemory(4 * _1_Ki);
  TEST_ASSERT_EQUAL(4 * _1_Ki, je_sallocx(p, 0));
  freeStorageMemory(p);
}

void testStorageMaxSizeAlloc()
{
  void *p = allocStorageMemory((64 * _1_Mi) + 1);
  TEST_ASSERT_EQUAL(NULL, p);
}

void runUnitTests()
{
  RUN_TEST(testExec1ByteAlloc);
  RUN_TEST(testExec4BytesAlloc);
  RUN_TEST(testExec8BytesAlloc);
  RUN_TEST(testExecMaxSizeAlloc);

  RUN_TEST(testStorage1ByteAlloc);
  RUN_TEST(testStorage4BytesAlloc);
  RUN_TEST(testStorage8BytesAlloc);
  RUN_TEST(testStorage4KBAlloc);
  RUN_TEST(testStorageMaxSizeAlloc);
}

void runFunctionalTests()
{
  
}

void runTests()
{
  runUnitTests();
  runFunctionalTests();
}

void setUp(void)
{
  /* Initialize Memory Manager */
  initMemoryManager(_1_Gi);
}

void tearDown(void)
{
  /* TODO: Test for memory leak */

  /* Destroy Memory Manager */
  destroyMemoryManager();
}

int main()
{
  UNITY_BEGIN();
  runTests();
  return UNITY_END();
}