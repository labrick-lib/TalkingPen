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

#if LOAD_FLOAT_MDOEL

extern ivPInt16        g_ps16MeanCoef; 
extern ivPInt16        g_ps16MeanCoefQ;
void ESCalcFiller(PEsSearch pThis);
void ESCalcSil(PEsSearch pThis);

float ESCalcFillerOutLike(PEsSearch pThis, PCMixSModel pFillerState, ivUInt16 nMixtures); 

EsErrID EsUpdateAllScore(PEsSearch pThis);

#if MINI5_TAG_SUPPORT
EsErrID EsTagUpdateAllScore(PEsSearch pThis);
#endif

void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID);
void EsGetPruneThreshByBucket(PEsSearch pThis);
void EsUpdateNBest(PEsSearch pThis);

#if LOG_SEARCH_SCORE
FILE *g_fpOutlike = NULL;
#endif

EsErrID EsSearchInit(PEsSearch pThis, ivCPointer pModel, PEsrEngine pEngine)
{
	PCResHeader pResHdr;
	ivUInt32 nCRC = 0, nCounter = 0;

	ivAssert(ivNull != pThis && ivNull != pModel && ivNull != pEngine);
	if(ivNull == pThis || ivNull == pModel ||ivNull == pEngine){
		return EsErr_InvCal;
	}

    ivMemZero(pThis, sizeof(TEsSearch));
	pResHdr = (PCResHeader)pModel;

    if((ESENGINE_TAG == pEngine->iEngineType && DW_TAG_MODEL_CHECK != pResHdr->dwCheck)
        || (ESENGINE_REC == pEngine->iEngineType && DW_MODEL_CHECK != pResHdr->dwCheck))
    {
        return EsErr_InvRes;
    }

    ivMakeCRC((ivPCInt16)((ivUInt32)pResHdr + sizeof(ivUInt32)*2), pResHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
	if(nCRC != pResHdr->nCRC){
		return EsErr_InvRes;
	}

	/* Init Resource */
	/* Load Model Info */
	pThis->pSilState = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nSilSModelOffset * sizeof(ivInt16))); /* 重定位. Sil State */
	pThis->pState = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nMixSModelOffset * sizeof(ivInt16)));
    pThis->nMixture = (ivUInt16)pResHdr->nMixture;
    pThis->nSilMixture = (ivUInt16)pResHdr->nSilMixture;    
#if MINI5_USE_NEWCM_METHOD
    pThis->pFillerState = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pThis->nFillerMixture = (ivUInt16)pResHdr->nFillerMixture;
#else
    pThis->pFillerState = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nOldFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pThis->nFillerCnt = pResHdr->nOldFillerCnt;
    pThis->nFillerMixture = (ivUInt16)pResHdr->nOldFillerMix;
    if(0 == pThis->nFillerCnt)
    {
        return EsErr_InvRes;
    }
#endif

    g_ps16MeanCoef = (ivPInt16)(pResHdr->s16MeanCoef);
    g_ps16MeanCoefQ = (ivPInt16)(pResHdr->s16MeanCoefQ);

	/* 桶排序要使用的内存复用FFT的内存块 */
	ivAssert(BUCKET_ALL_COUNT*2 < TRANSFORM_FFTNUM_DEF+2);
	pThis->pnBucketCacheLast = (ivPUInt16)(pEngine->tFront.m_pFrameCache);	

    pThis->pSearchNodeLast = ivNull;
	pThis->nTotalNodes = 0;
	pThis->nExtendNodes = 0;

    pThis->iEngineType = pEngine->iEngineType;

#if MINI5_TAG_SUPPORT
    pThis->pStaScoreCacheLast = ivNull;

	if(ESENGINE_TAG == pEngine->iEngineType){
	
        pThis->pStaScoreCacheLast = ivNull;

        pThis->pSPIBufBase = pEngine->pSPIBufBase;
        pThis->pSPIBufEnd = pEngine->pSPIBufEnd;
        pThis->pSPIBuf = pThis->pSPIBufBase;

        pThis->nClusters = (ivUInt16)(pResHdr->nMixSModelNum - 2 - pResHdr->nOldFillerCnt); /* -2:sil, filler */

        pThis->pStaScoreCacheLast = (ivPInt16)EsrAlloc(pEngine, (pThis->nClusters+2) * sizeof(ivInt16)); /* +2:BeginSil,EndSil */
		ivAssert(ivNull != pThis->pStaScoreCacheLast);		

		pThis->pnRepeatFrmCacheLast= (ivPInt16)EsrAlloc(pEngine, (pThis->nClusters+2) * sizeof(ivInt16));
		ivAssert(ivNull != pThis->pnRepeatFrmCacheLast);		

        pThis->pCacheSPI = (ivPInt8)EsrAlloc(pEngine, MINI5_SPI_PAGE_SIZE);
        ivAssert(ivNull != pThis->pCacheSPI);
	}
#endif

	EsSearchReset(pThis);

	return 0;
}

EsErrID EsSetLexicon(PEsSearch pThis,ivCPointer pLexicon, PEsrEngine pEngine)
{
	PLexiconHeader pLexHdr;
	ivUInt32 nSize;
    ivUInt16 i;

	pLexHdr = (PLexiconHeader)pLexicon;
	ivAssert(DW_LEXHEADER_CHECK == pLexHdr->dwCheck);
	if(DW_LEXHEADER_CHECK != pLexHdr->dwCheck){
		return EsErr_InvRes;
	}

	/* Cache for Calculation of all State */
	nSize = sizeof(TSearchNode)*(pLexHdr->nTotalNodes+1);

	ivAssert((ivUInt32)pEngine->pBuffer+nSize <= (ivUInt32)pEngine->pBufferEnd);
	if((ivUInt32)pEngine->pBuffer+nSize > (ivUInt32)pEngine->pBufferEnd){
		return EsErr_OutOfMemory;
	}

	ivAssert((pLexHdr->nExtendNodes<<1) < pLexHdr->nTotalNodes);
	pThis->pSearchNodeLast = (PSearchNode)pEngine->pBuffer;
	
	/* 重定位 */
	pThis->pCmdDesc = (PCmdDesc)((ivUInt32)pLexicon + (ivUInt32)(pLexHdr->nCmdDescOffset));
	pThis->nTotalNodes = pLexHdr->nTotalNodes;
	pThis->nExtendNodes = pLexHdr->nExtendNodes;
	pThis->pLexRoot = (PLexNode)((ivUInt32)pLexicon + (ivUInt32)(pLexHdr->nLexRootOffset));

	pThis->nCmdNum = (ivUInt16)(pLexHdr->nCmdNum);
	
	ivMemZero(pThis->pSearchNodeLast, sizeof(TSearchNode)*(pThis->nExtendNodes+1));

    for(i=pThis->nExtendNodes+1; i<=pThis->nTotalNodes; i++){
        pThis->pSearchNodeLast[i].fScore = LZERO;
#if MINI5_USE_NEWCM_METHOD
        pThis->pSearchNodeLast[i].fCMScore = 0;
        pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
    }

#if LOAD_FLOAT_MDOEL
    pThis->fFillerScoreSum = 0;
    if(ivNull != pThis->pSearchNodeLast)
    {
        i = 0;
        pThis->pSearchNodeLast[i].fScore = 0;
#if MINI5_USE_NEWCM_METHOD
        pThis->pSearchNodeLast[i].fCMScore = 0;
        pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
        for(i=1; i<=pThis->nTotalNodes; i++)
        {
            pThis->pSearchNodeLast[i].fScore = LZERO;
#if MINI5_USE_NEWCM_METHOD
            pThis->pSearchNodeLast[i].fCMScore = 0;
            pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
        }
    }

#endif
	
	return EsErrID_OK;
}

void EsSearchReset(PEsSearch pThis)
{
	ivUInt16 i;	

	for(i=0; i<ESR_MAX_NBEST; i++){
		pThis->tNBest[i].fScore = LZERO;
#if MINI5_USE_NEWCM_METHOD
		pThis->tNBest[i].nWordID = 0;
		pThis->tNBest[i].fCMScore = LZERO;
#endif
	}

	pThis->fScoreMaxLast = 0;	
	pThis->fScoreThreshLast = LZERO;

	pThis->iFrameIndexLast = 0;	

	if(ivNull != pThis->pSearchNodeLast){
		ivMemZero(pThis->pSearchNodeLast, sizeof(TSearchNode)*(pThis->nExtendNodes+1));
        for(i=pThis->nExtendNodes+1; i<=pThis->nTotalNodes; i++){
            pThis->pSearchNodeLast[i].fScore = LZERO;
#if MINI5_USE_NEWCM_METHOD
            pThis->pSearchNodeLast[i].fCMScore = 0;
            pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
        }
	}

#if LOAD_FLOAT_MDOEL
    pThis->fFillerScoreSum = 0;
    if(ivNull != pThis->pSearchNodeLast)
    {
        i = 0;
        pThis->pSearchNodeLast[i].fScore = 0;
#if MINI5_USE_NEWCM_METHOD
        pThis->pSearchNodeLast[i].fCMScore = 0;
        pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
        for(i=1; i<=pThis->nTotalNodes; i++)
        {
            pThis->pSearchNodeLast[i].fScore = LZERO;
#if MINI5_USE_NEWCM_METHOD
            pThis->pSearchNodeLast[i].fCMScore = 0;
            pThis->pSearchNodeLast[i].fCurStateCMScore = 0;
#endif
        }
    }
   
#endif

#if MINI5_TAG_SUPPORT
	if(ESENGINE_TAG == pThis->iEngineType){
		pThis->fScoreMaxLast = LZERO;
		pThis->iMaxStateLast = 0;
        pThis->iCacheSPIWrite = 0;

		ivMemZero(pThis->pStaScoreCacheLast, (pThis->nClusters+2) * sizeof(ivInt16));
		ivMemZero(pThis->pnRepeatFrmCacheLast, (pThis->nClusters+2) * sizeof(ivInt16));
	}
#endif
}

EsErrID EsSearchFrameStep(PEsSearch pThis)
{

#if LOG_SEARCH_SCORE
    {
        int i;
        if(NULL == g_fpOutlike)
        {
            g_fpOutlike = fopen("E:\\OutlikeFlt.mini5.log", "wb");
        }
        fprintf(g_fpOutlike, "\r\niFrame=%d\r\n", pThis->iFrameIndexLast);       
        fflush(g_fpOutlike);
    }
#endif

	++pThis->iFrameIndexLast;

	ESCalcFiller(pThis);

	ESCalcSil(pThis);

	EsUpdateAllScore(pThis);

	EsGetPruneThreshByBucket(pThis);

#if LOG_SEARCH_SCORE
    {
        int i;
        static FILE *g_fpSearch = NULL;
        if(NULL == g_fpSearch)
        {
            g_fpSearch = fopen("E:\\SearchFlt.mini5.log", "wb");
        }
        fprintf(g_fpSearch, "\r\niFrame=%d\r\n", pThis->iFrameIndexLast - 1);
        for(i=0; i<pThis->nTotalNodes; i++)
        {
            if(pThis->pSearchNodeLast[i].fScore > LZERO)
            {
                fprintf(g_fpSearch, "%d: %.2f\r\n", i, pThis->pSearchNodeLast[i].fScore);
            }
        }
        fflush(g_fpSearch);
    }
#endif

	return EsErrID_OK;
}

void ESCalcFiller(PEsSearch pThis)
{
	float fOutlike;
#if !MINI5_USE_NEWCM_METHOD
    ivUInt16 i;
#endif
	fOutlike = ESCalcOutLike(pThis, pThis->pFillerState, pThis->nFillerMixture);
#if !MINI5_USE_NEWCM_METHOD
    for(i=1; i<pThis->nFillerCnt; i++)
    {
        float fOutlike2;
        PMixStateModel pFillerModel = (PMixStateModel)((ivUInt32)pThis->pFillerState + i * (sizeof(TMixStateModel) + (pThis->nFillerMixture-1)*sizeof(TGModel)));
        fOutlike2 = ESCalcOutLike(pThis, pFillerModel, pThis->nFillerMixture);
        if(fOutlike2 > fOutlike)
        {
            fOutlike = fOutlike2;
        }
    }
#endif

	pThis->fFillerScore = fOutlike;

#if LOG_SEARCH_SCORE
    {
        static FILE *g_fpFiller = NULL;
        if(NULL == g_fpFiller)
        {
            g_fpFiller = fopen("E:\\FillerScoreFlt.mini5.log", "wb");
        }
        fprintf(g_fpFiller, "iFrame=%d, fFillerScore=%.3f\r\n", pThis->iFrameIndexLast-1,fOutlike);
        fflush(g_fpFiller);
    }
#endif

    pThis->fFillerScoreSum += fOutlike;
    pThis->fFillerScore = 0;
}

void ESCalcSil(PEsSearch pThis)
{
	float fOutlike;
	fOutlike = ESCalcOutLike(pThis, pThis->pSilState, pThis->nSilMixture);
	pThis->fSilScore = fOutlike - pThis->fFillerScore;
}

void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID)
{
	ivUInt16 i, j, k;
	float fSilScore = pThis->fSilScore;

	for(i=0; i<ESR_MAX_NBEST; i++){
		/* 该ID已经存在 */
		if(nWordID == pThis->tNBest[i].nWordID){
			if(pSearchNode->fScore + fSilScore > pThis->tNBest[i].fScore){
				ivUInt16 bChanged,iCur;
				TNBest tBestTmp;

				pThis->tNBest[i].fScore = pSearchNode->fScore + fSilScore;
#if MINI5_USE_NEWCM_METHOD
				pThis->tNBest[i].fCMScore = (pSearchNode->fCMScore + pSearchNode->fCurStateCMScore)/nStateCount;
#endif

				/* bubble sort */
				bChanged = 0;
				iCur = 0;
				for(j=0; j<ESR_MAX_NBEST-1; j++){
					iCur ++;
					for(k=0; k<=ESR_MAX_NBEST-1-iCur; k++){
						if(pThis->tNBest[k].fScore < pThis->tNBest[k+1].fScore){
							ivMemCopy(&tBestTmp, pThis->tNBest+k, sizeof(TNBest));
							ivMemCopy(pThis->tNBest+k, pThis->tNBest+k+1, sizeof(TNBest));
							ivMemCopy(pThis->tNBest+k+1, &tBestTmp, sizeof(TNBest));
							bChanged = 1;
						}
					}
					if(!bChanged){
						break;
					}
				}
			}
			return;
		}
	}

	for(i=0; i<ESR_MAX_NBEST; i++){
		if(pSearchNode->fScore + fSilScore > pThis->tNBest[i].fScore){
			for(j=ESR_MAX_NBEST-1; j>i; j--){
				ivMemCopy(pThis->tNBest+j, pThis->tNBest+j-1, sizeof(TNBest));
			}
			pThis->tNBest[i].fScore = pSearchNode->fScore + fSilScore; 
			pThis->tNBest[i].nWordID = nWordID;
#if MINI5_USE_NEWCM_METHOD
			pThis->tNBest[i].fCMScore = (pSearchNode->fCMScore + pSearchNode->fCurStateCMScore)/nStateCount;
#endif
			return ;
		}
	}

	return ;
}

#if MINI5_TAG_SUPPORT
EsErrID EsTagSearchFrameStep(PEsSearch pThis) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别 */
{
	EsErrID err;

    ESCalcFiller(pThis);

    ESCalcSil(pThis);

	err = EsTagUpdateAllScore(pThis);
	if(ivESR_OK != err){
		ivAssert(ivFalse);
		return err;
	}

	++pThis->iFrameIndexLast;

	return err;
}

EsErrID EsWriteToSPI(PEsSearch pThis, ivPInt8 pData, ivUInt16 nDataSize)
{
    ivUInt16 iWrite;
    if(ivNull == pThis || ivNull == pData)
    {
        ivAssert(ivFalse);
        return EsErr_InvCal;
    }
    if(0 == nDataSize)
    {
        ivAssert(ivFalse);
        return EsErr_InvArg;
    }

    /* 缓存要写入SPI里的内存为一页(一般256Bytes),当加入的数据满一页后,就向SPI写一次，剩余的数据再重头开始存储 */

    iWrite = pThis->iCacheSPIWrite;

    if(iWrite + nDataSize < MINI5_SPI_PAGE_SIZE)
    {
        /* 不足一页，直接缓存到buf里 */
        ivMemCopy(pThis->pCacheSPI + iWrite, pData, nDataSize); /* RAM操作 */
        iWrite += nDataSize;
    }
    else if(iWrite + nDataSize == MINI5_SPI_PAGE_SIZE)
    {
        /* 刚好一页 */
        ivMemCopy(pThis->pCacheSPI + iWrite, pData, nDataSize); /* RAM操作 */

        if((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE > (ivUInt32)pThis->pSPIBufEnd)
        {
            ivAssert(ivFalse);
            return EsErr_OutOfMemory;
        }
        /* ivMemCopy(pThis->pPathSPI, pThis->pCacheSPI, IVP_SPI_PAGE_SIZE); */ /* RAM拷贝到SPI */
        EsWriteOnePageToSPI(pThis->pSPIBuf, pThis->pCacheSPI);
        pThis->pSPIBuf = (ivPointer)((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE);

        iWrite = 0;
    }
    else
    {
        /* 多于一页少于二页 */
        ivAssert(iWrite + nDataSize < MINI5_SPI_PAGE_SIZE*2);

        ivMemCopy(pThis->pCacheSPI + iWrite, pData, MINI5_SPI_PAGE_SIZE - iWrite);	/* RAM操作 */

        if((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE > (ivUInt32)pThis->pSPIBufEnd)
        {
            ivAssert(ivFalse);
            return EsErr_OutOfMemory;
        }
        /* ivMemCopy(pThis->pPathSPI, pThis->pCacheSPI, IVP_SPI_PAGE_SIZE); */	/* RAM拷贝到SPI */
        EsWriteOnePageToSPI(pThis->pSPIBuf, pThis->pCacheSPI);
        pThis->pSPIBuf = (ivPointer)((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE);

        ivMemCopy(pThis->pCacheSPI, pData + MINI5_SPI_PAGE_SIZE - iWrite, nDataSize - (MINI5_SPI_PAGE_SIZE - iWrite));	/* RAM操作 */
        iWrite = nDataSize - (MINI5_SPI_PAGE_SIZE - iWrite);		
    }

    pThis->iCacheSPIWrite = iWrite;

    return EsErrID_OK;	
}

EsErrID EsWriteToSPIFlush(PEsSearch pThis)
{
    if(ivNull == pThis)
    {
        return EsErr_InvCal;
    }

    if(0 == pThis->iCacheSPIWrite)
    {
        return EsErrID_OK;
    }

    ivAssert(pThis->iCacheSPIWrite < MINI5_SPI_PAGE_SIZE);
    
    /* 将缓存里的剩余数据拷贝到SPI */
    if((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE > (ivUInt32)pThis->pSPIBufEnd)
    {
        return EsErr_OutOfMemory;
    }

    /* ivMemCopy(pThis->pPathSPI, pThis->pCacheSPI, pThis->iCacheSPIWrite); */	/* RAM拷贝到SPI */
    EsWriteOnePageToSPI(pThis->pSPIBuf, pThis->pCacheSPI);
    pThis->pSPIBuf = (ivPointer)((ivUInt32)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE);
    
    return EsErrID_OK;
}

#endif


#endif
