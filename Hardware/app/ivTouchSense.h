/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright(c) 2012 Iflytek Technology Corp. All rights reserved.            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#ifndef	__IVTOUCHSENSE_H__
#define	__IVTOUCHSENSE_H__

#include <stdio.h>

#include "DrvRTC.h"
#include "DrvGPIO.h"
#include "DrvCapSense.h"
#include "DrvUART.h"
#include "DrvACMP.h"

/* Function Declared */
void TouchSenseInit(void);
uint32_t DrvTouchScanning(void);
void DrvTouchScanning_Test(void);

#endif
