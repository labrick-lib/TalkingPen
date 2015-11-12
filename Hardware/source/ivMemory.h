/******************************************************************************
* File Name		       : ivMemory.h
* Description          : InterSound 内存操作头文件
* Author               : pingbu
* Date Of Creation     : 2004-05-21
* Platform             : Any
* Modification History : 
*------------------------------------------------------------------------------
* Date        Author     Modifications
*------------------------------------------------------------------------------
* 2004-05-21  pingbu     Created
******************************************************************************/

#ifndef IFLYTEK_VOICE__2004_05_21__MEMORY__H
#define IFLYTEK_VOICE__2004_05_21__MEMORY__H

#include "EsKernel.h"
#include "ivDefine.h"

#ifdef __cplusplus
extern "C" {
#endif


#if !IV_ANSI_MEMORY

	void ivCall ivMemZero( ivPointer pBuffer, ivSize nSize );
	void ivCall ivMemCopy( ivPointer pDesc, ivCPointer pSrc, ivSize nSize );

#else

	#ifdef	ivMemZero
	#undef  ivMemZero
	#endif
	#define ivMemZero(p,n)		((void)memset((p),0,(size_t)(n)))

	#ifdef	ivMemCopy
	#undef  ivMemCopy
	#endif
	#define ivMemCopy(d,s,n)	((void)memcpy((d),(s),(size_t)(n)))
#endif



#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__2004_05_21__MEMORY__H */
