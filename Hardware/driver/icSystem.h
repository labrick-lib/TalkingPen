#ifndef __HARD_WARE_H__N572_iFLYTEK
#define __HARD_WARE_H__N572_iFLYTEK
#include "stat.h"

//typedef unsigned char BOOL;

void HW_Init(void);
void HW_Term(void);
void HW_EnterPowerDown(PEN_STATE PenState);

void HW_Init(void);
void HW_Term(void);

uint16_t HW_PowerCheck(void);
uint8_t PowerCheck(void);

#endif	/* !__HARD_WARE_H__N572_iFLYTEK */
