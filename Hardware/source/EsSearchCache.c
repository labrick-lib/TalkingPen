/*********************************************************************#
//	文件名		：Search.cpp
//	文件功能	：Implements of ESR Search
//	作者		：Truman
//	创建时间	：2007年7月10日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#include "EsKernel.h"
#include "EsSearch.h"
#include "EsEngine.h"
#include "EsSearchConst.h"


#if !LOAD_FLOAT_MDOEL

extern void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID);

void EsGetPruneThresh(ivPUInt16 pnBucketVote, ivInt16 nPruneMaxNode, ivPInt16 pfScoreMaxPreFrm, ivPInt16 pfScoreThresh);

/* 和老于写的硕呈平台汇编码对应 20101201 */

/*  对识别影响不大，但是会导致和原始C代码对不上
1.当使用beam门限时，C代码是用的当前帧max预估的，汇编码是使用上帧max的？？
2.当n = ivMin(i+BUCKET_BEAM_COUNT+1, BUCKET_ALL_COUNT);取的是BUCKET_ALL_COUNT，并且所有桶个数都不足裁剪node个数时，后面的*pfScoreThresh = fScoreBase - (i-    	  
1)*(ivUInt16)BUCKET_SCORE_STEP和C代码也是不匹配的
*/

void EsGetPruneThresh(ivPUInt16 pnBucketVote, ivInt16 nPruneMaxNode, ivPInt16 pfScoreMaxPreFrm, ivPInt16 pfScoreThresh) /* 与硕呈平台AitalkMini1.0代码比较: 同EsPruneByBucketSort略微差别，通过参数传入信息 */
{
    ivInt16 i;
    ivUInt16 nCount;
    ivInt16 fScoreBase;
    ivInt16 n;

    fScoreBase = *pfScoreMaxPreFrm + BUCKET_EXCEED_MAX_SCORE;
    /* Get fScoreMax & fThreshScore */
    i = 0;
    while(pnBucketVote[i++] <= 0)
    {
        if (i>= BUCKET_ALL_COUNT )
        {
            *pfScoreThresh = LZERO;
            return;
        }
    }

    *pfScoreMaxPreFrm = fScoreBase - (i-1)*(ivInt16)BUCKET_SCORE_STEP;

    i--; /* 第一个不为空的桶 */
    nCount = 0;

    n = ivMin(i+BUCKET_BEAM_COUNT+1, BUCKET_ALL_COUNT);

    for(; i<n; i++){
        nCount = nCount + pnBucketVote[i];
        if(nCount >= nPruneMaxNode){
            break;
        }
    }

    if(BUCKET_ALL_COUNT == i){
        i--;	//为了和汇编码对应 20101220
    }
    *pfScoreThresh = fScoreBase - (i-1)*(ivUInt16)BUCKET_SCORE_STEP;
}

#if 0
/* 从桶排序结果中计算当前帧fScoreMax及fScorThresh,供下帧使用 */
void EsGetPruneThreshXXX(ivPUInt16 pnBucketVote, ivInt16 nPruneMaxNode, ivPInt16 pfScoreMaxPreFrm, ivPInt16 pfScoreThresh) /* 与硕呈平台AitalkMini1.0代码比较: 同EsPruneByBucketSort略微差别，通过参数传入信息 */
{
    ivInt16 i;
    ivUInt16 nCount;
    ivInt16 fScoreThresh = 0;
    ivInt16 fScoreMaxPreFrm;
    ivBool bGotScoreMax = ivFalse;

    fScoreMaxPreFrm = *pfScoreMaxPreFrm;
    /* Get fScoreMax & fThreshScore */
    nCount = 0;
    for(i=0; i<BUCKET_ALL_COUNT; i++){
        nCount = nCount + pnBucketVote[i];

        if(!bGotScoreMax && pnBucketVote[i] > 0){
            /* 以下根据投桶时公式反算得到 iIndex = (ivInt16)BUCKET_PLUS_SCORE_COUNT + (fScoreMaxPreFrame -  pStaScoreCache[i])/(ivInt16)BUCKET_SCORE_STEP;	 */
            *pfScoreMaxPreFrm += ((ivInt16)BUCKET_PLUS_SCORE_COUNT - i)* (ivInt16)BUCKET_SCORE_STEP;
            fScoreThresh = *pfScoreMaxPreFrm - (ivInt16)BEAM_THRESH_LOW; /* 当节点数小于裁剪结点门限值时,用BeamThresh作为门限 */
            bGotScoreMax = ivTrue;
        }

        if(nCount >= nPruneMaxNode){
            /* 取i-1个桶作为界限 */
            ivInt16 fTemp = fScoreMaxPreFrm + ((ivInt16)BUCKET_PLUS_SCORE_COUNT - i + 1)* (ivInt16)BUCKET_SCORE_STEP;
            if (fTemp > fScoreThresh)
            {
                fScoreThresh = fTemp;
            }			
            break;
        }
    }

    *pfScoreThresh = fScoreThresh;
}
#endif

void EsGetPruneThreshByBucket(PEsSearch pThis) /* 与硕呈平台AitalkMini1.0代码比较: 新增接口 */
{
    EsGetPruneThresh(pThis->pnBucketCacheLast, (ivInt16)BUCKTE_PRUNE_MAX_NODE, &pThis->fScoreMaxLast, &pThis->fScoreThreshLast);
}

/* return Q4 */
ivInt32 LAdd(ivInt32 x, ivInt32 y)
{
    ivInt32 diff, n32Ret;

    //log(1+exp(-x)) Q4. 0<x<4, 将0~4的区间均匀的分为64段，取每段的中间值.
    /*	ivStatic ivConst ivInt32 g_fLaddTab[44] = {
    10, // 0.0313
    10, // 0.0938
    9, // 0.1563
    9, // 0.2188
    8, // 0.2813
    8, // 0.3438
    8, // 0.4063
    7, // 0.4688
    7, // 0.5313
    7, // 0.5938
    6, // 0.6563
    6, // 0.7188
    6, // 0.7813
    5, // 0.8438
    5, // 0.9063
    5, // 0.9688
    4, // 1.0313
    4, // 1.0938
    4, // 1.1563
    4, // 1.2188
    3, // 1.2813
    3, // 1.3438
    3, // 1.4063
    3, // 1.4688
    3, // 1.5313
    2, // 1.5938
    2, // 1.6563
    2, // 1.7188
    2, // 1.7813
    2, // 1.8438
    2, // 1.9063
    2, // 1.9688
    1, // 2.0313
    1, // 2.0938
    1, // 2.1563
    1, // 2.2188
    1, // 2.2813
    1, // 2.3438
    1, // 2.4063
    1, // 2.4688
    1, // 2.5313
    1, // 2.5938
    1, // 2.6563
    1 // 2.7188
    };
    */
    ivStatic ivConst ivInt32 g_fLaddTab[28] = {
        11,
        10,
        9,
        8,
        7,
        7,
        6,
        5,
        5,
        4,
        4,
        3,
        3,
        3,
        2,
        2,
        2,
        2,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1
    };

    ivAssert(4 == ESR_Q_SCORE);
    if(y>x){
        diff = y-x;	/* Q4 */
        diff >>= 1;
        if(diff >= 28){
            return y;
        }
        else{
            n32Ret = y+g_fLaddTab[diff];
            return n32Ret;
        }
    }
    else{
        diff = x-y;
        diff >>= 1;
        if(diff >= 28){
            return x;
        }
        else{
            n32Ret = x+g_fLaddTab[diff];
            return n32Ret;
        }
    }
}

void EsUpdateNBest(PEsSearch pThis)
{
    ivUInt16 i;
    ivInt16 fSilScore = pThis->fSilScore;
    ivInt16 fScoreThresh = pThis->fScoreThreshLast;
    PSearchNode pSearchNode;

#if MINI5_USE_NEWCM_METHOD
    # if MINI5_SPI_MODE
    PLexNode pLexNode = (PLexNode)pThis->pnBucketCacheLast;

    ivAssert((TRANSFORM_FFTNUM_DEF+2) * sizeof(ivInt16) >= pThis->nCmdNum * sizeof(TLexNode));
    EsSPIReadBuf((ivPointer)pLexNode, (ivPointer)pThis->pLexRoot, pThis->nCmdNum * sizeof(TLexNode));
    pLexNode = pLexNode + (pThis->nCmdNum - 1);
    #else
    PLexNode pLexNode = pThis->pLexRoot + (pThis->nCmdNum - 1);
    #endif /* MINI5_SPI_MODE */
#endif /* MINI5_USE_NEWCM_METHOD */

    ivAssert(pThis->tNBest[0].fScore + fSilScore >= -32768 && pThis->tNBest[0].fScore + fSilScore <= 32767);
    pThis->tNBest[0].fScore = pThis->tNBest[0].fScore + fSilScore;
    if(pThis->tNBest[0].fScore <= LZERO){
        pThis->tNBest[0].fScore = LZERO;
    }

    ivAssert(pThis->tNBest[1].fScore + fSilScore >= -32768 && pThis->tNBest[1].fScore + fSilScore <= 32767);
    pThis->tNBest[1].fScore = pThis->tNBest[1].fScore + fSilScore;
    if(pThis->tNBest[1].fScore <= LZERO){
        pThis->tNBest[1].fScore = LZERO;
    }

    ivAssert(pThis->tNBest[2].fScore + fSilScore >= -32768 && pThis->tNBest[2].fScore + fSilScore <= 32767);
    pThis->tNBest[2].fScore = pThis->tNBest[2].fScore + fSilScore;
    if(pThis->tNBest[2].fScore <= LZERO){
        pThis->tNBest[2].fScore = LZERO;
    }

    pSearchNode = pThis->pSearchNodeLast + pThis->nTotalNodes - pThis->nCmdNum + 1;

    /* 叶子节点可能传到EndFiller */
    for(i = 1; i <= pThis->nCmdNum; i++, pSearchNode++){
        if(pSearchNode->fScore > fScoreThresh){
#if MINI5_USE_NEWCM_METHOD
            ivAssert(pLexNode->nStateCount > 0);
            EsUpdateEndFiller(pThis, pSearchNode, pLexNode->nStateCount, i);
#else
            EsUpdateEndFiller(pThis, pSearchNode, 0, i);
#endif /* MINI5_USE_NEWCM_METHOD */
        }
#if MINI5_USE_NEWCM_METHOD
        pLexNode--;
#endif /* MINI5_USE_NEWCM_METHOD */
    }
}

#if LOG_SEARCH_SCORE
extern FILE *g_fpOutlike;
#endif

#if !MINI5_SPI_MODE
ivInt16 EsCalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别，通过参数参入,返回值由int32改为int16 */
{
    ivInt16 i;
    ivInt32 fOutLike;
    PCGModel pGaussian;

    pGaussian = pState->tGModel;

    fOutLike = LZERO;
    for(i=0;i < nMixtures;++i,++pGaussian){
        ivInt16 fLike;

        fLike = EsCal1GaussOutlike(pThis, pGaussian);

        fOutLike = LAdd(fOutLike, (ivInt32)fLike);
    }

    ivAssert(fOutLike <= 32767 && fOutLike >= -32768);

#if LOG_SEARCH_SCORE
    {
        fprintf(g_fpOutlike, "%s : %.2f\r\n", "  ", fOutLike*1.0/(1<<ESR_Q_SCORE));       
        fflush(g_fpOutlike);
    }
#endif

    return (ivInt16)fOutLike;
}

ivInt16 EsCal1GaussOutlike(PEsSearch pThis, PCGModel pGModel) /* 与硕呈平台AitalkMini1.0代码比较: 大变动 */
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
    ivAssert(8 == ESR_Q_WEIGHT_GCOSNT_MANUAL);    

    for(i = FEATURE_DIMNESION; i; i -= 4){
        fValue = (*pfMeanVar++) -  (*pfFeature++); 
        ivAssert(fValue < (1<<(ESR_VAR_QUANT_BIT+ESR_MEAN_QUANT_BIT+1)));
        f32Like -= g_s16MeanVarTable[fValue];  /* Q8 */

        fValue = (*pfMeanVar++) -  (*pfFeature++); 
        ivAssert(fValue < (1<<(ESR_VAR_QUANT_BIT+ESR_MEAN_QUANT_BIT+1)));
        f32Like -= g_s16MeanVarTable[fValue];  /* Q8 */

        fValue = (*pfMeanVar++) -  (*pfFeature++); 
        ivAssert(fValue < (1<<(ESR_VAR_QUANT_BIT+ESR_MEAN_QUANT_BIT+1)));
        f32Like -= g_s16MeanVarTable[fValue];  /* Q8 */

        fValue = (*pfMeanVar++) -  (*pfFeature++); 
        ivAssert(fValue < (1<<(ESR_VAR_QUANT_BIT+ESR_MEAN_QUANT_BIT+1)));
        f32Like -= g_s16MeanVarTable[fValue];  /* Q8 */
    }

    ivAssert(ESR_Q_WEIGHT_GCOSNT_MANUAL == ESR_Q_MEANVARTAB_MANUAL);

    f32Like >>= (ESR_Q_MEANVARTAB_MANUAL+1-ESR_Q_SCORE);	/* Q8->ESR_Q_SCORE and *0.5 = (8-ESR_Q_SCORE+1) */

    ivAssert(f32Like >= -32768 && f32Like <= 32767);

    return (ivInt16)f32Like;
}

EsErrID EsUpdateAllScore(PEsSearch pThis)
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

    pLexNode = pThis->pLexRoot;
    for(i = pThis->nTotalNodes; i>=1; i--,pLexNode++)
    {
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

#if LOG_DEBUG_INFO
    {
        static FILE *g_fpSearchScore = NULL;
        if(1 == pThis->iFrameIndexLast)
        {
            g_fpSearchScore = fopen("E:\\SearchScore_mini5.log", "wb");
        }
        fprintf(g_fpSearchScore, "iFrame = %d\r\n", pThis->iFrameIndexLast);
        fprintf(g_fpSearchScore, " fSilScore=%d, fFillerScore=%d\r\n", pThis->fSilScore, pThis->fFillerScore);

        for(i=0; i<=pThis->nTotalNodes; i++)
        {
#if MINI5_USE_NEWCM_METHOD
            fprintf(g_fpSearchScore, " %d: fScore=%d, fCMScore=%d, fCurStateCMScore=%d\r\n", i, pSearchNode[i].fScore, pSearchNode[i].fCMScore, pSearchNode[i].fCurStateCMScore);   
#else
            fprintf(g_fpSearchScore, " %d: fScore=%d\r\n", i, pSearchNode[i].fScore);   
#endif
        }
        fflush(g_fpSearchScore);
    }
#endif

#if LOG_SEARCH_SCORE
    {
        static FILE *g_fpSearchScoreFlt = NULL;
        if(1 == pThis->iFrameIndexLast)
        {
            g_fpSearchScoreFlt = fopen("E:\\SearchScore_mini5.log", "wb");
        }
        fprintf(g_fpSearchScoreFlt, "iFrame = %d\r\n", pThis->iFrameIndexLast);
        fprintf(g_fpSearchScoreFlt, " fSilScore=%.2f, fFillerScore=%.2f\r\n", pThis->fSilScore*1.0/(1<<ESR_Q_SCORE), pThis->fFillerScore*1.0/(1<<ESR_Q_SCORE));

        for(i=0; i<=pThis->nTotalNodes; i++)
        {
#if  MINI5_USE_NEWCM_METHOD
            fprintf(g_fpSearchScoreFlt, " %d: fScore=%.2f, fCMScore=%.2f, fCurStateCMScore=%.2f\r\n", 
                i, pSearchNode[i].fScore*1.0/(1<<ESR_Q_SCORE), pSearchNode[i].fCMScore*1.0/(1<<ESR_Q_SCORE), pSearchNode[i].fCurStateCMScore*1.0/(1<<ESR_Q_SCORE));   
#else
            fprintf(g_fpSearchScoreFlt, " %d: fScore=%.2f\r\n", i, pSearchNode[i].fScore*1.0/(1<<ESR_Q_SCORE));   
#endif
        }
        fflush(g_fpSearchScoreFlt);
    }
#endif

    return EsErrID_OK;
}

#endif /* !MINI5_SPI_MODE */

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT

#if LOG_DEBUG_INFO
LogTagSearchScore(PEsSearch pThis)
{
    int i;
    FILE *fp;
    if(0 == pThis->iFrameIndexLast)
    {
        fp = fopen("E:\\TagSearchScore_mini5.log", "wb");
    }
    else
    {
        fp = fopen("E:\\TagSearchScore_mini5.log", "ab");
    }
    fprintf(fp, "iFrame=%d\r\n", pThis->iFrameIndexLast);
    fprintf(fp, "   fSilScore=%d, fFillerScore=%d\r\n", pThis->fSilScore, pThis->fFillerScore);
    for(i=0; i<pThis->nClusters+2; i++)
    {
        fprintf(fp, "   fScore=%d, nRepeatFrm=%d\r\n", pThis->pStaScoreCacheLast[i], pThis->pnRepeatFrmCacheLast[i]);
    }

    fclose(fp);
}
#endif

EsErrID EsTagUpdateAllScore(PEsSearch pThis) /* 与硕呈平台AitalkMini1.0代码比较: 变动较多，VADw未检测到语音时加入了裁剪 */
{
    ivInt16 fScore, fScoreMaxPre, fScoreMaxCur, fOutLike;
    ivInt16 i, iMaxPre, iMaxCur, nLastMaxRepeat;
    ivPInt16 pScoreCacheLast, pnRepeatFrmCacheLast;
    TLexNode tLexNode;
    ivInt16 fSilScore;
    ivInt16 fFillerScore;
    TPathItem tPathTmp;
    EsErrID err;
    PCMixSModel pState;

    fSilScore = pThis->fSilScore;
    fFillerScore = pThis->fFillerScore;

    pScoreCacheLast = pThis->pStaScoreCacheLast;
    pnRepeatFrmCacheLast = pThis->pnRepeatFrmCacheLast;

    fScoreMaxPre = pThis->fScoreMaxLast;
    iMaxPre = pThis->iMaxStateLast;

    /* Update BeginSil */
    pScoreCacheLast[0] += fSilScore;

    /* Update EndSil */
    fScore = ivMax(pScoreCacheLast[pThis->nClusters+1], fScoreMaxPre);
    pScoreCacheLast[pThis->nClusters+1] = fScore + fSilScore;

#ifdef TAG_SEARCH_ADD_SIL
    if(pScoreCacheLast[0] > fScoreMaxPre){
        fScoreMaxPre = pScoreCacheLast[0];
        iMaxPre = 0;
    }
#endif

    fScoreMaxCur = LZERO;
    fScoreMaxPre -= STATE_LPF_PUNISH_SCORE; //父节点跳出加入惩罚 
    iMaxCur = 1;
    nLastMaxRepeat = pnRepeatFrmCacheLast[iMaxPre]+1;
    for(i=1; i<=pThis->nClusters; i++){
        if(pScoreCacheLast[i] > fScoreMaxPre){
            /* 自跳 */
            fScore = pScoreCacheLast[i];
            pnRepeatFrmCacheLast[i]++;
        }
        else{
            fScore = fScoreMaxPre;
            pnRepeatFrmCacheLast[i] = 1; 
        }

        tLexNode.iSModeIndex = i-1;
        pState = (PCMixSModel)((ivUInt32)pThis->pState + tLexNode.iSModeIndex * sizeof(TGModel) * pThis->nMixture);
        fOutLike = EsCalcOutLike(pThis, pState, pThis->nMixture);

        pScoreCacheLast[i] = EsSatInt16(fScore + fOutLike - fFillerScore);
        ivAssert(pScoreCacheLast[i] == fScore + fOutLike - fFillerScore);
        if(pScoreCacheLast[i] > fScoreMaxCur){
            fScoreMaxCur = pScoreCacheLast[i];
            iMaxCur = i;
        }
    }

    pnRepeatFrmCacheLast[iMaxPre] = nLastMaxRepeat; /* 父节点是自身.仍自跳 */

    pThis->fScoreMaxLast = fScoreMaxCur;
    pThis->iMaxStateLast = iMaxCur;

    /* 将一帧的path保存.先缓存到256字节的ram中，如果ram满了256字节，就向SPI写一次 */
    tPathTmp.iState = iMaxCur - 1;
    tPathTmp.nRepeatFrm = pnRepeatFrmCacheLast[iMaxCur];
    err = EsSPIWriteData(&pThis->tSPICache, (ivPInt8)(&tPathTmp), sizeof(tPathTmp));
    if(EsErrID_OK != err)
    {
        return err;
    }
    err = EsSPIWriteData(&pThis->tSPICache, (ivPInt8)pThis->fFeature, sizeof(pThis->fFeature));
    if(EsErrID_OK != err)
    {
        return err;
    }

#if LOG_DEBUG_INFO
    LogTagSearchScore(pThis);
#endif

    return EsErrID_OK;
}
#endif

#endif
