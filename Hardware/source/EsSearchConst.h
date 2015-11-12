/*********************************************************************#
//	文件名		：ESSearchConst.h
//	文件功能	：ESSearch所使用的常量声明
//	作者		：qungao    
//	创建时间	：2013年5月17日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#if !defined(ES_TEAM__2013_05_17__ESSEARCHCONST__H)
#define ES_TEAM__2013_05_17__ESSEARCHCONST__H

#include "ivESR.h"

#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */

#define ESR_VAR_QUANT_BIT               (5)     /* 方差量化bit数 */
#define ESR_MEAN_QUANT_BIT              (6)     /* 均值量化bit数 */

#define ESR_Q_MEAN_MANUAL               (ESR_MEAN_QUANT_BIT)     /* 均值定标值. 模型定点化及建立均值方差表使用. */
#define ESR_Q_VAR_MANUAL                (0)     /* 方差定标值. 模型定点化及建立均值方差表使用. */
#define ESR_Q_WEIGHT_GCOSNT_MANUAL	    (8)     /*  */ 
#define ESR_Q_MEANVARTAB_MANUAL         (8)     /* 均值方差表返回值的定标值 */
#define ESR_Q_SCORE						(4)     /*  */
extern ivConst ivInt16 g_s16MeanVarTable[1<<(ESR_VAR_QUANT_BIT+ESR_MEAN_QUANT_BIT+1)];	

#if !MINI5_USE_NEWCM_METHOD 
/* 存储规整为0-100之间得分的CM得分的真实值, Q10 */
extern ivConst ivInt16 g_s16CMScore[101];
#endif

#endif /* !defined(ES_TEAM__2013_05_17__ESSEARCHCONST__H) */
