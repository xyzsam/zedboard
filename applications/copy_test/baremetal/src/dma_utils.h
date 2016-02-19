/*
 * dma_utils.h
 *
 *  Created on: Feb 15, 2016
 *      Author: Sam
 */

#ifndef DMA_UTILS_H_
#define DMA_UTILS_H_

#define DMA_DEV_ID XPAR_AXIDMA_0_DEVICE_ID

#define MEM_BASE_ADDR 0x11000000

#define TX_BD_SPACE_BASE (MEM_BASE_ADDR)
#define TX_BD_SPACE_HIGH (MEM_BASE_ADDR + 0x00000FFF)
#define RX_BD_SPACE_BASE (MEM_BASE_ADDR + 0x00001000)
#define RX_BD_SPACE_HIGH (MEM_BASE_ADDR + 0x00001FFF)
#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH (MEM_BASE_ADDR + 0x004FFFFF)

#define MAX_PKT_LEN 0x20

#define TEST_TX_INVALID_VALUE 0xEF
#define TEST_RX_INVALID_VALUE 0xDC

#define CLOCK_FREQ 666666666

#define CHECK(status, msg)                                                     \
  if (status != XST_SUCCESS) {                                                 \
    xil_printf(msg "status = %d.\r\n", status);                                \
  }

#define ASSERT(status, msg)                                                    \
  if (status != XST_SUCCESS) {                                                 \
    xil_printf(msg "status = %d.\r\n", status);                                \
    return XST_FAILURE;                                                        \
  }

u32 *Packet;

// TODO: These abstractions are completely misleading and badly designed, but I
// don't have the need to redesign it right now.

struct DmaChannel {
  XAxiDma_BdRing *TxRingPtr;
  XAxiDma_BdRing *RxRingPtr;
};

struct DmaPacket {
  u8 *TxBuf;
  u8 *RxBuf;
  size_t TxNumBytes;
  size_t NumBds;
  XAxiDma_Bd *TxBdPtr;
  XAxiDma_Bd *RxBdPtr;
  struct DmaChannel *channel;
};

int RxSetup(XAxiDma *AxiDmaInstPtr, struct DmaPacket *packet);
int TxSetup(XAxiDma *AxiDmaInstPtr, struct DmaChannel *channel);
int WaitForCompletion(struct DmaPacket *packet, int *ProcessedBdCount);
int PreparePacket(struct DmaPacket *packet);
int SendPacket(struct DmaPacket *packet);

#endif /* DMA_UTILS_H_ */
