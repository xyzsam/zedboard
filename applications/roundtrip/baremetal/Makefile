.PHONY: all hls

SRCS = roundtrip.c

CPPFLAGS +=
CFLAGS += -O3 $(CPPFLAGS)

all: zynq

PL_NAME = roundtrip_v1_0
BIN = roundtrip
BSP_HOME = ../../common/bsp
HW_PLATFORM = ../../common/hw_platform
include ../../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(BIN)
