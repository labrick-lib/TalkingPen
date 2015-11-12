/*----------------------------------------------+
 |																	|
 |		ivIVW.h - LowEnd API							|
 |																	|
 |		Copyright (c) 1999-2010, iFLYTEK Ltd.	|
 |		All rights reserved.								|
 |																	|
 +----------------------------------------------*/

#if !defined(IVW_TEAM__2012_10_18__IVW__H)
#define IVW_TEAM__2012_10_18__IVW__H


#include "ivIvwDefine.h"
#include "ivPlatform.h"
#include "ivIvwErrorCode.h"

/*
*	Interface
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Definition of IVW parameters and parameter value */

ivStatus 
ivCall
IvwCreate(
		  ivPointer		pIvwObj,				/* IVW Object */
		  ivSize ivPtr	pnIVWObjSize,		/* [In/Out] Size of IVW object */
		  ivPointer		pResidentRAM,			/* Resident RAM */
		  ivPUInt16		pnResidentRAMSize,			/* [In/Out] Size of Resident RAM */
		  ivCPointer	pResource			/* [In] Resource */	
		  );

ivStatus 
ivCall 
IvwSetParam(
			ivPointer pIvwObj,		/* IVW Object */
			ivUInt32 nParamID,		/* Parameter ID */
			ivInt16 nParamValue	/* Parameter Value */
			);


ivStatus
ivCall
IvwRunStep(
		   ivPointer	pIvwObj		/* IVW Object */		
		   );

ivStatus 
ivCall 
IvwAppendAudioData(
			  ivPointer		pIvwObj,	/* IVW Object */
			  ivCPointer		pData,		/* [In] Pointer to the address of PCM data buffer */
			  ivUInt16			nSamples	/* [In] Specifies the length, in samples, of PCM data */
			  );

ivStatus
ivCall
IvwReset(
		   ivPointer	pIvwObj		/* IVW Object */		
		   );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(IVW_TEAM__2012_10_18__IVW__H) */
