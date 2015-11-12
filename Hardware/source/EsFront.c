
/**************************************************************
ESFront   version:  1.0   ·  date: 08/13/2007
-------------------------------------------------------------
Front end.
-------------------------------------------------------------
Copyright (C) 2007 - All Rights Reserved
***************************************************************
Sheng Chen
**************************************************************/

#include "EsKernel.h"
#include "EsEngine.h"
#include "EsFront.h"

#if RATETSET_USE_VAD
extern ivUInt16 g_bDisableVad;
#endif

#define ESR_MATH_LN40_Q10			3777
#define ESR_MATH_LN50_Q10			4006
#define ESR_MATH_LN60_Q10          (0x1061L)  /* ln60, Q10 */

#define ESR_MATH_LN300_Q10			5841
#define ESR_MATH_LN400_Q10			6135
#define ESR_MATH_LN500_Q10			6364
#define ESR_MATH_LN600_Q10			6550
#define ESR_MATH_LN800_Q10			6845
#define ESR_MATH_LN1000_Q10			7074
#define ESR_MATH_LN1200_Q10        (0x1C5CL)  /* ln1200, Q10 */
#define ESR_MATH_LN1300_Q10        (0x1CAEL) /* ln1300, Q10 */

#define ESR_TRAIN_CMN_PARAM         (0) /* 进行CMN参数训练，CMN更新策略与产品不同，采用每句的CMN参数比重都是1. 20101029 */
#if ESR_TRAIN_CMN_PARAM
ivUInt32	g_nCMNPcm = 0;    /* 进行CMN训练,该值达到几十句后，就可以将ESRCreate里的常驻内存进行保存,后期使用时直接用那块内容就是训练的CMN参数了 */
#endif

#define ESR_VAD_PARAM_LOW			(0)/*VAD能量门限参数，一组比较取值比较大，一组取值比较小*/

ivStatic void ivCall EsFrontCheckVoice(PESRFront pThis);

#if ESR_ENERGY_LOG
void ivCall EsFrontCalcStaticMFCC(PESRFront pThis, ivPInt16 ps16MFCC, ivPInt16 ps16FrameEnergy);
# if ESR_CPECTRAL_SUB 
void ivCall EsFrontEnhanceGetEnergy(PESRFront pThis, ivPInt16 ps16FrameEnergy);
# endif /* ESR_CPECTRAL_SUB */
#else /* ESR_ENERGY_LOG */
void ivCall EsFrontCalcStaticMFCC(PESRFront pThis, ivPInt16 ps16MFCC, ivPInt32 ps16FrameEnergy);
# if ESR_CPECTRAL_SUB 
void ivCall EsFrontEnhanceGetEnergy(PESRFront pThis, ivPInt32 ps16FrameEnergy);
# endif /* ESR_CPECTRAL_SUB */
#endif /* ESR_ENERGY_LOG */

#if ESR_CPECTRAL_SUB
void ivCall EsFrontEnhanceCalcFFT(PESRFront pThis, ivPInt16 pFFTQ);
#endif

EsErrID ivCall EsFrontInit(PESRFront pThis, PEsrEngine pEngine)
{
#if MINI5_API_PARAM_CHECK
	if (ivNull == pThis || ivNull == pEngine) {
		return EsErr_InvArg;
	}
#endif

	pThis->m_iPCMStart = 0;
	pThis->m_iPCMEnd = 0;

	/* Memory allocate */
	pThis->m_ps16HisCMNMean = pEngine->pResidentRAMHdr->ps16CMNMean;

	pThis->m_pnCMNCRC = &(pEngine->pResidentRAMHdr->nCMNCRC);

	/* Memory allocate */	
	pThis->m_pFrameCache = (ivPInt16)EsAlloc(pEngine, (TRANSFORM_FFTNUM_DEF+2)*sizeof(ivInt16));
	
	/* For Bandpass Filter */
	pThis->m_ps32HiPassEnergy = (ivPInt32)EsAlloc(pEngine, ESR_HIPASSENERGY_NUM * sizeof(ivInt32));
	ivMemZero(pThis->m_ps32HiPassEnergy, sizeof(ivInt32) * ESR_HIPASSENERGY_NUM);
	pThis->m_iHiPassEnergy = 0;
	pThis->Z11 = 0;
	pThis->Z12 = 0;
	pThis->Z21 = 0;
	pThis->Z22 = 0;
	pThis->Z31 = 0;
	pThis->Z32 = 0;

    /* MFCC Buffer */
    pThis->m_ppMFCCTmp = (ivInt16 (ivPtr) [TRANSFORM_CEPSNUM_DEF+1])EsAlloc(pEngine, ESR_MFCC_BACK_FRAMES*(TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt16));
#if MINI5_API_PARAM_CHECK
    if(ivNull == pThis->m_ppMFCCTmp) {
		ivAssert(ivFalse);	
		return EsErr_OutOfMemory;
	}
#endif

#if ESR_CPECTRAL_SUB
    pThis->m_pFFTSpecCur = (ivPUInt16)EsAlloc(pEngine, (TRANSFORM_HALFFFTNUM_DEF+1) * sizeof(ivUInt16) * 3);
# if MINI5_API_PARAM_CHECK
    if(ivNull == pThis->m_pFFTSpecCur) {
        ivAssert(ivFalse);
        return EsErr_OutOfMemory;
    }
# endif

    pThis->m_pFFTSpecPre = (ivPUInt16)((ivAddress)pThis->m_pFFTSpecCur + (TRANSFORM_HALFFFTNUM_DEF+1) * sizeof(ivUInt16));
    pThis->m_ppFFTSpecAvg = (ivPUInt16)((ivAddress)pThis->m_pFFTSpecPre + (TRANSFORM_HALFFFTNUM_DEF+1) * sizeof(ivUInt16));
#endif

#if ESR_HISTORY_SIL
    pThis->m_s16HisESil = 0;
#endif

    EsFrontReset(pThis);	

	return EsErrID_OK;
}

void ivCall EsFrontReset(PESRFront pThis) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别，后面增加了几个变量=0的初始化 */
{
	pThis->m_iFrameEnd = 0 ;
	pThis->m_iFrameHead = 0;
	pThis->m_bEndAppendData= 0;
    pThis->m_bSpeechTimeOut = 0;

	ivMemZero(pThis->m_pPCMBuffer, sizeof(ivInt16)*ESR_PCMBUFFER_SIZE);

	pThis->m_iSpeechBegin = 0;
	pThis->m_s16ESil = 0;
	pThis->m_iVADState = ESVAD_SILENCE;
	/*pThis->pEngine->tSearch.bCanSwitch = ivTrue;*/
#if ESR_ENAHNCE_VAD
	pThis->m_nChkEndFrameNum = (ivInt32)EHIGH_CHKEND_FRAMENUM;
#endif
	
	pThis->m_fSpeechBegin = 0;
	pThis->m_fSpeechStart = 0;
	pThis->m_fSpeechEnd = 0;

	pThis->m_iFrameCurrent = 2;

	pThis->m_iSpeechEnd = 0;

	pThis->m_nSpeechTimeOut = VAD_TIMEOUT_LEN;
	/* Add by Truman, Here to Clear PCM buffer by an action "Read" */
	pThis->m_iPCMStart = 0;
    pThis->m_iPCMEnd = 0; 

	pThis->m_s32MeanSum = 0;

	pThis->m_nCMNFrameLast = 0;
#if RATETSET_USE_VAD
    if(g_bDisableVad) {
        pThis->m_nCMNHisWeight = 50;
    }
    else
#endif /* RATETSET_USE_VAD */
    {
        pThis->m_nCMNHisWeight = 56;
    }
	pThis->m_nCMNUpdateRate = 1024;
	/* CMN Mean 9.6, C0:11.4 */
	ivMemCopy(pThis->m_ps16CMNMean,pThis->m_ps16HisCMNMean,(TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt16));
	ivMemZero(pThis->m_ps32CMNMeanSum, (TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt32));
#if MINI5_ENHANCE_FRONT
	/* xqxie */
	/*抢说检测*/
	pThis->m_bSpeechCut = ivFalse;

	pThis->m_s32FrameEnergyDet = 0;
	pThis->m_s32FrameEnergyAll = 0;

	/*信噪比检测*/
	pThis->m_bSpeechLowSNR = ivFalse;

	pThis->m_s32FrameEnergySil = 0;
	pThis->m_s32FrameEnergySpeech = 0;

	pThis->m_iFrontSilBegin = 0;
	pThis->m_iFrontSilEnd = 0;
	pThis->m_iFrontSpeechBegin = 0;

	/*能量大小检测*/
	pThis->m_bSpeechLowEnergy = ivFalse;

	pThis->m_s32MaxFrameEnergy = 0;
	/*xqxie*/
#endif

#if ESR_CPECTRAL_SUB
    pThis->m_iMFCC = 0;
    pThis->m_nFFTSpecAvgQ = 0;
    pThis->m_nFFTSpecCurQ = 0;
    pThis->m_nFFTSpecPreQ = 0;
#endif

}

void ivCall EsFrontUpdateMean(PESRFront pThis) /* 与硕呈平台AitalkMini1.0代码比较: 小差别 */
{
	ivUInt16 i;
	ivInt16 nCMNFrame = (ivInt16)pThis->m_nCMNFrameLast;
	ivUInt32 nCounter,nCRC;

	if (nCMNFrame <= 20) {
		return;
	}

	nCounter = 0;
	nCRC = 0;
	for (i = 0;i < TRANSFORM_CEPSNUM_DEF+1; i++) {
		ivInt32 nVal;
		/* Q6                       Q2                          Q4 */     
#if (!ESR_TRAIN_CMN_PARAM) 
		/* 产品CMN策略:CMN=0.91His+0.09Cur=His+0.09(Cur-His). Cur-His = pThis->m_ps16CMNMeanSum/nCMNFrame */
		//ivAssert(pThis->m_ps16CMNMean[i] + pThis->m_ps16CMNMeanSum[i]/nCMNFrame*ES_CMN_UPDATE_RATE >= -32768);
		//ivAssert(pThis->m_ps16CMNMean[i] + pThis->m_ps16CMNMeanSum[i]/nCMNFrame*ES_CMN_UPDATE_RATE < 32768);
		nVal = (pThis->m_ps32CMNMeanSum[i])/nCMNFrame;
		nVal -= pThis->m_ps16HisCMNMean[i];
		pThis->m_ps16HisCMNMean[i] += (nVal * ES_CMN_UPDATE_RATE) >> 9;
		//pThis->m_ps32CMNMeanSum[i] += (nVal * ES_CMN_UPDATE_RATE) >> 9;

#elif ESR_TRAIN_CMN_PARAM
		/* 进行CMN训练的策略:CMN=0.5His+0.5Cur=His+0.5(Cur-His). Cur-His = pThis->m_ps16CMNMeanSum/nCMNFrame */
		xxxxxxxxxxxxxxpThis->m_ps16CMNMean[i] += pThis->m_ps32CMNMeanSum[i]/nCMNFrame*8; /* Q4 */	
#endif
	}
#if ESR_TRAIN_CMN_PARAM
	g_nCMNPcm++;
#endif

	/* CMN Floor 20100909 */
#if 1
	for(i=0; i<TRANSFORM_CEPSNUM_DEF+1; i++) {
		if(pThis->m_ps16HisCMNMean[i] > g_s16CMNCoefMax[i]){
			pThis->m_ps16HisCMNMean[i] = g_s16CMNCoefMax[i];
		}
		else if(pThis->m_ps16HisCMNMean[i] < g_s16CMNCoefMin[i]){
			pThis->m_ps16HisCMNMean[i] = g_s16CMNCoefMin[i];
		}
	}
#endif

	ivMakeCRC(pThis->m_ps16HisCMNMean, (TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt16), &nCRC, &nCounter);
	*pThis->m_pnCMNCRC = nCRC;
}

void ivCall EsFrontCalcDynamicMFCC(PESRFront pThis,  
                                   ivInt16 ppMFCC[ESR_MFCC_BACK_FRAMES][TRANSFORM_CEPSNUM_DEF+1], 
                                   ivInt16 iMFCCEnd,                                    
                                   ivPInt16 pFrameFeature)
{
    ivInt16 i,j;
    ivInt16 iLeft1,iLeft2,iRight1,iRight2;

#if (TEST_CMN_UPDATE != NOT_CMN_UPDATE) && (TEST_CMN_UPDATE != CMN_UPDATE_JUZI)
    ivInt16 nCMNHisWeight = pThis->m_nCMNHisWeight;
    ivInt16 nCMNHisWeight2 = 64-nCMNHisWeight;
    ivInt32 s32CMN;
#endif

    ivPInt32 ps32MFCC;

    ps32MFCC = (ivPInt32)pThis->m_pFrameCache;  /* 复用pFrameCache临时存39维MFCC */

    /* Update pMFCCDelACCFactor */
    iMFCCEnd = (iMFCCEnd +  ESR_MFCC_BACK_FRAMES)%ESR_MFCC_BACK_FRAMES;
    iLeft1 = (iMFCCEnd + ESR_MFCC_BACK_FRAMES - 1)%ESR_MFCC_BACK_FRAMES;
    iLeft2 = (iMFCCEnd + ESR_MFCC_BACK_FRAMES - 2)%ESR_MFCC_BACK_FRAMES;
    iRight1 = (iMFCCEnd + ESR_MFCC_BACK_FRAMES + 1)%ESR_MFCC_BACK_FRAMES;
    iRight2 = (iMFCCEnd + ESR_MFCC_BACK_FRAMES + 2)%ESR_MFCC_BACK_FRAMES;

    for(i=0; i<=TRANSFORM_CEPSNUM_DEF; i++){
        ivInt32 s32Val;

        s32Val = ppMFCC[iMFCCEnd][i];
        /* Update CMNSum */		
        pThis->m_ps32CMNMeanSum[i] += s32Val;

        /* Set Feature Value */

#if (TEST_CMN_UPDATE == NOT_CMN_UPDATE)
        ps32MFCC[i] = (s32Val-g_s16CMNCoef[i]) * pThis->m_ps16MeanCoef[i];	// Q:6+g_ps16MeanCoefQ[i]
#elif(TEST_CMN_UPDATE == CMN_UPDATE_JUZI)
        ps32MFCC[i] = (s32Val-pThis->m_ps16HisCMNMean[i]) * pThis->m_ps16MeanCoef[i];	// Q:6+g_ps16MeanCoefQ[i]
#else
        s32CMN = (nCMNHisWeight2*pThis->m_ps16CMNMean[i]+nCMNHisWeight*pThis->m_ps16HisCMNMean[i])>>6;
        ps32MFCC[i] = (s32Val-s32CMN) * pThis->m_ps16MeanCoef[i];	// Q:6+g_ps16MeanCoefQ[i]
#endif
        /* CMN */
        s32Val -= pThis->m_ps16CMNMean[i];
        pThis->m_ps16CMNMean[i] += /*1*(s32Val>>7);*/ (pThis->m_nCMNUpdateRate*s32Val) >> 15;

        /* Delta */		
        j = TRANSFORM_CEPSNUM_DEF + 1 + i;				
        ps32MFCC[j] = ppMFCC[iRight1][i] - ppMFCC[iLeft1][i];
        ps32MFCC[j] += ((ppMFCC[iRight2][i] - ppMFCC[iLeft2][i]) << 1);
		ps32MFCC[j] = ps32MFCC[j] * pThis->m_ps16MeanCoef[j];		// Q:6+g_ps16MeanCoefQ[i]
    }

    --pThis->m_nCMNHisWeight;

    if (pThis->m_nCMNUpdateRate > 170){
        pThis->m_nCMNUpdateRate -= (1200*pThis->m_nCMNUpdateRate)>>15;
    }
    pThis->m_nCMNHisWeight = ivMax(0,pThis->m_nCMNHisWeight);


    ++pThis->m_nCMNFrameLast;

    /* 配合量化策略,特征存成和mean量化一样的格式 */
    for(i=0; i<FEATURE_DIMNESION; i++){
        /* ---------------------------------------|-------此处为四舍五入----|--------   */
        pFrameFeature[i] = (ivInt16)((ps32MFCC[i]+(1<<(pThis->m_ps16MeanCoefQ[i]-1))) >> (ESR_Q_MFCC_MANUAL + pThis->m_ps16MeanCoefQ[i] - ESR_Q_MEAN_MANUAL - ESR_VAR_QUANT_BIT)); 	 

#if (5 == ESR_VAR_QUANT_BIT && 6 == ESR_MEAN_QUANT_BIT)
        pFrameFeature[i] &= 0xFFE0;

        //ivAssert(pFrameFeature[i] <= (ivInt16)0x03e0 && pFrameFeature[i] >= (ivInt16)0xfc00);
        /* 进行饱和，保证mean在定标范围内不爆标.mean用6bit表示,有效数值为5bit. 2011-01-20 */
        if(pFrameFeature[i] > (ivInt16)0x03E0){
            pFrameFeature[i] = (ivInt16)0x03E0;
        }
        else if(pFrameFeature[i] < (ivInt16)0xFC00){
            pFrameFeature[i] = (ivInt16)0xFC00;
        }
#elif (5 == ESR_VAR_QUANT_BIT && 7 == ESR_MEAN_QUANT_BIT)
        pFrameFeature[i] &= 0xFFE0;

        //ivAssert(pFrameFeature[i] <= (ivInt16)0x03e0 && pFrameFeature[i] >= (ivInt16)0xfc00);
        /* 进行饱和，保证mean在定标范围内不爆标.mean用6bit表示,有效数值为5bit. 2011-01-20 */
        if(pFrameFeature[i] > (ivInt16)0x07E0){
            pFrameFeature[i] = (ivInt16)0x07E0;
        }
        else if(pFrameFeature[i] < (ivInt16)0xF800){
            pFrameFeature[i] = (ivInt16)0xF800;
        }
#else
        ivAssert(ivFalse);
#endif

        pFrameFeature[i] -= 0x800;
    }
}

/* Start from check point checking whether have n frame's energy big than threshold energy */
#if ESR_ENERGY_LOG
ivStatic ivBool ivCall EsFrontCheckEngery(PESRFront pThis, ivInt16 nEnergyThresh, ivUInt16 nSeqCount, ivUInt16 nTotalCount) /* 与硕呈平台AitalkMini1.0代码相同 */
#else
ivStatic ivBool ivCall EsFrontCheckEngery(PESRFront pThis, ivInt32 nEnergyThresh, ivUInt16 nSeqCount, ivUInt16 nTotalCount)
#endif
{
	ivUInt16 i = 0;
	ivUInt16 nValidNum = 0 ;

	for ( i=0; i<nTotalCount; i++ ) {
		if ( pThis->m_ps16FrameEnergy[(pThis->m_iFrameCheck+i)%ESR_FRAME_MAXNUM]> nEnergyThresh) {
			nValidNum ++ ;
		}
		else{
			nValidNum = 0;
		}
		/* Continuous n frames > Energy */
		if ( nValidNum > nSeqCount ) {
			pThis->m_iFrameCheck =  i + pThis->m_iFrameCheck - nSeqCount;			
			return ivTrue;
		}
	}
	return ivFalse;
}

ivStatic void ivCall EsFrontCheckVoice(PESRFront pThis) /* 与硕呈平台AitalkMini1.0代码相同 */
{
	ivInt32 nFrameCount, nCheckCount, i;
	nFrameCount = pThis->m_iFrameEnd - pThis->m_iFrameHead;

#if MINI5_API_PARAM_CHECK
	/*加入语音结束检查，用于离线测试*/
	/*防止出现只检测出前端点，因为剩余语音太少而检测不出后端点的情况*/
	/*add by hkwang 2011-07-11*/
	if (pThis->m_bEndAppendData && 0 == pThis->m_fSpeechEnd) {
		if (ESVAD_ACTIVE == pThis->m_iVADState || ESVAD_CHECK_END == pThis->m_iVADState) {
			pThis->m_iVADState = ESVAD_INACTIVE;
			pThis->m_fSpeechEnd = 1;
			pThis->m_iFrameCheck = (pThis->m_iFrameHead+1);

			/* Patched by Truman 2007-9-11 5:01:36 */
			pThis->m_s16ESil = 0;

			pThis->m_iSpeechEnd = pThis->m_iFrameEnd;
			ivAssert(pThis->m_iSpeechEnd>0);
			return;	
		}
	}
#endif

	while(nFrameCount) {
		nFrameCount = pThis->m_iFrameEnd - pThis->m_iFrameHead;
		if(!nFrameCount) {
			return;
		}

		if(0 == pThis->m_s16ESil) {
			ivInt32 s32Temp = 0;
			if (nFrameCount < ESIL_CHKBG_FRAMENUM)
				return;

			/* ESIL = avg(3 Frame)*/
			for(i = 0; i < ESIL_CHKBG_FRAMENUM; i++) {
				s32Temp += pThis->m_ps16FrameEnergy[(pThis->m_iFrameHead+i)%ESR_FRAME_MAXNUM];
			}
			pThis->m_s16ESil = (s32Temp/(ivInt32)ESIL_CHKBG_FRAMENUM);
			ivAssert(pThis->m_s16ESil == (s32Temp/3));
			ivAssert(pThis->m_s16ESil > 11);

#if ESR_HISTORY_SIL
            if (0 != pThis->m_s16HisESil) {
                pThis->m_s16ESil = (((ivInt32)pThis->m_s16HisESil*40+(ivInt32)pThis->m_s16ESil*24)>>6);
            }
#endif
			/* Make check cursor point to next frame to head */			
			pThis->m_iFrameCheck = (pThis->m_iFrameHead+1);

			/* m_fELow = m_fESil * 60.0f/((float)ln(m_fESil) - 4.0f); */
			/* ln(m_fELow) = ln[m_fESil * 60.0f/((float)ln(m_fESil) - 4.0f)]; */                                  /* 4*(1<<10)  */
#if ESR_ENERGY_LOG
# if ESR_VAD_PARAM_LOW
			pThis->m_s16ELow = 1000 + pThis->m_s16ESil - (simple_table_ln((pThis->m_s16ESil-4096), 0)-ESR_MATH_10LN2_Q10);/* Q10 the second param*/
			pThis->m_s16ELow2 = pThis->m_s16ELow+800;/* Q10 the second param*/
# else
			pThis->m_s16ELow = 2000 + pThis->m_s16ESil - (simple_table_ln((pThis->m_s16ESil-4096), 0)-ESR_MATH_10LN2_Q10);/* Q10 */
			pThis->m_s16ELow2 = pThis->m_s16ELow+1000;/* Q10 */
# endif /* ESR_VAD_PARAM_LOW */
#else /* ESR_ENERGY_LOG */
# if ESR_VAD_PARAM_LOW
            pThis->m_s16ELow = pThis->m_s16ESil * 30 / (simple_table_ln(pThis->m_s16ESil, 0) + ESR_MATH_10LN2_Q10 - (4 << 10));/* Q10 */
            pThis->m_s16ELow <<= 10;

            pThis->m_s16ELow2 = pThis->m_s16ELow;/* Q10 */
# else
            pThis->m_s16ELow = pThis->m_s16ESil * 30 / (simple_table_ln(pThis->m_s16ESil, 0) + ESR_MATH_10LN2_Q10 - (4 << 10));/* Q10 */
            pThis->m_s16ELow <<= 10;

            pThis->m_s16ELow2 = pThis->m_s16ELow;/* Q10 */
# endif /* ESR_VAD_PARAM_LOW */
#endif /* ESR_ENERGY_LOG */
		}

		switch(pThis->m_iVADState) {
		case ESVAD_SILENCE:
			nCheckCount = (pThis->m_iFrameEnd-pThis->m_iFrameCheck);

			/* Count form check point, need ELOW_CHK_FRAMENUM frame, or not calculate EHigh */
			if (nCheckCount < ELOW_CHKBG_FRAMENUM)
				return;

			if (EsFrontCheckEngery(pThis,pThis->m_s16ELow,ELOW_VALIDREQ_THRESH,ELOW_CHKBG_FRAMENUM)) {
				/* Whether continuous n frame > ELow*/
				ivInt32 s32Rt;

				/* m_fEHigh = m_fESil * 1200.0f/(((float)log(m_fESil) - 4.0f) * ((float)log(m_fESil) - 11.0f)); */
				/* ln(m_fEHigh) = ln[m_fESil * 1200.0f/(((float)ln(m_fESil) + 10ln2 - 4.0f) * ((float)ln(m_fESil) + 10ln2 - 11.0f))]; */
				/*                                         4*(1<<10),Q10    11*(1<<10) */
#if ESR_ENERGY_LOG
                s32Rt = simple_table_ln((pThis->m_s16ESil-4096), 0) + simple_table_ln((pThis->m_s16ESil-11264), 0) - ESR_MATH_20LN2_Q10; /* Q10 */
                ivAssert(s32Rt <= 32767 && s32Rt >= -32768);
# if ESR_VAD_PARAM_LOW
				pThis->m_s16EHigh2 = (ivInt16)(ESR_MATH_LN400_Q10 + pThis->m_s16ESil - s32Rt);   /* Q10 the second param*/		
# else
				pThis->m_s16EHigh2 = (ivInt16)(ESR_MATH_LN600_Q10 + pThis->m_s16ESil - s32Rt);   /* Q10 */		
# endif /* ESR_VAD_PARAM_LOW */
#else /* ESR_ENERGY_LOG */
                s32Rt = simple_table_ln(pThis->m_s16ESil, 0) + ESR_MATH_10LN2_Q10; /* Q10 */
                s32Rt = (s32Rt - (4<<10)) * (s32Rt - (11<<10)) >> 10;
# if ESR_VAD_PARAM_LOW
				pThis->m_s16EHigh2 = (pThis->m_s16ESil * 400 / s32Rt) << 10;   /* Q10 */	
# else
				pThis->m_s16EHigh2 = (pThis->m_s16ESil * 600 / s32Rt) << 10;   /* Q10 */		
# endif /* ESR_VAD_PARAM_LOW */
#endif /* ESR_ENERGY_LOG */
				/* Passed ELow check, it's a possible begin point */
				pThis->m_iSpeechBegin = pThis->m_iFrameCheck;

				pThis->m_iVADState = ESVAD_CHECK_BEGIN;
				/*ESFrontUpdateMean(pThis,ivFalse)*/;
			}
			else {
				pThis->m_s16ESil = 0;
				pThis->m_iVADState = ESVAD_SILENCE;
				pThis->m_iFrameHead ++;
			}

			break;
		case ESVAD_CHECK_BEGIN:
			nCheckCount = (pThis->m_iFrameEnd-pThis->m_iFrameCheck);

			if ( nCheckCount < EHIGH_CHKBG_FRAMENUM)
				return;

			if (EsFrontCheckEngery(pThis,pThis->m_s16EHigh2,EHIGH_VALIDREQ_THRESH,EHIGH_CHKBG_FRAMENUM)) {
				/* Has n frame > EHigh */
				pThis->m_iFrameHead = pThis->m_iSpeechBegin; 

#if MINI5_ENHANCE_FRONT
				//xqxie
				pThis->m_iFrontSpeechBegin = pThis->m_iFrameCheck;
				pThis->m_iFrontSilEnd = pThis->m_iFrameHead;
				//xqxie
#endif

				/* Make check cursor point to next frame to head */			
				pThis->m_iFrameCheck = ( pThis->m_iFrameHead + 1 );
				/* Add begin margin */
				pThis->m_iFrameCurrent = ivMax(pThis->m_iSpeechBegin - SPEECH_BEGIN_MARGIN, 2);

				pThis->m_iVADState = ESVAD_ACTIVE;	

#if MINI5_ENHANCE_FRONT
				//xqxie
				pThis->m_iFrontSilBegin = pThis->m_iFrameCurrent;
				//xqxie
#endif
				pThis->m_fSpeechStart = 1;

				pThis->m_iSpeechEnd = ivMin(pThis->m_iSpeechBegin+SPEECH_END_MARGIN, pThis->m_iFrameEnd);
				ivAssert(pThis->m_iSpeechEnd>0);
			}
			else {
				pThis->m_s16ESil = 0;
				pThis->m_iVADState = ESVAD_SILENCE;
				pThis->m_iFrameHead ++;
			}

			break;
		case ESVAD_ACTIVE:
			if ( pThis->m_ps16FrameEnergy[pThis->m_iFrameHead%ESR_FRAME_MAXNUM] < pThis->m_s16ELow2 ) {
				pThis->m_iVADState = ESVAD_CHECK_END;
				pThis->m_iFrameCheck = (pThis->m_iFrameHead+1); 
			}
			else {
				/* Big than ELow is ok. */
				pThis->m_iFrameHead ++;
			}

			pThis->m_iSpeechEnd = ivMin(pThis->m_iFrameHead+SPEECH_END_MARGIN, pThis->m_iFrameEnd);
			ivAssert(pThis->m_iSpeechEnd>0);
		
			if(0 == pThis->m_fSpeechBegin) {
				pThis->m_fSpeechBegin = 1;
			}
		
			break;
		case ESVAD_CHECK_END:
			pThis->m_iSpeechEnd = ivMin(pThis->m_iFrameHead+SPEECH_END_MARGIN, pThis->m_iFrameEnd);
			nCheckCount = (pThis->m_iFrameEnd-pThis->m_iFrameCheck);
#if ESR_ENAHNCE_VAD
			if(nCheckCount < pThis->m_nChkEndFrameNum)
#else
			if(nCheckCount < EHIGH_CHKEND_FRAMENUM)
#endif
			{
				return;
			}

#if ESR_ENAHNCE_VAD
			if (EsFrontCheckEngery(pThis,pThis->m_s16EHigh2,EHIGH_ENDVALID_THRESH,(ivUInt16)pThis->m_nChkEndFrameNum))
#else
			if (EsFrontCheckEngery(pThis,pThis->m_s16EHigh2,EHIGH_ENDVALID_THRESH,EHIGH_CHKEND_FRAMENUM))
#endif			
			{
				/* Voice yet */
				pThis->m_iFrameHead++;

				pThis->m_iVADState = ESVAD_ACTIVE;
			}
			else {
				pThis->m_iVADState = ESVAD_INACTIVE;
				pThis->m_iFrameCheck = (pThis->m_iFrameHead+1);

				/* Patched by Truman 2007-9-11 5:01:36 */
#if ESR_HISTORY_SIL
                if (0 == pThis->m_s16HisESil) {
                    pThis->m_s16HisESil = pThis->m_s16ESil;
                }
                else {
                    // 历史、当前静音能量权重为（0.75，0.25）
                    pThis->m_s16HisESil = (3*pThis->m_s16HisESil + pThis->m_s16ESil)>>2;
                }
#endif
				pThis->m_s16ESil = 0;
			}

			return;
		//case ESVAD_INACTIVE:
		//	pThis->m_iVADState = pThis->m_iVADState;

		//	return;
		}
	} /* while(nFrameCount) */
}

EsErrID ivCall EsFrontAppendData(PESRFront pThis, ivCPointer pData, ivUInt16 nSamples)
{
	ivInt16 nLen;
	ivPInt16 pSrcData;
	ivInt16 i;

#if MINI5_API_PARAM_CHECK
	if(ivNull == pThis){
		return EsErr_InvCal;
	}

	if(ivNull == pData){
		return EsErr_InvArg;
	}
#endif

	if(0 == nSamples){
		return EsErr_InvCal;
	}
	else if(nSamples >= ESR_PCMBUFFER_SIZE){
		return EsErr_InvCal;
	} 

	pSrcData = (ivPInt16)pData;

	/* 将减s32Mean操作移到此处处理 20100309 */
	if(1 == nSamples){
		ivUInt16 iEnd;		
		iEnd = pThis->m_iPCMEnd;		
		
		++iEnd;
		iEnd = iEnd & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1));
		if(iEnd == pThis->m_iPCMStart){
			return EsErr_BufferFull;
		}

		pThis->m_s32MeanSum += ((ivInt32)(*pSrcData) - (ivInt32)(pThis->m_pPCMBuffer[pThis->m_iPCMEnd]));
		pThis->m_pPCMBuffer[pThis->m_iPCMEnd] = *pSrcData;

		pThis->m_iPCMEnd = iEnd;
		return EsErrID_OK;
	}

	/* Check overflow. */
	nLen = pThis->m_iPCMEnd - pThis->m_iPCMStart;
	if(nLen<0){
		nLen += ESR_PCMBUFFER_SIZE;
	}

	nLen = nLen + nSamples;
	/* nLen += nSamples; */
	if(nLen>ESR_PCMBUFFER_SIZE-1)	/* 这里必需减一 */
	{								/* 否则iEnd==iStart代表两个状态：缓冲区全空或全满 */
		return EsErr_BufferFull;		/* 无法判断,进而导致返回错误的状态，造成死锁 */
	}

	for(i = 0; i < nSamples; i++){
		ivUInt16 iCurPcm = ((pThis->m_iPCMEnd + i) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));
		pThis->m_s32MeanSum += ((ivInt32)(pSrcData[i]) - (ivInt32)(pThis->m_pPCMBuffer[iCurPcm]));
		pThis->m_pPCMBuffer[iCurPcm] = pSrcData[i];

	}

	pThis->m_iPCMEnd = ((pThis->m_iPCMEnd + nSamples) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));

	return EsErrID_OK;
}

ivStatic ivBool ivCall ESFrontGetOnePCM(PESRFront pThis, ivPInt16 pLen) /* 与硕呈平台AitalkMini1.0代码相同 */
{
	ivInt16 nLen;
	
	*pLen = nLen = pThis->m_iPCMEnd - pThis->m_iPCMStart;
	if(nLen<0)
		nLen += ESR_PCMBUFFER_SIZE;

	if(nLen<ESR_FRAME_SIZE) {
		return ivFalse;
	}

	pThis->m_s32MeanSum2 = pThis->m_s32MeanSum;

	return ivTrue;
}

ivBool g_bGotFeature = ivFalse;
EsErrID ivCall EsFrontGetFeature(PESRFront pThis, ivPInt16 pFeature) /* 与硕呈平台AitalkMini1.0代码比较: 有增加代码 */
{
	ivInt16 nLen;

#if MINI5_API_PARAM_CHECK
	if((ivNull == pThis) || (ivNull == pFeature)) {
		return EsErr_InvArg;
    }
#endif

	/* 与硕呈平台AitalkMini1.0代码比较: 增加 */
	if(pThis->m_iSpeechEnd - pThis->m_iSpeechBegin - (ivInt32)SPEECH_END_MARGIN > pThis->m_nSpeechTimeOut){
		pThis->m_iVADState = ESVAD_INACTIVE;
		pThis->m_fSpeechEnd = 1;
		pThis->m_iFrameCheck = (pThis->m_iFrameHead+1);

		pThis->m_s16ESil = 0;

        pThis->m_bSpeechTimeOut = 1;

		return EsErr_SpeechTimeOut;
	}

#if RATETSET_USE_VAD
    if(g_bDisableVad) {
        g_bGotFeature = ivFalse;
    }
#endif /* RATETSET_USE_VAD */

    /* 将PCMBuffer里能算的mfcc都算完,再Try解码一次 */
    for(; ;){

        if(ESVAD_INACTIVE == pThis->m_iVADState){
            break;
        }

        /* Get PCM frame for PCM buffer and Check buffer empty */
        if(!ESFrontGetOnePCM(pThis, &nLen))
        {
            break;
            //return EsErr_BufferEmpty;
        }

#if ESR_CPECTRAL_SUB
        EsFrontEnhanceGetEnergy(pThis, &pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM]);

        EsFrontCalcStaticMFCC(pThis, pThis->m_ppMFCCTmp[pThis->m_iMFCC%ESR_MFCC_BACK_FRAMES], &(pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM]));

        /* Update feature cursor */
        pThis->m_iMFCC ++;

#else /* ESR_CPECTRAL_SUB */
        EsFrontCalcStaticMFCC(pThis, pThis->m_ppMFCCTmp[pThis->m_iFrameEnd%ESR_MFCC_BACK_FRAMES], &(pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM]));

#endif /* ESR_CPECTRAL_SUB */

#if RATETSET_USE_VAD
        if(g_bDisableVad) {
            g_bGotFeature = ivTrue;
        }
#endif /* RATETSET_USE_VAD */

#if MINI5_ENHANCE_FRONT
        EsCalEnhanceFrontFrame(pThis, pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM]);
#endif

#if !ESR_CPECTRAL_SUB
        /* Update feature cursor */
        pThis->m_iFrameEnd ++;
#endif

        /*if (pThis->m_iFrameEnd < ESIL_CHKBG_FRAMENUM) {
            continue;
        }*/

#if RATETSET_USE_VAD
        if(g_bDisableVad) {
            pThis->m_fSpeechBegin = 1;
        }
        else
#endif /* RATETSET_USE_VAD */
        {
            EsFrontCheckVoice(pThis);
        }
    }

#if RATETSET_USE_VAD
    if(g_bDisableVad) {
        if(pThis->m_iFrameEnd >= 5 && g_bGotFeature) {
            EsFrontCalcDynamicMFCC(pThis, pThis->m_ppMFCCTmp, pThis->m_iFrameEnd - 3, pFeature);   
            return EsErrID_OK;
        }
        else {
            return EsErr_BufferEmpty;
        }
    }
    else
#endif /* RATETSET_USE_VAD */
    {
        if(1 == pThis->m_fSpeechBegin) {
            pThis->m_fSpeechBegin ++;
            return EsErr_FindStart;
        }

        if(ESVAD_INACTIVE == pThis->m_iVADState && pThis->m_iFrameCurrent >= pThis->m_iSpeechEnd-2){
            ivAssert(pThis->m_iFrameCurrent >= pThis->m_iSpeechEnd-2);
            pThis->m_fSpeechEnd = 1;
            return EsErrID_OK;
        }

        if (pThis->m_iFrameCurrent < pThis->m_iFrameEnd - 2 && pThis->m_iFrameCurrent < pThis->m_iSpeechEnd) {
            ivAssert(pThis->m_iFrameCurrent - 2 >= pThis->m_iFrameEnd - ESR_MFCC_BACK_FRAMES);
            EsFrontCalcDynamicMFCC(pThis, pThis->m_ppMFCCTmp, pThis->m_iFrameCurrent, pFeature);
            pThis->m_iFrameCurrent++;
            return EsErrID_OK;
        }

        return EsErr_BufferEmpty;
    }	
}

#if MINI5_ENHANCE_FRONT
#if ESR_ENERGY_LOG
void EsCalEnhanceFrontFrame(PESRFront pThis, ivInt16 s16Energy)
#else
void EsCalEnhanceFrontFrame(PESRFront pThis, ivInt32 s16Energy)
#endif
{
	//xqxie 抢说检测
	if(ESR_CUTDET_FRAMES > pThis->m_iFrameEnd) {
		pThis->m_s32FrameEnergyDet += (ivInt32)s16Energy;
	}

	pThis->m_s32FrameEnergyAll += (ivInt32)s16Energy;
	//xqxie 抢说检测

	//xqxie 信噪比及能量大小检测
	//如果VAD找到了开始点
	if((pThis->m_fSpeechStart) && (pThis->m_s32FrameEnergySil == 0)) {
		int i;
		//静音段开始点、结束点判断
		if (pThis->m_iFrontSilBegin >= pThis->m_iFrontSilEnd) {
			pThis->m_iFrontSilBegin = 0;
		}
		//静音段能量计算
		for (i=pThis->m_iFrontSilBegin; i<pThis->m_iFrontSilEnd; i++) {
			pThis->m_s32FrameEnergySil += pThis->m_ps16FrameEnergy[i%ESR_FRAME_MAXNUM];
		}
		pThis->m_s32FrameEnergySil /= (pThis->m_iFrontSilEnd - pThis->m_iFrontSilBegin);

		//语音段能量计算
		for (i=pThis->m_iFrontSpeechBegin; i<pThis->m_iFrameEnd; i++ ) {
			pThis->m_s32FrameEnergySpeech += pThis->m_ps16FrameEnergy[i%ESR_FRAME_MAXNUM];
			//从所有帧中，寻找能量最大的帧
			if(pThis->m_ps16FrameEnergy[i%ESR_FRAME_MAXNUM] > pThis->m_s32MaxFrameEnergy) {
				pThis->m_s32MaxFrameEnergy = pThis->m_ps16FrameEnergy[i%ESR_FRAME_MAXNUM];
			}
		}
	}
	//如果VAD找到了语音结束点，计算最终的语音段能量、能量最大帧
	if((!pThis->m_fSpeechEnd) && (pThis->m_fSpeechStart)) {
		pThis->m_s32FrameEnergySpeech += pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM];
		if(pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM] > pThis->m_s32MaxFrameEnergy) {
			pThis->m_s32MaxFrameEnergy = pThis->m_ps16FrameEnergy[pThis->m_iFrameEnd%ESR_FRAME_MAXNUM];
		}
	}
	//xqxie 信噪比及能量大小检测
}
void EsGetEnhanceFrontResult(PESRFront pThis)
{
	ivInt32 i;
	//xqxie 抢说检测
	if (pThis->m_fSpeechStart) {
		pThis->m_s32FrameEnergyAll /= pThis->m_iFrameEnd;
        pThis->m_s32FrameEnergyAll *= ESR_CUTDET_FRAMES;

		//如果 m_i32FrameEnergyAll/m_i32FrameEnergyAll < ESR_CUTDET_THR，则判断为抢说
		if((ivInt32)pThis->m_s32FrameEnergyDet * ESR_CUTDET_THR_Numerator >= pThis->m_s32FrameEnergyAll*ESR_CUTDET_THR_Demoninator) {
			pThis->m_bSpeechCut = ivTrue;
		}
	}
	//xqxie 抢说检测

	//xqxie 信噪比及能量检测
	if ((pThis->m_fSpeechStart) && (pThis->m_s32FrameEnergySpeech > 0)) {
		//语音段能量更新，将VAD检测到的结束点以及当前语音送入点之间的能量去掉
		for (i=pThis->m_iFrameHead; i<pThis->m_iFrameEnd; i++) {
			pThis->m_s32FrameEnergySpeech -= pThis->m_ps16FrameEnergy[i%ESR_FRAME_MAXNUM];
		}
		pThis->m_s32FrameEnergySil *= (pThis->m_iFrameHead - pThis->m_iFrontSpeechBegin);

		//如果 m_i32FrameEnergySpeech /m_i32FrameEnergySil < ESR_SNRDET_THR，则判断为信噪比过低
		if (pThis->m_s32FrameEnergySpeech * ESR_SNRDET_THR_Demoninator <= (pThis->m_s32FrameEnergySil * ESR_SNRDET_THR_Numerator)) {
			pThis->m_bSpeechLowSNR = ivTrue;
		}

		//如果 m_i32MaxFrameEnergy  < ESR_ENERGYDET_THR，则判断为能量过低
		if (pThis->m_s32MaxFrameEnergy <= ESR_ENERGYDET_THR) {
			pThis->m_bSpeechLowEnergy = ivTrue;
		}
	}
	//xqxie 信噪比及能量检测
}

#endif
