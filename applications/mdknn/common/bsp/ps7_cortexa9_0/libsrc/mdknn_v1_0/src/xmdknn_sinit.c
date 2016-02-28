// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xmdknn.h"

extern XMdknn_Config XMdknn_ConfigTable[];

XMdknn_Config *XMdknn_LookupConfig(u16 DeviceId) {
	XMdknn_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XMDKNN_NUM_INSTANCES; Index++) {
		if (XMdknn_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XMdknn_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMdknn_Initialize(XMdknn *InstancePtr, u16 DeviceId) {
	XMdknn_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMdknn_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMdknn_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif
