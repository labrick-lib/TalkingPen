
#include "ISD9xx.h"
#include "DrvGPIO.h"

void Uart_Init(void)
{
	
	/* Set UART Pin */
	//DrvGPIO_InitFunction(FUNC_UART0);
	SYS->GPA_ALT.GPA8	=1;		// UART_TX: (0x5000_0000 + 14<<2)[31:0]= 0x5000_0038[17:16] 
//	SYS->GPA_ALT.GPA9 	=1;		// UART_RX: (0x5000_0000 + 14<<2)[31:0]= 0x5000_0038[19:18]

    //DrvUART_Open(UART_PORT0,&sParam);
	SYS->IPRSTC2.UART0_RST = 1;
	SYS->IPRSTC2.UART0_RST = 0;
	SYSCLK->CLKDIV.UART_N = 0;
	SYSCLK->APBCLK.UART0_EN = 1;

	*(uint32_t *)&UART0->FCR	= 0x06;	//reset RX,TX
	*(uint8_t *)&UART0->LCR = 3;		//8N1
	*(uint32_t *)&UART0->BAUD = 24;		//48000000/(16(24+2)) = 115384 
				
}


void Uart_UnInit(void)
{
	//DrvUART_Close(UART_PORT0);
	while(!UART0->FSR.TX_EMPTY);
		SYSCLK->APBCLK.UART0_EN = 0;
}

