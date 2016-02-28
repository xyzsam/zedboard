.PHONY: all hls

TOPLEVEL = $(CURDIR)/../../..
SRCS = mdknn.c generate.c platform.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)
ARM_LFLAGS += -L$(BSP_LIBS) -lxil

all: zynq

PL_NAME = mdknn_v1_0
BIN = mdknn
BSP_HOME = ../common/bsp
HW_PLATFORM = ../common/hw_platform
include ../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
