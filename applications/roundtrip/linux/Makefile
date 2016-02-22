BIN = roundtrip
PL_NAME = roundtrip_v1_0
SRCS = roundtrip.c
BSP_HOME = /group/vlsiarch/samxi/active_projects/xilinx/vivado_workspace/roundtrip/roundtrip.sdk/system2_wrapper_fsbl_bsp

all: zynq

hls:
	vivado_hls hls.tcl

include Makefile.arm
