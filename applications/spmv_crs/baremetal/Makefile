.PHONY: all hls

TOPLEVEL = $(CURDIR)/../../..
SRCS = spmv_crs.c generate.c platform.c 494_bus_full.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)
ARM_LFLAGS += -L$(BSP_LIBS) -lxil

all: zynq

PL_NAME = spmv_v1_0
BIN = spmv
BSP_HOME = ../common/bsp
HW_PLATFORM = ../common/hw_platform
include ../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
