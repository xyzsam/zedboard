.PHONY: all hls

MODE = AXI_MODE
SRCS = main.c dma_utils.c platform.c

CPPFLAGS = -D$(MODE)
CFLAGS = -O3 $(CPPFLAGS)

all: zynq

PL_NAME = copy_axi_v1_0
BIN = copy-axi
BSP_HOME = ../../common/bsp
HW_PLATFORM = ../../common/hw_platform
include ../../../../common/makefiles/Makefile.arm

clean:
	rm -rf $(HLS_BIN)
