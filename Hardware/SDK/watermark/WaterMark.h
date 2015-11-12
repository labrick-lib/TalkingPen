/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef __WATERMARK_H__
#define __WATERMARK_H__

//#include "platform.h"
//#include "System/SysInfra.h"

#ifdef  __cplusplus
extern "C"
{
#endif

//#define MSVC		

//#define BIT20ms
//#define BIT26_7ms
//#define BIT33_3ms
//#define BIT40ms
//#define BIT46_7ms
//#define BIT53_3ms

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define	WATERMARK_MAJOR_NUM 1
#define	WATERMARK_MINOR_NUM 20
#define	WATERMARK_BUILD_NUM 004
#ifndef MSVC
#define WATERMARK_VERSION_NUM	_SYSINFRA_VERSION(WATERMARK_MAJOR_NUM, WATERMARK_MINOR_NUM, WATERMARK_BUILD_NUM)
#endif
#ifdef MSVC
#define WATERMARK_VERSION_NUM	100
#endif

#ifdef BIT20ms
#define BIN_DECIMATE_ORDER		3
#endif
#ifdef BIT26_7ms
#define BIN_DECIMATE_ORDER		4
#endif
#ifdef BIT33_3ms
#define BIN_DECIMATE_ORDER		5
#endif
#ifdef BIT40ms
//#define BIN_DECIMATE_ORDER		6	// pick 10 out of 60 frame
#endif
#ifdef BIT46_7ms
#define BIN_DECIMATE_ORDER		7
#endif
#ifdef BIT53_3ms
#define BIN_DECIMATE_ORDER		8
#endif


//#define WM_FRAMELEN	(16 * PCM_DECIMATE_ORDER) // count in original ADC sample. sample rate is 53.333K * 2 for filter 1-Z**(-3)
#define WM_FRAMELEN	(32) 
#define CORR_ANAL_WIN			(128)

//#define BP_FTR_ORDER_D2			(48)
//#define LP_FTR_ORDER_D2		(12)
//#define HP_FTR_ORDER		(24)
//#define HP_FTR_ORDER		(25) // including (1-Z**1)
#define HP_FTR_ORDER		(26) // including (1-Z**1)

// void WaterMarkParaSetting(struct WMParaStructTemp *, ECCOnOffMode, WMLeadToneNumTh,WMOverFreqBNumTh, WMToneScaleFacBy8, WMFreqBScaleFacBy8,DataBitNumPerPack,Reserv0 )
struct WMParaStructTemp	
{
short ECCOnOffMode	;	// ECC mode setting for ECC operation
			//  0: ECC disable, data is 16 bits.  1~3: reserved
			// 4: data is 11 bits(b15~b5), one error correct automatically, two error detected.
				// 5: three error detected, no correct.
short WMLeadToneNumTh	;	// 	Specify how many frame's "Lead Tone" energy over energy threshold would be recognized as a "Lead Tone" is coming. And code to be followed. Suggest in the range of 7 ~ 15. proposed value is 9.
short WMOverFreqBNumTh	; // 	reserved and set as 33.
short WMToneScaleFacBy8	; // Used to define "LeadTone" energy threshold. The energy threshold is define as.	WMToneScaleFacBy8 * Average_Tone_Energy / 8. Suggest in the range of 16~24. Proposed value is 22.
short WMFreqBScaleFacBy8	;	// reserved
short DataBitNumPerPack	;
short BitDurationX667ms;
short Reserv0	;
short Reserv1	;


}	;



/************************************************************************************************************
 * Description:
 *	
 * Argument:
 *	None.
 * Return:
 *	None.
 ************************************************************************************************************/
void WaterMark_Initial(void);

/************************************************************************************************************
 * Description:
 *	
 * Argument:
 *	
 * Return:
 *	None.
 ************************************************************************************************************/

/************************************************************************************************************
 * Description:
 *	
 * Argument:
 *	
 * Return:
 *	
 ************************************************************************************************************/

short WaterMark_Process(
	 short * pi16EnaWMReceiveFlag,
	unsigned short * pi16RxData,
	 short * pi16ADIsrBuf,
	short i16ADMainBufPageIdx, 
	short i16InDataBufNum,
	struct WMParaStructTemp	*pWMParaStruct
	)	;

/******************************************************************************************
 * Description:
 * 	Version control API.
 * Arguments:
 *  None.
 ******************************************************************************************/ 
//unsigned long 
unsigned long
WaterMark_GetVersion(void);

#ifdef  __cplusplus
}
#endif

#endif	//#ifndef __WATERMARK_H__

