/* ***************************************************************************# 
* 文件名      ：ivEsrFFTW.h
* 文件功能    ：Fixed-point FFTW (Fast Fourier Transform In The West). 
* 作者        ：djqu
* 创建时间    ：2013.05.10
* 项目名称    ：AitalkMini 5.0
* 备注        ：http://www.fftw.org/
*---------------------------------------------------------------------# 
* 历史记录：  
* 日期        作者        备注 
*---------------------------------------------------------------------# 
**************************************************************************** */ 

#ifndef ES_TEAM__2013_05_14__ESRFFT__H
#define ES_TEAM__2013_05_14__ESRFFT__H

#include "EsFFTConst.h"

#define ESR_EXPONENT_FFT     (-1)   /* 真实送入进行fft的数据的Q标值 */

typedef struct tagFFTDesc{          /* sizeof(TFFTDesc)= 2 words */
    ivInt16 nRealOut;               /* offset:0 words */
    ivInt16 nImageOut;              /* offset:1 words */
}TFFTDesc, ivPtr PFFTDesc;

#ifdef __cplusplus
extern "C"{
#endif

#if !MINI5_USE_FFTW
ivUInt16 EsFixedFFTCore(PFFTDesc p, ivInt16 nQ);

#else
#define FMUL(a, b)              (((a) * (b))>>15)
#define FMUL_Q(a, b, Q)         (((a) * (b))>>(15-(Q)))
#define FMA_Q(a, b, c, Q)       ((((a) * (b)) + (c))>>(15-(Q)))
#define FMS(a, b, c)            ((((a) * (b)) - (c))>>15)
#define FMS_Q(a, b, c, Q)       ((((a) * (b)) - (c))>>(15-(Q)))
#define FNMA(a, b, c)           ((- (((a) * (b)) + (c)))>>15)
#define FNMA_Q(a, b, c, Q)      ((- (((a) * (b)) + (c)))>>(15-(Q)))
#define FNMS_Q(a, b, c, Q)      (((c) - ((a) * (b)))>>(15-(Q)))

#if (512 == TRANSFORM_FFTNUM_DEF)
void EsRealFFT512Core(PFFTDesc p, ivInt16 nQ);
#elif (256 == TRANSFORM_FFTNUM_DEF)
void EsRealFFT256Core(PFFTDesc p, ivInt16 nQ);
#else
    #error  "Must define TRANSFORM_FFTNUM_DEF, 256, or 512"
#endif

#endif /* !MINI5_USE_FFTW */

#ifdef __cplusplus
    }  /* extern "C"{ */
#endif

#endif /* ES_TEAM__2013_05_14__ESRFFT__H */
