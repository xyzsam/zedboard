// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

#ifndef XCOPY_AXI_H
#define XCOPY_AXI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xcopy_axi_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Bus_a_BaseAddress;
} XCopy_axi_Config;
#endif

typedef struct {
    u32 Bus_a_BaseAddress;
    u32 IsReady;
} XCopy_axi;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XCopy_axi_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XCopy_axi_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XCopy_axi_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XCopy_axi_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XCopy_axi_Initialize(XCopy_axi *InstancePtr, u16 DeviceId);
XCopy_axi_Config* XCopy_axi_LookupConfig(u16 DeviceId);
int XCopy_axi_CfgInitialize(XCopy_axi *InstancePtr, XCopy_axi_Config *ConfigPtr);
#else
int XCopy_axi_Initialize(XCopy_axi *InstancePtr, const char* InstanceName);
int XCopy_axi_Release(XCopy_axi *InstancePtr);
#endif

void XCopy_axi_Start(XCopy_axi *InstancePtr);
u32 XCopy_axi_IsDone(XCopy_axi *InstancePtr);
u32 XCopy_axi_IsIdle(XCopy_axi *InstancePtr);
u32 XCopy_axi_IsReady(XCopy_axi *InstancePtr);
void XCopy_axi_EnableAutoRestart(XCopy_axi *InstancePtr);
void XCopy_axi_DisableAutoRestart(XCopy_axi *InstancePtr);


void XCopy_axi_InterruptGlobalEnable(XCopy_axi *InstancePtr);
void XCopy_axi_InterruptGlobalDisable(XCopy_axi *InstancePtr);
void XCopy_axi_InterruptEnable(XCopy_axi *InstancePtr, u32 Mask);
void XCopy_axi_InterruptDisable(XCopy_axi *InstancePtr, u32 Mask);
void XCopy_axi_InterruptClear(XCopy_axi *InstancePtr, u32 Mask);
u32 XCopy_axi_InterruptGetEnabled(XCopy_axi *InstancePtr);
u32 XCopy_axi_InterruptGetStatus(XCopy_axi *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
