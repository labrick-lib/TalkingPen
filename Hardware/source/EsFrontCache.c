/**************************************************************
ESTransform   version:  1.0   ·  date: 08/13/2007
-------------------------------------------------------------
Transform PCM data to feature.
-------------------------------------------------------------
Copyright (C) 2007 - All Rights Reserved
***************************************************************
Sheng Chen
**************************************************************/

#include "EsKernel.h"
#include "EsEngine.h"
#include "EsFront.h"
#include "EsFFT.h"

#if MINI5_ENHANCE_VAD
ivUInt16 g_bEnhanceVAD = ES_DEFAULT_ENHANCEVAD;

# if (8000 == MINI5_SAMPLERATE)
/* 新的6阶带通滤波系数 20100916 */
ivStatic ivInt16 ivConst S1 = 12891;		/* (ivInt16)(0.39341182399519459*(1<<15)); */
ivStatic ivInt16 ivConst A21 = -31764;		/* (ivInt16)(-0.9693549790935525*(1<<15)); */
ivStatic ivInt16 ivConst A31 = 24458;		/* (ivInt16)(0.74638924015844843*(1<<15)); */
ivStatic ivInt16 ivConst B21 = -3179;		/* (ivInt16)(-0.097000326686653232*(1<<15)); */
ivStatic ivInt16 ivConst S2 = 12891;		/* (ivInt16)(0.39341182399519459*(1<<15)); */
ivStatic ivInt16 ivConst A22 = -30750;		/* (ivInt16)(-1.8768386700639974*(1<<14));	*/
ivStatic ivInt16 ivConst A32 = 30316;		/* (ivInt16)(0.92516533133150081*(1<<15)); */
ivStatic ivInt16 ivConst B22 = -32506;		/* (ivInt16)(-1.9840335524505093*(1<<14)); */	
ivStatic ivInt16 ivConst S3 = 13380;		/* (ivInt16)(0.40832565593520037*(1<<15)); */
ivStatic ivInt16 ivConst A23 = -22712;		/* (ivInt16)(-1.3862131909753999*(1<<14)); */
ivStatic ivInt16 ivConst A33 = 18486;		/* (ivInt16)(0.56413798685087402*(1<<15)); */
# elif (16000 == MINI5_SAMPLERATE)
/* 6阶带通滤波系数 20110801 */
ivStatic ivInt16  ivConst S1 = 11321;		/* (ivInt16)(0.34555402477922426*(1<<15)) */ /* (ivInt16)(0.39341182399519459*(1<<15)); */
ivStatic ivInt16  ivConst A21 = -27340;		/* (ivInt16)(-1.6687136239648064*(1<<14)) */ /* (ivInt16)(-0.9693549790935525*(1<<15)); */
ivStatic ivInt16  ivConst A31 = 28784;		/* (ivInt16)(0.878426069630041*(1<<15)) */ /* (ivInt16)(0.74638924015844843*(1<<15)); */
ivStatic ivInt16  ivConst B21 = -23400;		/* (ivInt16)(-1.4282436927396134*(1<<14)) */ /* (ivInt16)(-0.097000326686653232*(1<<15)); */
ivStatic ivInt16  ivConst S2 = 11323;		/* (ivInt16)(0.34555402477922426*(1<<15)) */ /* (ivInt16)(0.39341182399519459*(1<<15)); */
ivStatic ivInt16  ivConst A22 = -23022;		/* (ivInt16)(-1.9545078670513414*(1<<14)) */ /* (ivInt16)(-1.8768386700639974*(1<<14));	*/
ivStatic ivInt16  ivConst A32 = 31713;		/* (ivInt16)(0.96780664350376*(1<<15)) */ /* (ivInt16)(0.92516533133150081*(1<<15)); */
ivStatic ivInt16  ivConst B22 = -23689;		/* (ivInt16)(-1.9951980877160387*(1<<14)) */ /* (ivInt16)(-1.9840335524505093*(1<<14)); */	
ivStatic ivInt16  ivConst S3 = 7108;			/* (ivInt16)(0.21692804252521097*(1<<15)) */ /* (ivInt16)(0.40832565593520037*(1<<15)); */
ivStatic ivInt16  ivConst A23 = -28641;		/* (ivInt16)(-1.7481292971647813*(1<<14)) */ /* (ivInt16)(-1.3862131909753999*(1<<14)); */
ivStatic ivInt16  ivConst A33 = 26160;		/* (ivInt16)(0.79834087907743889*(1<<15));*/ /* (ivInt16)(0.56413798685087402*(1<<15)); */
# else
# endif /* MINI5_SAMPLERATE */

#endif /* MINI5_ENHANCE_VAD */

#if ESR_CPECTRAL_SUB
void EsrFront_SpectEst(PESRFront pThis,ivBool bSpeech);
void ivCall EsFrontEnhanceCalcFFT(PESRFront pThis, ivPInt16 pFFTQ);
void ivCall EsFrontEnhanceGetEnergy(PESRFront pThis, ivPInt16 ps16FrameEnergy);

void EsrFront_SpectEst(PESRFront pThis,ivBool bSpeech)
{
    ivInt16 i;
    ivInt32 nNoiseSpec,nSpecAvg;
    ivInt16 nEstQ,s16Exponent,s16SpecCurQ,s16SpecPreQ;
    ivPUInt16 pFFTSpecCur,pFFTSpecPre;

    nEstQ = -pThis->m_nFFTSpecAvgQ;
    ivAssert(nEstQ>=0 && nEstQ<=15);

    /* speech*/
    pFFTSpecCur = pThis->m_pFFTSpecCur;
    s16SpecCurQ = pThis->m_nFFTSpecCurQ;
    pFFTSpecPre = pThis->m_pFFTSpecPre;
    s16SpecPreQ = pThis->m_nFFTSpecPreQ;

#if AVG_RSHIFT
    s16Exponent = 0;
    s16Exponent = ivMin(s16Exponent,pThis->m_nFFTSpecCurQ);
    s16Exponent = ivMin(s16Exponent,pThis->m_nFFTSpecPreQ);
    s16SpecPreQ = s16SpecPreQ - s16Exponent +9;
    s16SpecCurQ = s16SpecCurQ - s16Exponent +9;
#endif

    for (i= 0;i<=TRANSFORM_HALFFFTNUM_DEF;i++){
        ivInt32 nSpecSub;
        nNoiseSpec = g_nNoiseInitValue[i];
        nSpecAvg = pThis->m_ppFFTSpecAvg[i]<<nEstQ;	/*Q0*/
        ivAssert(((ivUInt32)pThis->m_ppFFTSpecAvg[i]<<nEstQ) <=IV_MAX_INT32);

#if AVG_RSHIFT
        pThis->m_ppFFTSpecAvg[i] = ((((ivInt32)pFFTSpecPre[i])*171)>>s16SpecPreQ) + ((((ivInt32)pFFTSpecCur[i])*171)>>s16SpecCurQ);//Avg+=pThis->m_ppFFTSpec[0]/3
        IV_CHECK_SATURATE16((((((ivInt32)pFFTSpecPre[i])*171)>>s16SpecPreQ) + ((((ivInt32)pFFTSpecCur[i])*171)>>s16SpecCurQ)));
#endif
        nSpecSub = ivMax(nSpecAvg - ENHANCE_DEF*nNoiseSpec,0);
        /*nSpecSub = ivMax(nSpecAvg - (ivInt32)(1.5*nNoiseSpec),0);*/
        pFFTSpecPre[i] = nSpecSub>>nEstQ;	/*-Q:nEst*/
    }
    pThis->m_nFFTSpecAvgQ = s16Exponent;
    pThis->m_nFFTSpecPreQ = -nEstQ;
}

void ivCall EsFrontEnhanceCalcFFT(PESRFront pThis, ivPInt16 pFFTQ)
{
    /* Need cache */
    ivInt32 s32Register1;
    /* Need cache */
    ivInt32 s32Register2 = DOUBLE_Q15;
    ivInt16 i;
    ivInt16 s16Exponent;
    ivInt16 s16TmpExponent = 16;
    PFFTDesc pFFTDesc;
    ivInt16 nData;
    ivUInt16 iReverse;

    pFFTDesc = (PFFTDesc)pThis->m_pFrameCache;
    ivMemZero(pThis->m_pFrameCache, (TRANSFORM_FFTNUM_DEF+2)*sizeof(ivInt16));
    for(i = 0; i < ESR_FRAME_SIZE; i += 2 ) {
        iReverse = g_ucReverseTable[i>>1];

        /* -------------i-------------------------- */
        s32Register1 = pThis->m_pPCMBuffer[((pThis->m_iPCMStart + i) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)))];
        /* int16 * int16 */
        s32Register1 = s32Register1*(ivInt32)(g_sHamWindow[i]); /* Q16 */	
        s32Register2 |= s32Register1 > 0 ? s32Register1 : -s32Register1;

        nData = EsSatInt16(s32Register1>>s16TmpExponent);  /* Q(16-s16Exponent)  s16TmpExponent=16 */
        ivAssert((s32Register1>>16)>=IV_MIN_INT16 && (s32Register1>>16)<=IV_MAX_INT16);

#if !MINI5_USE_FFTW
        if(i < TRANSFORM_HALFFFTNUM_DEF){
            pFFTDesc[iReverse].nRealOut = pFFTDesc[iReverse].nRealOut + nData; 
            pFFTDesc[iReverse + 1].nRealOut = pFFTDesc[iReverse + 1].nRealOut + nData; 
        }
        else{
            pFFTDesc[iReverse].nRealOut = pFFTDesc[iReverse].nRealOut - nData; 
            pFFTDesc[iReverse - 1].nRealOut = pFFTDesc[iReverse - 1].nRealOut + nData; 
        }
#else
        pFFTDesc[iReverse].nRealOut = nData;
#endif /* !MINI5_USE_FFTW */

        /* ---------------------------i+1 -------------------------- */
        s32Register1 = pThis->m_pPCMBuffer[((pThis->m_iPCMStart + i + 1) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)))];

        /* int16 * int16 */
        s32Register1 = s32Register1*(ivInt32)(g_sHamWindow[i+1]); /* Q16 */
        s32Register2 |= s32Register1 > 0 ? s32Register1 : -s32Register1;

        nData = EsSatInt16(s32Register1>>s16TmpExponent);  /* Q(16-s16Exponent)  s16TmpExponent=16 */
        ivAssert((s32Register1>>16)>=IV_MIN_INT16 && (s32Register1>>16)<=IV_MAX_INT16);
#if !MINI5_USE_FFTW
        if(i < TRANSFORM_HALFFFTNUM_DEF){
            pFFTDesc[iReverse].nImageOut =pFFTDesc[iReverse].nImageOut + nData; 
            pFFTDesc[iReverse + 1].nImageOut =pFFTDesc[iReverse + 1].nImageOut + nData; 
        }
        else{
            pFFTDesc[iReverse].nImageOut =pFFTDesc[iReverse].nImageOut - nData;
            pFFTDesc[iReverse - 1].nImageOut =pFFTDesc[iReverse - 1].nImageOut + nData;
        }
#else
        pFFTDesc[iReverse].nImageOut = nData;
#endif /* !MINI5_USE_FFTW */
    }

    /* Left shift bits number */
    s16Exponent = es_norm_l(s32Register2) - 2;

#if !MINI5_USE_FFTW
    s16Exponent = EsFixedFFTCore(pFFTDesc, s16Exponent);
#else
# if ( 8000 == MINI5_SAMPLERATE)
    EsRealFFT256Core(pFFTDesc, s16Exponent + 2);
# elif ( 16000 == MINI5_SAMPLERATE)
    EsRealFFT512Core(pFFTDesc, s16Exponent + 2);
# endif  /* 8000 == MINI5_SAMPLERATE */

    s16Exponent = s16Exponent + 2 - FFTW_OUTPUT_RIGHTSHIFT_BIT;  /* Q0 */
#endif  /* !MINI5_USE_FFTW */

    *pFFTQ = s16Exponent;
}

#if ESR_ENERGY_LOG
void ivCall EsFrontEnhanceGetEnergy(PESRFront pThis, ivPInt16 ps16FrameEnergy)
#else
void ivCall EsFrontEnhanceGetEnergy(PESRFront pThis, ivPInt32 ps16FrameEnergy)
#endif
{
    ivInt16         i;
    ivInt16         s16Exponent, s16ExponentTmp;
    ivInt32         s32Energy;
    PFFTDesc        pFFTDesc;
    ivInt32         n32Data;
    ivPUInt16       pFFTSpecTmp;

    EsFrontEnhanceCalcFFT(pThis, &s16Exponent);
    pFFTDesc = (PFFTDesc)pThis->m_pFrameCache;

    ivAssert(s16Exponent <= 0);
    pFFTSpecTmp = pThis->m_pFFTSpecPre;
    pThis->m_pFFTSpecPre = pThis->m_pFFTSpecCur;
    pThis->m_pFFTSpecCur = pFFTSpecTmp;
    pThis->m_nFFTSpecPreQ = pThis->m_nFFTSpecCurQ;
    pThis->m_nFFTSpecCurQ = s16Exponent;

    s16ExponentTmp = s16Exponent - pThis->m_nFFTSpecAvgQ;
    if (s16ExponentTmp>0){
        for (i=0;i<=TRANSFORM_HALFFFTNUM_DEF;i++){

            /* 幅度谱,FFTFRAME_MAXNUM每一帧的Q不一样！*/
            n32Data = (ivInt32)pFFTDesc[i].nImageOut*pFFTDesc[i].nImageOut;/* Q = 2*m_nFFTSpecQ[0]*/
            n32Data += (ivInt32)pFFTDesc[i].nRealOut*pFFTDesc[i].nRealOut; /* Q = 2*m_nFFTSpecQ[0]*/

            pFFTSpecTmp[i] = CalSqrtRecip32(n32Data);
            ivAssert(pFFTSpecTmp[i]<=IV_MAX_UINT16);
#if AVG_RSHIFT
            IV_CHECK_SATURATE16((ivInt32)pThis->m_ppFFTSpecAvg[i]+(((ivInt32)pFFTSpecTmp[i]*171)>>(s16ExponentTmp+9)));
            pThis->m_ppFFTSpecAvg[i] += (((ivInt32)pFFTSpecTmp[i]*171)>>(s16ExponentTmp+9));
#endif
        }
    } 
    else{
        s16ExponentTmp = -s16ExponentTmp;
        for (i=0;i<=TRANSFORM_HALFFFTNUM_DEF;i++){

            /* 幅度谱,每一帧的Q不一样！*/
            n32Data = (ivInt32)pFFTDesc[i].nImageOut*pFFTDesc[i].nImageOut;/* 2*nFFTQ*/
            n32Data += (ivInt32)pFFTDesc[i].nRealOut*pFFTDesc[i].nRealOut; /* 2*nFFTQ*/
            pFFTSpecTmp[i] = CalSqrtRecip32(n32Data);/* nFFTQ*/
            ivAssert(pFFTSpecTmp[i]<=IV_MAX_UINT16);		
#if AVG_RSHIFT
            IV_CHECK_SATURATE16((((ivInt32)pThis->m_ppFFTSpecAvg[i])>>s16ExponentTmp) + (((ivInt32)pFFTSpecTmp[i]*171)>>9));
            pThis->m_ppFFTSpecAvg[i] = (((ivInt32)pThis->m_ppFFTSpecAvg[i])>>s16ExponentTmp) + (((ivInt32)pFFTSpecTmp[i]*171)>>9);
#endif
        }
    }
#if AVG_RSHIFT
    pThis->m_nFFTSpecAvgQ = ivMin(pThis->m_nFFTSpecAvgQ,s16Exponent);
#endif

    if (pThis->m_iMFCC >= FFTFRAME_MAXNUM+1) {
        EsrFront_SpectEst(pThis,ivTrue);

        s32Energy = 0;
        for (i = 0; i < TRANSFORM_HALFFFTNUM_DEF; i++){
            n32Data = (ivInt32)pThis->m_pFFTSpecPre[i]*(ivInt32)pThis->m_pFFTSpecPre[i];/* Q = 2*m_nFFTSpecQ[0]*/

            s32Energy += n32Data >> 10;
        }
        s32Energy /= TRANSFORM_HALFFFTNUM_DEF;
        ivAssert(s16Exponent <= 0);
        s32Energy = s32Energy << (- 2*s16Exponent);

#if ESR_ENERGY_LOG
        *ps16FrameEnergy = simple_table_ln((ivUInt32)(s32Energy + 390), 0) + ESR_MATH_10LN2_Q10;
#else
        *ps16FrameEnergy = s32Energy;
#endif /* ESR_ENERGY_LOG */
        pThis->m_iFrameEnd++;
    }
}
#endif /* ESR_CPECTRAL_SUB */

void EsFrontFilterBankAndToMFCC(PESRFront pThis, ivInt16 s16Exponent, ivPInt16 ps16MFCC);

void EsFrontFilterBankAndToMFCC(PESRFront pThis, ivInt16 s16Exponent, ivPInt16 ps16MFCC)
{
    ivInt32 dwTemp, dwTemp2;
    ivInt16  i,j,k;
    ivInt16 ps16FilterBank[TRANSFORM_CHANSNUM_DEF];
    PFFTDesc pFFTDesc;

    pFFTDesc = (PFFTDesc)pThis->m_pFrameCache;

#if !MINI5_USE_FFTW
    s16Exponent = s16Exponent - ESR_EXPONENT_FFT;
    s16Exponent = EsFixedFFTCore(pFFTDesc, s16Exponent);
#else
    s16Exponent = s16Exponent + 2;
    ivAssert(s16Exponent >= 0);
# if ( 8000 == MINI5_SAMPLERATE)
    EsRealFFT256Core(pFFTDesc, s16Exponent);
# elif ( 16000 == MINI5_SAMPLERATE)
    EsRealFFT512Core(pFFTDesc, s16Exponent);
# endif  /* 8000 == MINI5_SAMPLERATE */

    s16Exponent = s16Exponent - FFTW_OUTPUT_RIGHTSHIFT_BIT;  /* 真实Q */

#endif  /* !MINI5_USE_FFTW */

    s16Exponent = s16Exponent - 1;
    k = 0;
    for(i=0; i< TRANSFORM_CHANSNUM_DEF; i++) {
        ivUInt32 s32TmpFilterBank = 0;
        ivInt16 iStartFFT = g_pwFilterbankInfo[k++];
        ivInt16 nFFTCnt = g_pwFilterbankInfo[k++];
        for(j=0; j<nFFTCnt; j++) {
            /* 乘法 int16*int16 */
            dwTemp = (ivInt32)(pFFTDesc[iStartFFT+j].nRealOut) * (ivInt32)(pFFTDesc[iStartFFT+j].nRealOut);
            dwTemp += (ivInt32)(pFFTDesc[iStartFFT+j].nImageOut) * (ivInt32)(pFFTDesc[iStartFFT+j].nImageOut);

            /* 乘法 int32*int16 */
            dwTemp2 = L_mls(dwTemp, g_pwFilterbankInfo[k]); /* pWeight: Q15 */
            k++;

            s32TmpFilterBank += dwTemp2;
        }
        ps16FilterBank[i] = simple_table_ln((ivUInt32)s32TmpFilterBank,2*s16Exponent); /* 5.10 */
    }

    for(i = 0;i < TRANSFORM_CEPSNUM_DEF; i++) {
        dwTemp = 0;
        for(j = 0;j < TRANSFORM_CHANSNUM_DEF; j++) {
            /* 乘法 int16*int16 */
            dwTemp += (ivInt32)(ps16FilterBank[j]) * (ivInt32)(g_wFBToMFCCTable[TRANSFORM_CHANSNUM_DEF * i + j]); /* 5.10 * 2.13  -> .23 */
        }

        ps16MFCC[i] = (ivInt16)(dwTemp>>(23-ESR_Q_MFCC_MANUAL)); /* .23 -> ESR_Q_MFCC_MANUAL */
        ivAssert((dwTemp>>17) >= -32768 && (dwTemp>>17) <= 32767);
    }

    /* C0 */
    dwTemp = 0;
    for(i=0; i<TRANSFORM_CHANSNUM_DEF; ++i) {
        dwTemp += ps16FilterBank[i]; /* Q10 */
    }

    /* 9.6 */
    ivAssert(ESR_Q_MFCC_MANUAL < 10);
    ps16MFCC[TRANSFORM_CEPSNUM_DEF] = (ivInt16)(L_mls(dwTemp, 9459)>>(10-ESR_Q_MFCC_MANUAL));	/* Q0.15 0.288675 = 9459 */	
    ivAssert((dwTemp*0.288675/(1<<ESR_Q_MFCC_MANUAL)) >= -32768 && (dwTemp*0.288675/(1<<ESR_Q_MFCC_MANUAL)) <= 32767);
}

#if 0
ivInt16 g_pData[1000];
int g_nData = 0;
#include <stdio.h>
#endif

#if MINI5_ENHANCE_VAD
ivInt32 ESFrontFilter(PESRFront pThis)
{
	ivPInt16 pPcmBuffer;
	ivInt32 nTmpZ11, nTmpZ21, nTmpZ31, nOut;
	ivInt32 Z11, Z12, Z21, Z22, Z31, Z32;
	ivInt16 i, nData;
	ivInt32 s32Energy = 0;

	pPcmBuffer = pThis->m_pPCMBuffer;

	Z11 = pThis->Z11;
	Z12 = pThis->Z12;
	Z21 = pThis->Z21;
	Z22 = pThis->Z22;
	Z31 = pThis->Z31;
	Z32 = pThis->Z32;

	/* 要存一帧200个点的能量,为了节约内存,取帧长和帧移的最大公约数25, 25个点的能量加在一起存在一个int32里，则只需要8个int32来存200个点的能量 */
	if(0 == pThis->m_iFrameEnd){
		i = 0;
		ivMemZero(pThis->m_ps32HiPassEnergy, sizeof(ivInt32)*ESR_HIPASSENERGY_NUM);
	}
	else{
		i = ESR_FRAME_SIZE - ESR_FRAME_STEP;
		pThis->m_ps32HiPassEnergy[pThis->m_iHiPassEnergy/ESR_GCD_FRAMESIZE_FRAME_STEP] = 0;
		pThis->m_ps32HiPassEnergy[(pThis->m_iHiPassEnergy/ESR_GCD_FRAMESIZE_FRAME_STEP + 1)%ESR_HIPASSENERGY_NUM] = 0;
	}/* laoyu added */

	for (; i < ESR_FRAME_SIZE; i ++) {
		/* s32Register1 = pPcmBuffer[(g_iPCMStart + i) & 0xFF]; */
		/* nTmpZ11 = L_mls(s32Register1, S1) - L_mls(Z11, A21) - L_mls(Z12, A31); */
		nData = pPcmBuffer[(pThis->m_iPCMStart + i) & (ESR_PCMBUFFER_SIZE-1)];					
# if (8000 == MINI5_SAMPLERATE)
		nTmpZ11 = (((ivInt32)nData*(ivInt32)S1)>>15) - L_mls(Z11, A21) - L_mls(Z12, A31);

		nOut = nTmpZ11 + L_mls(Z11, B21) + Z12;
# elif (16000 == MINI5_SAMPLERATE)
		nTmpZ11 = (((ivInt32)nData*(ivInt32)S1)>>15) - (L_mls(Z11, A21)<<1) - L_mls(Z12, A31);
		nOut = nTmpZ11 + (L_mls(Z11, B21)<<1) + Z12;
# else
# endif
		Z12 = Z11;
		Z11 = nTmpZ11;
		/* ivAssert(nTmpZ11 >= -32768 && nTmpZ11 <= 32767); */
		/* ivAssert(nOut >= -32768*16 && nOut <= 32767*16); */

		nTmpZ21 = L_mls(nOut, S2) - (L_mls(Z21, A22)<<1) - L_mls(Z22, A32);
		nOut = nTmpZ21 + (L_mls(Z21, B22)<<1) + Z22;
		Z22 = Z21;
		Z21 = nTmpZ21;
		/* ivAssert(nTmpZ21 >= -32768 && nTmpZ21 <= 32767); */
		/* ivAssert(nOut >= -32768*16 && nOut <= 32767*16); */

		nTmpZ31 = L_mls(nOut, S3) - (L_mls(Z31, A23)<<1) - L_mls(Z32, A33);
		nOut = nTmpZ31 - Z32;
		Z32 = Z31; 
		Z31 = nTmpZ31;
		/* ivAssert(nTmpZ31 >= -32768 && nTmpZ31 <= 32767); */
		/* ivAssert(nOut >= -32768 && nOut <= 32767); */

		pThis->m_ps32HiPassEnergy[pThis->m_iHiPassEnergy/ESR_GCD_FRAMESIZE_FRAME_STEP] += (nOut * nOut + 63)>>7;	

		pThis->m_iHiPassEnergy++;
		if(pThis->m_iHiPassEnergy >= ESR_FRAME_SIZE) {
			pThis->m_iHiPassEnergy -= ESR_FRAME_SIZE;
		}

# if 0
		g_pData[g_nData++] = (ivInt16)nOut;
		if(g_nData == 1000){
			FILE *fp;
			fp = fopen("E:\\XX.pcm", "ab");
			fwrite(g_pData, 1000*2, 1, fp);
			g_nData = 0;
			fclose(fp);
		}
# endif
	}
	pThis->Z11 = Z11;
	pThis->Z12 = Z12;
	pThis->Z21 = Z21;
	pThis->Z22 = Z22;
	pThis->Z31 = Z31;
	pThis->Z32 = Z32;

	for (i = 0; i < ESR_HIPASSENERGY_NUM; i++) {
		s32Energy += pThis->m_ps32HiPassEnergy[i];
	}

	s32Energy >>= 2;

	s32Energy = ivMax(/*nFloor*/391, s32Energy);

	return s32Energy;
}
#endif /* MINI5_ENHANCE_VAD */

#if ESR_ENERGY_LOG
void ivCall EsFrontCalcStaticMFCC(PESRFront pThis, ivPInt16 ps16MFCC, ivPInt16 ps16FrameEnergy)
#else
void ivCall EsFrontCalcStaticMFCC(PESRFront pThis, ivPInt16 ps16MFCC, ivPInt32 ps16FrameEnergy)
#endif
{
    /* Need cache */
    ivInt32 s32Mean;
    /* Need cache */
    ivInt32 s32Energy = 0;
    /* Need cache */
    ivInt32 s32Register1,s32Register2;
    /* Need cache */
    ivInt32 s32Register3 = DOUBLE_Q15;
    ivInt16 i;
    ivInt16 s16Exponent;

    ivInt16 s16TmpExponent = 16;
    ivUInt16 iCurPcm;
    ivPInt16 pPcmBuffer;
    PFFTDesc pFFTDesc;
    ivInt16 nData;
    ivUInt16 iReverse;

    /* ivInt32 s32MeanSum = 0; */

    pPcmBuffer = pThis->m_pPCMBuffer;
    pFFTDesc = (PFFTDesc)pThis->m_pFrameCache;
    s32Mean = (pThis->m_s32MeanSum2 >> ESR_PCMBUFFER_BIT);

    ivAssert(-32768 <= s32Mean && s32Mean <=32767);

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
    if(g_bEnhanceVAD){
        s32Energy = ESFrontFilter(pThis);	
    }
#endif /* !ESR_CPECTRAL_SUB */

    /* ----------------------------20091113---------------------------------------- */	
    /* 将预处理和倒字节序及第一次蝶形和在一起 20100310 */

    ivMemZero(pThis->m_pFrameCache, (TRANSFORM_FFTNUM_DEF+2)*sizeof(ivInt16));

    /* Do ZERO-MEAN, energy calculation, PreEmphasize and Haming window */
    iCurPcm = ((pThis->m_iPCMStart - 1) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));
    s32Register1 = pPcmBuffer[iCurPcm] - s32Mean;	
    for(i = 0; i < ESR_FRAME_SIZE; i += 2 ) {
        iReverse = g_ucReverseTable[i>>1];

        /* -------------i-------------------------- */
        s32Register2 = s32Register1; 
        iCurPcm = ((pThis->m_iPCMStart + i) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));
        s32Register1 = pPcmBuffer[iCurPcm] - s32Mean;

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
        if(!g_bEnhanceVAD){
#endif
            /* Calculate energy */
            /* 可以保证是int16 * int16，因为录音精度是12bit，送进来的数据转为15bit的即可 */
            s32Energy += ((s32Register2*s32Register2)>>15);
#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
        }
#endif /* !ESR_CPECTRAL_SUB */

        /* PreEmphasise */		
        ivAssert(abs(((s32Register2-s32Register1)>>1) + (s32Register1>>6)) <= (1<<15));
        s32Register2 = ((s32Register1-s32Register2)>>1) + (s32Register2>>6);   /* Q(-1),保证是int16范围内的 */

        /* int16 * int16 */
        s32Register2 = s32Register2*(ivInt32)(g_sHamWindow[i]); /* Q(-1)*Q16 -> Q15 */

        /* Q14 */		
        s32Register3 |= s32Register2 > 0 ? s32Register2 : -s32Register2;

        nData = EsSatInt16(s32Register2>>s16TmpExponent);  /* Q(15-s16Exponent)  s16TmpExponent=16 */

#if !MINI5_USE_FFTW
        if(i < TRANSFORM_HALFFFTNUM_DEF){
            pFFTDesc[iReverse].nRealOut = pFFTDesc[iReverse].nRealOut + nData; 
            pFFTDesc[iReverse + 1].nRealOut = pFFTDesc[iReverse + 1].nRealOut + nData; 
        }
        else{
            pFFTDesc[iReverse].nRealOut = pFFTDesc[iReverse].nRealOut - nData; 
            pFFTDesc[iReverse - 1].nRealOut = pFFTDesc[iReverse - 1].nRealOut + nData; 
        }
#else
        pFFTDesc[iReverse].nRealOut = nData;
#endif /* !MINI5_USE_FFTW */

        /* ---------------------------i+1 -------------------------- */
        s32Register2 = s32Register1;
        iCurPcm = ((pThis->m_iPCMStart + i + 1) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));
        s32Register1 = pPcmBuffer[iCurPcm] - s32Mean;

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
        if(!g_bEnhanceVAD) {
#endif
            /* Calculate energy */
            /* int16 * int16 */
            s32Energy += ((s32Register2*s32Register2)>>15);

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
        }
#endif /* !ESR_CPECTRAL_SUB */

        /* PreEmphasise */		
        s32Register2 = ((s32Register1-s32Register2)>>1) + (s32Register2>>6);  /* Q(-1),保证是int16范围内的 */

        /* int16 * int16 */
        s32Register2 = s32Register2*(ivInt32)(g_sHamWindow[i+1]); /* Q15 */

        s32Register3 |= s32Register2 > 0 ? s32Register2 : -s32Register2;

        nData = EsSatInt16(s32Register2>>s16TmpExponent);  /* Q(15-s16Exponent)  s16TmpExponent=16 */

#if !MINI5_USE_FFTW
        if(i < TRANSFORM_HALFFFTNUM_DEF){
            pFFTDesc[iReverse].nImageOut =pFFTDesc[iReverse].nImageOut + nData; 
            pFFTDesc[iReverse + 1].nImageOut =pFFTDesc[iReverse + 1].nImageOut + nData; 
        }
        else{
            pFFTDesc[iReverse].nImageOut =pFFTDesc[iReverse].nImageOut - nData;
            pFFTDesc[iReverse - 1].nImageOut =pFFTDesc[iReverse - 1].nImageOut + nData;
        }
#else /* !MINI5_USE_FFTW */
        pFFTDesc[iReverse].nImageOut = nData;
#endif /* !MINI5_USE_FFTW */
    }

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
    if(!g_bEnhanceVAD){
#endif 
        s32Energy <<= 5;

#if !ESR_CPECTRAL_SUB && MINI5_ENHANCE_VAD
    }
#endif /* #if !ESR_CPECTRAL_SUB */

    /* Calculate energy */	
    /*　VAD energy right shift 10 bit to get real energy plus basic energy 400000>>10 */
    /* *ps32FrameEnergy = s32Energy + 390; */
    /* = ln(s32Energy + 390)*2^10 */
#if !ESR_CPECTRAL_SUB
# if ESR_ENERGY_LOG
    *ps16FrameEnergy = simple_table_ln((ivUInt32)(s32Energy + 390), 0) + ESR_MATH_10LN2_Q10; /* 5.10 */
    ivAssert(*ps16FrameEnergy > 0);
# else
    *ps16FrameEnergy = s32Energy;
# endif /* ESR_ENERGY_LOG */
#endif /* #if !ESR_CPECTRAL_SUB */

    /* 更新m_iPCMStart */
    pThis->m_iPCMStart = ((pThis->m_iPCMStart + ESR_FRAME_STEP) & ((ivUInt16)(ESR_PCMBUFFER_SIZE-1)));

    /* Left shift bits number */
    s16Exponent = es_norm_l(s32Register3) - 2;

    /*  Out:ps16MFCC 9.6(C0:11.4) */
    EsFrontFilterBankAndToMFCC(pThis, s16Exponent, ps16MFCC); /* 真实输入的Q是ESR_EXPONENT_FFT, 希望输入的Q是s16Exponent */

#if LOG_DEBUG_INFO
    {
        static FILE *g_fpMFCC = NULL;
        if(NULL == g_fpMFCC)
        {
            g_fpMFCC = fopen("E:\\MFCC_mini5.log", "wb");
        }
        fprintf(g_fpMFCC, "iFrame=%d: ", pThis->m_iFrameEnd);
        for(i=0; i<8; i++)
        {
            fprintf(g_fpMFCC, "%d, ", ps16MFCC[i]);
        }
        fprintf(g_fpMFCC, "\r\n");
        fflush(g_fpMFCC);
    }
#endif
}
