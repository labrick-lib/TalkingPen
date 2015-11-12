#include "DrvSPI.h"
#include "DrvPDMA.h"
#include "icSPI.h"


/*
 * Instruction set
 */
#define IS_WRITE_ENABLE				((unsigned char)0x06)
#define IS_WRITE_DISABLE			((unsigned char)0x04)
#define IS_READ_STATUS_REGISTER		((unsigned char)0x05)
#define IS_WRITE_STATUS_REGISTER	((unsigned char)0x01)
#define IS_READ_DATA				((unsigned char)0x03)
#define IS_PAGE_PROGRAM				((unsigned char)0x02)
#define IS_CHIP_ERASE				((unsigned char)0xC7)
#define IS_BLOCK_ERASE				((unsigned char)0xD8)
#define IS_READ_ID					((unsigned char)0x9F)
#define IS_Power_Down				((unsigned char)0xB9)
#define IS_Release_DP				((unsigned char)0xAB)


/*
 * Status register definition
 */
#define SR_BIT7_SRP		0x80	// status register protect bit
#define SR_BIT6			0x40	// reserved
#define SR_BIT5_TB		0x20	// top/bottom write protect
#define SR_BIT4_BP2		0x10	// block protect bits
#define SR_BIT3_BP1		0x08	// block protect bits
#define SR_BIT2_BP0		0x04	// block protect bits
#define SR_BIT1_WEL		0x02	// write enable latch
#define SR_BIT0_BUSY	0x01	// erase or write in progress


#define __SPI0_CS_ACTIVE	SPI0->SSR.SSR = 0x01;
#define __SPI0_CS_INACTIVE	SPI0->SSR.SSR = 0x00;


#define SPI0_SendByte(cSend) \
{\
	SPI0->TX[0] = cSend & 0x000000FF;\
	SPI0->CNTRL.GO_BUSY	= 1;\
	while ( SPI0->CNTRL.GO_BUSY )\
	{\
	}\
}

#define SPI0_ReadByte(cRead) \
{\
	SPI0->CNTRL.GO_BUSY	= 1;\
	while ( SPI0->CNTRL.GO_BUSY )\
	{\
	}\
\
	(cRead) = (unsigned char)(SPI0->RX[0]);\
}


/*-----------------------------------------------------------------------
  data bit latchs on the rising edge of clock
  data bit drives on negative edge of clock
  only one byte transmits/receive in a transfer
  only one transmit/receive will be executed in a transfer
  MSB first
  sclk idle low
  disable SPI interrupt
 -----------------------------------------------------------------------*/
void SPI0_Init(void)
{
	unsigned int i;

	/*
	 * initialize
	 */
	DrvSPI_Open(eDRVSPI_PORT0, eDRVSPI_MASTER, eDRVSPI_TYPE1, 8);				/* configure SPI0 as a master, 32-bit transaction */
	DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT0, eDRVSPI_ACTIVE_LOW_FALLING);/* set the active level of slave select. */
	DrvSPI_DisableAutoCS(eDRVSPI_PORT0);										/* disable the automatic slave select function of SS0. */
	DrvSPI_SetClock(eDRVSPI_PORT0, 2000000, 0);									/* SPI clock rate 2MHz */
	SPI0->DIVIDER.DIVIDER = 0;
	DrvSPI_SetEndian(eDRVSPI_PORT0, eDRVSPI_MSB_FIRST);							/* transfer data with MSB first */
	//DrvSPI_SetByteEndian(eDRVSPI_PORT0,  eDRVSPI_LSB_FIRST);					/* set byte endian */
	DrvSPI_DisableInt(eDRVSPI_PORT0);
	//DrvSPI_EnableInt(eDRVSPI_PORT0, NULL, NULL);

	/*
	 * wakeup from sleep mode
	 */
	__SPI0_CS_ACTIVE	/* activate */
	SPI0_SendByte(IS_Release_DP);	
	__SPI0_CS_INACTIVE	/* deactivate */

	/* delay for a while for waiting SPIFlash stable */
	for ( i = 0; i < 0x100; i++ );
}

void SPI0_Uninit(void)
{
	DrvSPI_Close(eDRVSPI_PORT0);
}

void SPI0_ReadData(unsigned int u32StartAddr, void* pBuf, unsigned int u32Bytes)
{
	unsigned int i;

	__SPI0_CS_ACTIVE	/* activate */

	SPI0_SendByte(IS_READ_DATA);						/* Write Read Array command */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 16));	/* Write Sector address = AD1 = A23 ~ A16 */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 8));	/* Write Sector address = AD2 = A15 ~ A8 */
	SPI0_SendByte((unsigned char)(u32StartAddr));		/* Write Sector address = AD3 = A7 ~ A0 */

	/* Read data from SPI Flash */
	for ( i = 0; i < u32Bytes; i++ )
	{
		SPI0_ReadByte(((unsigned char*)pBuf)[i]);
	}

	__SPI0_CS_INACTIVE	/* deactivate */
}


//#if LOG_BY_SPIFlash

// Read status register
__inline unsigned char SPI0_ReadStatusRegister()
{
	unsigned char uSR;

	__SPI0_CS_ACTIVE

	SPI0_SendByte(IS_READ_STATUS_REGISTER);
	SPI0_ReadByte(uSR);

	__SPI0_CS_INACTIVE

	return uSR;
}

__inline void SPI0_Write_Enable()
{
	__SPI0_CS_ACTIVE

	SPI0_SendByte(IS_WRITE_ENABLE);

	__SPI0_CS_INACTIVE
}

__inline void SPI0_Write_Disable()
{
	__SPI0_CS_ACTIVE

	SPI0_SendByte(IS_WRITE_DISABLE);

	__SPI0_CS_INACTIVE
}

// Wait write operation complete
__inline void WaitWriteComplete(void)
{
	// Read status register
	__SPI0_CS_ACTIVE

	SPI0_SendByte(IS_READ_STATUS_REGISTER);

	while ( 1 )
	{
		unsigned char uSR;

		SPI0_ReadByte(uSR);
		if ( SR_BIT0_BUSY != (uSR & SR_BIT0_BUSY) )
		{
			break;
		}
		WDT->WTCR.WTR =1 ;
		WDT->WTCR.WTR =1 ;
		//WDT_Clear();
		//WDT_Clear();
	}

	__SPI0_CS_INACTIVE
}

// Write status register
__inline void SPI0_WriteStatusRegister(unsigned char uSR)
{
	// Enable writes to status register
	SPI0_Write_Enable();

	// Write status register data byte
	__SPI0_CS_ACTIVE

	SPI0_SendByte(IS_WRITE_STATUS_REGISTER);
	SPI0_SendByte(uSR);

	__SPI0_CS_INACTIVE

	// WEL bit is automatically reset after Power-up and upon completion of the 
	// Write status register, Page program, Sector Erase, Block erase and Chip erase instruction.
	// SPI0_Write_Disable();;

	// Wait for complete	
	WaitWriteComplete();
}

__inline int IsBusy()
{
	return (SR_BIT0_BUSY == (SPI0_ReadStatusRegister() & SR_BIT0_BUSY)) ? 1: 0;
}

__inline int IsWriteEnabled()
{
	unsigned char uSR = SPI0_ReadStatusRegister();

	if ( SR_BIT1_WEL == (uSR & SR_BIT1_WEL) )
	{
		return 1;
	}

	return 0;
}

__inline int IsProtected()
{
	unsigned char uSR = SPI0_ReadStatusRegister();

	if ( (0 == (uSR & SR_BIT4_BP2)) &&
		 (0 == (uSR & SR_BIT3_BP1)) &&
		 (0 == (uSR & SR_BIT2_BP0)) )
	{
		return 0;
	}

	return 1;
}

__inline int IsWriteSRPEnabled()
{
	unsigned char uSR = SPI0_ReadStatusRegister();

	if ( SR_BIT7_SRP == (SR_BIT7_SRP & uSR) )
	{
		return 1;
	}

	return 0;
}

void SPI0_ClearProtect(void)
{
	unsigned char uSR = 0;

	uSR |= SR_BIT7_SRP;
	uSR &= ~SR_BIT4_BP2;
	uSR &= ~SR_BIT3_BP1;
	uSR &= ~SR_BIT2_BP0;

	SPI0_WriteStatusRegister(uSR);
}

void SPI0_EraseChip(void)
{
	SPI0_ClearProtect();

	SPI0_Write_Enable();

	__SPI0_CS_ACTIVE				/* activate */
	SPI0_SendByte(IS_CHIP_ERASE);	/* send instruction code */
	__SPI0_CS_INACTIVE				/* deactivate */

	// WEL bit is automatically reset after Power-up and upon completion of the 
	// Write status register, Page program, Sector Erase, Block erase and Chip erase instruction.
	// SPI0_Write_Disable();;

	// wait for complete	
	WaitWriteComplete();
}

int SPI0_EraseBlock(unsigned int u32BeginBlockIndex, unsigned int u32EndBlockIndex)
{
	unsigned int i;

	if ( u32BeginBlockIndex > u32EndBlockIndex )
	{
		return 0;
	}

	SPI0_ClearProtect();

	for ( i = u32BeginBlockIndex; i <= u32EndBlockIndex; i++ )
	{
 		unsigned int u32StartAddr = i * 0x10000;	/* 64Kbytes per block */

		SPI0_Write_Enable();

		__SPI0_CS_ACTIVE				/* activate */
	
		SPI0_SendByte(IS_BLOCK_ERASE);	/* send instruction code */
	
		SPI0_SendByte((unsigned char)(u32StartAddr >> 16));	/* Write Sector address = AD1 = A23 ~ A16 */
		SPI0_SendByte((unsigned char)(u32StartAddr >> 8));	/* Write Sector address = AD2 = A15 ~ A8 */
		SPI0_SendByte((unsigned char)(u32StartAddr));		/* Write Sector address = AD3 = A7 ~ A0 */
	
		__SPI0_CS_INACTIVE				/* deactivate */

		// wait for complete	
		WaitWriteComplete();		
	}

	return 1;
}

void SPI0_WriteData(void* pBuf, unsigned int u32StartAddr, unsigned int u32Bytes)
{
	unsigned int i;

	SPI0_Write_Enable();

	__SPI0_CS_ACTIVE	/* activate */

	SPI0_SendByte(IS_PAGE_PROGRAM);						/* Write page program command */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 16));	/* Write Sector address = AD1 = A23 ~ A16 */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 8));	/* Write Sector address = AD2 = A15 ~ A8 */
	SPI0_SendByte((unsigned char)(u32StartAddr));		/* Write Sector address = AD3 = A7 ~ A0 */

	/* Write data to SPI Flash */
	for ( i = 0; i < u32Bytes; i++ )
	{
		SPI0_SendByte(((unsigned char*)pBuf)[i]);
	}

	__SPI0_CS_INACTIVE	/* deactivate */

	// WEL bit is automatically reset after Power-up and upon completion of the 
	// Write status register, Page program, Sector Erase, Block erase and Chip erase instruction.
	// SPI0_Write_Disable();;

	// Wait for complete	
	WaitWriteComplete();
}

void PDMA2_Callback(uint32_t status)
{
	__SPI0_CS_INACTIVE
}


void InitialPDMA_SPI(void *pBuf, unsigned int u32Bytes)
{
	STR_PDMA_T sPDMA;
	
	// PDMA Init 
	DrvPDMA_Init();

	// CH2 ADC RX Setting 
	sPDMA.sSrcAddr.u32Addr          = (uint32_t)pBuf; 
	sPDMA.sDestAddr.u32Addr         = (uint32_t)&SPI0->TX[0];
	sPDMA.u8Mode                    = eDRVPDMA_MODE_MEM2APB;
	sPDMA.u8TransWidth              = eDRVPDMA_WIDTH_8BITS;
	sPDMA.sSrcAddr.eAddrDirection   = eDRVPDMA_DIRECTION_INCREMENTED; 
	sPDMA.sDestAddr.eAddrDirection  = eDRVPDMA_DIRECTION_FIXED;   
	sPDMA.i32ByteCnt				= u32Bytes;
	DrvPDMA_Open(eDRVPDMA_CHANNEL_2, &sPDMA);
	
	// PDMA Setting 
	DrvPDMA_SetCHForAPBDevice(
	    eDRVPDMA_CHANNEL_2, 
	    eDRVPDMA_SPI0,
	    eDRVPDMA_WRITE_APB    
	);
	
	
	// Enable INT 
	DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_2, eDRVPDMA_BLKD );
	    
	// Install Callback function    
	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_2, eDRVPDMA_BLKD, (PFN_DRVPDMA_CALLBACK) PDMA2_Callback ); 
	
	// start ADC PDMA transfer
//	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);
}


void SPI0_WriteData_PDMA(void* pBuf, unsigned int u32StartAddr)
{
	unsigned int i;

	SPI0_Write_Enable();

	__SPI0_CS_ACTIVE	/* activate */

	SPI0_SendByte(IS_PAGE_PROGRAM);						/* Write page program command */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 16));	/* Write Sector address = AD1 = A23 ~ A16 */
	SPI0_SendByte((unsigned char)(u32StartAddr >> 8));	/* Write Sector address = AD2 = A15 ~ A8 */
	SPI0_SendByte((unsigned char)(u32StartAddr));		/* Write Sector address = AD3 = A7 ~ A0 */

	PDMA->channel[eDRVPDMA_CHANNEL_2].SAR = (uint32_t)pBuf;

	DrvSPI_StartPDMA(eDRVSPI_PORT0, eDRVSPI_TX_DMA, TRUE);

	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);

//	/* Write data to SPI Flash */
//	for ( i = 0; i < u32Bytes; i++ )
//	{
//		SPI0_SendByte(((unsigned char*)pBuf)[i]);
//	}
//
//	__SPI0_CS_INACTIVE	/* deactivate */
//
//	// WEL bit is automatically reset after Power-up and upon completion of the 
//	// Write status register, Page program, Sector Erase, Block erase and Chip erase instruction.
//	// SPI0_Write_Disable();;
//
//	// Wait for complete	
//	WaitWriteComplete();
}

//#endif // !LOG_BY_SPIFlash
