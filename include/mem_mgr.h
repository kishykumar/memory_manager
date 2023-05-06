#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include <stdbool.h>
#include <stdlib.h>

bool init(size_t total_memory_bytes);

void destroy(void);

void *allocStorageMemory(size_t num_bytes);

void freeStorageMemory(void *p, size_t num_bytes);

void *allocExecMemory(size_t num_bytes);

void freeExecMemory(void *p);

#endif /* MEM_MGR_H_ */