.PHONY: all hls

TOPLEVEL = $(CURDIR)/../../..
SRCS = bfs_queue.c generate.c lfsr.c platform.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)
ARM_LFLAGS += -L$(BSP_LIBS) -lxil

all: zynq

PL_NAME = bfs_queue_v1_0
BIN = bfs_queue
BSP_HOME = ../common/bsp
HW_PLATFORM = ../common/hw_platform
include ../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
