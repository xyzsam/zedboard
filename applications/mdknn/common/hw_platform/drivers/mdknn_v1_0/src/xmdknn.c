// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xmdknn.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XMdknn_CfgInitialize(XMdknn *InstancePtr, XMdknn_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Bus_a_BaseAddress = ConfigPtr->Bus_a_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XMdknn_Start(XMdknn *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL) & 0x80;
    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL, Data | 0x01);
}

u32 XMdknn_IsDone(XMdknn *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XMdknn_IsIdle(XMdknn *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XMdknn_IsReady(XMdknn *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XMdknn_EnableAutoRestart(XMdknn *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL, 0x80);
}

void XMdknn_DisableAutoRestart(XMdknn *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_AP_CTRL, 0);
}

void XMdknn_InterruptGlobalEnable(XMdknn *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_GIE, 1);
}

void XMdknn_InterruptGlobalDisable(XMdknn *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_GIE, 0);
}

void XMdknn_InterruptEnable(XMdknn *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_IER);
    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_IER, Register | Mask);
}

void XMdknn_InterruptDisable(XMdknn *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_IER);
    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_IER, Register & (~Mask));
}

void XMdknn_InterruptClear(XMdknn *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMdknn_WriteReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_ISR, Mask);
}

u32 XMdknn_InterruptGetEnabled(XMdknn *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_IER);
}

u32 XMdknn_InterruptGetStatus(XMdknn *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMdknn_ReadReg(InstancePtr->Bus_a_BaseAddress, XMDKNN_BUS_A_ADDR_ISR);
}
