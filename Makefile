CC      = gcc
CFLAGS  = -g
RM      = rm -f

default: all

all: mem_mgr

med_of_meds: mem_mgr.c
	$(CC) $(CFLAGS) -o mem_mgr mem_mgr.c

clean veryclean:
	$(RM) mem_mgr
