/*********************************************************************#
//	文件名		：EsSearch.h
//	文件功能	：Declare of structures for ESR Search corresponding
//	作者		：Truman
//	创建时间	：2007年7月10日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#if !defined(ES_TEAM__2007_07_10__ESKERNEL_ESSEARCH__H)
#define ES_TEAM__2007_07_10__ESKERNEL_ESSEARCH__H

#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */

#include "EsNetwork.h"
#include "EsSearchConst.h"
#include "ivMath.h"

/* ---------------------CMN策略 配置项.Begin--------------------------------- */
#define NOT_CMN_UPDATE              (1) //不进行CMN更新
#define CMN_UPDATE_JUZI             (2) //只进行句间更新
#define CMN_UPDATE_JUNEI            (3) //句间句内都更新
#define TEST_CMN_UPDATE             (CMN_UPDATE_JUZI)  
/* ---------------------CMN策略 配置项.End---------------------------------- */

/* ---------------------Debug 配置项.Begin--------------------------------- */
#define LOAD_FLOAT_MDOEL            (0)  /* 将浮点模型也打入资源中 */
#define LOAD_FLOAT_FEA              (0) /* Load研究院float feature */

#define LOG_FIXED_FEATURE           (0) /* Log产品定点特征 */
#define LOG_SEARCH_SCORE            (0)
#define LOG_DEBUG_INFO				(0)
#define RATETEST_TOOL_LOG           (0)     /* 识别率工具log */
#define RATETSET_USE_VAD            (0)     /* 识别率测试支持关VAD */

/* ---------------------Debug 配置项.End--------------------------------- */


/* --------------------桶排序裁剪 配置项.Begin---------------------------- */
#define BEAM_THRESH_LOW					((1<<ESR_Q_SCORE)*200)
/* Bucket sort 最大得分，后帧比前帧最多多40分内,不含Q值的 */
#define BUCKET_SCORE_STEP				(4*(1<<ESR_Q_SCORE))
#define BUCKET_EXCEED_MAX_SCORE			((40)*(1<<ESR_Q_SCORE))   /* 每次使用上帧的最大值，那本帧最大值可能超过上帧最大值的最大分数 */
#define BUCKET_PLUS_SCORE_COUNT			(BUCKET_EXCEED_MAX_SCORE/BUCKET_SCORE_STEP) /* 得分比最大值大的桶数 */
#define BUCKET_ALL_COUNT				(32 + (BEAM_THRESH_LOW+BUCKET_EXCEED_MAX_SCORE)/BUCKET_SCORE_STEP)  /* 桶数，注意!!!!!:最大值不能超过258,内存是复用的front端FrameCache */
#define	BUCKET_BEAM_COUNT				(BEAM_THRESH_LOW/BUCKET_SCORE_STEP)
#define BUCKTE_PRUNE_MAX_NODE			(150) /* VAD检测到语音开始后的裁剪的最大node数 */

#define ESR_MAX_NBEST					 (3)
  
/* log0 */
#if LOAD_FLOAT_MDOEL
    #define LZERO						(-1.0E6) //(log0)
    #define LMINEXP						(-6.0)
    #define LSMALL						(-0.5E6)
#else
    #define LZERO                           (-0x7800)
#endif


#define DW_MINI_RES_CHECK               (0x20100107)
#define DW_MODEL_CHECK                  (0x20110107)
#define DW_TAG_MODEL_CHECK              (0x20130503)

/* 多个grm，共用一个精简后的Model，合并存储为一个bin */
typedef struct tagGrmDesc{
	ivUInt32  nGrmID;									 
	ivInt32   nSpeechTimeOutFrm;    /* 语音时长限制(帧数) */ 
	ivInt32   nMinSpeechFrm;	    /* 最短语音时长(帧),用于环境噪音判断的 */
 	ivUInt32  nGrmOffset;			/* Unit:word. */					 
	ivUInt32  nGrmSize;		        /* Unit:word. */			  
}TGrmDesc, ivPtr PGrmDesc;

typedef struct tagGrmMdlHeader	
{
    ivUInt32 nCRC;	        /* 该CRC是对去掉sizeof(TGrmMdlHdr)的CRC */			
    ivUInt32 nCRCCnt;      /* Unit:word */
	ivUInt32 dwCheck;			
	ivUInt32 nModelOffset;	/* Unit:word */	
	ivUInt32 nModelSize;						
	ivUInt32 nTotalGrm;			
	TGrmDesc tGrmDesc[1];		
}TGrmMdlHdr, ivPtr PGrmMdlHdr;

typedef struct tagResHeader						
{
	ivUInt32    nCRC;	        /* 对去掉nCRC和nCRCSize字段后的所有size进行CRC */				
	ivUInt32    nCRCCnt;		/* Unit:word */		
	ivUInt32    dwCheck;                /* 校验字，为了和内核代码对应的 */	
    ivUInt32    nMixSModelOffset;       /* Unit:word */
    ivUInt32    nSilSModelOffset;       /* Unit:word */
    ivUInt32    nFillerSModelOffset;    /* Unit:word. 新拒识方案的filler模型 */

    ivUInt32    nOldFillerSModelOffset; /* Unit:word. 老拒识方案的filler模型 新加*/
    ivUInt32    nOldFillerCnt;          /* En_Child_filler,En_Adult_filler,Cn_Child_filler,Cn_Adult_filler */
    ivUInt32    nOldFillerMix;

    ivUInt32    nSilMixture;           
    ivUInt32    nFillerMixture;
    ivUInt32    nMixture;              /* 除了sil和filller，其他模型的mixture是一样的 */
    ivUInt32    nMixSModelNum;         /* 包含sil和filler */
    
    ivInt16     s16MeanCoef[FEATURE_DIMNESION];
    ivInt16     s16MeanCoefQ[FEATURE_DIMNESION];

}TResHeader,ivPtr PResHeader;
typedef TResHeader ivCPtr PCResHeader;

typedef struct tagGaussianModel		
{
	ivInt16		fGConst;			/* Together with GWeight, Q8 */
	ivInt16		fMeanVar[FEATURE_DIMNESION]; 
#if LOAD_FLOAT_MDOEL    /* 浮点数也打出资源 hbtao*/
    float       fltGConst;
    float       fltWeight;
    float       fltMean[FEATURE_DIMNESION];
    float       fltVar[FEATURE_DIMNESION];
#endif
}TGaussianModel,TGModel,ivPtr PGaussianModel,ivPtr PGModel;
typedef TGaussianModel ivCPtr PCGModel;

typedef struct tagMixStateModel{		
	/* Gauss Model */
#if LOAD_FLOAT_MDOEL
    char            szName[256];
#endif
	TGaussianModel tGModel[1];			/* 变长数组,实际个数根据模型情况确定. */
}TMixStateModel, ivPtr PMixStateModel;
typedef TMixStateModel ivCPtr PCMixSModel;

/* Word Result */
typedef struct tagNBest		
{
	ivUInt16	nWordID;
#if !LOAD_FLOAT_MDOEL
	ivInt16		fScore;
#else
    float       fScore;
#endif
#if MINI5_USE_NEWCM_METHOD
#if !LOAD_FLOAT_MDOEL
	ivInt16		fCMScore;
#else
    float       fCMScore;
#endif
#endif
}TNBest,ivPtr PNBest;

#if MINI5_SPI_MODE
#define MINI5_SPI_PAGE_SIZE         ((ivUInt16)256)       /* 标注9160平台SPI一页大小.单位:字节 */
#endif

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
/*------------------------------------------------------------------------------------------*/
/* ------------------------------效果测试配置项----------------------------- */
//#define TAG_LOG						/* Log */
//#define TAG_SEARCH_ADD_SIL		/* 是否在解码时就加入可选sil参与解码 */
/* ------------------------------------------------------------------------- */

#define DW_TAGCMD_CHECK				(0x20120503) /* 生成的tag的校验码 */
#define MAXNUM_TAGCMD_STATENUM		(160)		/* 制作的语音标签命令词的最多state个数 */
#define STATE_LPF_PUNISH_SCORE		(3*(1<<ESR_Q_SCORE))		/* 防止state乱跳，在跳出时加入的乘法得分 */

typedef struct tagPathItem{		
    ivUInt16 iState;		    /* 当前最优state */
    ivUInt16 nRepeatFrm;		/* 当前最优state已经重复了多少帧 */
}TPathItem ,ivPtr PPathItem;

typedef struct tagTagPathDesc{	
	TPathItem   tPath[1];
	ivInt16     pFeature[FEATURE_DIMNESION];		/* 本帧的特征 */
}TTagPathDesc ,ivPtr PTagPathDesc;
typedef TTagPathDesc ivCPtr PCTagPathDesc;

typedef struct tagTagCmdDesc		
{
	ivUInt32		dwCheck;		
    ivUInt16		nID;			
    ivUInt16		nState;			
	ivUInt16        nMixture;		
	TMixStateModel	pMixSModel[1];  /* 不定长数组,实际个数为nState个 */
}TTagCmd, ivPtr PTagCmd;

/*------------------------------------------------------------------------------------------*/
#endif

/* Declare of ASR object */
typedef struct tagEsrEngine TEsrEngine,ivPtr PEsrEngine;

/* 新的CM策略:比如最优路径: */
/*           s1---s1---s1---s2---s2-----s3-----s4--- */
/*           |-----------|  |-----|    |--|    |--| */
/*  nCMScore =   (min1     +  min2  +  min3 +  min4) / 4  */
/*  其中min1表示状态s1的三帧中最小的stateoutlike-filleroutlike */
typedef struct tagSearchNode
{
#if !LOAD_FLOAT_MDOEL
    ivInt16 fScore;               /* 正常解码得分传递 */
#else
    float fScore;               /* 正常解码得分传递 */
#endif
#if MINI5_USE_NEWCM_METHOD
#if !LOAD_FLOAT_MDOEL
	ivInt16 fCMScore;             /* 保存当前state之前所有state的路径上的nCMScoreSum(还没有除以nStateCount) */
	ivInt16 fCurStateCMScore;     /* 当前state的outlike-FillerOutlike。如果当前状态连续重复多帧，则取最小值. */
#else
    float fCMScore;             /* 保存当前state之前所有state的路径上的nCMScoreSum(还没有除以nStateCount) */
    float fCurStateCMScore;     /* 当前state的outlike-FillerOutlike。如果当前状态连续重复多帧，则取最小值. */
#endif
#endif  /* #if MINI5_USE_NEWCM_METHOD */

}TSearchNode, ivPtr PSearchNode;

typedef struct tagSPICache
{
    ivPointer	pSPIBufBase;
    ivPointer   pSPIBuf;            /* 页对齐的SPI当前可用地址 */
    ivPointer   pSPIBufEnd;

    ivPInt8		pCacheSPI;		/* RAM.缓存需要存储到SPI的内容,攒满一页写一次SPI,减少SPI写运算量 */
    ivUInt16	iCacheSPIWrite; 
}TSPICache, ivPtr PSPICache;

typedef struct tagEsSearch			
{
	ivUInt16    iFrameIndexLast;	
	
    /* Model */
    ivUInt16    nMixture;	
    ivUInt16    nSilMixture;
    ivUInt16    nFillerMixture;
#if !MINI5_USE_NEWCM_METHOD
    ivUInt16    nFillerCnt;
#endif
    PCMixSModel pState;		
    PCMixSModel pSilState;
    PCMixSModel pFillerState;

	/* Cache for Calculation of all State */
	PSearchNode pSearchNodeLast;
	/*-------------------------------------*/

#if !LOAD_FLOAT_MDOEL
	/* Frame data */
	ivInt16		fFeature[FEATURE_DIMNESION];		
	/*------------*/

	/* For Filler */
	ivInt16		fSilScore;		/* 每帧的sil 得分 */	
	ivInt16     fFillerScore;	/* 每帧的Filler得分 */		
#else
    /* Frame data */
    float		fFeature[FEATURE_DIMNESION];		
    /*------------*/

    /* For Filler */
    float		fSilScore;		/* 每帧的sil 得分 */	
    float       fFillerScore;	/* 每帧的Filler得分 */		
    double      fFillerScoreSum;
#endif
	/* Network */
	ivUInt16	nTotalNodes;						
	ivUInt16    nExtendNodes;						
	PLexNode    pLexRoot;							
	PCmdDesc	pCmdDesc;							

	/* 记录endfiller上的WordID */
	TNBest		tNBest[ESR_MAX_NBEST];					
		
	/* 桶排序裁剪 */
	ivPUInt16	pnBucketCacheLast;	

#if !LOAD_FLOAT_MDOEL
	ivInt16     fScoreMaxLast;					
	ivInt16		fScoreThreshLast;				
#else
    float     fScoreMaxLast;					
    float		fScoreThreshLast;				
#endif

	ivUInt16	nCmdNum;						

    ivUInt16	iEngineType;
#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT

    TSPICache   tSPICache;
    ivUInt16    nClusters;

    ivPInt16	pStaScoreCacheLast;		
	ivPInt16	pnRepeatFrmCacheLast;	/* 记录每个state重复出现的帧数 */
	ivInt16     iMaxStateLast;			/* 记录得分max是哪个state */

#endif
}TEsSearch,ivPtr PEsSearch;

EsErrID EsSearchInit(PEsSearch pThis, ivCPointer pModel,PEsrEngine pEngine);

EsErrID EsSetLexicon(PEsSearch pThis,ivCPointer pLexicon, PEsrEngine pEngine);

void EsSearchReset(PEsSearch pThis);

EsErrID EsSearchFrameStep(PEsSearch pThis);

void EsUpdateNBest(PEsSearch pThis);

ivInt32 LAdd(ivInt32 x, ivInt32 y);

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
EsErrID EsTagSearchFrameStep(PEsSearch pThis);
EsErrID EsSPIWriteData(PSPICache pThis, ivCPointer pData, ivUInt32 nDataSize);
EsErrID EsSPIWriteSPIData(PSPICache pThis, ivPointer pRAMBuf,ivCPointer pData, ivUInt32 nDataSize, ivPUInt32 pnCRC,ivPUInt32 piCounter);
EsErrID EsSPIWriteFlush(PSPICache pThis);
void    EsSPIWriteOnePage(ivPointer pSPIBuf, ivPointer pRAMBuf);
#endif  /* MINI5_SPI_MODE && MINI5_TAG_SUPPORT */

/* SPI: See EsPlatform.c */
#if MINI5_SPI_MODE
#define EsCal1GaussOutlike  EsSPICal1GaussOutlike
#define EsCalcOutLike       EsSPICalcOutLike
#define EsUpdateAllScore    EsSPIUpdateAllScore
#endif

#if MINI5_SPI_MODE
EsErrID EsSPIReadBuf(ivPointer pRAMBuf, ivCPointer pSPIBuf, ivUInt16 nSize);
ivInt16 EsSPICal1GaussOutlike(PEsSearch pThis, PCGModel pGModel);
ivInt16 EsSPICalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures);
EsErrID EsSPIUpdateAllScore(PEsSearch pThis);

/* SPI方案CRC校验，每次读一页大小到RAM中计算CRC */
EsErrID EsSPICalcCRC(ivPointer pRAMBuf, ivCPointer pSPIData,ivUInt32 nSize,ivPUInt32 pnCRC,ivPUInt32 piCounter);
#else
#if !LOAD_FLOAT_MDOEL
ivInt16 EsCal1GaussOutlike(PEsSearch pThis, PCGModel pGModel);
ivInt16 EsCalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures);
EsErrID EsUpdateAllScore(PEsSearch pThis);
#else /* MINI5_SPI_MODE */
float EsCal1GaussOutlike(PEsSearch pThis, PCGModel pGModel);
float EsCalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures);
EsErrID EsUpdateAllScore(PEsSearch pThis);
#endif
#endif /* MINI5_SPI_MODE */

#endif /* !defined(ES_TEAM__2007_07_10__ESKERNEL_ESSEARCH__H) */
