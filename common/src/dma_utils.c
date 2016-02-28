#include <stdbool.h>
#include <stdio.h>
#include "xaxidma.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "arm_timing_utils.h"
#include "profiling.h"

u32 *Packet = (u32 *) TX_BUFFER_BASE;

void InitPacket(struct DmaPacket* packet, struct DmaChannel *channel) {
	packet->TxBuf = (u8*) TX_BUFFER_BASE;
	packet->RxBuf = (u8*) RX_BUFFER_BASE;
	packet->channel = channel;
}

int InitDma(XAxiDma* dma_device, struct DmaChannel * channel, int dma_device_id) {
  int status = 0;
  XAxiDma_Config *config;
  config = XAxiDma_LookupConfig(dma_device_id);
  if (!config) {
	  printf("Failed to lookup DMA config.\r\n");
	  return -1;
  }
  status = XAxiDma_CfgInitialize(dma_device, config);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }

  XAxiDma_Reset(dma_device);
  while (!XAxiDma_ResetIsDone(dma_device)) {}
  XAxiDma_IntrEnable(dma_device,
                     (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK),
                     XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrEnable(dma_device,
                     (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK),
                     XAXIDMA_DEVICE_TO_DMA);

  g_dma_err = 0;
  g_mm2s_done = 0;
  g_s2mm_done = 0;
  return 0;
}

int RxSetup(XAxiDma * AxiDma, struct DmaPacket * packet)
{
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	XAxiDma_Bd BdTemplate;
	u32 BdCount;

	packet->channel->RxRingPtr = XAxiDma_GetRxRing(AxiDma);

	/* Disable all RX interrupts before RxBD space setup */
	XAxiDma_BdRingIntDisable(packet->channel->RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set delay and coalescing */
	XAxiDma_BdRingSetCoalesce(packet->channel->RxRingPtr, Coalesce, Delay);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
			RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(packet->channel->RxRingPtr, RX_BD_SPACE_BASE,
			RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);

	if (Status != XST_SUCCESS) {
		xil_printf("RX create BD ring failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/*
	 * Setup an all-zero BD as the template for the Rx channel.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(packet->channel->RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("RX clone BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(packet->channel->RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX start hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function sets up the TX channel of a DMA engine to be ready for packet
 * transmission
 *
 * @param	AxiDmaInstPtr is the instance pointer to the DMA engine.
 *
 * @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
 *
 * @note		None.
 *
 ******************************************************************************/
int TxSetup(XAxiDma * AxiDma, struct DmaChannel * channel)
{
	XAxiDma_Bd BdTemplate;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	u32 BdCount;

	// xil_printf("Setting up TX channel.\r\n");
	channel->TxRingPtr = XAxiDma_GetTxRing(AxiDma);

	/* Disable all TX interrupts before TxBD space setup */
	XAxiDma_BdRingIntDisable(channel->TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set TX delay and coalesce */
	XAxiDma_BdRingSetCoalesce(channel->TxRingPtr, Coalesce, Delay);

	/* Setup TxBD space  */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
			TX_BD_SPACE_HIGH - TX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(channel->TxRingPtr, TX_BD_SPACE_BASE,
			TX_BD_SPACE_BASE,
			XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("failed create BD ring in txsetup\r\n");

		return XST_FAILURE;
	}

	/*
	 * We create an all-zero BD as the template.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(channel->TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("failed bdring clone in txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start the TX channel */
	Status = XAxiDma_BdRingStart(channel->TxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("failed start bdring txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int PreparePacket(struct DmaPacket *packet)
{
  int Status;
  int i;
  int cycle_start, cycle_end;
  u32 TxBufferPtr, RxBufferPtr;
  int last_bd_size, num_bds;
  int mask;
  XAxiDma_Bd *BdCurPtr;

	/////////////////////////////// RX //////////////////////////////

  cycle_start = get_cyclecount();
  	last_bd_size = packet->RxNumBytes % MAX_PKT_LEN;
  	num_bds = packet->RxNumBytes / MAX_PKT_LEN;
  	if (last_bd_size > 0)
  		num_bds++;
      packet->NumRxBds = num_bds;

  	// xil_printf("Free rx bd count: %d\r\n", FreeBdCount);


  	Status = XAxiDma_BdRingAlloc(
  			packet->channel->RxRingPtr, packet->NumRxBds, &(packet->RxBdPtr));
  	if (Status != XST_SUCCESS) {
  		xil_printf("RX alloc BD failed %d\r\n", Status);

  		return XST_FAILURE;
  	}

  	// xil_printf("  RxBdPtr = 0x%x\r\n", packet->RxBdPtr);

  	BdCurPtr = packet->RxBdPtr;
  	RxBufferPtr = (u32)packet->RxBuf;
  	for (i = 0; i < packet->NumRxBds; i++) {
  		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);

  		if (Status != XST_SUCCESS) {
  			xil_printf("Set buffer addr %x on BD %x failed %d\r\n",
  					(unsigned int)RxBufferPtr,
  					(unsigned int)BdCurPtr, Status);
  			return XST_FAILURE;
  		}

  		if (i != packet->NumRxBds - 1) {
  			Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
  							packet->channel->RxRingPtr->MaxTransferLen);
  		} else {
  			Status = XAxiDma_BdSetLength(BdCurPtr, last_bd_size,
  							packet->channel->RxRingPtr->MaxTransferLen);
  		}

  		if (Status != XST_SUCCESS) {
  			xil_printf("Rx set length %d on BD %x failed %d\r\n",
  					MAX_PKT_LEN, (unsigned int)BdCurPtr, Status);
  			return XST_FAILURE;
  		}

  		// Receive BDs do not need to set anything for the control
  		// The hardware will set the SOF/EOF bits per stream status
  		XAxiDma_BdSetCtrl(BdCurPtr, 0);
  		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);

  		RxBufferPtr += MAX_PKT_LEN;
  		BdCurPtr = XAxiDma_BdRingNext(packet->channel->RxRingPtr, BdCurPtr);
  	}

  	Status = XAxiDma_BdRingToHw(packet->channel->RxRingPtr, packet->NumRxBds,
  			packet->RxBdPtr);
  	if (Status != XST_SUCCESS) {
  		xil_printf("RX submit hw failed %d\r\n", Status);

  		return XST_FAILURE;
  	}

  	cycle_end = get_cyclecount();
  	rx_prepare = cycle_end - cycle_start;

  	/////////////////////////////// TX //////////////////////////////


  cycle_start = get_cyclecount();
  last_bd_size = packet->TxNumBytes % MAX_PKT_LEN;
  num_bds = packet->TxNumBytes / MAX_PKT_LEN;
  if (last_bd_size > 0)
	  num_bds++;

	/* Allocate a BD */
	Status = XAxiDma_BdRingAlloc(
      packet->channel->TxRingPtr, num_bds, &(packet->TxBdPtr));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	// xil_printf("  TxBdPtr = 0x%x\r\n", packet->TxBdPtr);

  packet->NumTxBds = num_bds;
  TxBufferPtr = (u32) packet->TxBuf;
  BdCurPtr = packet->TxBdPtr;
  for (i = 0; i < num_bds; i++) {
	  mask = 0;
      /* Set up the BD using the information of the packet to transmit */
      Status = XAxiDma_BdSetBufAddr(BdCurPtr, TxBufferPtr);

      if (Status != XST_SUCCESS) {
        xil_printf("Tx set buffer addr %x on BD %x failed %d\r\n",
            TxBufferPtr, BdCurPtr, Status);
        return XST_FAILURE;
      }

      if (i == num_bds - 1 && last_bd_size > 0) {
    	  Status = XAxiDma_BdSetLength(
    	           BdCurPtr, last_bd_size,
    	           packet->channel->TxRingPtr->MaxTransferLen);
      } else {
    	  Status = XAxiDma_BdSetLength(
    	           BdCurPtr, MAX_PKT_LEN,
    	           packet->channel->TxRingPtr->MaxTransferLen);
      }

      if (Status != XST_SUCCESS) {
        xil_printf("Tx set length %d on BD %x failed %d\r\n",
            MAX_PKT_LEN, BdCurPtr, Status);
        return XST_FAILURE;
      }
      /* For single packet, both SOF and EOF are to be set
       */
      if (i == 0)
        mask |= XAXIDMA_BD_CTRL_TXSOF_MASK;
      if (i == num_bds - 1)
        mask |= XAXIDMA_BD_CTRL_TXEOF_MASK;
      XAxiDma_BdSetCtrl(BdCurPtr, mask);
      XAxiDma_BdSetId(BdCurPtr, TxBufferPtr);

      TxBufferPtr += MAX_PKT_LEN;
      BdCurPtr = XAxiDma_BdRingNext(packet->channel->TxRingPtr, BdCurPtr);
  }

#if (XPAR_AXIDMA_0_SG_INCLUDE_STSCNTRL_STRM == 1)
	Status = XAxiDma_BdSetAppWord(packet->TxBdPtr,
			XAXIDMA_LAST_APPWORD, packet->TxNumBytes);

	/* If Set app length failed, it is not fatal
	 */
	if (Status != XST_SUCCESS) {
		xil_printf("Set app word failed with %d\r\n", Status);
	}
#endif

	cycle_end = get_cyclecount();
	tx_prepare = cycle_end - cycle_start;

	return XST_SUCCESS;
}

int SendPacket(struct DmaPacket* packet)
{
	int Status;
	int cycle_start, cycle_end;
	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	cycle_start = get_cyclecount();
	Xil_DCacheFlushRange((u32)packet->TxBuf, packet->TxNumBytes);
	cycle_end = get_cyclecount();
	dcache_flush = cycle_end - cycle_start;

	/* Critical if using anything but ACP! You must invalidate the receive buffer address range. */
	cycle_start = get_cyclecount();
	Xil_DCacheInvalidateRange((u32)packet->RxBuf, packet->TxNumBytes);
	cycle_end = get_cyclecount();
	dcache_invalidate = cycle_end - cycle_start;

	/* Give the BD to DMA to kick off the transmission. */
	Status = XAxiDma_BdRingToHw(
      packet->channel->TxRingPtr, packet->NumTxBds, packet->TxBdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("tx hw failed %d\r\n", Status);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int WaitForCompletion(struct DmaPacket* packet, int* ProcessedBdCount, bool interrupts)
{
	int timeout = 500;
	int status;
	// Wait until the one BD TX transaction is done.
	// TODO: We need to check for the right number of BDs.
  if (interrupts) {
    while (!g_mm2s_done && !g_dma_err && timeout > 0) {
      timeout--;
    }
  } else {
    while ((*ProcessedBdCount = XAxiDma_BdRingFromHw(
        packet->channel->TxRingPtr, XAXIDMA_ALL_BDS, &(packet->TxBdPtr))) < packet->NumTxBds && timeout > 0) {
      timeout--;
    }
  }

	if (timeout == 0) {
		xil_printf("TX timed out!\r\n");
	} else if (g_dma_err) {
    xil_printf("DMA error!\r\n");
	} else {
		xil_printf("TX complete.\r\n");
	}
	xil_printf("TX processed %d BDs\r\n", *ProcessedBdCount);

	status = XAxiDma_BdRingFree(
			packet->channel->TxRingPtr, *ProcessedBdCount, packet->TxBdPtr);
	if (status != XST_SUCCESS) {
		xil_printf("Failed to free %d tx BDs %d\r\n",
				*ProcessedBdCount, status);
		return XST_FAILURE;
	}
	timeout = 16384;
	/* Wait until the data has been received by the Rx channel */

  if (interrupts) {
    while (!g_s2mm_done && !g_dma_err && timeout > 0)
      timeout--;
  } else {
    while ((*ProcessedBdCount = XAxiDma_BdRingFromHw(
        packet->channel->RxRingPtr, XAXIDMA_ALL_BDS, &(packet->RxBdPtr))) < packet->NumRxBds && timeout > 0) {
      timeout--;
    }
  }

	if (timeout == 0) {
		xil_printf("RX timed out!\r\n");
	} else if (g_dma_err) {
    xil_printf("DMA error!\r\n");
  } else {
		xil_printf("RX complete with timeout remaining %d.\r\n", timeout);
	}
	xil_printf("RX processed %d BDs\r\n", *ProcessedBdCount);
	/* Free all processed RX BDs for future transmission */
	status = XAxiDma_BdRingFree(
			packet->channel->RxRingPtr, *ProcessedBdCount, packet->RxBdPtr);
	if (status != XST_SUCCESS) {
		xil_printf("Failed to free %d rx BDs %d\r\n",
				ProcessedBdCount, status);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int ReleaseBds(struct DmaPacket* packet, int ProcessedBdCount)
{
	int FreeBdCount;
	int Status;
	/* Free all processed TX BDs for future transmission */
	Status = XAxiDma_BdRingFree(
			packet->channel->TxRingPtr, ProcessedBdCount, packet->TxBdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to free %d tx BDs %d\r",
				ProcessedBdCount, Status);
		return XST_FAILURE;
	}
	/* Free all processed RX BDs for future transmission */
	Status = XAxiDma_BdRingFree(
			packet->channel->RxRingPtr, ProcessedBdCount, packet->RxBdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to free %d rx BDs %d\r",
				ProcessedBdCount, Status);
		return XST_FAILURE;
	}

	/* Return processed BDs to RX channel so we are ready to receive new
	 * packets:
	 *    - Allocate all free RX BDs
	 *    - Pass the BDs to RX channel
	 */
	FreeBdCount = XAxiDma_BdRingGetFreeCnt(packet->channel->RxRingPtr);
	Status = XAxiDma_BdRingAlloc(
			packet->channel->TxRingPtr, FreeBdCount, &(packet->TxBdPtr));
	if (Status != XST_SUCCESS) {
		xil_printf("bd alloc failed\r\n");
		return XST_FAILURE;
	}
	Status = XAxiDma_BdRingToHw(
			packet->channel->RxRingPtr, FreeBdCount, packet->RxBdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Submit %d rx BDs failed %d\r\n", FreeBdCount, Status);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
