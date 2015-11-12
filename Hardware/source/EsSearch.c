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

#if !LOAD_FLOAT_MDOEL

void EsCalcFiller(PEsSearch pThis);
void ESCalcSil(PEsSearch pThis);

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
EsErrID EsTagUpdateAllScore(PEsSearch pThis);
#endif

void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID);
void EsGetPruneThreshByBucket(PEsSearch pThis);

#if !MINI5_SPI_MODE && LOG_DEBUG_INFO
#include <stdio.h>
void LogModelInfo(PEsrEngine pEngine, ivCPointer pModel)
{
    PCResHeader pResHdr;
    FILE *fp;
    PCMixSModel pSModel, pSilModel, pFillerModel, pOldFillerModel;
    PGaussianModel pGauss;
    int i, j, k;

    if(ESENGINE_REC == pEngine->iEngineType)
    {
        fp = fopen("E:\\ModelInfo.mini5", "wb");
    }
    else
    {
        fp = fopen("E:\\TagModelInfo.mini5", "wb");
    }
    

    pResHdr = (PCResHeader)pModel;

    /* Init Resource */
    /* Load Model Info */
    pSilModel = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nSilSModelOffset * sizeof(ivInt16))); /* 重定位. Sil State */
    pFillerModel = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pOldFillerModel = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nOldFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pSModel = (PCMixSModel)((ivUInt32)pModel+(pResHdr->nMixSModelOffset * sizeof(ivInt16)));
   
 //   g_ps16MeanCoef = (ivPInt16)pResHdr2->ps16MeanCoef;
 //   g_ps16MeanCoefQ = (ivPInt16)pResHdr2->ps16MeanCoefQ;

 //   g_s16MeanVarTable = (ivPInt16)pResHdr2->ps16MeanVarTable;

    fprintf(fp, "SilModel:\r\n");
    for(i=0; i<pResHdr->nSilMixture; i++)

    {
        fprintf(fp, " iMix=%d: fGCosnt=%d, MeanVar= ", i, pSilModel->tGModel[i].fGConst);

        for(j=0; j<FEATURE_DIMNESION; j++)
        {
            fprintf(fp, "%d, ", pSilModel->tGModel[i].fMeanVar[j]);
        }
        fprintf(fp, "\r\n");
    }

    fprintf(fp, "FillerModel:\r\n");
    for(i=0; i<pResHdr->nFillerMixture; i++)
    {
        fprintf(fp, " iMix=%d: fGCosnt=%d, MeanVar= ", i, pFillerModel->tGModel[i].fGConst);

        for(j=0; j<FEATURE_DIMNESION; j++)
        {
            fprintf(fp, "%d, ", pFillerModel->tGModel[i].fMeanVar[j]);
        }
        fprintf(fp, "\r\n");
    }

    pGauss = pSModel->tGModel;

    for(i=0; i<pResHdr->nMixSModelNum-2-pResHdr->nOldFillerCnt; i++)
    {
        fprintf(fp, "iSModel=%d:\r\n", i);
        for(k=0; k<pResHdr->nMixture; k++, pGauss++)
        {
            fprintf(fp, " iMix=%d: fGCosnt=%d, MeanVar= ", k, pGauss->fGConst);

            for(j=0; j<FEATURE_DIMNESION; j++)
            {
                fprintf(fp, "%d, ", pGauss->fMeanVar[j]);
            }
            fprintf(fp, "\r\n");
        }       
    }

    if(pResHdr->nOldFillerCnt > 0)
    {
        int k;
        pGauss = pOldFillerModel->tGModel;
        for(k=0; k<pResHdr->nOldFillerCnt; k++)
        {
            fprintf(fp, "OldFillerModel%d:\r\n", k);
            for(i=0; i<pResHdr->nOldFillerMix; i++, pGauss++)
            {
                fprintf(fp, " iMix=%d: fGCosnt=%d, MeanVar= ", i, pGauss->fGConst);

                for(j=0; j<FEATURE_DIMNESION; j++)
                {
                    fprintf(fp, "%d, ", pGauss->fMeanVar[j]);
                }
                fprintf(fp, "\r\n");
            }
        }
        
    }    

    fclose(fp);
}
#endif

#if LOG_SEARCH_SCORE
FILE *g_fpOutlike = NULL;
#endif
EsErrID EsSearchInit(PEsSearch pThis, ivCPointer pModel, PEsrEngine pEngine)
{
	PCResHeader pResHdr;
	ivUInt32 nCRC = 0, nCounter = 0;

#if MINI5_SPI_MODE
    EsErrID  err;
    ivPointer   pTmpRam;
    ivPointer   pSPIBuf;
#endif

#if MINI5_API_PARAM_CHECK
	ivAssert(ivNull != pThis && ivNull != pModel && ivNull != pEngine);
	if(ivNull == pThis || ivNull == pModel ||ivNull == pEngine){
		return EsErr_InvCal;
	}
#endif

    ivMemZero(pThis, sizeof(TEsSearch));
#if MINI5_SPI_MODE
    pTmpRam = pEngine->tFront.m_pPCMBuffer;
    ivAssert(ESR_PCMBUFFER_SIZE * sizeof(ivInt16) > sizeof(TResHeader));
    err = EsSPIReadBuf(pTmpRam, pModel, sizeof(TResHeader));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }

    pResHdr = (PCResHeader)pTmpRam;
#else
	pResHdr = (PCResHeader)pModel;
#endif

#if MINI5_API_PARAM_CHECK
    if((ESENGINE_TAG == pEngine->iEngineType && DW_TAG_MODEL_CHECK != pResHdr->dwCheck)
        || (ESENGINE_REC == pEngine->iEngineType && DW_MODEL_CHECK != pResHdr->dwCheck))
    {
        return EsErr_InvRes;
    }
#endif

#if MINI5_SPI_MODE
    pSPIBuf = (ivPointer)((ivAddress)pModel + sizeof(ivUInt32)*2);
    pTmpRam = (ivPointer)((ivAddress)pTmpRam + ivGridSize(sizeof(TResHeader)));

    ivAssert(ESR_PCMBUFFER_SIZE * sizeof(ivInt16) > sizeof(TResHeader) + MINI5_SPI_PAGE_SIZE);
    err = EsSPICalcCRC(pTmpRam, pSPIBuf, pResHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
#else
    ivMakeCRC((ivPCInt16)((ivAddress)pResHdr + sizeof(ivUInt32)*2), pResHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
#endif
	if(nCRC != pResHdr->nCRC) {
		return EsErr_InvRes;
	}

	/* Init Resource */
	/* Load Model Info */
	pThis->pSilState = (PCMixSModel)((ivAddress)pModel+(pResHdr->nSilSModelOffset * sizeof(ivInt16))); /* 重定位. Sil State */
	pThis->pState = (PCMixSModel)((ivAddress)pModel+(pResHdr->nMixSModelOffset * sizeof(ivInt16)));
    pThis->nMixture = (ivUInt16)pResHdr->nMixture;
    pThis->nSilMixture = (ivUInt16)pResHdr->nSilMixture;
#if MINI5_USE_NEWCM_METHOD
    pThis->pFillerState = (PCMixSModel)((ivAddress)pModel+(pResHdr->nFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pThis->nFillerMixture = (ivUInt16)pResHdr->nFillerMixture;
#else
    pThis->pFillerState = (PCMixSModel)((ivAddress)pModel+(pResHdr->nOldFillerSModelOffset * sizeof(ivInt16))); /* Filler State */
    pThis->nFillerCnt = pResHdr->nOldFillerCnt;
    pThis->nFillerMixture = (ivUInt16)pResHdr->nOldFillerMix;
    if(0 == pThis->nFillerCnt) {
        return EsErr_InvRes;
    }
#endif

#if MINI5_SPI_MODE
    ivMemCopy(pEngine->tFront.m_ps16MeanCoef,pResHdr->s16MeanCoef, FEATURE_DIMNESION * sizeof(ivInt16));
    ivMemCopy(pEngine->tFront.m_ps16MeanCoefQ,pResHdr->s16MeanCoefQ, FEATURE_DIMNESION * sizeof(ivInt16));
#else
    pEngine->tFront.m_ps16MeanCoef = (ivPInt16)(pResHdr->s16MeanCoef);
    pEngine->tFront.m_ps16MeanCoefQ = (ivPInt16)(pResHdr->s16MeanCoefQ);
#endif

#if LOG_DEBUG_INFO
    LogModelInfo(pEngine, pModel);
#endif

	/* 桶排序要使用的内存复用FFT的内存块 */
	ivAssert(BUCKET_ALL_COUNT < TRANSFORM_FFTNUM_DEF+2);
	pThis->pnBucketCacheLast = (ivPUInt16)(pEngine->tFront.m_pFrameCache);	

    pThis->pSearchNodeLast = ivNull;
	pThis->nTotalNodes = 0;
	pThis->nExtendNodes = 0;

    pThis->iEngineType = pEngine->iEngineType;

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
    pThis->pStaScoreCacheLast = ivNull;

	if(ESENGINE_TAG == pEngine->iEngineType) {
	
        pThis->pStaScoreCacheLast = ivNull;

        pThis->tSPICache.pSPIBufBase = pEngine->pSPIBufBase;
        pThis->tSPICache.pSPIBufEnd = pEngine->pSPIBufEnd;
        pThis->tSPICache.pSPIBuf = pThis->tSPICache.pSPIBufBase;

        pThis->nClusters = (ivUInt16)(pResHdr->nMixSModelNum - 2 - pResHdr->nOldFillerCnt); /* -2:sil, filler */

        pThis->pStaScoreCacheLast = (ivPInt16)EsAlloc(pEngine, (pThis->nClusters+2) * sizeof(ivInt16)); /* +2:BeginSil,EndSil */
		ivAssert(ivNull != pThis->pStaScoreCacheLast);		

		pThis->pnRepeatFrmCacheLast= (ivPInt16)EsAlloc(pEngine, (pThis->nClusters+2) * sizeof(ivInt16));
		ivAssert(ivNull != pThis->pnRepeatFrmCacheLast);		

        pThis->tSPICache.pCacheSPI = (ivPInt8)EsAlloc(pEngine, MINI5_SPI_PAGE_SIZE);
        ivAssert(ivNull != pThis->tSPICache.pCacheSPI);
	}
#endif

#if MINI5_SPI_MODE 
    ivMemZero(pEngine->tFront.m_pPCMBuffer, ESR_PCMBUFFER_SIZE * sizeof(ivInt16));
#endif

	EsSearchReset(pThis);

	return 0;
}

EsErrID EsSetLexicon(PEsSearch pThis,ivCPointer pLexicon, PEsrEngine pEngine)
{
	PLexiconHeader pLexHdr;
	ivUInt32 nSize;
    ivUInt16 i;

#if MINI5_SPI_MODE
    EsErrID  err;

    pLexHdr = (PLexiconHeader)(pEngine->tFront.m_pPCMBuffer);
    err = EsSPIReadBuf((ivPointer)pLexHdr, pLexicon, sizeof(TLexiconHeader));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
#else
	pLexHdr = (PLexiconHeader)pLexicon;
#endif

#if MINI5_API_PARAM_CHECK
	ivAssert(DW_LEXHEADER_CHECK == pLexHdr->dwCheck);
	if(DW_LEXHEADER_CHECK != pLexHdr->dwCheck){
		return EsErr_InvRes;
	}
#endif

	/* Cache for Calculation of all State */
	nSize = sizeof(TSearchNode)*(pLexHdr->nTotalNodes+1);

#if MINI5_API_PARAM_CHECK
	ivAssert((ivAddress)pEngine->pBuffer+nSize <= (ivAddress)pEngine->pBufferEnd);
	if((ivAddress)pEngine->pBuffer+nSize > (ivAddress)pEngine->pBufferEnd){
		return EsErr_OutOfMemory;
	}
#endif

	ivAssert((pLexHdr->nExtendNodes<<1) < pLexHdr->nTotalNodes);
	pThis->pSearchNodeLast = (PSearchNode)pEngine->pBuffer;
	
	/* 重定位 */
	pThis->pCmdDesc = (PCmdDesc)((ivAddress)pLexicon + (ivUInt32)(pLexHdr->nCmdDescOffset));
	pThis->nTotalNodes = pLexHdr->nTotalNodes;
	pThis->nExtendNodes = pLexHdr->nExtendNodes;
	pThis->pLexRoot = (PLexNode)((ivAddress)pLexicon + (ivUInt32)(pLexHdr->nLexRootOffset));

	pThis->nCmdNum = (ivUInt16)(pLexHdr->nCmdNum);
	
    ivMemZero(pThis->pSearchNodeLast, sizeof(TSearchNode)*(pThis->nTotalNodes+1));
    for(i=1; i<=pThis->nTotalNodes; i++){
        /* i=1;  起始解码只允许从sil开始扩展 */
        /* i=pThis->nExtendNodes + 1; 起始解码允许所有命令词的前7个state可扩展 */
        pThis->pSearchNodeLast[i].fScore = LZERO;
    }

#if MINI5_SPI_MODE
    /* 解码器端SPI方案复用PCMBuffer内存 */
	ivMemZero(pEngine->tFront.m_pPCMBuffer, sizeof(ivInt16)*ESR_PCMBUFFER_SIZE);
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
        ivMemZero(pThis->pSearchNodeLast, sizeof(TSearchNode)*(pThis->nTotalNodes+1));
        for(i=1; i<=pThis->nTotalNodes; i++){
            /* i=1;  起始解码只允许从sil开始扩展 */
            /* i=pThis->nExtendNodes + 1; 起始解码允许所有命令词的前7个state可扩展 */
            pThis->pSearchNodeLast[i].fScore = LZERO;
        }
	}

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
	if(ESENGINE_TAG == pThis->iEngineType){
		pThis->fScoreMaxLast = LZERO;
		pThis->iMaxStateLast = 0;
        pThis->tSPICache.iCacheSPIWrite = 0;

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
            g_fpOutlike = fopen("E:\\Outlike.mini5.log", "wb");
        }
        fprintf(g_fpOutlike, "\r\niFrame=%d\r\n", pThis->iFrameIndexLast);       
        fflush(g_fpOutlike);
    }
#endif

	++pThis->iFrameIndexLast;

	EsCalcFiller(pThis);

	ESCalcSil(pThis);

	EsUpdateAllScore(pThis);

	EsGetPruneThreshByBucket(pThis);

#if LOG_SEARCH_SCORE
    {
        int i;
        static FILE *g_fpSearch = NULL;
        if(NULL == g_fpSearch)
        {
            g_fpSearch = fopen("E:\\Search.mini5.log", "wb");
        }
        fprintf(g_fpSearch, "\r\niFrame=%d\r\n", pThis->iFrameIndexLast - 1);
        for(i=0; i<pThis->nTotalNodes; i++)
        {
            if(pThis->pSearchNodeLast[i].fScore > LZERO)
            {
                fprintf(g_fpSearch, "%d: %.2f\r\n", i, pThis->pSearchNodeLast[i].fScore * 1.0 /(1<<ESR_Q_SCORE));
            }
        }
        fflush(g_fpSearch);
    }
#endif
	return EsErrID_OK;
}

void EsCalcFiller(PEsSearch pThis)
{
	ivInt16 fOutlike;
#if !MINI5_USE_NEWCM_METHOD
    ivUInt16 i;
#endif
	fOutlike = EsCalcOutLike(pThis, pThis->pFillerState, pThis->nFillerMixture);
#if !MINI5_USE_NEWCM_METHOD
    for(i=1; i<pThis->nFillerCnt; i++) {
        ivInt16 fOutlike2;
        PMixStateModel pFillerModel = (PMixStateModel)((ivUInt32)pThis->pFillerState + i * (sizeof(TMixStateModel) + (pThis->nFillerMixture-1)*sizeof(TGModel)));
        fOutlike2 = EsCalcOutLike(pThis, pFillerModel, pThis->nFillerMixture);
        if(fOutlike2 > fOutlike) {
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
            g_fpFiller = fopen("E:\\FillerScore.mini5.log", "wb");
        }
        fprintf(g_fpFiller, "iFrame=%d, fFillerScore=%.3f\r\n", pThis->iFrameIndexLast-1,fOutlike*1.0/(1<<ESR_Q_SCORE));
        fflush(g_fpFiller);
    }
#endif
}

void ESCalcSil(PEsSearch pThis)
{
	ivInt16 fOutlike;
	fOutlike = EsCalcOutLike(pThis, pThis->pSilState, pThis->nSilMixture);
	pThis->fSilScore = fOutlike - pThis->fFillerScore;
}

void EsUpdateEndFiller(PEsSearch pThis, PSearchNode pSearchNode, ivUInt16 nStateCount, ivUInt16 nWordID)
{
	ivUInt16 i, j, k;
	ivInt16 fSilScore = pThis->fSilScore;

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

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT
EsErrID EsTagSearchFrameStep(PEsSearch pThis) /* 与硕呈平台AitalkMini1.0代码比较: 略微差别 */
{
	EsErrID err;

    EsCalcFiller(pThis);

    ESCalcSil(pThis);

	err = EsTagUpdateAllScore(pThis);
	if(ivESR_OK != err){
		ivAssert(ivFalse);
		return err;
	}

	++pThis->iFrameIndexLast;

	return err;
}

EsErrID EsSPIWriteData(PSPICache pThis, ivCPointer pData, ivUInt32 nDataSize)
{
    ivUInt16 iWrite;

    ivPointer  pTmpData = (ivPointer)pData;
    if(ivNull == pThis || ivNull == pData) {
        ivAssert(ivFalse);
        return EsErr_InvCal;
    }
    if(0 == nDataSize) {
        ivAssert(ivFalse);
        return EsErr_InvArg;
    }

    /* 缓存要写入SPI里的内存为一页(一般256Bytes),当加入的数据满一页后,就向SPI写一次，剩余的数据再重头开始存储 */

    iWrite = pThis->iCacheSPIWrite;

    if(iWrite + nDataSize < MINI5_SPI_PAGE_SIZE) {
        /* 不足一页，直接缓存到buf里 */
        ivMemCopy(pThis->pCacheSPI + iWrite, pTmpData, nDataSize); /* RAM操作 */
        iWrite += nDataSize;
    }
    else {
        /* 一页以上 */
        while(1) {

            if (iWrite + nDataSize < MINI5_SPI_PAGE_SIZE) {
                /* 剩余的零头直接拷贝到RAM */
                ivMemCopy(pThis->pCacheSPI + iWrite, pTmpData, nDataSize); /* RAM操作 */
                iWrite += nDataSize;

                break;
            }

            /* 整页写入SPI */
            ivMemCopy(pThis->pCacheSPI + iWrite, pTmpData, MINI5_SPI_PAGE_SIZE - iWrite);
            if((ivAddress)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE > (ivAddress)pThis->pSPIBufEnd) {
                ivAssert(ivFalse);
                return EsErr_OutOfMemory;
            }
            EsSPIWriteOnePage(pThis->pSPIBuf, pThis->pCacheSPI);
            pThis->pSPIBuf = (ivPointer)((ivAddress)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE);
            pTmpData = (ivPointer)((ivAddress)pTmpData + (MINI5_SPI_PAGE_SIZE - iWrite));

            nDataSize -= (MINI5_SPI_PAGE_SIZE - iWrite);
            iWrite = 0;
        }
    }

    pThis->iCacheSPIWrite = iWrite;

    return EsErrID_OK;	
}

/* 从一个SPI拷贝到另外的SPI, 顺带做CRC */
EsErrID EsSPIWriteSPIData(PSPICache pThis, ivPointer pRAMBuf,ivCPointer pData, ivUInt32 nDataSize, ivPUInt32 pnCRC,ivPUInt32 piCounter)
{
    ivPointer pSPITmp = (ivPointer)pData;
    EsErrID   err;

    while(1) {
        /* 小零头 */
        if(MINI5_SPI_PAGE_SIZE >= nDataSize) {
            err = EsSPIReadBuf(pRAMBuf, pSPITmp, nDataSize);
            ivAssert(EsErrID_OK == err);

            ivMakeCRC(pRAMBuf, nDataSize, pnCRC, piCounter);
            EsSPIWriteData(pThis, pRAMBuf, nDataSize);

            return EsErrID_OK;
        }
        else {
            err = EsSPIReadBuf(pRAMBuf, pSPITmp, MINI5_SPI_PAGE_SIZE);
            ivAssert(EsErrID_OK == err);
            ivMakeCRC(pRAMBuf, MINI5_SPI_PAGE_SIZE, pnCRC, piCounter);

            nDataSize -= MINI5_SPI_PAGE_SIZE;
            pSPITmp = (ivPointer)((ivAddress)pSPITmp + MINI5_SPI_PAGE_SIZE);

            EsSPIWriteData(pThis, pRAMBuf, MINI5_SPI_PAGE_SIZE);
        }
    }

    return EsErrID_OK;
}

EsErrID EsSPIWriteFlush(PSPICache pThis)
{
    if(ivNull == pThis) {
        return EsErr_InvCal;
    }

    if(0 == pThis->iCacheSPIWrite) {
        return EsErrID_OK;
    }

    ivAssert(pThis->iCacheSPIWrite < MINI5_SPI_PAGE_SIZE);
    
    /* 将缓存里的剩余数据拷贝到SPI */
    if((ivAddress)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE > (ivAddress)pThis->pSPIBufEnd) {
        return EsErr_OutOfMemory;
    }

    /* ivMemCopy(pThis->pPathSPI, pThis->pCacheSPI, pThis->iCacheSPIWrite); */	/* RAM拷贝到SPI */
    EsSPIWriteOnePage(pThis->pSPIBuf, pThis->pCacheSPI);
    pThis->pSPIBuf = (ivPointer)((ivAddress)pThis->pSPIBuf + MINI5_SPI_PAGE_SIZE);
    pThis->iCacheSPIWrite = 0;

    return EsErrID_OK;
}

#endif

#if MINI5_SPI_MODE
EsErrID EsSPICalcCRC(ivPointer pRAMBuf, ivCPointer pSPIData,ivUInt32 nSize,ivPUInt32 pnCRC,ivPUInt32 piCounter)
{
    ivSize   nReadSize = 0;
    ivUInt16 nPerSize  = 0;
    EsErrID  err = EsErrID_OK;
    ivPointer pSPITemp = (ivPointer)pSPIData;

    nPerSize = MINI5_SPI_PAGE_SIZE;
    while(1) {
        if (nReadSize >= nSize) {
            break;
        }
        if (nSize - nReadSize < MINI5_SPI_PAGE_SIZE) {
            nPerSize = nSize - nReadSize;
        }

        err = EsSPIReadBuf(pRAMBuf, pSPITemp, nPerSize);
        if (EsErrID_OK != err) {
            ivAssert(ivFalse);
            return err;
        }
        ivMakeCRC((ivPCInt16)pRAMBuf, nPerSize, pnCRC, piCounter);

        nReadSize += nPerSize;
        pSPITemp = (ivPointer)((ivAddress)pSPITemp + nPerSize);
    } /* while(1) */

    return err;
}
#endif /* MINI5_SPI_MODE */

#endif
