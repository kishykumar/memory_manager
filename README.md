# memory_manager
A memory manager that manages different pools of memory: storage and 
execution memory

===================
# Problem Statement

Design a memory manager
You need to design a memory manager for a running system. Let’s assume all the memory requirements
of the system passes through this module.

The memory manager should have two pools of storage
1. Execution Memory
2. Storage Memory

Functionality:
1. Memory manager would be initialized with some amount of memory I.e 1 GB, 2GB, etc.
2. Each of the pools would be allocated 50% of the above allocated memory.
3. Each memory request is tagged either as storage memory request or execution memory request.
4. Based on the availability the memory manager will either grant the memory request, decline or
hold the request. The design is your choice.
5. When execution memory is insufficient & an execution memory request comes in, it can borrow
memory from storage memory. When the work is done the memory borrowed should be returned
to the storage memory pool.
6. If storage memory is full and a storage memory request comes in, it can randomly evict away
some allocated memory.

Requirements:
Design the interface to your choice. See below for some example interfaces

  acquireStorageMemory ( pageId : Integer, numBytes : Long);
  acquireExecutionMemory ( execId : Integer, numBytes : Long);
  releaseStorageMemory(numBytes : Long)
  releaseExecutionMemory(numBytes : Long)

1. Your implementation to the interface should be a working code with associated unit tests.
2. You will be designing only the memory manager. Rest of the system is a black box to you and
they will interact with the manager through the interface.
3. You can make assumptions , but they should be spelled out clearly in comments & design.

===========
# Questions

Questions to ask:

- >>> If storage memory is full and a storage memory request comes in , it can
  >>> randomly evict away some allocated memory.
  Evicting/releasing allocated memory is dangerous because releasing memory 
  in the middle of client usage of the allocated memory can cause seg faults.
  I believe what is meant is storage memory can be used as a cache.

- What is the largest and the smallest size of the allocations for each pool?

- What is the workload? The workload will tell us typical size of the 
  allocations, and the memory manager can optimize for that.

=============
# Assumptions

- OS Page size = 4KB
  Cache line size = 64 bytes
  L1 cache size per core = 64KB (instruction cache + data cache)

- Recovery of the memory manager's data structures has not been implemented
  (in the event of process death). Can be done using latches and recovery 
  areas if needed.

=============
# Design

1. General Memory Allocator
  Industrial grade memory allocator requirements:

  a. Allocation and free should be blazingly fast
  b. Reduce fragmentation (internal and external)
  c. Support a high concurrency of memory requests
  d. Caching: Avoid hardware cache and page misses
  e. Avoid false sharing: single cache line shared between multiple processes
     on different cores
  f. Thread locality/caching: different threads can allocate/free without 
     contention

2. Memory Policy Management
  The requirement is to 
  a) give 50-50 memory to the 2 pools, but
  b) the execution pool can borrow memory from storage pool and return
  it back, and 
  c) the storage pool can evict allocated memory.

  a) There are 2 ways to implement the 50-50 allocation scheme:

    i) Hard partitioning
    In this approach, there are 2 allocators that manage the 2 pools.
    Each allocator is optimized for its own pool. The storage allocator 
    is optimized for large allocations, and execution memory for small 
    and medium allocations.
    But, there is no gain in the allocator performance with hard partitioning
    because even a general-purpose memory allocator can be very efficient for 
    different size classes.

    Since we also have to implement transfer of memory blocks (borrowing) 
    between the 2 pools/allocators, following are the limitations of hard
    partitioning along with its implementation complexity:
    1. Locking overhead: have to lock both the allocators (but not at the 
       same time) to remove and add memory blocks.
    2. Latency during transfer: Incoming memory request has to wait for the transfer process to 
       complete. So, some memory requests will have higher latency (outliers).

    ii) Soft partitioning
    There is a single efficient allocator that manages all size classes: 
    small, medium, and large. A policy manager accounts for the free and 
    allocated memory in each pool. It decides whether to serve or fail a 
    memory request based on different policies i.e., whether the storge 
    pool has exhausted its quota, all executions requests are served unless
    the system memory is full, etc.

  Soft partitioning is preferred and has been implemented over hard 
  partitioning due to its simplicity and equivalent performance.

  b) Borrowing memory
   An execution memory request can borrow memory from the storage pool when
   execution memory is insufficient. Since the memory is soft-partitioned,
   borrowing memory is naturally solved. Execution memory is simply allocated
   from the base allocator and returned back to the base allocator.
   An execution request will fail only when the entire memory is full. 
   Whereas, a storage request will fail when the storage memory quota is over.

  c) Evicting allocated storage memory is not feasible as discussed in the
   questions section.

3. Aligned memory
  - The allocator always returns memory aligned to word boundary for execution
    memory allocations (small and medium size) and makes sure the memory resides
    in a single OS page.
  - For storage pool allocations, memory is (OS) page-aligned.

# Modules:

1. Allocator (jemalloc)

  On comparison with other allocators like Linux buddy and slab allocator, 
  tcmalloc, Hoard, etc., the performance numbers suggest using jemalloc.

  Link to the jemalloc paper: https://www.bsdcan.org/2006/papers/jemalloc.pdf

  Here are the main advantages of jemalloc that shine out over other 
  allocators:

  a. Thread Local Cache (no lock contention)
  b. Built to support heavy concurrency in a multi-core / SMP system (required 
     for databases).
  c. Reduces internal and external fragmentation by finding a balance of 
     spacing of sizes within each size class.

  - Deciding Maximum allocatable size:
    By using the Linux buddy allocator approach to calculate the max 
    = log-base2(total_memory / page_size)
    For example, 2^(log-base2(1GB / 4KB)) * 4KB = 256MB
    TODO: Verify
    jemalloc uses a buddy allocator underneath.

  - Deciding the maximum and minimum alloc size of Execution Pool:
    
    Max: 8MB (characteristic of the application)
         Any request bigger than 8 MB should consider:
         - using segmented memory allocation technique where pages are
           allocated when accessed. Segmented array is one example where 
           large array sizes are desired.
         - client can split the memory request into multiple requests

    Min: The minimum amount of memory that any app can allocate is 1 byte.
         The allocate may allocate more than 1 byte 

  - Deciding the maximum and minimum alloc size of Storage Pool:

    Max: Maximum I/O size is the characteristic of the underlying storage 
         hardware and the app workload. This is typically in MBs. 
         Therefore, we support the allocator's max allocatable size. 

    Min: The minimum I/O size for a storage page manager would be 4KB i.e., 
         assuming typical hard disk/ SSD sector size of 4KB. The minimum 
         storage pool memory allocation size should hence by 4KB.

  Public Interfaces:
  1. boolean init(size_t total_memory_bytes);
  2. void destroy();
  3. void *allocStorageMemory(size_t num_bytes);
  4. void freeStorageMemory(void *p);
  5. void *allocExecMemory(void *p);
  6. void freeExecMemory(void *p);

  Internal Routines:
  1. void *alloc(size_t num_bytes);
  2. void free(void *buffer);

  Handling Out-of-Memory:
  When the allocator runs out of memory to allocate to a pool, it retries
  a fixed number of times. If it can't allocate even after that, the 
  allocation interfaces returns NULL.

2. Policy Manager

  Layer on top of the allocator that implements the policy of 
  maintaining the 50-50 allocation between the 2 memory pools,
  among other policies:

  - Accounts the current and total memory space for the 2 pools
  - Enforces the min and max alloc size for each pool
  
  Must separate the policy manager from the allocator module for code 
  modularity and future-proofing (if more policies are added later or 
  the allocator design changes).

  Data Structures:
  1. A global struct that maintains the following statistics:
    - total system memory
    - Per-pool stats:
      - used memory
      - minimum and maximum allowed memory request size
      - total memory

    This struct is protected under an exclusive lock.

  Public interfaces:
  - boolean allocRequest(poolId pool, size_t num_bytes);
  /* This routine is called by the memory allocator to enforce memory 
   * policies. The policy manager checks and update its statistics to make
   * a decision.
   * If the allocation can be made, return TRUE. 
   * Otherwise, FALSE is returned.
   */

  - void freeRequest(poolId pool, size_t num_bytes);
  /* This routine is called by the memory allocator at memory free time 
   * to update memory statistics of the policy manager.
   */