/*********************************************************************#
//	文件名		：EsPlatform.c
//	文件功能	：和平台相关的函数.比如SPI操作.
//	作者		：qungao
//	创建时间	：2013年5月14日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#include "EsKernel.h"
#include "EsSearch.h"

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
void EsSPIWriteOnePage(ivPointer pSPIBuf, ivPointer pRAMBuf)
{
	ivMemCopy(pSPIBuf, pRAMBuf, MINI5_SPI_PAGE_SIZE);	/* SPI写操作 */
}
#endif

#if MINI5_SPI_MODE
EsErrID EsSPIReadBuf(ivPointer pRAMBuf, ivCPointer pSPIBuf, ivUInt16 nSize)
{
	ivMemCopy(pRAMBuf, pSPIBuf, nSize);

	return EsErrID_OK;
}

ivInt16 EsSPICal1GaussOutlike(PEsSearch pThis, PCGModel pGModel) /* 与硕呈平台AitalkMini1.0代码比较: 大变动 */
{
	ivInt16 i;

	ivInt16 fValue;
	ivInt32 f32Like;

	ivPCInt16 pfFeature;

	ivPCInt16 pfMeanVar;

	pfFeature = pThis->fFeature; /*  16位数据High-Low  | 5bit符号扩展 | 7bit fea (Q6) | 0000 | */

	/* 原始计算公式 1/2[-fConst - ((fea - mean)^2)/var]	 */
	/*    变换: ((fea - mean)^2)/var = [(fea*meancoef - mean*meancoef)*sqrt(1/(var*meancoef*meancoef))]^2     AAA */
	/*    全局均值方差表g_s16MeanVarTable中存储的就是公式AAA的结果,(fea*meancoef - mean*meancoef)占8bit, var'占4bit，该表是2^12=2028大小的表 */
	/* 为了实现mean7bit,var4bit存储，在打包模型时做了处理,存储的mean' = mean*meancoef, var'=ln(var*meancoef*meancoef)*16/varcoef */

	pfMeanVar = pGModel->fMeanVar; /* 16位数据High-Low  | 5bit符号扩展 | 7bit mean (Q6) | 4bit Var (Q0) | */ 

	f32Like = (ivInt32)pGModel->fGConst;  /* Q8 */

	for(i = FEATURE_DIMNESION; i; i--){
		fValue = (*pfMeanVar++) -  (*pfFeature++); 
		ivAssert(fValue < 4096);
		f32Like -= g_s16MeanVarTable[fValue];  /* Q8 */
	}

	f32Like >>= (ESR_Q_MEANVARTAB_MANUAL+1-ESR_Q_SCORE);	/* Q8->ESR_Q_SCORE and *0.5 = (8-ESR_Q_SCORE+1) */

	ivAssert(f32Like >= -32768 && f32Like <= 32767);

	return (ivInt16)f32Like;
}

ivInt16 EsSPICalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别，通过参数参入,返回值由int32改为int16 */
{
	ivInt16 i;
	ivInt32 fOutLike;
	PCGModel pGauss;

	pGauss = pState->tGModel;
	fOutLike = LZERO;

	for(i = 0; i < nMixtures; ++i){
		ivInt16 fLike;
		fLike = EsSPICal1GaussOutlike(pThis, pGauss + i);
		fOutLike = LAdd(fOutLike, (ivInt32)fLike);
	}

	ivAssert(fOutLike <= 32767 && fOutLike >= -32768);

	return (ivInt16)fOutLike;
}

EsErrID EsSPIUpdateAllScore(PEsSearch pThis)
{ 
    ivInt16 fSilScore;
    ivInt16 i;

    PLexNode pLexNode;	
    PSearchNode pSearchNode;
    ivInt16 fScoreThresh;
    ivInt16 fScore;

    ivInt16 iIndex;
    ivInt16 fFillerScore;
    ivPUInt16 pnBucketVote;
    ivInt16 fScoreMaxPreFrame;

    ivInt16 fOutlike;
    PCMixSModel pState;

#if MINI5_USE_NEWCM_METHOD
    ivBool bTranpLPR;
#endif
#if MINI5_SPI_MODE
    ivPointer pSPIBuffer;
#endif

    fSilScore = pThis->fSilScore;
    fFillerScore = pThis->fFillerScore;

    pSearchNode = pThis->pSearchNodeLast;
    pnBucketVote = pThis->pnBucketCacheLast;
    fScoreThresh = pThis->fScoreThreshLast;
    fScoreMaxPreFrame = pThis->fScoreMaxLast;

    EsUpdateNBest(pThis);
    ivMemZero(pnBucketVote, sizeof(ivInt16)*BUCKET_ALL_COUNT);

    // ivAssert(BUCKET_ALL_COUNT + sizeof(TLexNode)/sizeof(ivInt16) < ivGridSize(TRANSFORM_FFTNUM_DEF+2));
    pLexNode = (PLexNode)((ivAddress)pThis->pnBucketCacheLast + ivGridSize(BUCKET_ALL_COUNT * sizeof(ivInt16)));
    pSPIBuffer = pThis->pLexRoot;

    for(i = pThis->nTotalNodes; i>=1; i--)
    {
        EsSPIReadBuf((ivPointer)pLexNode, pSPIBuffer, sizeof(TLexNode));
        pSPIBuffer = (ivPointer)((ivUInt32)pSPIBuffer + sizeof(TLexNode));

        fScore = pSearchNode[pLexNode->iParent].fScore;
#if MINI5_USE_NEWCM_METHOD
        bTranpLPR = ivFalse;
#endif
        if(pSearchNode[i].fScore > fScoreThresh)
        {
            /* 父节点前跳和其自跳PK */
            if(pSearchNode[i].fScore > fScore)
            {
                fScore = pSearchNode[i].fScore;
#if MINI5_USE_NEWCM_METHOD
                bTranpLPR = ivTrue;
#endif
            }
        }
        else{  //if(pStaScoreCache[i] > fScoreThresh) No
            /* 看父节点 */
            if(fScore <= fScoreThresh)
            {				
                pSearchNode[i].fScore = LZERO;
                continue;
            }
        }			

        pState = (PCMixSModel)((ivUInt32)pThis->pState + pLexNode->iSModeIndex * sizeof(TGModel) * pThis->nMixture);
        fOutlike = EsCalcOutLike(pThis, pState, pThis->nMixture);
        pSearchNode[i].fScore = EsSatInt16(fScore - fFillerScore + fOutlike);
        ivAssert(pSearchNode[i].fScore == fScore - fFillerScore + fOutlike);

#if MINI5_USE_NEWCM_METHOD
        /* 新CM策略相关 */
        ivAssert(0 == pSearchNode[1].fCMScore);
        fScore = fOutlike - fFillerScore;
        if(bTranpLPR){				
            if(fScore < pSearchNode[i].fCurStateCMScore){
                pSearchNode[i].fCurStateCMScore = fScore;
            }
        }
        else{
            pSearchNode[i].fCurStateCMScore = fScore;
            pSearchNode[i].fCMScore = pSearchNode[pLexNode->iParent].fCMScore + pSearchNode[pLexNode->iParent].fCurStateCMScore;	
        }
#endif
        /* Bucket Sort  以上帧MaxScore为标准进行比较, 从ID=0的桶开始:|...|12|8|4|0|-4|-8|-12|...例如某个值x属于(-4,0],则投入标0的那个桶 */	
        ivAssert(64 == BUCKET_SCORE_STEP);
        iIndex = (ivInt16)BUCKET_PLUS_SCORE_COUNT + ((fScoreMaxPreFrame -  pSearchNode[i].fScore)>>6);			
        if(iIndex < 0){
            iIndex = 0; /* 防止本次得分比上次最高得分偏差太多，写越界!!! */
        }
        if(iIndex < BUCKET_ALL_COUNT){ /*  如果该结点得分在预订桶范围内,则进行投桶计数 */
            pnBucketVote[iIndex]++;	
        }

    }//for(; i>=1; i--,pLexNode--){

    pSearchNode[0].fScore = EsSatInt16(pSearchNode[0].fScore + fSilScore);
#if MINI5_USE_NEWCM_METHOD
    pSearchNode[0].fCMScore = 0; //sil CM得分不传递
    pSearchNode[0].fCurStateCMScore = 0;
#endif

    return EsErrID_OK;
}

#endif
