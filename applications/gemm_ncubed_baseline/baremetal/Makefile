.PHONY: all hls

TOPLEVEL = $(CURDIR)/../../..
SRCS = dma_utils.c gemm.c interrupts.c profiling.c platform.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)
ARM_LFLAGS += -L$(BSP_LIBS) -lxil

all: zynq

PL_NAME = gemm_v1_0
BIN = gemm
BSP_HOME = ../common/bsp
HW_PLATFORM = ../common/hw_platform
include ../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
