CC      = clang
CONFIGURE_CFLAGS = -Werror=int-conversion -Werror=uninitialized
CFLAGS  = -g $(CONFIGURE_CFLAGS)
RM      = rm -rf

PROJECT_ROOT =
SRC_ROOT = $(PROJECT_ROOT)src
INCLUDE_ROOT = $(PROJECT_ROOT)include
JEMALLOC_ROOT = $(PROJECT_ROOT)jemalloc
JEMALLOC_PROJECT_ROOT = $(JEMALLOC_ROOT)/obj

UNITY_ROOT = $(PROJECT_ROOT)unity
UNITY_SRC_ROOT = $(UNITY_ROOT)/src
INCLUDE_UNITY = -I$(UNITY_SRC_ROOT)

LIB_JEMALLOC = $(JEMALLOC_PROJECT_ROOT)/lib
LINK_JEMALLOC_LIB = -L$(LIB_JEMALLOC)

INCLUDE_JEMALLOC = -I$(JEMALLOC_PROJECT_ROOT)/include
EXTRAS_JEMALLOC = -Wl,-rpath,$(LIB_JEMALLOC) -lstdc++ -lpthread \
									$(LIB_JEMALLOC)/libjemalloc.dylib

C_HDRS = $(INCLUDE_ROOT)/mem_mgr.h

C_SRCS = $(SRC_ROOT)/mem_mgr.c \
				 $(SRC_ROOT)/policy_mgr.c \
				 $(SRC_ROOT)/tests.c

C_TEST_SRCS = $(UNITY_SRC_ROOT)/unity.c

INCLUDE = -I$(INCLUDE_ROOT) -I$(SRC_ROOT)

cmk: clean all
default: all
all: mem_mgr

mem_mgr: $(C_SRCS)
	$(CC) $(CFLAGS) $(LINK_JEMALLOC_LIB) $(INCLUDE_JEMALLOC) $(INCLUDE_UNITY) \
	$(EXTRAS_JEMALLOC) $(INCLUDE) -o mem_mgr $(C_SRCS) $(C_TEST_SRCS)

clean:
	$(RM) mem_mgr mem_mgr.dSYM