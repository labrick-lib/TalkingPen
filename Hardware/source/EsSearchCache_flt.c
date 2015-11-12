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
#include "ESSearchConst.h"

#if LOAD_FLOAT_MDOEL

extern void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID);

extern void EsUpdateNBest(PEsSearch pThis);

void EsGetPruneThresh(ivPUInt16 pnBucketVote, ivInt16 nPruneMaxNode, float * pfScoreMaxPreFrm, float * pfScoreThresh);

/* 和老于写的硕呈平台汇编码对应 20101201 */

/*  对识别影响不大，但是会导致和原始C代码对不上
1.当使用beam门限时，C代码是用的当前帧max预估的，汇编码是使用上帧max的？？
2.当n = ivMin(i+BUCKET_BEAM_COUNT+1, BUCKET_ALL_COUNT);取的是BUCKET_ALL_COUNT，并且所有桶个数都不足裁剪node个数时，后面的*pfScoreThresh = fScoreBase - (i-    	  
1)*(ivUInt16)BUCKET_SCORE_STEP和C代码也是不匹配的
*/
void EsGetPruneThresh(ivPUInt16 pnBucketVote, ivInt16 nPruneMaxNode, float * pfScoreMaxPreFrm, float * pfScoreThresh) /* 与硕呈平台AitalkMini1.0代码比较: 同EsPruneByBucketSort略微差别，通过参数传入信息 */
{
    ivInt16 i;
    ivUInt16 nCount;
    float fScoreBase;
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
        nCount += pnBucketVote[i];
        if(nCount >= nPruneMaxNode){
            break;
        }
    }

    if(BUCKET_ALL_COUNT == i){
        i--;	//为了和汇编码对应 20101220
    }
    *pfScoreThresh = fScoreBase - (i-1)*(ivUInt16)BUCKET_SCORE_STEP;
}

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

void EsGetPruneThreshByBucket(PEsSearch pThis) /* 与硕呈平台AitalkMini1.0代码比较: 新增接口 */
{
    EsGetPruneThresh(pThis->pnBucketCacheLast, (ivInt16)BUCKTE_PRUNE_MAX_NODE, &pThis->fScoreMaxLast, &pThis->fScoreThreshLast);
}

void EsUpdateNBest(PEsSearch pThis)
{
    ivUInt16 i;
    float fSilScore = pThis->fSilScore;
    float fScoreThresh = pThis->fScoreThreshLast;
    PSearchNode pSearchNode;
    PLexNode pLexNode = pThis->pLexRoot;

    pThis->tNBest[0].fScore = pThis->tNBest[0].fScore + fSilScore;
    if(pThis->tNBest[0].fScore <= LZERO){
        pThis->tNBest[0].fScore = LZERO;
    }

   
    pThis->tNBest[1].fScore = pThis->tNBest[1].fScore + fSilScore;
    if(pThis->tNBest[1].fScore <= LZERO){
        pThis->tNBest[1].fScore = LZERO;
    }

    pThis->tNBest[2].fScore = pThis->tNBest[2].fScore + fSilScore;
    if(pThis->tNBest[2].fScore <= LZERO){
        pThis->tNBest[2].fScore = LZERO;
    }

    pSearchNode = pThis->pSearchNodeLast + pThis->nTotalNodes - pThis->nCmdNum + 1;
    pLexNode += (pThis->nCmdNum - 1);

    /* 叶子节点可能传到EndFiller */
    for(i = 1; i <= pThis->nCmdNum; i++, pSearchNode++, pLexNode--){
        if(pSearchNode->fScore > fScoreThresh){
            ivAssert(pLexNode->nStateCount > 0);
            EsUpdateEndFiller(pThis, pSearchNode, pLexNode->nStateCount, i);
        }
    }
}

#include <math.h>
double LAddFloat(double x, double y)
{
    double temp,diff,z;
    if (x < y)
    {
        temp = x; x = y; y = temp;
    }
    diff = y - x;
    if (diff < LMINEXP)
    {
        return (x < LSMALL)?LZERO:x;
    }
    else 
    {
        z = exp(diff);
        return x + log(1.0 + z);
    }
}

float ESCalcFillerOutLike(PEsSearch pThis, PCMixSModel pFillerState, ivUInt16 nMixtures) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别，通过参数参入,返回值由int32改为int16 */
{
    ivInt16 i;
    double fOutLike;
    PCGModel pGaussian;

    pGaussian = pFillerState->tGModel;

    fOutLike = LZERO;
    for(i=0;i < nMixtures;++i,++pGaussian){
        float fLike;

        fLike = EsCal1GaussOutlike(pThis, pGaussian);

        fOutLike = LAddFloat(fOutLike, fLike);
    }

    return (float)fOutLike;   //这里误差大概是6
}

#if LOG_SEARCH_SCORE
extern FILE *g_fpOutlike;
#endif
float ESCalcOutLike(PEsSearch pThis, PCMixSModel pState, ivUInt16 nMixtures) /* 与硕呈平台AitalkMini1.0代码比较: 前面相同，后面增加了模糊音的计算 */
{
    ivInt16 i;
    double fOutLike;

    fOutLike = LZERO;
    for(i=0; i < nMixtures; ++i){
        float fLike;
        fLike = EsCal1GaussOutlike(pThis, pState->tGModel + i);
        fOutLike = LAddFloat(fOutLike, fLike);
    }

#if LOG_SEARCH_SCORE
    {
        fprintf(g_fpOutlike, "%s : %.2f\r\n", pState->szName, fOutLike);       
        fflush(g_fpOutlike);
    }
#endif

    return (float)fOutLike;
}

float EsCal1GaussOutlike(PEsSearch pThis, PCGModel pGModel) /* 与硕呈平台AitalkMini1.0代码比较: 大变动 */
{
    ivInt16 i;

    float fValue;
    double f32Like;

    float * pfFeature;

    float * pfMean;
    float * pfVar;

    pfFeature = pThis->fFeature; /*  16位数据High-Low  | 5bit符号扩展 | 7bit fea (Q6) | 0000 | */

    /* 原始计算公式 1/2[-fConst - ((fea - mean)^2)/var]	 */
    /*    变换: ((fea - mean)^2)/var = [(fea*meancoef - mean*meancoef)*sqrt(1/(var*meancoef*meancoef))]^2     AAA */
    /*    全局均值方差表g_s16MeanVarTable中存储的就是公式AAA的结果,(fea*meancoef - mean*meancoef)占8bit, var'占4bit，该表是2^12=2028大小的表 */
    /* 为了实现mean7bit,var4bit存储，在打包模型时做了处理,存储的mean' = mean*meancoef, var'=ln(var*meancoef*meancoef)*16/varcoef */

    pfMean = pGModel->fltMean; /* 16位数据High-Low  | 5bit符号扩展 | 7bit mean (Q6) | 4bit Var (Q0) | */ 
    pfVar = pGModel->fltVar;
    f32Like = 2.0*(pGModel->fltWeight) - pGModel->fltGConst;  /* Q8 */

    for(i = FEATURE_DIMNESION; i; i--){
        fValue = (*pfMean++) -  (*pfFeature++); 
        
        f32Like -= (fValue*fValue)/(*pfVar++);  /* Q8 */
    }

    f32Like = 0.5 * f32Like;

    /* f32Like >>= (9-ES_Q_TRANSP); */	/* Q8->ES_Q_TRANSP and *0.5 = (8-ES_Q_TRANSP+1) */

    /*ivAssert(f32Like >= -32768 && f32Like <= 32767);*/

    return (double)f32Like;
}

#include <stdio.h>
EsErrID EsUpdateAllScore(PEsSearch pThis)
{ 
    float fSilScore;
    ivInt16 i;

    PLexNode pLexNode;	
    PSearchNode pSearchNode;
    float fScoreThresh;
    float fScore;

    ivInt16 iIndex;
    float fFillerScore;
    ivPUInt16 pnBucketVote;
    float fScoreMaxPreFrame;

    float fOutlike;

    PCMixSModel pState;

#if MINI5_USE_NEWCM_METHOD
    ivBool bTranpLPR;
#endif

    fSilScore = pThis->fSilScore;
    fFillerScore = pThis->fFillerScore;

    pLexNode = pThis->pLexRoot;
    pSearchNode = pThis->pSearchNodeLast;
    pnBucketVote = pThis->pnBucketCacheLast;
    fScoreThresh = pThis->fScoreThreshLast;
    fScoreMaxPreFrame = pThis->fScoreMaxLast;

    fScoreThresh = LZERO;

    EsUpdateNBest(pThis);

    ivMemZero(pnBucketVote, sizeof(ivInt16)*BUCKET_ALL_COUNT);

    i = pThis->nTotalNodes;

    for(; i>=1; i--,pLexNode++)
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

        pState = (PCMixSModel)((ivUInt32)pThis->pState + pLexNode->iSModeIndex * (sizeof(TMixStateModel) + sizeof(TGModel) * (pThis->nMixture-1)));
        fOutlike = ESCalcOutLike(pThis, pState, pThis->nMixture);
        pSearchNode[i].fScore = fScore - fFillerScore + fOutlike;

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
        iIndex = (ivInt16)BUCKET_PLUS_SCORE_COUNT + ((fScoreMaxPreFrame -  pSearchNode[i].fScore)/4);			
        if(iIndex < 0){
            iIndex = 0; /* 防止本次得分比上次最高得分偏差太多，写越界!!! */
        }
        if(iIndex < BUCKET_ALL_COUNT){ /*  如果该结点得分在预订桶范围内,则进行投桶计数 */
            pnBucketVote[iIndex]++;	
        }
    }//for(; i>=1; i--,pLexNode--){

    pSearchNode[0].fScore = pSearchNode[0].fScore + fSilScore;
#if MINI5_USE_NEWCM_METHOD
    pSearchNode[0].fCMScore = 0; //sil CM得分不传递
    pSearchNode[0].fCurStateCMScore = 0;
#endif

#if LOAD_FLOAT_MDOEL && LOG_SEARCH_SCORE
    {
        static FILE *g_fpSearchScoreFlt = NULL;
        if(1 == pThis->iFrameIndexLast)
        {
            g_fpSearchScoreFlt = fopen("E:\\SearchScoreFlt_mini5.log", "wb");
        }
        fprintf(g_fpSearchScoreFlt, "iFrame = %d\r\n", pThis->iFrameIndexLast);
        fprintf(g_fpSearchScoreFlt, " fSilScore=%.2f, fFillerScore=%.2f\r\n", pThis->fSilScore, pThis->fFillerScore);

        for(i=0; i<=pThis->nTotalNodes; i++)
        {
#if  MINI5_USE_NEWCM_METHOD
            fprintf(g_fpSearchScoreFlt, " %d: fScore=%.2f, fCMScore=%.2f, fCurStateCMScore=%.2f\r\n", i, pSearchNode[i].fScore, pSearchNode[i].fCMScore, pSearchNode[i].fCurStateCMScore);   
#else
            fprintf(g_fpSearchScoreFlt, " %d: fScore=%.2f\r\n", i, pSearchNode[i].fScore);   
#endif
        }
        fflush(g_fpSearchScoreFlt);
    }
#endif

    return EsErrID_OK;
}

#endif

