// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.1
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
// 
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xfft_transpose.h"

extern XFft_transpose_Config XFft_transpose_ConfigTable[];

XFft_transpose_Config *XFft_transpose_LookupConfig(u16 DeviceId) {
	XFft_transpose_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XFFT_TRANSPOSE_NUM_INSTANCES; Index++) {
		if (XFft_transpose_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XFft_transpose_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XFft_transpose_Initialize(XFft_transpose *InstancePtr, u16 DeviceId) {
	XFft_transpose_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XFft_transpose_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XFft_transpose_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

