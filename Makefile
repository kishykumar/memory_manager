CC      = clang
CFLAGS  = -g -Werror=int-conversion
RM      = rm -rf

PROJECT_ROOT =
SRC_ROOT = $(PROJECT_ROOT)src
INCLUDE_ROOT = $(PROJECT_ROOT)include
JEMALLOC_ROOT = $(PROJECT_ROOT)jemalloc
JEMALLOC_PROJECT_ROOT = $(JEMALLOC_ROOT)/obj

LIB_JEMALLOC = -L$(JEMALLOC_PROJECT_ROOT)/lib
INCLUDE_JEMALLOC = -I$(JEMALLOC_PROJECT_ROOT)/include

C_HDRS = $(INCLUDE_ROOT)/mem_mgr.h

C_SRCS = $(SRC_ROOT)/mem_mgr.c \
					$(SRC_ROOT)/policy_mgr.c

INCLUDE = -I$(INCLUDE_ROOT) -I$(SRC_ROOT)

cmk: clean all
default: all
all: mem_mgr

mem_mgr: $(C_SRCS)
	$(CC) $(CFLAGS) $(LIB_JEMALLOC) $(INCLUDE_JEMALLOC) \
	$(INCLUDE) -o mem_mgr $(C_SRCS)

clean:
	$(RM) mem_mgr mem_mgr.dSYM