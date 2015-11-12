/*----------------------------------------------+
 |												|
 |	ivMV.h - MV 3 API							|
 |												|
 |		Copyright (c) 1999-2012, iFLYTEK Ltd.	|
 |		All rights reserved.					|
 |												|
 +----------------------------------------------*/

#if !defined(MV_TEAM__2012_02_27__MV__H)
#define MV_TEAM__2012_02_27__MV__H

#include "ivErrorCode.h"
#include "ivDebug.h"
#include "ivDefine.h"

/* Definition of MV parameters and parameter value */
#define  MV_PARAM_SAMPLE_RATE			(1)
	#define MV_SAMPLE_DEFAULT			(ivCPointer)8000
	#define MV_SAMPLE_MIN				(ivCPointer)8000
	#define MV_SAMPLE_MAX				(ivCPointer)16000

#define MV_PARAM_ROLE					(ivUInt32)3        /* 音色 */
	#define MV_ROLE_DEFAULT				MV_ROLE_TWO        
	#define MV_ROLE_ONE					(ivCPointer)1        /* 效果1 2.0 */
	#define MV_ROLE_TWO					(ivCPointer)2        /* 效果2 1.8 */
	#define MV_ROLE_THREE				(ivCPointer)3        /* 效果3 1.5 */
	#define MV_ROLE_FOUR				(ivCPointer)4        /* 效果4 1.0 */
	#define MV_ROLE_FIVE				(ivCPointer)5        /* 效果5 0.72 */
	#define MV_ROLE_SIX					(ivCPointer)6        /* 效果6 0.67 */
	#define MV_ROLE_SEVEN				(ivCPointer)7        /* 效果7 0.55 */
/*
*	Interface
*/
#ifndef	 _DEBUG
#define  MVCreate								 MV31234789gh2asdfjeisdkfjieusdhui2h
#define  MVDestroy								 MV31234789gh2asdfje1234568d6ieusd2h
#define  MVAppendAudioData						 MV3D3055C9368DE48229F61A8802CE608DB
#define  MVStart								 MV300494118753C43628F7E5B2D0918239D
#define  MVStop									 MV312347s74cuywdh62gsjdibbc4568d6ie
#define  MVSetParam								 MV31234789gasdoieyr8asdf1234568d6ie
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Create MV object */
ivStatus								/* Returned Error Info */
ivCall MVCreate(
		ivPointer		pMVObj,			/* [Out] To Receive the MV object handle */
		ivSize ivPtr	pnMVObjSize,
		ivCPointer		pCUser			/* [In] Describe the user's OS and Interface */
	);

/* Destroy MV object */
void
ivCall MVDestroy(ivHandle phMVObj);			/* The MV object handle */


/* Append Audio data to the MV object,In general, Call this function in record thread */
ivStatus								/* Returned Error Info */
ivCall MVAppendAudioData(
		ivPointer		pMVObj,			/* The MV object handle */
		ivCPointer	 	pAudioData,		/* [In] Input Audio data buffer */
		ivInt32			nSamples		/* Specifies the length, in samples, of Audio data */
	);

/* Start process */
ivStatus								/* Returned Error Info */
ivCall MVStart(ivPointer pMVObj);			/* The MV object handle */

/* Stop process */
ivStatus								/* Returned Error Info */
ivCall MVStop(ivPointer pMVObj);			/* The MV object handle */


ivStatus								/* Returned Error Info */
ivCall MVSetParam(
		ivPointer		pMVObj,			/* The MV object handle */
		ivUInt32		nParamID,		/* Parameter ID */
		ivCPointer 		nParamValue		/* Parameter Value */
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(MV_TEAM__2012_02_27__MV__H) */
