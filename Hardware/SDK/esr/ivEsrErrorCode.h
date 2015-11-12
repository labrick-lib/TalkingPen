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

typedef	ivInt32		EsErrID;
typedef ivInt32		ivESRStatus;

#define	EsErrID_OK			ivNull
#define EsErr_InvCal		((EsErrID)1)
#define EsErr_InvArg		((EsErrID)2)
#define EsErr_TellSize		((EsErrID)3)
#define EsErr_OutOfMemory	((EsErrID)4)
#define EsErr_BufferFull	((EsErrID)5)
#define EsErr_Result		((EsErrID)6)
#define EsErr_ResultNone    ((EsErrID)7)
#define EsErr_BufferEmpty	((EsErrID)8)
#define EsErr_InvRes		((EsErrID)9)
#define EsErr_Failed		((EsErrID)10)
#define EsErr_ReEnter       ((EsErrID)11)
#define EsErr_AmbientNoise  ((EsErrID)12)
#define EsErr_SpeechTimeOut  ((EsErrID)13)
#define EsErr_UnbelievablySpeech	((EsErrID)14)
#define EsErr_ForceResult	((EsErrID)15)
/*
*	ivESR Status return
*/

/* General */
#define ivESR_OK			EsErrID_OK
#define ivESR_INVCAL		((ivESRStatus)EsErr_InvCal)
#define ivESR_INVARG		((ivESRStatus)EsErr_InvArg)

/* For create grammar object */
#define ivESR_OUTOFMEMORY	((ivESRStatus)EsErr_OutOfMemory)

/* For create ESR object */
#define ivESR_SIZECACULATED	((ivESRStatus)EsErr_TellSize)

/* For append PCM data */
#define ivESR_BUFFERFULL	((ivESRStatus)EsErr_BufferFull)

/* For run Step  */
#define ivESR_RESULT		((ivESRStatus)EsErr_Result)
#define ivESR_RESULTNONE    ((ivESRStatus)EsErr_ResultNone)
#define ivESR_FORCERESULT   ((ivESRStatus)EsErr_ForceResult)
#define ivESR_AMBIENTNOISE	((ivESRStatus)EsErr_AmbientNoise)
#define ivESR_BUFFEREMPTY	((ivESRStatus)EsErr_BufferEmpty)
#define ivESR_REENTER		((ivESRStatus)EsErr_ReEnter)

/* For resource */
#define ivESR_INVRESOURCE	((ivESRStatus)EsErr_InvRes)

#define ivESR_FAILED		((ivESRStatus)EsErr_Failed)

#define ivESR_SPEECHTIMEOUT		((ivESRStatus)EsErr_SpeechTimeOut) /* 当语音超时且拒识时给出该返回值，由用户决定作何处理 */

#define ivESR_UNBELIEVABLYSPEECH	((ivESRStatus)EsErr_UnbelievablySpeech) /* 当VAD检测到的语音在(0.25s,0.35s)之间且拒识时给出该返回值，由用户决定作何处理 */

#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__ESRERRORCODE__H */
