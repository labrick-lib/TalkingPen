/* ***************************************************************************# 
* 文件名      ：EsFFTConst.h
* 文件功能    ：Fixed-point FFTW (Fast Fourier Transform In The West). 
* 作者        ：djqu
* 创建时间    ：2013.05.10
* 项目名称    ：AitalkMini 5.0
* 备注        ：http://www.fftw.org/
*---------------------------------------------------------------------# 
* 历史记录：  
* 日期        作者        备注 
**************************************************************************** */ 

#ifndef ES_TEAM__2013_05_14__ESRFFTCONST__H
#define ES_TEAM__2013_05_14__ESRFFTCONST__H

#include "EsFrontParam.h"
#include "EsKernel.h"

#if (8000 == MINI5_SAMPLERATE)
    #define TRANSFORM_FFTNUM_DEF            (256)   /* FFT Number*/
    #define TRANSFORM_HALFFFTNUM_DEF        (128)   /* Half FFT Number*/
    #define TRANSFORM_QUARTFFTNUM_DEF       (64)    /* Quarter FFT Number*/
    #if !MINI5_USE_FFTW
        #define FFT_STEP_NUM                (7)
    #else
        #define FFTW_STEP_Q                 (6)     /* And right shift one bit additionally,real nQ = 4 */
        #define FFTW_BLOCK_Q                (9)
        #define FFTW_OUTPUT_RIGHTSHIFT_BIT  (7)
    #endif /* #if !MINI5_USE_FFTW */
#elif (16000 == MINI5_SAMPLERATE)
    #define TRANSFORM_FFTNUM_DEF            (512)   /* FFT Number*/
    #define TRANSFORM_HALFFFTNUM_DEF        (256)   /* Half FFT Number*/
    #define TRANSFORM_QUARTFFTNUM_DEF       (128)   /* Quarter FFT Number*/
    #if !MINI5_USE_FFTW
        #define FFT_STEP_NUM                (8) 
    #else
        #define FFTW_STEP_Q                 (6)     /* And right shift two bits additionally, real nQ = 4 */
        #define FFTW_BLOCK_Q                (9)
        #define FFTW_OUTPUT_RIGHTSHIFT_BIT  (7)
    #endif /* #if !MINI5_USE_FFTW */
#else
#error "Audio Sample Rate NOT support!"
#endif

#if (8000 == MINI5_SAMPLERATE)
    #if !MINI5_USE_FFTW
        /* Cosine table */
        extern ivConst ivInt16 g_pwFFTCosTable[192];
    #else
        extern ivConst ivInt16 g_fCosTab[256];
    #endif /* #if !MINI5_USE_FFTW */

    /* Bit reverse table */
    extern ivConst ivUInt16 g_ucReverseTable[100]; /* ivUInt8 */

#elif (16000 == MINI5_SAMPLERATE)
     #if !MINI5_USE_FFTW
        /* Cosine table */
        extern ivConst ivInt16 g_pwFFTCosTable[384];
    #else
        extern ivConst ivInt16 g_fCosTab[512];
    #endif /* #if !MINI5_USE_FFTW */

    /* Bit reverse table */
    extern ivConst ivUInt16 g_ucReverseTable[200]; /* ivUInt8 */
#endif

#endif /* ES_TEAM__2013_05_14__ESRFFTCONST__H */

