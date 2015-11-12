/*----------------------------------------------+
|												|
|	ivErrorCode.h - Basic Definitions		|
|												|
|		Copyright (c) 1999-2012, iFLYTEK Ltd.	|
|		All rights reserved.					|
|												|
+----------------------------------------------*/

#ifndef IFLYTEK_VOICE__2012_02_27_ERRORCODE__H
#define IFLYTEK_VOICE__2012_02_27_ERRORCODE__H

#include "ivDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ivInt32 ivErrID;
typedef ivInt32	ivStatus;
typedef ivInt32	EsErrID;
typedef ivInt32	ivESRStatus;

/* For license check */
#define ivErr_InvSN				((ivStatus)2)

/* General */
#define ivErr_InvArg			((ivStatus)3)
#define ivErr_BufferFull		((ivStatus)4)
#define ivErr_Failed			((ivStatus)5)
#define ivErr_NotSupport		((ivStatus)6)
#define ivErr_OutOfMemory		((ivStatus)7)
#define ivErr_InvResource		((ivStatus)8)
#define ivErr_NotFound			((ivStatus)9)

/* For object status */
#define ivErr_InvCall			((ivStatus)10)
#define ivErr_BufferEmpty		((ivStatus)11)
#define ivErr_GetResult			((ivStatus)12)
#define ivErr_ReactOutTime		((ivStatus)13)		/* 反应超时 */
#define ivErr_SpeechOutTime		((ivStatus)14)		/* 语音超时 */

#define ivErr_CUT               ((ivStatus)15)      /* 录音质量过高 */
#define ivErr_LOWER             ((ivStatus)16)      /* 录音质量过小 */
#define ivErr_Limitted          ((ivStatus)17)      /* 授权不够 */

#define ivErr_Done              ((ivStatus)18)      /* 数据处理结束 */

#ifdef __cplusplus
}
#endif


#define	ivErr_OK				((ivStatus)0)
#define	ivErr_FALSE				((ivStatus)1)

#define ivSucceeded(hr)			((ivUInt32)(hr) <= 1)

#endif /* !IFLYTEK_VOICE__2008_10_13_ERRORCODE__H */
