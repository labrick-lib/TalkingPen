/******************************************************************************
* File Name		       : ivMath.h
* Description          : InterSound 算术运算头文件
* Author               : pingbu
* Date Of Creation     : 2005-06-24
* Platform             : Any
* Modification History : 
*------------------------------------------------------------------------------
* Date        Author     Modifications
*------------------------------------------------------------------------------
* 2005-06-24  pingbu     Created
******************************************************************************/

#ifndef IFLYTEK_VOICE__2005_06_24__MATH__H
#define IFLYTEK_VOICE__2005_06_24__MATH__H

#include "EsKernel.h"
#include "ivDefine.h"

#ifdef __cplusplus
extern "C" {
#endif


	/*
	*	浮点值修饰前缀
	*/
#define Q15
#define Q31
#define Q63


	/*
	*	浮点常量
	*/
#define ROUND_Q1			0x0001		/* FLOAT_Q0 */
#define ROUND_Q15			0x4000		/* FLOAT_Q14 */
#define DOUBLE_Q15			0x00008000L
#define DOUBLE_Q30			0x40000000L



	/*
	*	浮点算术运算
	*
	*	L_	- 类型为Long(默认为Short)
	*	i_	- 整型运算
	*	_s	- 输入为Short
	*	_l	- 输入为Long
	*	_r	- with rounding
	*	_c	- with carry
	*/

	/* 乘 */
#define L_mult(var1,var2)			(((Q31 ivInt32)(var1)*(var2))<<1)

	/* 加,乘加 */
#define L_add(L_var1,L_var2)		((Q31 ivInt32)(L_var1)+(L_var2))
#define L_mac(L_var3,var1,var2)		L_add(L_var3,L_mult(var1,var2))

	/* 拆分双精度值 */
#define extract_h(L_var1)			(Q15 ivInt16)((L_var1)>>16)

	/* 乘 */
#ifdef IV_TYPE_INT64
#define L_mls(L_var1,var2)			(Q31 ivInt32)(((Q63 ivInt64)(L_var1)*(var2))>>15)
#else
/* #define L_mls(L_var1,var2)			L_mac((((L_var1)&0xFFFF)*(var2))>>15,extract_h(L_var1),var2) */
#define L_mls(L_var1,var2)			L_mac(  (ivInt32)((((L_var1)&0xFFFF)*(var2))>>15), (ivInt32)(extract_h(L_var1)), (ivInt32)(var2)  )
#endif

	/* 求归1化因子 */
	ivUInt16 ivCall es_norm_l(Q31 ivInt32 L_var1);


	/* 求相对大值、相对小值 */
#define ivMax(a,b)  (((a)>(b))?(a):(b))
#define ivMin(a,b)  (((a)<(b))?(a):(b))

#define	EsSaturate(v,l,u)	ivMin(ivMax(v,l),u)
#define	EsSatInt16(v)	(ivInt16)EsSaturate(v,IV_MIN_INT16,IV_MAX_INT16)
#define	EsSatUInt16(v)	(ivUInt16)EsSaturate(v,0,IV_MAX_UINT16)

#define ESR_MATH_10LN2_Q10  0x1BBA  /* format Q10 */
#define ESR_MATH_11LN2_Q10  0x1E80   /* format Q10 */
#define ESR_MATH_20LN2_Q10  0x3774  /* format Q10 */

#define ivGridSize(n)	((ivSize)(((ivSize)(n)+(IV_PTR_GRID-1))&(~IV_PTR_GRID + 1)))

ivInt16 ivCall simple_table_ln(ivUInt32 w32Param, ivInt16 cParamQ);

#if ESR_CPECTRAL_SUB
#define IV_CHECK_SATURATE16(x)      IV_ASSERT((x)>= IV_MIN_INT16 && (x) <= IV_MAX_INT16)
ivUInt16 CalSqrtRecip32(ivUInt32 s32Param);
#endif /* ESR_CPECTRAL_SUB */

void ivCall ivMakeCRC(ivPCInt16 pData,ivUInt32 nSize,ivPUInt32 pnCRC,ivPUInt32 piCounter); 

#ifdef __cplusplus
}
#endif


#endif/*!IFLYTEK_VOICE__2005_06_24__MATH__H*/
