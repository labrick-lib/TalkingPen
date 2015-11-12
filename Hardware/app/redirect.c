#include "redirect.h"

/*---------------------------------------------------------------------------------------------------------*/
/* InitialFMC                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void InitialFMC(void)
{
//	uint32_t i;

	UNLOCKREG();

	/* Enable ISP function */
	DrvFMC_EnableISP(1);

	DrvFMC_EnableConfigUpdate(TRUE);
	DrvFMC_Erase(CONFIG_BASE);
	DrvFMC_Write(CONFIG_BASE, 0xFFFFFFFE);
	DrvFMC_Write(CONFIG_BASE+4, DATAF_BASE_ADDR);
	DrvFMC_EnableConfigUpdate(FALSE);
//	for (i=DATAF_BASE_ADDR; i<DATAF_BASE_ADDR+FMC_SECTOR_SIZE*12; i+=FMC_SECTOR_SIZE)
//	{
//		DrvFMC_Erase(i);
//	}

	/* Disable ISP function */
//	DrvFMC_EnableISP(0);

	LOCKREG();
}
int32_t FMC_Erase(uint32_t u32addr)
{
	UNLOCKREG();
	DrvFMC_Erase(u32addr);
	LOCKREG();
	return 0;
}
int32_t FMC_ReadBuffer(uint32_t u32addr, uint32_t * u32data,uint32_t u32buffersize)
{
//	UNLOCKREG();
//	DrvFMC_ReadBuffer(u32addr, u32data, u32buffersize);
//	LOCKREG();
	uint32_t iTemp;
	for(iTemp = 0; iTemp < u32buffersize/4; iTemp++){
		*(u32data+iTemp) = *((uint32_t *)u32addr+iTemp);
	}
	return 0;
}
int32_t FMC_WriteBuffer(uint32_t u32addr, uint32_t *u32data,uint32_t u32buffersize)
{
	UNLOCKREG();
	DrvFMC_WriteBuffer(u32addr, u32data, u32buffersize);
	LOCKREG();
	return 0;
}
