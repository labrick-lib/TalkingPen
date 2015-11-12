#ifndef __SPI0_MASTER_H__N572_iFLYTEK
#define __SPI0_MASTER_H__N572_iFLYTEK


#ifdef __cplusplus
extern "C"{
#endif



/*
 * Function declarations
 */ 
void SPI0_Init(void);
void SPI0_Uninit(void);
void SPI0_ReadData(unsigned int u32StartAddr, void* pBuf, unsigned int u32Bytes);

//#if LOG_BY_SPIFlash
void SPI0_EraseChip(void);
void SPI0_WriteData(void* pBuf, unsigned int u32StartAddr, unsigned int u32Bytes);
int SPI0_EraseBlock(unsigned int u32BeginBlockIndex, unsigned int u32EndBlockIndex);/* 64Kbytes per block */
//#endif


#ifdef  __cplusplus
}
#endif


#endif	/* !__SPI0_MASTER_H__N572_iFLYTEK */
