// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xbfs_queue.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XBfs_queue_CfgInitialize(XBfs_queue *InstancePtr, XBfs_queue_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Bus_a_BaseAddress = ConfigPtr->Bus_a_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XBfs_queue_Start(XBfs_queue *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL) & 0x80;
    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL, Data | 0x01);
}

u32 XBfs_queue_IsDone(XBfs_queue *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XBfs_queue_IsIdle(XBfs_queue *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XBfs_queue_IsReady(XBfs_queue *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XBfs_queue_EnableAutoRestart(XBfs_queue *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL, 0x80);
}

void XBfs_queue_DisableAutoRestart(XBfs_queue *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_AP_CTRL, 0);
}

void XBfs_queue_InterruptGlobalEnable(XBfs_queue *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_GIE, 1);
}

void XBfs_queue_InterruptGlobalDisable(XBfs_queue *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_GIE, 0);
}

void XBfs_queue_InterruptEnable(XBfs_queue *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_IER);
    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_IER, Register | Mask);
}

void XBfs_queue_InterruptDisable(XBfs_queue *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_IER);
    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_IER, Register & (~Mask));
}

void XBfs_queue_InterruptClear(XBfs_queue *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XBfs_queue_WriteReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_ISR, Mask);
}

u32 XBfs_queue_InterruptGetEnabled(XBfs_queue *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_IER);
}

u32 XBfs_queue_InterruptGetStatus(XBfs_queue *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XBfs_queue_ReadReg(InstancePtr->Bus_a_BaseAddress, XBFS_QUEUE_BUS_A_ADDR_ISR);
}
