#ifndef PRP_REDIRECT_H__YN
#define PRO_REDIRECT_H__YN

#include "icSPI.h"
#include "DrvFMC.h"

int32_t FMC_Erase(uint32_t u32addr);
int32_t FMC_ReadBuffer(uint32_t u32addr, uint32_t * u32data,uint32_t u32buffersize);
int32_t FMC_WriteBuffer(uint32_t u32addr, uint32_t *u32data,uint32_t u32buffersize);

#endif
