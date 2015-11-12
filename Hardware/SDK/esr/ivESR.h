/*----------------------------------------------+
 |												|
 |	ivESR.h - LowEnd API			|
 |												|
 |		Copyright (c) 1999-2010, iFLYTEK Ltd.	|
 |		All rights reserved.					|
 |												|
 +----------------------------------------------*/

#if !defined(ES_TEAM__2010_09_09__ESR__H)
#define ES_TEAM__2010_09_09__ESR__H


#include "ivDefine.h"
#include "ivPlatform.h"
#include "ivESRErrorCode.h"

/*
*	Interface
*/



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct tagEsrResult{
	ivUInt16	nResultID;			/* Result Cmd ID */
	ivUInt16	nConfidenceScore;	/* Score of Confidence 0-100 */
	ivInt16     nFeedbackGain;     /* the level of feedback */
	ivBool		nSpeechStart;
	ivBool		nSpeechEnd;
}TEsrResult, ivPtr PEsrResult;
typedef TEsrResult ivCPtr PCEsrResult;

/* Definition of ESR parameters and parameter value */
/* Parameter ID for ESR Enhance VAD */
#define ES_PARAM_ENHANCEVAD			(1)
#define ES_ENHANCEVAD_ON			((ivUInt16)1)
#define ES_ENHANCEVAD_OFF			((ivUInt16)0)
#define ES_DEFAULT_ENHANCEVAD		ES_ENHANCEVAD_ON

/* Parameter ID for ESR ambient noise */
#define ES_PARAM_AMBIENTNOISE		(2)
#define ES_AMBIENTNOISE_ON			((ivUInt16)1)
#define ES_AMBIENTNOISE_OFF			((ivUInt16)0)
#define ES_DEFAULT_AMBIENTNOISE		ES_AMBIENTNOISE_ON


/* 该宏设为1,表示采用ESRCreate和ESRSetGrammar二个函数，并且暂时去掉ESRSetParam和ESRSetCmdActive函数功能，减少汇编量 20101214 */
#define ESR_USE_NEW_CREATE_API         (0)

#if ESR_USE_NEW_CREATE_API
ivESRStatus 
ivCall 
ESRCreate(
		  ivPointer		pEsrObj,			/* ESR Object */
		  ivSize ivPtr	pnESRObjSize,		/* [In/Out] Size of ESR object */	
		  ivPointer		pCMNBuf, 
		  ivSize ivPtr	pnCMNSize,
		  ivCPointer	pResource,			/* [In] Resource */		
		  ivCPointer   pResource2			/* [In] Resource:Model HLDA & MeanVar Table */	

		  );

ivESRStatus 
ivCall 
ESRSetGrammar(
			  ivPointer		pEsrObj,		/* ESR Object */
			  ivUInt32		nUsedGrmID		/* [In]Set used GrmID */
			  );

#else
ivESRStatus 
ivCall
ESRCreate(
		ivPointer	 pEsrObj,				/* ESR Object */
		ivSize ivPtr pnESRObjSize,		/* [In/Out] Size of ESR object */
		ivPointer	 pResidentRAM,			/* Resident RAM */
		ivPUInt16    pnResidentRAMSize, /* [In/Out] Size of Resident RAM */
		ivCPointer   pResource,			/* [In] Resource */	
		ivCPointer   pResource2,			/* [In] Resource:Model HLDA & MeanVar Table */	
		ivUInt32     nGrmID               /*  [In] Grammar ID */
		);

ivESRStatus 
ivCall 
ESRSetParam(
			ivPointer pEsrObj,		/* ESR Object */
			ivUInt32 nParamID,		/* Parameter ID */
			ivUInt16 nParamValue	/* Parameter Value */
			);

ivESRStatus 
ivCall 
ESRSetCmdActive(
				ivPointer	pEsrObj,  /* ESR Object */ 
				ivUInt16	nCmdID,  /* [In] Cmd ID */
				ivBool		bActive   /* [In] Set cmd state. ivFalse or ivTrue  */
				);
#endif

ivESRStatus 
ivCall 
ESRReset(ivPointer pEsrObj);  /* Reset ESR Object */

ivESRStatus
ivCall
ESRRunStep(
		ivPointer	pEsrObj,		/* ESR Object */		
		PCEsrResult ivPtr ppResult  /* [Out] To Receive Recognize result */
		);

ivESRStatus 
ivCall 
ESRAppendData(
			   ivPointer		pEsrObj,	/* ESR Object */
			   ivCPointer		pData,		/* [In] Pointer to the address of PCM data buffer */
			   ivUInt16			nSamples	/* [In] Specifies the length, in samples, of PCM data */
			   );

ivESRStatus 
ivCall 
ESREndAppendData(
				ivPointer			pEsrObj,	/* ESR Object */
				ivBool				bEndAppend	/*[In] End append data*/
			  );

ivESRStatus
ivCall 
ESR_GetResult(
			  ivPointer pEsrObj, 
			  PCEsrResult ivPtr ppResult	/* out */
			 );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(ES_TEAM__2010_09_09__ESR__H) */
