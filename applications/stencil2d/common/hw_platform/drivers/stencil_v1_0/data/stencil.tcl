# ==============================================================
# File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
# Version: 2015.1
# Copyright (C) 2015 Xilinx Inc. All rights reserved.
# 
# ==============================================================

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XStencil" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_BUS_A_BASEADDR" \
        "C_S_AXI_BUS_A_HIGHADDR"

    xdefine_config_file $drv_handle "xstencil_g.c" "XStencil" \
        "DEVICE_ID" \
        "C_S_AXI_BUS_A_BASEADDR"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XStencil" \
        "DEVICE_ID" \
        "C_S_AXI_BUS_A_BASEADDR" \
        "C_S_AXI_BUS_A_HIGHADDR"
}

