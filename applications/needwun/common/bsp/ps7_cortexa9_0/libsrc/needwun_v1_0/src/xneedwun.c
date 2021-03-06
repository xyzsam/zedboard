// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xneedwun.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XNeedwun_CfgInitialize(XNeedwun *InstancePtr, XNeedwun_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Bus_a_BaseAddress = ConfigPtr->Bus_a_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XNeedwun_Start(XNeedwun *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL) & 0x80;
    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL, Data | 0x01);
}

u32 XNeedwun_IsDone(XNeedwun *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XNeedwun_IsIdle(XNeedwun *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XNeedwun_IsReady(XNeedwun *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XNeedwun_EnableAutoRestart(XNeedwun *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL, 0x80);
}

void XNeedwun_DisableAutoRestart(XNeedwun *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_AP_CTRL, 0);
}

void XNeedwun_InterruptGlobalEnable(XNeedwun *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_GIE, 1);
}

void XNeedwun_InterruptGlobalDisable(XNeedwun *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_GIE, 0);
}

void XNeedwun_InterruptEnable(XNeedwun *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_IER);
    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_IER, Register | Mask);
}

void XNeedwun_InterruptDisable(XNeedwun *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_IER);
    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_IER, Register & (~Mask));
}

void XNeedwun_InterruptClear(XNeedwun *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XNeedwun_WriteReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_ISR, Mask);
}

u32 XNeedwun_InterruptGetEnabled(XNeedwun *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_IER);
}

u32 XNeedwun_InterruptGetStatus(XNeedwun *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XNeedwun_ReadReg(InstancePtr->Bus_a_BaseAddress, XNEEDWUN_BUS_A_ADDR_ISR);
}

