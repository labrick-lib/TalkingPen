/*----------------------------------------------+
 |												|
 |		ivESR.h - LowEnd API					|
 |												|
 |		Copyright (c) 1999-2018, iFLYTEK Ltd.	|
 |		All rights reserved.					|
 |												|
 +----------------------------------------------*/

#if !defined(ES_TEAM__2013_05_20__ESR__H)
#define ES_TEAM__2013_05_20__ESR__H

#include "ivConfig.h"
#include "ivDefine.h"
#include "ivPlatform.h"
#include "ivEsErrorCode.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct tagEsrResult {
    ivUInt16    nResultType;        /* 输出结果类型：识别结果、拒识、超时、环境噪音 */
    ivUInt16    nResultID;          /* Result Cmd ID */
    ivUInt16    nConfidenceScore;   /* Score of Confidence 0-100 */
#if MINI5_ENHANCE_FRONT
    ivBool      bSpeechCut;         /* 是否抢说 */
    ivBool      bSNRLower;          /* 是否信噪比过低 */
    ivBool      bSpeechEnergyLower; /* 是否语音能量过低 */
#endif
}TEsrResult, ivPtr PEsrResult;
typedef TEsrResult ivCPtr PCEsrResult;

/* Definition of ESR parameters and parameter value */
/* Parameter ID for ESR Enhance VAD */
#if MINI5_ENHANCE_VAD
# define ES_PARAM_ENHANCEVAD         ((ivUInt32)1)
# define ES_ENHANCEVAD_ON            ((ivUInt16)1)
# define ES_ENHANCEVAD_OFF           ((ivUInt16)0)
# define ES_DEFAULT_ENHANCEVAD       ES_ENHANCEVAD_OFF
#endif /* MINI5_ENHANCE_VAD */

/* Parameter ID for ESR ambient noise */
#define ES_PARAM_AMBIENTNOISE       ((ivUInt32)2)
#define ES_AMBIENTNOISE_ON          ((ivUInt16)1)
#define ES_AMBIENTNOISE_OFF         ((ivUInt16)0)
#define ES_DEFAULT_AMBIENTNOISE     ES_AMBIENTNOISE_ON

/* Parameter ID for ESR VAD Check End Frame Numbers */
#define ES_PARAM_VADCHKBACK_FRMNUM  ((ivUInt32)3)

/* Parameter ID for ESR VAD Speech Max/Min Frame Numbers */
#define ES_PARAM_SPEECH_FRM_MAX     ((ivUInt32)4)
#define ES_PARAM_SPEECH_FRM_MIN     ((ivUInt32)5)

/* Message ID description */
#define ES_MSG_ENDAPPENDDATA        ((ivUInt32)1) /* End Append Audio */
#define ES_MSG_RESERTSEARCHR        ((ivUInt32)2) /* Ready for Search */

/* Result Type */
#define ES_TYPE_RESULT              ((ivUInt16)1)
#define ES_TYPE_RESULTNONE          ((ivUInt16)2)
#define ES_TYPE_FORCERESULT         ((ivUInt16)3)
#define ES_TYPE_AMBIENTNOISE        ((ivUInt16)4)

/* Status Type */
#define ES_STATUS_RESULT            ((ivESRStatus)1)
#define ES_STATUS_FINDSTART         ((ivESRStatus)2)
#define ES_STATUS_TAGSUCCESS        ((ivESRStatus)3)
#define ES_STATUS_TAGFAILED         ((ivESRStatus)4)

#define DEBUG_CRITICAL              (0)  /* 为了调试多线程，加上临界区 */

/*
*   Interface
*/
EsErrID 
ivCall 
ESRCreate(
           ivPointer         pEsrObj,           /* ESR Object */
           ivSize ivPtr      pnESRObjSize,      /* [In/Out] Size of ESR object */
           ivPointer         pResidentRAM,      /* Resident RAM */
           ivPUInt16         pnResidentRAMSize, /* [In/Out] Size of Resident RAM */
           ivCPointer        pResource,         /* [In] Resource */
           ivUInt32          nGrmID,            /* [In] Grammar ID */
           ivUInt16          nSampleRate,       /* [In] SampleRate 16K/8K */
           ivUInt16          nVAD_ChkEnd_FrmNum /* [In] VAD Check Back Frame Numbers */
         );

EsErrID 
ivCall 
ESRSetParam(
            ivPointer        pEsrObj,            /* ESR Object */
            ivUInt32         nParamID,           /* [In] Parameter ID */
            ivUInt16         nParamValue         /* [In] Parameter Value */
           );

EsErrID 
ivCall 
ESRReset(ivPointer           pEsrObj);           /* ESR Object */

EsErrID 
ivCall 
ESRRunStep(
             ivPointer         pEsrObj,          /* ESR Object */
             ivUInt32          dwMessage,        /* [In] Call Information */
             ivESRStatus ivPtr pStatus,          /* [Out] To Receive Operating Status */
             PCEsrResult ivPtr ppResult          /* [Out] To Receive Recognize Result */
          );

EsErrID 
ivCall 
ESRAppendData(
              ivPointer        pEsrObj,          /* ESR Object */
              ivCPointer       pData,            /* [In] Pointer to the address of PCM data buffer */
              ivUInt16         nSamples          /* [In] Specifies the length, in samples, of PCM data */
             );

EsErrID 
ivCall 
ESRCreateTagObj(
               ivPointer      pTagObj,           /* Tag Object */
               ivSize ivPtr   pnTagObjSize,      /* [In/Out] Size of Tag object */
               ivCPointer     pModel,            /* [In] Model */
               ivUInt16       nTagID,            /* [In] Tag Cmd ID */
               ivPointer      pSPIBuf,           /* [In] SPI Buffer */
               ivSize         nSPIBufSize,       /* [In] SPI Buffer Size */
               ivUInt16       nSampleRate,       /* [In] SampleRate 16K/8K */
               ivUInt16       nVAD_ChkEnd_FrmNum /* [In] VAD Check Back Frame Numbers */
               );

EsErrID 
ivCall 
ESRAddTagToGrm (
               ivPointer      pTagObj,            /* Tag Object */
               ivCPointer     pOrgRes,            /* [In] Original Resource */
               ivPointer      pNewRes,            /* [In/Out] New Resource */
               ivPUInt32      pnNewResSize,       /* [In/Out] Size of pNewRes */
               ivPUInt16      pGrmIDLst,          /* [In] Grammar ID to Add VoiceTag */
               ivUInt16       nGrmIDNum,          /* [In] Count of pGrmIDLst */
               ivUInt16       nTagThresh          /* [In] Rejection Threshold of VoiceTag */
               );

EsErrID 
ivCall 
ESRDelTagFromGrm(
               ivPointer      pTagObj,            /* Tag Object */
               ivSize ivPtr   pnTagObjSize,       /* [In/Out] Size of Tag object */
               ivCPointer     pOrgRes,            /* [In] Original Resource */
               ivPointer      pNewRes,            /* [In/Out] Original Resource */
               ivPUInt32      pnNewResSize,       /* [In/Out]Size of pNewRes */ 
               ivUInt16       nDelGrmID,          /* [In] Grammar ID to Delete VoiceTag */
               ivUInt16       nDelTagID           /* [In] VoiceTag ID to Delete */
               );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(ES_TEAM__2013_05_20__ESR__H) */
