#ifndef __SPI1_MASTER_H__N572_iFLYTEK
#define __SPI1_MASTER_H__N572_iFLYTEK

#include "hw.h"

#define ENABLE_WRITE_SPIFLASH	(1)


void SPI_Init(void);
void SPI_Term(void);
void SPI_InitDMA(void *pBuf, unsigned int u32Bytes);
BOOL SPI_CheckWriteBusy(void);
void FMD_ReadData(void* pBuf, unsigned int u32StartAddr, unsigned int u32Bytes);

void FMD_EraseBlock(unsigned int u32StartAddr);
void FMD_EraseBlock_NoWait(unsigned int u32StartAddr);
void FMD_WriteData(void* pBuf, unsigned int u32StartAddr);
void FMD_WriteDataWithDMA_NoWait(void* pBuf, unsigned int u32StartAddr);
void GPIOToSpi(void);
void FMD_EraseBlock_32K(unsigned int u32StartAddr);
void FMD_Write2Byte(void* pBuf, unsigned int u32StartAddr);

#endif	/* !__SPI1_MASTER_H__N572_iFLYTEK */
