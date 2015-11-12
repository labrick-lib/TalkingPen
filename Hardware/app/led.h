#ifndef __LED_H__YN
#define __LED_H__YN
#include "stdint.h"

#define GREEN_LED	0
#define RED_LED 	1

#define LED_ON			(1)
#define LED_OFF			(0)

#define GREEN_LED_ON	DrvGPIO_ClrBit(GPB,GREEN_LED)
#define GREEN_LED_OFF	DrvGPIO_SetBit(GPB,GREEN_LED)
#define RED_LED_ON		DrvGPIO_ClrBit(GPB,RED_LED)
#define RED_LED_OFF		DrvGPIO_SetBit(GPB,RED_LED)

void LedInit(void);
void LedFlashForCam(uint8_t bFlag);
void LedFlashForEsr(uint8_t bFlag);
void LedFlashForNoEsr(void);
void LedFlashForLowPwr(uint8_t bFlag);
void LedFlashForCharge(void);
void LedFlashForChargeover(void);
void AdapterDisconnect(void);
void LedFlashBeforeRec(uint8_t bFlag);
void LedUninit(void);
#endif
