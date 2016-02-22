// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

/***************************** Include Files *********************************/
#include "xroundtrip.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XRoundtrip_CfgInitialize(XRoundtrip *InstancePtr, XRoundtrip_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Bus_a_BaseAddress = ConfigPtr->Bus_a_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XRoundtrip_Start(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL) & 0x80;
    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL, Data | 0x01);
}

u32 XRoundtrip_IsDone(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XRoundtrip_IsIdle(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XRoundtrip_IsReady(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XRoundtrip_EnableAutoRestart(XRoundtrip *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL, 0x80);
}

void XRoundtrip_DisableAutoRestart(XRoundtrip *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_CTRL, 0);
}

u32 XRoundtrip_Get_return(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_AP_RETURN);
    return Data;
}
void XRoundtrip_Set_in_r(XRoundtrip *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IN_R_DATA, Data);
}

u32 XRoundtrip_Get_in_r(XRoundtrip *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IN_R_DATA);
    return Data;
}

void XRoundtrip_InterruptGlobalEnable(XRoundtrip *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_GIE, 1);
}

void XRoundtrip_InterruptGlobalDisable(XRoundtrip *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_GIE, 0);
}

void XRoundtrip_InterruptEnable(XRoundtrip *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IER);
    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IER, Register | Mask);
}

void XRoundtrip_InterruptDisable(XRoundtrip *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IER);
    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IER, Register & (~Mask));
}

void XRoundtrip_InterruptClear(XRoundtrip *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XRoundtrip_WriteReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_ISR, Mask);
}

u32 XRoundtrip_InterruptGetEnabled(XRoundtrip *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_IER);
}

u32 XRoundtrip_InterruptGetStatus(XRoundtrip *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XRoundtrip_ReadReg(InstancePtr->Bus_a_BaseAddress, XROUNDTRIP_BUS_A_ADDR_ISR);
}

