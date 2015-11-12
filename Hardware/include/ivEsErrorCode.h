/*----------------------------------------------+
|												|
|	ivESRErrorCode.h - Basic Definitions		|
|												|
|		Copyright (c) 1999-2008, iFLYTEK Ltd.	|
|		All rights reserved.					|
|												|
+----------------------------------------------*/

#ifndef IFLYTEK_VOICE__ESRERRORCODE__H
#define IFLYTEK_VOICE__ESRERRORCODE__H

#include "ivDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ivInt32 EsErrID;
typedef ivInt32 ivESRStatus;

#define EsErrID_OK              ivNull
#define EsErr_InvCal            ((EsErrID)1)
#define EsErr_InvArg            ((EsErrID)2)
#define EsErr_TellSize          ((EsErrID)3)
#define EsErr_OutOfMemory       ((EsErrID)4)
#define EsErr_BufferFull        ((EsErrID)5)
#define EsErr_Result            ((EsErrID)6)
#define EsErr_BufferEmpty       ((EsErrID)7)
#define EsErr_InvRes            ((EsErrID)8)
#define EsErr_Failed            ((EsErrID)9)
#define EsErr_ReEnter           ((EsErrID)10)
#define EsErr_SpeechTimeOut     ((EsErrID)11)
#define EsErr_UnbelievablySpeech    ((EsErrID)12)
#define EsErr_FindStart         ((EsErrID)13)
#define EsErr_TagSuccess        ((EsErrID)14)
#define EsErr_TagFailed         ((EsErrID)15)

#define EsErr_MFCCBufferFull    ((EsErrID)16)      /* 提特征放在AppendData中，防止MFCCBuffer满 */
#define EsErr_RESETING          ((EsErrID)17)      /* 解码器出结果，此时不能AppendData */

/*
*   ivESR ErrorID return
*/

/* General */
#define ivESR_OK                    ((EsErrID)EsErrID_OK)
#define ivESR_INVCAL                ((EsErrID)EsErr_InvCal)
#define ivESR_INVARG                ((EsErrID)EsErr_InvArg)
#define ivESR_FAILED                ((EsErrID)EsErr_Failed)

/* For Create Grammar Object */
#define ivESR_OUTOFMEMORY           ((EsErrID)EsErr_OutOfMemory)

/* For Run Step  */
#define ivESR_REENTER               ((EsErrID)EsErr_ReEnter)

/* For resource */
#define ivESR_INVRESOURCE           ((EsErrID)EsErr_InvRes)

/*
*   ivESR Warning ID return
*/

/* For Append PCM Data */
#define ivESR_BUFFERFULL            ((EsErrID)EsErr_BufferFull)

/* For Run Step  */
#define ivESR_BUFFEREMPTY           ((EsErrID)EsErr_BufferEmpty)

#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__ESRERRORCODE__H */
