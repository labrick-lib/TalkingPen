/******************************************************************************
* File Name		       : ivADPCM.h
* Description          : ADPCM 编解码算法头文件
* Author               : sszhou
* Date Of Creation     : 2005-03-03
* Platform             : Any
* Modification History : 
*------------------------------------------------------------------------------
* Date        Author     Modifications
*------------------------------------------------------------------------------
* 2005-03-03  sszhou     Created
******************************************************************************/

#ifndef IFLYTEK_VOICE__CODEC__2005_03_03__ADPCM__H
#define IFLYTEK_VOICE__CODEC__2005_03_03__ADPCM__H


/*
 *	包含相关头文件
 */

#include "ivDefine.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Define ADPCM encode data structure and function */

typedef struct tagADPCMEncoder ivTADPCMEncoder, ivPtr ivPADPCMEncoder;

struct tagADPCMEncoder
{
	ivInt16		m_nPrevVal;		/* Previous output value */
	ivInt16		m_nIndex;		/* Index into stepsize table */
};

#define ivADPCM_InitCoder(pEncoder)	{(pEncoder)->m_nPrevVal=0;(pEncoder)->m_nIndex=0;}
ivSize ivCall ivADPCM_Encode(ivPADPCMEncoder pEncoder, ivPInt16 pPcmIn, ivSize nSamples, ivPUInt8 pCodeOut);


/* Define ADPCM decode data structure and function */

typedef struct tagADPCMDecoder ivTADPCMDecoder, ivPtr ivPADPCMDecoder;

struct tagADPCMDecoder
{
	ivInt16		m_nPrevVal;		/* Previous output value */
	ivUInt16	m_nIndex;		/* Index into stepsize table */
};

#define ivADPCM_InitDecoder(pDecoder)	{(pDecoder)->m_nPrevVal=0;(pDecoder)->m_nIndex=0;}
ivSize ivCall ivADPCM_Decode(ivPADPCMDecoder pDecoder, ivPUInt8 pCodeIn, ivSize nCodeSize, ivPInt16 pPcmOut);


#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__CODEC__2005_03_03__ADPCM__H */
