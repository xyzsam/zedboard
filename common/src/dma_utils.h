/*
 * dma_utils.h
 *
 *  Created on: Feb 15, 2016
 *      Author: Sam
 */

#ifndef DMA_UTILS_H_
#define DMA_UTILS_H_

#include <stdbool.h>

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

// TODO: Update these address mappings.
#define MEM_BASE_ADDR		0x11000000

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define TX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00000FFF)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00001000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define MAX_PKT_LEN		0x8000

// TODO: Replace this with my own testing procedure.
#define TEST_RX_INVALID_VALUE 0xDC
#define TEST_TX_INVALID_VALUE 0xEF
#define TEST_RX_INVALID_32 0xDCDCDCDC
#define TEST_TX_INVALID_32 0xEFEFEFEF

#define CLOCK_FREQ 666666666

#define CHECK_STATUS_AND_QUIT(status, msg) \
	if (status != XST_SUCCESS) { \
		xil_printf(msg " failed, exiting\r\n."); \
	}

u32 *Packet;

/* All information needed to transmit a packet.
 * TODO: This abstraction is really misleading. Fix it later after things are working...
 */

struct DmaChannel {
  XAxiDma_BdRing* TxRingPtr;
	XAxiDma_BdRing *RxRingPtr;
};

struct DmaPacket {
  u8* TxBuf;
  u8* RxBuf;
  size_t TxNumBytes;
  size_t RxNumBytes;
  size_t NumTxBds;
  size_t NumRxBds;
  XAxiDma_Bd* TxBdPtr;
  XAxiDma_Bd* RxBdPtr;
  struct DmaChannel *channel;
};

int RxSetup(XAxiDma * AxiDmaInstPtr, struct DmaPacket * packet);
int TxSetup(XAxiDma * AxiDmaInstPtr, struct DmaChannel * channel);
int WaitForCompletion(struct DmaPacket* packet, int* ProcessedBdCount, bool interrupts);
int ReleaseBds(struct DmaPacket* packet, int ProcessedBdCount);
int PreparePacket(struct DmaPacket *packet);
int SendPacket(struct DmaPacket *packet);

#endif /* DMA_UTILS_H_ */
