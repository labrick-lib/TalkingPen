/**************************************************************
ESFrontParam  version:  1.0   ·  date: 08/13/2007
-------------------------------------------------------------
Front end parameters.
-------------------------------------------------------------
Copyright (C) 2007 - All Rights Reserved
***************************************************************
Sheng Chen
**************************************************************/

#ifndef ES_TEAM__2007_08_13__ESCOMMON__H
#define ES_TEAM__2007_08_13__ESCOMMON__H

#include "ivESR.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


	/* CMN update rate Q9 */
#define ES_CMN_UPDATE_RATE          (92)    /* ((ivUInt32)(0.18*2^9)) */ /* Q9 */

	/* PCM data frame setting */
#define ESR_PCMBUFFER_BIT           (10)
#define ESR_PCMBUFFER_SIZE          (1<<ESR_PCMBUFFER_BIT)
#define APPENDDATA_SAMPLES_MAX      (ESR_PCMBUFFER_SIZE - ESR_FRAME_SIZE)

#define FEATURE_DIMNESION           (16)
#if (8000 == MINI5_SAMPLERATE)
	#define ESR_FRAME_SIZE          (200)
	#define ESR_FRAME_STEP          (80) 
#elif (16000 == MINI5_SAMPLERATE)
	#define ESR_FRAME_SIZE          (400)
	#define ESR_FRAME_STEP          (160)
#else
	#error "Audio Sample Rate NOT support!"
#endif

#define ESR_ENAHNCE_VAD             (0)
#define ESR_HISTORY_SIL             (0)   /* 是否使用历史E_Sil信息 */

#define ESR_CPECTRAL_SUB            (0)   /* 是否开启谱减降噪 */
#define AVG_RSHIFT                  (1)
#define ENHANCE_DEF                 (1)
#define FFTFRAME_MAXNUM             (2)

#define ESR_ENERGY_LOG              (1)   /* 是否对能量进行压缩 */

#if ESR_ENAHNCE_VAD
#define ESR_BACK_FRAMES             (40)
#else
#define ESR_BACK_FRAMES             (40)
#endif

#define ESR_GCD_FRAMESIZE_FRAME_STEP (40)   /* HiPassEnergy 帧长200和帧移80的最大公约数 */
#define ESR_HIPASSENERGY_NUM         (ESR_FRAME_SIZE/ESR_GCD_FRAMESIZE_FRAME_STEP) /* HiPassEnergy Num */

    /* For voice activity detection */
#define ESIL_CHKBG_FRAMENUM         (3)     /* Frame number for Check Begin ESil */
#define ELOW_CHKBG_FRAMENUM         (20)    /* (20) (13) Frame number for Check Begin ELow */
#define EHIGH_CHKBG_FRAMENUM        (20)    /* (20) (13) Frame number for Check Begin EHigh */	
#define VAD_TIMEOUT_LEN             (400)
#define TAG_SPEECH_TIMEOUT          (400)

#define ESR_FRAME_MAXNUM            ESR_BACK_FRAMES     /* Buffer size */
#define MINIMUM_SPEECH_FRAMENUM     (20)    /* (15) Minimum voice frame */
#define ELOW_VALIDREQ_THRESH        (5)     /* (5) */
#define EHIGH_VALIDREQ_THRESH       (4)     /* (4) */
#define EHIGH_ENDVALID_THRESH       (3)     /* (3) */
#define SPEECH_BEGIN_MARGIN         (25)    /* (25)  (16) */
#define SPEECH_END_MARGIN           (40)    /* (40)  (25) */

#if ESR_ENAHNCE_VAD
    #define EHIGH_CHKEND_FRAMENUM       (40)    /* Frame number for Check End EHigh. 0.4s */
    #define MINIMUM2_SPEECH_FRAMENUM    (35)    /* (15) Minimum voice frame */
    #define EHIGH2_CHKEND_FRAMENUM      (75)    /* Frame number for Check End EHigh. 0.75s */
#else
    #define EHIGH_CHKEND_FRAMENUM       (ESR_BACK_FRAMES)   /* Frame number for Check End EHigh. 0.4s */
#endif

    /* Static MFCC parameter */
#define TRANSFORM_CHANSNUM_DEF          (24)    /* FilterBank channel:24*/
#define TRANSFORM_CEPSNUM_DEF           (7)     /* static MFCC :7 */

#if MINI5_ENHANCE_FRONT                            /* xqxie 前端智能化*/
    #define ESR_CUTDET_FRAMES           (10)
    //#define ESR_CUTDET_THR            (0.9)
    #define ESR_CUTDET_THR_Numerator    9   //分子
    #define ESR_CUTDET_THR_Demoninator  10  //分母

    //#define ESR_SNRDET_THR            (1.09)
    #define ESR_SNRDET_THR_Numerator    109  //分子
    #define ESR_SNRDET_THR_Demoninator  100  //分母

    #define ESR_ENERGYDET_THR           (20000)
#endif

#define ESR_MFCC_BACK_FRAMES        64//(SPEECH_BEGIN_MARGIN+ELOW_CHKBG_FRAMENUM+2)

#define ESR_Q_MFCC_MANUAL           (6) /* MFCC定标值 */

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* !ES_TEAM__2007_08_13__ESCOMMON__H */
