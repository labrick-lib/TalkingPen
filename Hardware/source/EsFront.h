/**************************************************************
ESFront   version:  1.0   ·  date: 08/13/2007
-------------------------------------------------------------
Front end.
-------------------------------------------------------------
Copyright (C) 2007 - All Rights Reserved
***************************************************************
Sheng Chen
**************************************************************/

#ifndef ES_TEAM__2007_08_13__ESFRONT__H
#define ES_TEAM__2007_08_13__ESFRONT__H

#include "ivMath.h"
#include "EsFrontConst.h"
#include "EsFFTConst.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Config */
#define ESVAD_SILENCE		(0)
#define ESVAD_CHECK_BEGIN	(1)
#define ESVAD_ACTIVE		(2)
#define ESVAD_CHECK_END		(3)
#define ESVAD_INACTIVE		(4)

typedef struct tagESRFront 
{
	ivUInt16		m_iVADState;
#if ESR_ENERGY_LOG
    #if ESR_HISTORY_SIL
	ivInt16			m_s16HisESil;
    #endif
	ivInt16			m_s16ESil;		
	ivInt16			m_s16ELow;		
	ivInt16			m_s16ELow2;		
	ivInt16			m_s16EHigh2;	
#else /* ESR_ENERGY_LOG */
    #if ESR_HISTORY_SIL
    ivInt32			m_s16HisESil;
    #endif
    ivInt32			m_s16ESil;		
    ivInt32			m_s16ELow;		
    ivInt32			m_s16ELow2;		
    ivInt32			m_s16EHigh2;	
#endif /* ESR_ENERGY_LOG */

#if ESR_CPECTRAL_SUB
    ivInt16			m_nFFTSpecCurQ;
    ivInt16			m_nFFTSpecPreQ;
    ivInt16			m_nFFTSpecAvgQ;	
    ivInt32			m_iMFCC;		
    ivPUInt16		m_pFFTSpecCur;
    ivPUInt16		m_pFFTSpecPre;
    ivPUInt16		m_ppFFTSpecAvg; /* 三帧Spectrogram的平均*/	
#endif

	ivUInt32		m_nCMNFrameLast;

	/* ---------------------- */
	/* Frame buffer,复用存储FFT时的实虚部和filterbank */
	ivPInt16		m_pFrameCache;	

	/* CMN */
	ivInt16			m_nCMNHisWeight;	
	ivInt16			m_nCMNUpdateRate;	

	ivInt32		    m_ps32CMNMeanSum[TRANSFORM_CEPSNUM_DEF+1];
	ivInt16		    m_ps16CMNMean[TRANSFORM_CEPSNUM_DEF+1];		
	ivPInt16		m_ps16HisCMNMean;	
	/* ---------------------- */
	
    /* PCM buffer */
    ivUInt16        m_iPCMStart;
    ivUInt16        m_iPCMEnd;
    ivInt32         m_s32MeanSum;
    ivInt16         m_pPCMBuffer[ESR_PCMBUFFER_SIZE];

	/* buffer energy */
#if ESR_ENERGY_LOG
	ivInt16		    m_ps16FrameEnergy[ESR_BACK_FRAMES];
#else
	ivInt32		    m_ps16FrameEnergy[ESR_BACK_FRAMES];
#endif /* ESR_ENERGY_LOG */

	/* VAD parameter */
	/* Last cursor of buffer */
	ivInt32			m_iFrameEnd;		
	/* Head cursor of buffer */
	ivInt32			m_iFrameHead;		
	/* Check cursor of buffer */
	ivInt32			m_iFrameCheck;		
	ivInt32			m_iSpeechBegin;		
	ivInt32			m_iSpeechEnd;		
	ivInt32			m_iFrameCurrent;	

	ivUInt16		m_fSpeechStart;		
	ivUInt16		m_fSpeechEnd;		

	ivInt32         m_s32MeanSum2;		

	/* Speech time out frame */
	ivInt32         m_nSpeechTimeOut;	

	ivPUInt32		m_pnCMNCRC;	

	/* Filter Bandpass buffer */
	ivPInt32		m_ps32HiPassEnergy;	
	/* HiPass buffer Cursor */
	ivInt32			m_iHiPassEnergy;	
	ivInt32			Z11;				
	ivInt32			Z12;				
	ivInt32			Z21;				
	ivInt32			Z22;				
	ivInt32			Z31;				
	ivInt32			Z32;				

#if ESR_ENAHNCE_VAD
	ivInt32			m_nChkEndFrameNum;   /* VAD判断语音结束时向后看的帧数 20110121 */
#endif

	ivBool			m_bEndAppendData;	
    ivBool          m_bSpeechTimeOut;

#if MINI5_ENHANCE_FRONT
	//xqxie
	/* 抢说检测，所需的变量 */
	ivBool	        m_bSpeechCut;
	ivInt32			m_s32FrameEnergyDet;
	ivInt32			m_s32FrameEnergyAll;

	/* 信噪比检测，所需的变量 */
	ivBool          m_bSpeechLowSNR;
	ivInt32			m_s32FrameEnergySil;
	ivInt32			m_s32FrameEnergySpeech;

	ivInt32	        m_iFrontSilBegin;
	ivInt32         m_iFrontSilEnd;
	ivInt32	        m_iFrontSpeechBegin;

	/* 能量大小检测，所需的变量 */
	ivBool          m_bSpeechLowEnergy;
	ivInt32         m_s32MaxFrameEnergy;
	//xqxie
#endif
	ivUInt16		m_fSpeechBegin;

#if MINI5_SPI_MODE
    ivInt16         m_ps16MeanCoef[FEATURE_DIMNESION];
    ivInt16         m_ps16MeanCoefQ[FEATURE_DIMNESION];
#else
    ivPInt16        m_ps16MeanCoef;
    ivPInt16        m_ps16MeanCoefQ;   /* MeanCoef的各维定标 */
#endif

	ivInt16         (ivPtr m_ppMFCCTmp)[TRANSFORM_CEPSNUM_DEF+1];

}TESRFront, ivPtr PESRFront;

typedef TESRFront ivCPtr PCESRFront;

EsErrID ivCall EsFrontInit(PESRFront pThis, PEsrEngine pEngine);

void ivCall EsFrontReset(PESRFront pThis);

EsErrID ivCall EsFrontAppendData(PESRFront pThis, ivCPointer pData, ivUInt16 nSamples);

EsErrID ivCall EsFrontGetFeature(PESRFront pThis, ivPInt16 pFeature);

void ivCall EsFrontUpdateMean(PESRFront pThis);

void ivCall EsFrontCalcDynamicMFCC(PESRFront pThis,                                     
                                       ivInt16 ppMFCC[ESR_MFCC_BACK_FRAMES][TRANSFORM_CEPSNUM_DEF+1], 
                                       ivInt16 iMFCCEnd, 
                                       ivPInt16 pFrameFeature);

#if MINI5_ENHANCE_FRONT
#if ESR_ENERGY_LOG
void EsCalEnhanceFrontFrame(PESRFront pThis, ivInt16 s16Energy);
#else
void EsCalEnhanceFrontFrame(PESRFront pThis, ivInt32 s16Energy);
#endif
void EsGetEnhanceFrontResult(PESRFront pThis);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !ES_TEAM__2007_08_13__ESFRONT__H */
