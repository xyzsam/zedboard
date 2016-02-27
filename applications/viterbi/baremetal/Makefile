.PHONY: all hls

TOPLEVEL = $(CURDIR)/../../..
SRCS = viterbi.c generate.c lfsr.c platform.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)
ARM_LFLAGS += -L$(BSP_LIBS) -lxil

all: zynq

PL_NAME = viterbi_v1_0
BIN = viterbi
BSP_HOME = ../common/bsp
HW_PLATFORM = ../common/hw_platform
include ../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
