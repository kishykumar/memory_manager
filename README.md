# memory_manager
A memory manager that manages storage and execution memory

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

- We do not have to implement memory sharing between different processes. Can be achieved by allocating and mapping Shared Memory to processes' virtual address space.





- 

=============
# Design

Industrial grade memory allocator requirements:
1. Reduce fragmentation ...

Modules: 

1. Allocator (jemalloc)
  On comparison with other allocators like Linux buddy and slab allocator, 
  tcmalloc, Hoard, etc., the performance numbers suggest using jemalloc.

  TODO: Provide the link to the paper

  Here are the main advantages of jemalloc that shine out over other 
  allocators:

  1. Thread Local Cache (no lock contention)
  2. Built to support heavy concurrency in a multi-core / SMP system (required 
     for databases).
  3. Reduces internal and external fragmentation by finding a balance of 
     spacing of sizes within each size class.

  - Deciding Maximum allocatable size:
    We use the linux buddy allocator approach to calculate the max 
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

2. Policy Manager
  Layer on top of the allocator that implements the following two policies:
  1. Maintain the 50-50 allocation between the 2 pools
  2. Exec pool's borrowing of memory from Storage pool and releasing it back

  - Accounting for total memory space allocated to different/ the 2 pools
  - More policies can be added on-demand
  - Must keep policy manager separate from the allocator code for code 
    modularity and future-proofing (if more policies are added later or 
    the allocator design changes).

  Data Structures:
  1. 

  Interfaces: 
  1. 

=============
# Section

=============
# Section

=============
# Section

=============
# Section
