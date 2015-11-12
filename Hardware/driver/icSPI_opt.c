
#include "hw.h"

/*
 * Instruction set
 */
#define IS_WRITE_ENABLE				((unsigned int)0x06)
#define IS_WRITE_DISABLE			((unsigned int)0x04)
#define IS_READ_STATUS_REGISTER		((unsigned int)0x05)
#define IS_WRITE_STATUS_REGISTER	((unsigned int)0x01)
#define IS_READ_DATA				((unsigned int)0x03)
#define IS_PAGE_PROGRAM				((unsigned int)0x02)
#define IS_BLOCK_ERASE				((unsigned int)0xD8)
#define IS_SECTOR_ERASE				((unsigned int)0x20)
#define IS_CHIP_ERASE				((unsigned int)0xC7)
#define IS_READ_ID					((unsigned int)0x9F)
#define IS_Power_Down				((unsigned int)0xB9)
#define IS_Release_DP				((unsigned int)0xAB)

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


//#define __SPI1_CS_ACTIVE	DrvSPI_SetCS(eDRVSPI_PORT0, eDRVSPI_SS0);
//#define __SPI1_CS_INACTIVE	DrvSPI_ClrCS(eDRVSPI_PORT0, eDRVSPI_SS0);
#define __SPI1_CS_ACTIVE	SPI0->SSR.SSR = eDRVSPI_SS0;
#define __SPI1_CS_INACTIVE	SPI0->SSR.SSR = eDRVSPI_NONE;

#define PAGE_SIZE 256

extern volatile uint32_t spi_addr_offset; 

static __inline void SPI1_SendByte(unsigned char cSend)
{
	uint32_t i = cSend;

	SPI0->CNTRL.TX_BIT_LEN = 8; //DrvSPI_SetBitLength(eDRVSPI_PORT0, 8);
	//DrvSPI_SingleWrite(eDRVSPI_PORT0, &i);
	SPI0->TX[0] = i;
	
	SPI0->CNTRL.GO_BUSY = 1;
	
	//while ( DrvSPI_Busy(eDRVSPI_PORT0) ) {}
	while ( SPI0->CNTRL.GO_BUSY ) {}
	
}

static __inline unsigned char SPI1_ReadByte(void)
{
	uint32_t i = 0;
	SPI0->CNTRL.TX_BIT_LEN = 8; //DrvSPI_SetBitLength(eDRVSPI_PORT0, 8);	
	SPI0->TX[0] = 0; //防止往SPI总线上写数据。
	//DrvSPI_SetGo(eDRVSPI_PORT0);
	SPI0->CNTRL.GO_BUSY = 1;
	
	//while ( DrvSPI_Busy(eDRVSPI_PORT0) ) {}
	while ( SPI0->CNTRL.GO_BUSY ) {}
	
	//DrvSPI_SingleRead(eDRVSPI_PORT0, &i);
	i = SPI0->RX[0];
	
	return (unsigned char) i;
}

// 一次发送32位
static __inline void SPI1_SendInt(uint32_t cSend)
{
	SPI0->CNTRL.TX_BIT_LEN = 0; // //DrvSPI_SetBitLength(eDRVSPI_PORT0, 32);
	
	//DrvSPI_SingleWrite(eDRVSPI_PORT0, &i);
	SPI0->TX[0] = cSend;
	
	SPI0->CNTRL.GO_BUSY = 1;
	
	//while ( DrvSPI_Busy(eDRVSPI_PORT0) ) {}
	while ( SPI0->CNTRL.GO_BUSY ) {}
	
	SPI0->CNTRL.TX_BIT_LEN = 8; // //DrvSPI_SetBitLength(eDRVSPI_PORT0, 8);
}

void SPI_Init(void)
{
	//DrvSPI_Open(eDRVSPI_PORT0, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
	SYSCLK->APBCLK.SPI0_EN = 1;
	SYS->IPRSTC2.SPI0_RST = 1;
	SYS->IPRSTC2.SPI0_RST = 0;
	SYS->GPA_ALT.GPA0 = 1; // MOSI0
	SYS->GPA_ALT.GPA1 = 1; // SCLK
	SYS->GPA_ALT.GPA2 = 1; // SSB0
	SYS->GPA_ALT.GPA3 = 1; // MISO0
	/* Check busy*/
	while(SPI0->CNTRL.GO_BUSY) {}
	
	SPI0->CNTRL.TX_BIT_LEN = 8; // //DrvSPI_SetBitLength(eDRVSPI_PORT0, 8);
	SPI0->JS.JS = 0;
	
	// DrvSPI_DisableAutoCS(eDRVSPI_PORT0);
	SPI0->SSR.SSR = eDRVSPI_NONE;
	SPI0->SSR.ASS = 0;
	
	//eDRVSPI_TYPE1
	SPI0->CNTRL.CLKP = 0;
	SPI0->CNTRL.TX_NEG = 1;
	SPI0->CNTRL.RX_NEG = 0;
	
	// set SPI clk
	SPI0->DIVIDER.DIVIDER = 2;
	SPI0->DIVIDER.DIVIDER2 = 1;
	
	SPI0->CNTRL.LSB = 0; //DrvSPI_SetEndian(eDRVSPI_PORT0, eDRVSPI_MSB_FIRST);
	
	SPI0->CNTRL.SLEEP = 0; //DrvSPI_SetByteSleep(eDRVSPI_PORT0, FALSE);
	
	SPI0->CNTRL.BYTE_ENDIAN = 0; //DrvSPI_SetByteEndian(eDRVSPI_PORT0, FALSE);
	
	SPI0->SSR.SS_LVL = eDRVSPI_ACTIVE_LOW_FALLING; //DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT0, eDRVSPI_ACTIVE_LOW_FALLING);
	//DrvSPI_BurstTransfer(eDRVSPI_PORT0, 1, 2);
	SPI0->CNTRL.TX_NUM = 1 - 1;
	SPI0->CNTRL.SLEEP = 2 - 2;

	/*
	 * Release from power down
	 */
	__SPI1_CS_ACTIVE
	SPI1_SendByte(IS_Release_DP);
	__SPI1_CS_INACTIVE

	// Delay for a while (3us) for waiting SPIFlash stable
	//HW_WaitTimeout(3);
	
	/*
	{
		// 测试数据读取
		unsigned char buff[16];
		FMD_ReadData(buff, 0x340, 4);
		FMD_ReadData(buff, 0x340, 4);
		FMD_ReadData(buff, 0x340, 4);
	}
	*/
}
void GPIOToSpi(void)
{
	SYS->GPA_ALT.GPA0 = 1; // MOSI0
	SYS->GPA_ALT.GPA1 = 1; // SCLK
//	SYS->GPA_ALT.GPA2 = 1; // SSB0
//	SYS->GPA_ALT.GPA3 = 1; // MISO0
}

void SPI_Term(void)
{
	/*
	 * SPIFlash power down mode
	 */
	__SPI1_CS_ACTIVE
	SPI1_SendByte(IS_Power_Down);
	__SPI1_CS_INACTIVE

	//DrvSPI_Close(eDRVSPI_PORT0);
	/* Wait SPIMS Busy */
	while(SPI0->CNTRL.GO_BUSY) {}
	SYS->IPRSTC2.SPI0_RST = 1;
	SYS->IPRSTC2.SPI0_RST = 0;
	SYSCLK->APBCLK.SPI0_EN = 0;
}

static void PDMA2_Callback(uint32_t intflag)
{
	__SPI1_CS_INACTIVE	/* deactivate */
	
	DrvPDMA_ClearInt(eDRVPDMA_CHANNEL_2, eDRVPDMA_BLKD_FLAG);
}

void SPI_InitDMA(void *pBuf, unsigned int u32Bytes)
{
	STR_PDMA_T sPDMA;

	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_2, eDRVPDMA_SPI0, eDRVPDMA_WRITE_APB);
	
	sPDMA.sSrcAddr.u32Addr = (uint32_t)(pBuf);
    sPDMA.sDestAddr.u32Addr = (uint32_t)&(SPI0->TX[0]);
    sPDMA.u8TransWidth = eDRVPDMA_WIDTH_8BITS;
	sPDMA.u8Mode = eDRVPDMA_MODE_MEM2APB;
	sPDMA.sSrcAddr.eAddrDirection = eDRVPDMA_DIRECTION_INCREMENTED;
	sPDMA.sDestAddr.eAddrDirection = eDRVPDMA_DIRECTION_FIXED;
	sPDMA.u8WrapBcr = eDRVPDMA_WRA_NO_INT;
	sPDMA.i32ByteCnt = u32Bytes;
	DrvPDMA_Open(eDRVPDMA_CHANNEL_2, &sPDMA);

	/* Enable INT */
	outpw(&PDMA->channel[eDRVPDMA_CHANNEL_2].IER, inpw(&PDMA->channel[eDRVPDMA_CHANNEL_2].IER) | eDRVPDMA_BLKD); 
	NVIC_EnableIRQ(PDMA_IRQn);  

	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_2,eDRVPDMA_BLKD, PDMA2_Callback );

	return;
}

/* 这是一个经过优化的SPI读取函数。 */
void FMD_ReadData(void* pBuf, unsigned int u32StartAddr, unsigned int u32Bytes)
{
	unsigned int i;

	__SPI1_CS_ACTIVE	/* activate */
	
	if (u32Bytes == 2) {
		// 如果只是读两个字节，我们需要优化一下
		
		SPI0->CNTRL.TX_BIT_LEN = 0; //DrvSPI_SetBitLength(eDRVSPI_PORT0, 32);
		
		// 先发送读命令和地址，然后等待。SPI1_SendInt((IS_READ_DATA << 24) | u32StartAddr);
		//DrvSPI_SingleWrite(eDRVSPI_PORT0, &cSend);
		SPI0->TX[0] = (IS_READ_DATA << 24) | u32StartAddr;
		SPI0->CNTRL.GO_BUSY = 1;
		
		// 等待读命令和地址被成功接收
		//while ( DrvSPI_Busy(eDRVSPI_PORT0) ) {}
		while ( SPI0->CNTRL.GO_BUSY ) {}
		
		SPI0->CNTRL.TX_BIT_LEN = 16; //DrvSPI_SetBitLength(eDRVSPI_PORT0, 16);
		SPI0->CNTRL.BYTE_ENDIAN = 1; //DrvSPI_SetByteEndian(eDRVSPI_PORT0, TRUE);
		
		SPI0->TX[0] = 0; //防止往SPI总线上写数据。
		//DrvSPI_SetGo(eDRVSPI_PORT0);
		SPI0->CNTRL.GO_BUSY = 1;
		
		// 等待完成数据的接收。
		//while ( DrvSPI_Busy(eDRVSPI_PORT0) ) {}
		while ( SPI0->CNTRL.GO_BUSY ) {}
		
		i = SPI0->RX[0];
		
		SPI0->CNTRL.BYTE_ENDIAN = 0; //DrvSPI_SetByteEndian(eDRVSPI_PORT0, FALSE);
		SPI0->CNTRL.TX_BIT_LEN = 8; //DrvSPI_SetBitLength(eDRVSPI_PORT0, 8);
		
		*(unsigned short *)pBuf = (unsigned short) i;
		
	} else {
		
		SPI1_SendInt((IS_READ_DATA << 24) | u32StartAddr); /* read array command */
		/* Write data to SPI Flash */
		for ( i = 0; i < u32Bytes; i++ ) {
			((unsigned char *)pBuf)[i] = SPI1_ReadByte();
		}
	}
	
	__SPI1_CS_INACTIVE	/* deactivate */
}

#if ENABLE_WRITE_SPIFLASH

__inline void SPI1_Write_Enable()
{
	__SPI1_CS_ACTIVE

	SPI1_SendByte(IS_WRITE_ENABLE);

	__SPI1_CS_INACTIVE
}

#if 0
// Wait write operation complete
__inline void WaitWriteComplete(void)
{
	// Read status register
	__SPI1_CS_ACTIVE
	SPI1_SendByte(IS_READ_STATUS_REGISTER);

	while ( 1 )
	{
		unsigned char uSR = SPI1_ReadByte();
		if ( !(uSR & SR_BIT0_BUSY) ) {
			break;
		}
	}

	__SPI1_CS_INACTIVE
}
#endif
// Wait write operation complete
__inline void WaitWriteComplete(void)
{
    uint32_t u32TimeOut, i;
	u32TimeOut = 10;
__WaitWriteComplete_BUS_ERROR:
	i = 1000;
	__SPI1_CS_ACTIVE
	SPI1_SendByte(IS_READ_STATUS_REGISTER);
	//LOG(("/"));
	while ( 1 )
	{
		unsigned char uSR = SPI1_ReadByte();
		if ( !(uSR & SR_BIT0_BUSY) ) {
			break;
		}
        if ( u32TimeOut-- )
		{
			while ( i-- )
			{
				__SPI1_CS_INACTIVE
            }
			goto __WaitWriteComplete_BUS_ERROR;
        }
		//LOG(("-"));
	}
	__SPI1_CS_INACTIVE
}


BOOL SPI_CheckWriteBusy(void)
{
	unsigned char uSR;
	// Read status register
	__SPI1_CS_ACTIVE
	SPI1_SendByte(IS_READ_STATUS_REGISTER);

	uSR = SPI1_ReadByte();
	
	__SPI1_CS_INACTIVE
		
	return (uSR & SR_BIT0_BUSY);

}

void FMD_EraseBlock(unsigned int u32StartAddr)
{
	FMD_EraseBlock_NoWait(u32StartAddr);
	WaitWriteComplete();
}

void FMD_EraseBlock_NoWait_32K(unsigned int u32StartAddr)
{
	// 发送完块擦除指令后就走，不等待擦除完毕；
	WaitWriteComplete();
	SPI1_Write_Enable();
	
	__SPI1_CS_ACTIVE	/* activate */
//	SPI1_SendInt((IS_BLOCK_ERASE << 24) | u32StartAddr);
//	SPI1_SendInt((IS_SECTOR_ERASE << 24) | u32StartAddr);
	SPI1_SendInt((IS_SECTOR_ERASE << 24) | u32StartAddr);

	__SPI1_CS_INACTIVE	/* deactivate */

	//WaitWriteComplete();
}
void FMD_EraseBlock_32K(unsigned int u32StartAddr)
{
	FMD_EraseBlock_NoWait_32K(u32StartAddr);
	WaitWriteComplete();
}

void FMD_EraseBlock_NoWait(unsigned int u32StartAddr)
{
	// 发送完块擦除指令后就走，不等待擦除完毕；
	WaitWriteComplete();
	SPI1_Write_Enable();
	
	__SPI1_CS_ACTIVE	/* activate */
	SPI1_SendInt((IS_BLOCK_ERASE << 24) | u32StartAddr);
//	SPI1_SendInt((IS_SECTOR_ERASE << 24) | u32StartAddr);

	__SPI1_CS_INACTIVE	/* deactivate */

	//WaitWriteComplete();
}

void FMD_WriteData(void* pBuf, unsigned int u32StartAddr)
{
	unsigned int i;

//	WaitWriteComplete();
	SPI1_Write_Enable();

	__SPI1_CS_ACTIVE	/* activate */

	SPI1_SendInt((IS_PAGE_PROGRAM << 24) | u32StartAddr); /* Write page program command */
	
	/* Write data to SPI Flash */
	for ( i = 0; i < PAGE_SIZE; i++ )
	{
		SPI1_SendByte(((unsigned char *)pBuf)[i]);
	}
	
	__SPI1_CS_INACTIVE	/* deactivate */

	// WEL bit is automatically reset after Power-up and upon completion of the 
	// Write status register, Page program, Sector Erase, Block erase and Chip erase instruction.
	// SPI1_Write_Disable();;

	// Wait for complete	
	WaitWriteComplete();
}
void FMD_Write2Byte(void* pBuf, unsigned int u32StartAddr)
{
	unsigned int i;
	uint32_t MoreByte = u32StartAddr%PAGE_SIZE;
	
	SPI1_Write_Enable();

	__SPI1_CS_ACTIVE	/* activate */

	SPI1_SendInt((IS_PAGE_PROGRAM << 24) | (u32StartAddr - MoreByte)); /* Write page program command */
	
	/* Write data to SPI Flash */
	for ( i = 0; i < PAGE_SIZE; i++ )
	{
		if(i == MoreByte)
			SPI1_SendByte(((unsigned char *)pBuf)[0]);
		else if(i == (MoreByte+1))
			SPI1_SendByte(((unsigned char *)pBuf)[1]);
		else
			SPI1_SendByte(0xFF);
	}
	
	__SPI1_CS_INACTIVE	/* deactivate */

	WaitWriteComplete();
}

void FMD_WriteDataWithDMA_NoWait(void* pBuf, unsigned int u32StartAddr)
{

	WaitWriteComplete();
	SPI1_Write_Enable();

	__SPI1_CS_ACTIVE	/* activate */
	
	SPI1_SendInt((IS_PAGE_PROGRAM << 24) | u32StartAddr); /* Write page program command */
	
	SPI0->DMA.TX_DMA_GO = 1;

	PDMA->channel[eDRVPDMA_CHANNEL_2].SAR = (uint32_t)pBuf;

	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);
}

#endif // !ENABLE_WRITE_SPIFLASH
