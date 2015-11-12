/*----------------------------------------------+
|																	|
|		ivIvwErrorCode.h - Basic Definitions		|
|		Ivw2.0 Status return								|
|		Copyright (c) 1999-2008, iFLYTEK Ltd.	|
|		All rights reserved.								|
|																	|
+----------------------------------------------*/

#ifndef IFLYTEK_VOICE__ESRERRORCODE__H
#define IFLYTEK_VOICE__ESRERRORCODE__H

#include "ivIvwDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef	ivInt8		IvwErrID;
typedef ivInt8		ivStatus;

#define	IvwErrID_OK						ivNull
#define IvwErr_InvCal					((IvwErrID)1)
#define IvwErr_InvArg					((IvwErrID)2)
#define IvwErr_TellSize					((IvwErrID)3)
#define IvwErr_OutOfMemory		((IvwErrID)4)
#define IvwErr_BufferFull				((IvwErrID)5)
#define IvwErr_BufferEmpty			((IvwErrID)6)
#define IvwErr_InvRes					((IvwErrID)7)
#define IvwErr_ReEnter					((IvwErrID)8)


/*
*	ivIVW Status return
*/

/* General */
#define ivESR_OK								IvwErrID_OK
#define ivESR_INVCAL						((ivStatus)IvwErr_InvCal)
#define ivESR_INVARG						((ivStatus)IvwErr_InvArg)


/* For create ESR object */
#define ivESR_SIZECACULATED			((ivStatus)IvwErr_TellSize)
#define ivESR_OUTOFMEMORY		((ivStatus)IvwErr_OutOfMemory)

/* For append PCM data */
#define ivESR_BUFFERFULL				((ivStatus)IvwErr_BufferFull)

/* For run Step  */
#define ivESR_BUFFEREMPTY			((ivStatus)IvwErr_BufferEmpty)
#define ivESR_REENTER						((ivStatus)IvwErr_ReEnter)

/* For resource */
#define ivESR_INVRESOURCE			((ivStatus)IvwErr_InvRes)


#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__ESRERRORCODE__H */
