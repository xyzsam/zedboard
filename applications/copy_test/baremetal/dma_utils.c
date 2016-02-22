#include "xaxidma.h"
#include "dma_utils.h"

u32 *Packet = (u32 *)TX_BUFFER_BASE;

int RxSetup(XAxiDma *AxiDma, struct DmaPacket *packet) {
  int Delay = 0;
  int Coalesce = 1;
  int Status;
  XAxiDma_Bd BdTemplate;
  XAxiDma_Bd *BdCurPtr;
  u32 BdCount;
  u32 FreeBdCount;
  u32 RxBufferPtr;
  int Index;

  xil_printf("Setting up RX channel\r\n");
  packet->channel->RxRingPtr = XAxiDma_GetRxRing(AxiDma);

  /* Disable all RX interrupts before RxBD space setup */
  XAxiDma_BdRingIntDisable(packet->channel->RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

  /* Set delay and coalescing */
  XAxiDma_BdRingSetCoalesce(packet->channel->RxRingPtr, Coalesce, Delay);

  /* Setup Rx BD space */
  BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                  RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

  Status = XAxiDma_BdRingCreate(packet->channel->RxRingPtr, RX_BD_SPACE_BASE,
                                RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                BdCount);
  ASSERT(Status, "RX create BD ring failed.");

  /*
   * Setup an all-zero BD as the template for the Rx channel.
   */
  XAxiDma_BdClear(&BdTemplate);

  Status = XAxiDma_BdRingClone(packet->channel->RxRingPtr, &BdTemplate);
  ASSERT(Status, "RX clone BD ring failed.");

  // Attach buffers to RxBD ring so we are ready to receive packets.

  FreeBdCount = XAxiDma_BdRingGetFreeCnt(packet->channel->RxRingPtr);

  Status = XAxiDma_BdRingAlloc(packet->channel->RxRingPtr, FreeBdCount,
                               &(packet->RxBdPtr));
  ASSERT(Status, "RX alloc BD failed.");

  BdCurPtr = packet->RxBdPtr;
  RxBufferPtr = (u32)packet->RxBuf;
  for (Index = 0; Index < FreeBdCount; Index++) {
    Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
    ASSERT(Status, "Set buffer address on BD failed.");

    Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
                                 packet->channel->RxRingPtr->MaxTransferLen);
    ASSERT(Status, "Set buffer length on BD failed.");

    // Receive BDs do not need to set anything for the control
    // The hardware will set the SOF/EOF bits per stream status
    XAxiDma_BdSetCtrl(BdCurPtr, 0);
    XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);

    RxBufferPtr += MAX_PKT_LEN;
    BdCurPtr = XAxiDma_BdRingNext(packet->channel->RxRingPtr, BdCurPtr);
  }

  Status = XAxiDma_BdRingToHw(packet->channel->RxRingPtr, FreeBdCount,
                              packet->RxBdPtr);
  ASSERT(Status, "Rx submit to HW failed.");

  /* Start RX DMA channel */
  Status = XAxiDma_BdRingStart(packet->channel->RxRingPtr);
  ASSERT(Status, "Rx ring start failed.");

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function sets up the TX channel of a DMA engine to be ready for packet
 * transmission
 *
 * @param  AxiDmaInstPtr is the instance pointer to the DMA engine.
 *
 * @return  XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
 *
 * @note    None.
 *
 ******************************************************************************/
int TxSetup(XAxiDma *AxiDma, struct DmaChannel *channel) {
  XAxiDma_Bd BdTemplate;
  int Delay = 0;
  int Coalesce = 1;
  int Status;
  u32 BdCount;

  xil_printf("Setting up TX channel.\r\n");
  channel->TxRingPtr = XAxiDma_GetTxRing(AxiDma);

  /* Disable all TX interrupts before TxBD space setup */
  XAxiDma_BdRingIntDisable(channel->TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

  /* Set TX delay and coalesce */
  XAxiDma_BdRingSetCoalesce(channel->TxRingPtr, Coalesce, Delay);

  /* Setup TxBD space  */
  BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                  TX_BD_SPACE_HIGH - TX_BD_SPACE_BASE + 1);

  Status = XAxiDma_BdRingCreate(channel->TxRingPtr, TX_BD_SPACE_BASE,
                                TX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                BdCount);
  ASSERT(Status, "TX create BD ring failed.");

  /*
   * We create an all-zero BD as the template.
   */
  XAxiDma_BdClear(&BdTemplate);

  Status = XAxiDma_BdRingClone(channel->TxRingPtr, &BdTemplate);
  ASSERT(Status, "TX clone BD ring failed.");

  /* Start the TX channel */
  Status = XAxiDma_BdRingStart(channel->TxRingPtr);
  ASSERT(Status, "TX BD ring start failed.");

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function prepares a buffer descriptor for one packet to be sent
 * non-blockingly through the DMA engine.
 *
 * @param  AxiDmaInstPtr points to the DMA engine instance
 *
 * @return  - XST_SUCCESS if the BD is formed successfully.
 *    - XST_FAILURE otherwise.
 *
 * @note     None.
 *
 ******************************************************************************/
int PreparePacket(struct DmaPacket *packet) {
  int Status;
  int i;
  u32 TxBufferPtr;
  XAxiDma_Bd *BdCurPtr;
  // TODO: Support lengths that are not a multiple of the packet length.
  int num_bds = packet->TxNumBytes / MAX_PKT_LEN;
  int mask;

  /* Allocate a BD */
  Status = XAxiDma_BdRingAlloc(packet->channel->TxRingPtr, num_bds,
                               &(packet->TxBdPtr));
  ASSERT(Status, "TX alloc BD failed.");

  packet->NumBds = num_bds;
  TxBufferPtr = (u32)packet->TxBuf;
  BdCurPtr = packet->TxBdPtr;
  for (i = 0; i < num_bds; i++) {
    mask = 0;
    /* Set up the BD using the information of the packet to transmit */
    Status = XAxiDma_BdSetBufAddr(BdCurPtr, TxBufferPtr);
    ASSERT(Status, "Set TX buf address failed.");

    Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
                                 packet->channel->TxRingPtr->MaxTransferLen);
    ASSERT(Status, "Set TX buf length failed.");
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
  Status = XAxiDma_BdSetAppWord(packet->TxBdPtr, XAXIDMA_LAST_APPWORD,
                                packet->TxNumBytes);

  /* If Set app length failed, it is not fatal
   */
  ASSERT(Status, "Set app word failed.\r\n");
#endif

  return XST_SUCCESS;
}

int SendPacket(struct DmaPacket *packet) {
  int Status;

  /* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
   * is enabled
   */
  Xil_DCacheFlushRange((u32)packet->TxBuf, packet->TxNumBytes);
  /* Critical if using anything but ACP! You must invalidate the receive buffer
   * address range. */
  Xil_DCacheInvalidateRange((u32)packet->RxBuf, packet->TxNumBytes);

  Status = XAxiDma_BdRingToHw(packet->channel->RxRingPtr, packet->NumBds,
      packet->RxBdPtr);
  ASSERT(Status, "RX submit to hw failed.");

  /* Give the BD to DMA to kick off the transmission. */
  Status = XAxiDma_BdRingToHw(packet->channel->TxRingPtr, packet->NumBds,
                              packet->TxBdPtr);
  ASSERT(Status, "TX submit to hw failed.");
  return XST_SUCCESS;
}

int WaitForCompletion(struct DmaPacket *packet) {
  int timeout = 500;
  int status = 0;
  int ProcessedBdCount = 0;
  // TODO: We need to check for the right number of BDs.
  while ((ProcessedBdCount =
              XAxiDma_BdRingFromHw(packet->channel->TxRingPtr, XAXIDMA_ALL_BDS,
                                   &(packet->TxBdPtr))) != packet->NumBds && timeout > 0) {
    timeout--;
  }

  if (timeout == 0) {
    xil_printf("TX failed!\r\n");
    return -1;
  } else {
    xil_printf("TX complete.\r\n");
  }
  xil_printf("TX processed %d BDs\r\n", ProcessedBdCount);

  status = XAxiDma_BdRingFree(packet->channel->TxRingPtr, ProcessedBdCount,
                              packet->TxBdPtr);
  CHECK(status, "freeing tx BDs failed.\r\n");

  timeout = 5000;
  /* Wait until the data has been received by the Rx channel */
  while ((ProcessedBdCount =
              XAxiDma_BdRingFromHw(packet->channel->RxRingPtr, XAXIDMA_ALL_BDS,
                                   &(packet->RxBdPtr))) != packet->NumBds && timeout > 0) {
    timeout--;
  }
  if (timeout == 0) {
    xil_printf("RX failed!\r\n");
  } else {
    xil_printf("RX complete with timeout remaining %d.\r\n", timeout);
  }
  xil_printf("RX processed %d BDs\r\n", ProcessedBdCount);
  /* Free all processed RX BDs for future transmission */
  status = XAxiDma_BdRingFree(packet->channel->RxRingPtr, ProcessedBdCount,
                              packet->RxBdPtr);
  CHECK(status, "freeing rx BDs failed. \r\n");
  return XST_SUCCESS;
}
