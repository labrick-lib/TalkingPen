/*********************************************************************#
//	文件名		：ivESR.c
//	文件功能	：
//	作者		：Truman
//	创建时间	：2007年8月9日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#include "EsKernel.h"
#include "ivESR.h"
#include "EsEngine.h"
#include "EsKernel.c"

/* Parameter ID for ESR Disable VAD */
#define ES_PARAM_DISABLEVAD				(7)
#define ES_DISABLEVAD_ON			((ivCPointer)1)
#define ES_DISABLEVAD_OFF			((ivCPointer)0)
#define ES_DEFAULT_DISABLEVAD		ES_DISABLEVAD_OFF/* yfhu.added at [2009-4-8 9:37:08] */

#define DW_OBJ_CHECK			(0x20091009)

#define DW_RESIDENT_RAM_CHECK	(0x20100914) 

#if MINI5_ENHANCE_VAD
extern ivUInt16 g_bEnhanceVAD;
#endif /* MINI5_ENHANCE_VAD */

extern ivUInt16 g_bChkAmbientNoise;

#if RATETSET_USE_VAD
extern ivUInt16 g_bDisableVad;
#endif

EsErrID ivCall ESRCreate(ivPointer pEsrObj, ivSize ivPtr pnESRObjSize, ivPointer pResidentRAM, ivPUInt16 pnResidentRAMSize, ivCPointer pResource, ivUInt32 nGrmID)
{
	PEsrEngine pEngine;
	EsErrID err;
	ivPUInt32 pTmp;

	ivCPointer  pModel;
	PGrmMdlHdr  pGrmMdlHdr;
	ivCPointer  pLexicon = ivNull;
    PGrmDesc    pGrmDesc = ivNull;
	PLexiconHeader pLexHdr;

	ivSize nSize;
	ivPUInt16 pBuffer;
	ivUInt32 nCounter =0, nCRC = 0;
	ivPInt16 ps16CMNMean;

	ivUInt32 i;

    PResidentRAMHdr pResidentHdr;

#if MINI5_SPI_MODE
    ivPointer   pSPIBuf;
#endif

#if MINI5_API_PARAM_CHECK
    if(ivNull == pEsrObj || ivNull == pnESRObjSize || ivNull == pResidentRAM 
        || ivNull == pnResidentRAMSize || ivNull == pResource) {
            ivAssert(ivFalse);
            return EsErr_InvArg;
    }
#endif
    /* Memory align */
    pEsrObj = (ivPointer)(((ivAddress)pEsrObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));	 

#if MINI5_SPI_MODE
    /* pResource存在SPI中 */
    err = EsSPIReadBuf(pEsrObj, pResource, sizeof(TGrmMdlHdr));
    if (EsErrID_OK != err) {
        ivAssert(ivFalse);
        return EsErr_Failed;
    }
    pGrmMdlHdr = (PGrmMdlHdr)pEsrObj;
#else
	pGrmMdlHdr = (PGrmMdlHdr)pResource;
#endif
	if(DW_MINI_RES_CHECK != pGrmMdlHdr->dwCheck) {
        ivAssert(ivFalse);
		return EsErr_InvRes;
	}

#if MINI5_SPI_MODE
    pSPIBuf = (ivPointer)((ivAddress)pResource+sizeof(TGrmMdlHdr)+sizeof(TGrmDesc)*(pGrmMdlHdr->nTotalGrm-1));
    ivAssert(sizeof(TEsrEngine) >= ivGridSize(sizeof(TGrmMdlHdr)) + MINI5_SPI_PAGE_SIZE);
    pBuffer = (ivPUInt16)((ivAddress)pEsrObj + ivGridSize(sizeof(TGrmMdlHdr)));

    err = EsSPICalcCRC((ivPointer)pBuffer, pSPIBuf, pGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
#else
	ivMakeCRC((ivPointer)((ivAddress)pResource+sizeof(TGrmMdlHdr)+sizeof(TGrmDesc)*(pGrmMdlHdr->nTotalGrm-1)), pGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
#endif
	if(nCRC != pGrmMdlHdr->nCRC) {
        ivAssert(ivFalse);
        return EsErr_InvRes;
	}

#if MINI5_API_PARAM_CHECK
    nSize = ivGridSize(sizeof(TEsrEngine) + IV_PTR_GRID);   /* Obj size */
    nSize += ivGridSize(sizeof(ivInt16)*(TRANSFORM_FFTNUM_DEF+2)); /* FFT */    
    nSize += ivGridSize(ESR_MFCC_BACK_FRAMES*(TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt16)); /* MFCC Buffer */
	nSize += ivGridSize(ESR_HIPASSENERGY_NUM * sizeof(ivInt32));    /* For Filter BandPass */

    #if ESR_CPECTRAL_SUB
    nSize += ivGridSize((TRANSFORM_HALFFFTNUM_DEF+1) * sizeof(ivUInt16) * 3);
    #endif /* #if ESR_CPECTRAL_SUB */
#endif

	/* Set lexicon */
	ivAssert(pGrmMdlHdr->nTotalGrm >= 1);
	if(0 == pGrmMdlHdr->nTotalGrm) {
        ivAssert(ivFalse);
        return EsErr_InvRes;
	}

#if MINI5_SPI_MODE
    pGrmDesc = (PGrmDesc)((ivAddress)pEsrObj + ivGridSize(sizeof(TGrmMdlHdr)));
    pSPIBuf = (ivPointer)((ivAddress)pResource+sizeof(TGrmMdlHdr) -sizeof(TGrmDesc));
    err = EsSPIReadBuf((ivPointer)pGrmDesc, pSPIBuf, sizeof(TGrmDesc)*(pGrmMdlHdr->nTotalGrm));
#endif

	for(i=0; i<pGrmMdlHdr->nTotalGrm; i++) {
#if !MINI5_SPI_MODE
        pGrmDesc = &(pGrmMdlHdr->tGrmDesc[i]);
#endif
        if(nGrmID == pGrmDesc->nGrmID) {
            pLexicon = (ivCPointer)((ivAddress)pResource + pGrmDesc->nGrmOffset * sizeof(ivInt16));
            ivAssert(pGrmDesc->nSpeechTimeOutFrm >= 0 && pGrmDesc->nSpeechTimeOutFrm <= 2000);			
            break;
        }
#if MINI5_SPI_MODE
        pGrmDesc = (PGrmDesc)((ivAddress)pGrmDesc + sizeof(TGrmDesc));
#endif
	}
	if(i >= pGrmMdlHdr->nTotalGrm) {
        ivAssert(ivFalse);
        return EsErr_Failed;
	}

#if MINI5_SPI_MODE
    ivMemCopy(pGrmMdlHdr->tGrmDesc, pGrmDesc, sizeof(TGrmDesc));
    pGrmDesc = pGrmMdlHdr->tGrmDesc;

    pBuffer = (ivPUInt16)((ivAddress)pEsrObj + ivGridSize(sizeof(TGrmMdlHdr)));
    ivAssert(sizeof(TEsrEngine) >= ivGridSize(sizeof(TGrmMdlHdr)) + sizeof(TLexiconHeader));

    err = EsSPIReadBuf((ivPointer)pBuffer, pLexicon, sizeof(TLexiconHeader));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
    pLexHdr = (PLexiconHeader)pBuffer;
#else
	pLexHdr = (PLexiconHeader)pLexicon;
#endif
	ivAssert(DW_LEXHEADER_CHECK == pLexHdr->dwCheck);
	if(DW_LEXHEADER_CHECK != pLexHdr->dwCheck) {
        ivAssert(ivFalse);
        return EsErr_InvRes;
	}

#if MINI5_API_PARAM_CHECK
	nSize += sizeof(TSearchNode) * (pLexHdr->nTotalNodes + 1);
	if(*pnESRObjSize < nSize) {
		*pnESRObjSize = nSize;
        ivAssert(ivFalse);
        return EsErr_OutOfMemory;
	} 

    if(*pnResidentRAMSize < sizeof(TResidentRAMHdr)) {
        /* Check buf size if enough */
        *pnResidentRAMSize = sizeof(TResidentRAMHdr); 
        ivAssert(ivFalse);
        return EsErr_OutOfMemory;
    }
#endif

	pTmp = (ivPUInt32)pEsrObj;
	*pTmp = DW_OBJ_CHECK;
	pEsrObj = (ivPointer)(pTmp+1);

	pBuffer = (ivPUInt16)pEsrObj;

	pEngine = (PEsrEngine)pBuffer;
	pEngine->tFront.m_nSpeechTimeOut = (ivInt32)(pGrmDesc->nSpeechTimeOutFrm); /* Speech time out frame */
	pEngine->nMinSpeechFrm = (ivUInt16)(pGrmDesc->nMinSpeechFrm);
	pBuffer = (ivPUInt16)((ivAddress)pBuffer + sizeof(TEsrEngine));

	/* ResidentRAM */
    pResidentHdr = (PResidentRAMHdr)pResidentRAM;		
	pEngine->pResidentRAMHdr = pResidentHdr;

	ps16CMNMean = pResidentHdr->ps16CMNMean;
	nCRC = 0;
	nCounter = 0;
	ivMakeCRC(ps16CMNMean, sizeof(pResidentHdr->ps16CMNMean), &nCRC, &nCounter);
	if(DW_RESIDENT_RAM_CHECK != pResidentHdr->dwCheck || nCRC != pResidentHdr->nCMNCRC) {
		/* 传入的常驻内存已经被用户改写,则不使用常驻内存中的CMN参数信息 */
		
		/* Init CMN Mean 9.6, C0:11.4 */
		for(i=0; i<TRANSFORM_CEPSNUM_DEF+1; i++) {
			ps16CMNMean[i] = g_s16CMNCoef[i];
		}

		nCRC = 0;
		nCounter = 0;
		ivMakeCRC(ps16CMNMean, sizeof(pResidentHdr->ps16CMNMean), &nCRC, &nCounter);
		pResidentHdr->nCMNCRC = nCRC;
		pResidentHdr->dwCheck = DW_RESIDENT_RAM_CHECK;
	}

	pEngine->pBufferBase = pBuffer;
	pEngine->pBuffer = pBuffer;
	pEngine->pBufferEnd = (ivPUInt16)((ivAddress)pEsrObj + *pnESRObjSize);
    pEngine->iEngineType = ESENGINE_REC;

	pModel = (ivCPointer)((ivAddress)pResource + pGrmMdlHdr->nModelOffset * sizeof(ivInt16));
	err = EsInit(pEngine, pModel);
	ivAssert(EsErrID_OK == err);
#if MINI5_API_PARAM_CHECK
	if(EsErrID_OK != err) {
		return err;
	}
#endif

	/* Set lexicon */
	err = EsSetLexicon(&pEngine->tSearch, pLexicon, pEngine);
	ivAssert(EsErrID_OK == err);
#if MINI5_API_PARAM_CHECK
	if(EsErrID_OK != err) {
		return err;
	}
#endif

	return err;
}

EsErrID ivCall ESRSetParam(ivPointer pEsrObj, ivUInt32 nParamID,ivUInt16 nParamValue) /* 与硕呈平台AitalkMini1.0代码比较: 新增接口 */
{
	PEsrEngine pEngine;
    EsErrID err;

	/* Memory align */
	pEsrObj = (ivPointer)(((ivUInt32)pEsrObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));

#if MINI5_API_PARAM_CHECK
	ivAssert(ivNull != pEsrObj);
	if(ivNull == pEsrObj ){
		return EsErr_InvArg;
	}
	if(DW_OBJ_CHECK != *(ivPUInt32)pEsrObj){
		return EsErr_InvCal;
	}
#endif

	pEsrObj = (ivPointer)((ivPUInt32)pEsrObj + 1);

	pEngine = (PEsrEngine)pEsrObj;
#if MINI5_API_PARAM_CHECK
	err = EsValidate(pEngine);
	if(EsErrID_OK != err){
		return err;
	}
#endif

	switch(nParamID){
#if MINI5_ENHANCE_VAD
	case ES_PARAM_ENHANCEVAD:
		g_bEnhanceVAD = nParamValue;
		break;
#endif
	case ES_PARAM_AMBIENTNOISE:
		g_bChkAmbientNoise = nParamValue;
		break;
#if RATETSET_USE_VAD
	case ES_PARAM_DISABLEVAD:
		g_bDisableVad = nParamValue;
		break;
#endif /* RATETSET_USE_VAD */
	default:
		return EsErr_InvArg;
	}

	return EsErrID_OK;
}

EsErrID ivCall ESRReset(ivPointer pEsrObj) /* 与硕呈平台AitalkMini1.0代码相同 */
{
	PEsrEngine pEngine;
    EsErrID err;

	/* Memory align */
	pEsrObj = (ivPointer)(((ivUInt32)pEsrObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));

#if MINI5_API_PARAM_CHECK
	ivAssert(ivNull != pEsrObj);
	if(ivNull == pEsrObj ){
		return EsErr_InvArg;
	}
	if(DW_OBJ_CHECK != *(ivPUInt32)pEsrObj){
		return EsErr_InvCal;
	}
#endif

	pEsrObj = (ivPointer)((ivPUInt32)pEsrObj + 1);

	pEngine = (PEsrEngine)pEsrObj;
#if MINI5_API_PARAM_CHECK
	err = EsValidate(pEngine);
	if(EsErrID_OK != err){
		return err;
	}
#endif

	return EsReset(pEngine);
}

EsErrID ivCall ESRRunStep(ivPointer pEsrObj, ivUInt32 dwMessage, ivESRStatus ivPtr pStatus, PCEsrResult ivPtr ppResult)
{
    PEsrEngine pEngine;	
    EsErrID err;

#if MINI5_API_PARAM_CHECK
    ivAssert(ivNull != pEsrObj && ivNull != pStatus && ivNull != ppResult);
    if(ivNull == pEsrObj || ivNull == pStatus || ivNull == ppResult){
        return EsErr_InvArg;
    }
#endif

    /* Memory align */
    pEsrObj = (ivPointer)(((ivUInt32)pEsrObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));	 
#if MINI5_API_PARAM_CHECK
    if(ivNull == pEsrObj ){
        return EsErr_InvArg;
    }
    if(DW_OBJ_CHECK != *(ivPUInt32)pEsrObj){
        return EsErr_InvCal;
    }
#endif

    pEsrObj = (ivPointer)((ivPUInt32)pEsrObj + 1);
    pEngine = (PEsrEngine)pEsrObj;
#if MINI5_API_PARAM_CHECK
    err = EsValidate(pEngine);
    if(EsErrID_OK != err){
        return err;
    }
#endif

    *pStatus = 0;
    *ppResult = ivNull;

    return EsRunStep(pEngine, dwMessage, pStatus, ppResult);
}

EsErrID ivCall ESRAppendData(ivPointer pEsrObj,
								  ivCPointer pData,
								  ivUInt16 nSamples		/* In samples */ 	 						 
								  )
{
	PEsrEngine pEngine;
    EsErrID err;

	/* Memory align */
	pEsrObj = (ivPointer)(((ivUInt32)pEsrObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));

#if MINI5_API_PARAM_CHECK
	ivAssert(ivNull != pEsrObj && ivNull != pData);
	if(ivNull == pEsrObj || ivNull == pData){
		return EsErr_InvArg;
	}
	if(DW_OBJ_CHECK != *(ivPUInt32)pEsrObj){
		return EsErr_InvCal;
	}

    if(nSamples > APPENDDATA_SAMPLES_MAX) {
        return EsErr_InvCal;
    }
#endif

	pEsrObj = (ivPointer)((ivPUInt32)pEsrObj + 1);

	pEngine = (PEsrEngine)pEsrObj;

	err = EsFrontAppendData(&pEngine->tFront,pData,nSamples);
	//ivAssert(EsErrID_OK == iStatus);

	return err;
}

#if MINI5_SPI_MODE && MINI5_TAG_SUPPORT

/* 函数编号:48, 2012.5.24完成 */
EsErrID ivCall ESRCreateTagObj(
								   ivPointer		pTagObj,	/* Tag Object */
								   ivSize ivPtr 	pnTagObjSize,	/* [In/Out] Size of Tag object */
								   ivCPointer  		pModel,		/* [In] Model */	
								   ivUInt16			nTagID,
								   ivPointer		pSPIBuf,		/* PC临时模拟.用于存储每帧path和feature信息的SPI地址*/
								   ivSize			nSPIBufSize
								   )
{
	PEsrEngine pEngine;
	EsErrID err;
	ivPUInt32 pTmp;

	ivSize nSize;
	ivPUInt16 pBuffer;
	ivInt16 i;
    PResHeader pResHeader;

	if(ivNull == pTagObj || ivNull == pnTagObjSize || ivNull == pModel || ivNull == pSPIBuf){
		ivAssert(ivFalse);
		return EsErr_InvArg;
	}

    /* Memory align */
    pBuffer = (ivPointer)(((ivUInt32)pTagObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));	

	/* -----------------------Obj Size------------------- */
	nSize = ivGridSize(sizeof(TEsrEngine) + sizeof(ivUInt32) + IV_PTR_GRID);

    /* -----------------------Front--------------------- */
    /* PCMBuffer  */
    /* nSize += ivGridSize(sizeof(ivInt16)*ESR_PCMBUFFER_SIZE); */

    /* Energy Buffer */
#if ESR_ENERGY_LOG
    nSize += ivGridSize(sizeof(ivInt16)*ESR_BACK_FRAMES);
#else
    nSize += ivGridSize(sizeof(ivInt16)*ESR_BACK_FRAMES);
#endif

    /* Frame Cache */	
    nSize += ivGridSize(sizeof(ivInt16)*(TRANSFORM_FFTNUM_DEF+2));
    /* CMN Buffer */
    nSize += ivGridSize(sizeof(ivInt16)*(TRANSFORM_CEPSNUM_DEF+1));
    nSize += ivGridSize(sizeof(ivInt16)*(TRANSFORM_CEPSNUM_DEF+1));
    nSize += ivGridSize(sizeof(ivInt32)*(TRANSFORM_CEPSNUM_DEF+1));

    /* MFCC Buffer */
    nSize += ivGridSize(ESR_MFCC_BACK_FRAMES*(TRANSFORM_CEPSNUM_DEF+1)*sizeof(ivInt16));

    /* #if ESR_ENABLE_ENHANCE_VAD */
    /* For Filter BandPass */
    nSize += ivGridSize(ESR_HIPASSENERGY_NUM * sizeof(ivInt32));

#if MINI5_SPI_MODE
    err = EsSPIReadBuf(pTagObj,pModel, sizeof(TResHeader));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
    pResHeader = (PResHeader)pTagObj;
#else
    pResHeader = (PResHeader)pModel;
#endif
    if(DW_TAG_MODEL_CHECK != pResHeader->dwCheck) {
        return EsErr_InvRes;
    }

	/* -----------------------Search--------------------- */
	nSize += ivGridSize(sizeof(ivInt16) * ((pResHeader->nMixSModelNum-2)+4));	/* pScoreCache. -2:sil,filler */
	nSize += ivGridSize(sizeof(ivInt16) * ((pResHeader->nMixSModelNum-2)+4));   /* pnRepeatFrmCache */ 

	if(*pnTagObjSize < nSize){
		*pnTagObjSize = nSize;
		return EsErr_OutOfMemory;
	} 

	pTmp = (ivPUInt32)pBuffer;
	*pTmp = DW_OBJ_CHECK;
	pBuffer = (ivPointer)(pTmp+1);

	pEngine = (PEsrEngine)pBuffer;
	pBuffer = (ivPUInt16)((ivAddress)pBuffer + ivGridSize(sizeof(TEsrEngine)));
	pEngine->pResidentRAMHdr = (PResidentRAMHdr)pBuffer;
	
	/* Init CMN Mean 9.6, C0:11.4 */
	for(i=0; i<TRANSFORM_CEPSNUM_DEF+1; i++){
		pEngine->pResidentRAMHdr->ps16CMNMean[i] = g_s16CMNCoef[i];
	}
	
	pBuffer = (ivPUInt16)((ivAddress)pBuffer + sizeof(TResidentRAMHdr));
	pEngine->pBufferBase = pBuffer;
	pEngine->pBuffer = pBuffer;
	pEngine->pBufferEnd = (ivPUInt16)((ivAddress)pTagObj + *pnTagObjSize);
	pEngine->tFront.m_nSpeechTimeOut = (ivInt32)TAG_SPEECH_TIMEOUT; /* Speech time out frame */

	pEngine->iEngineType = ESENGINE_TAG;
	pEngine->nTagID = nTagID;
	pEngine->pSPIBufBase = pSPIBuf;
	pEngine->pSPIBufEnd = (ivPointer)((ivAddress)pSPIBuf + nSPIBufSize);
	pEngine->pTagCmdBuf = ivNull;

	err = EsInit(pEngine, pModel);
	ivAssert(EsErrID_OK == err);
	if(EsErrID_OK != err){
		return err;
	}

	return err;
}

//#define TAG_LOG
#ifdef TAG_LOG
#include <stdio.h>
ivBool ParserLexicon(ivPointer pLex, PMixStateModel pMixSModel, int nMix, char *szOptFile)
{
	PLexiconHeader pLexHdr;
	PLexNode pLexRoot, pLexNode;
	PCmdDesc pCmdDesc;
	int i, j, n, m;
	FILE *fp;
	ivPUInt16 pStateLst;

	if(NULL == pLex || NULL == pMixSModel || NULL == szOptFile){
		return ivFalse;
	}
	pLexHdr = (PLexiconHeader)pLex;
	pLexRoot = (PLexNode)((ivUInt32)pLex + pLexHdr->nLexRootOffset);
	pCmdDesc = (PCmdDesc)((ivUInt32)pLex + pLexHdr->nCmdDescOffset);

	pStateLst = (ivPUInt16)malloc(1000 * sizeof(ivInt16));

	/* 将整个网络转正 */
	for(i=0; i<pLexHdr->nTotalNodes/2; i++){
		TLexNode tTmpNode;
		n = i;
		m = pLexHdr->nTotalNodes - 1 - i;			
		ivMemCopy(&tTmpNode, pLexRoot+n, sizeof(TLexNode));
		ivMemCopy(pLexRoot+n, pLexRoot+m, sizeof(TLexNode));
		ivMemCopy(pLexRoot+m, &tTmpNode, sizeof(TLexNode));
	}

	fp = fopen(szOptFile, "wb");

	fprintf(fp, "nTotalNode=%d\r\n", pLexHdr->nTotalNodes);
	for(i=0; i<pLexHdr->nTotalNodes; i++){
		fprintf(fp, "i=%d, iParent=%d\r\n", i, pLexRoot[i].iParent);
	}
	fprintf(fp, "\r\n");

	j = 0;
	for(i=0; i<pLexHdr->nCmdNum; i++){
		int nState = 0;
		pLexNode = pLexRoot + pLexHdr->nTotalNodes - pLexHdr->nCmdNum + i; /* Leaf Node */
		while(1){
			pStateLst[nState++] = pLexNode->iSModeIndex;

			if(0 == pLexNode->iParent){
				break;
			}

			pLexNode = pLexRoot+ pLexNode->iParent - 1;
		}

		fprintf(fp, "CmdID=%d, Thresh=%d, bTag=%d\r\n", pCmdDesc[i].nID, pCmdDesc[i].nCMThresh, pCmdDesc[i].bTag);
		//for(j=0; j<nState; j++){
		//	fprintf(fp, "%d, ", pStateLst[nState - 1 - j]);
		//}

		//fprintf(fp,"\r\n");
		for(j=0; j<nState; j++){
			PMixStateModel pState;
			int k, idx, m, n;
			idx = pStateLst[nState - 1 -j]/ sizeof(TStateModel);
			pState = (PMixStateModel)((ivUInt32)pMixSModel + pStateLst[nState - 1 -j] );
			fprintf(fp, " iState=%d:\r\n", j);
			for(k=0; k<nMix; k++){
				fprintf(fp, "   iMix=%d, fGConst=%d, fFea=", k, pState->tGModel[k].fGConst);
				for(m=0; m<FEATURE_DIMNESION; m++){
					fprintf(fp, "%d, ", pState->tGModel[k].fMeanVar[m]);
				}
				fprintf(fp,"\r\n");
			}
		}

		fprintf(fp, "\r\n\r\n");
	}

	fclose(fp);

	/* 将整个网络再倒过去 */
	for(i=0; i<pLexHdr->nTotalNodes/2; i++){
		TLexNode tTmpNode;
		n = i;
		m = pLexHdr->nTotalNodes - 1 - i;			
		ivMemCopy(&tTmpNode, pLexRoot+n, sizeof(TLexNode));
		ivMemCopy(pLexRoot+n, pLexRoot+m, sizeof(TLexNode));
		ivMemCopy(pLexRoot+m, &tTmpNode, sizeof(TLexNode));
	}

	return ivTrue;
}

ivBool ParserGrm(PGrmMdlHdr pGrmMdlHdr, char *szFileName)
{
	int nSize, i;
	ivUInt32 nCRC =0, nCounter = 0;
	ivPointer pLexicon;
	char szTmp[1024];
	PResHeader pModelHdr;
	PMixStateModel pMixSModel;
	FILE *fp;
	int nMix;
	
	nSize = sizeof(TGrmMdlHdr) + sizeof(TGrmDesc) * (pGrmMdlHdr->nTotalGrm - 1);
	ivMakeCRC((ivPCInt16)((ivUInt32)pGrmMdlHdr+nSize), pGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
	if(nCRC != pGrmMdlHdr->nCRC){
		return ivFalse;
	}

	pModelHdr = (PResHeader)((ivUInt32)pGrmMdlHdr + pGrmMdlHdr->nModelOffset * sizeof(ivInt16));
	pMixSModel = (PMixStateModel)((ivUInt32)pModelHdr+(pModelHdr->nMixSModelOffset * sizeof(ivInt16)));
	nMix = pModelHdr->nMixture;

	for(i=0; i<pGrmMdlHdr->nTotalGrm; i++){
		pLexicon = (ivPointer)((ivUInt32)pGrmMdlHdr + pGrmMdlHdr->tGrmDesc[i].nGrmOffset * sizeof(ivInt16));
		sprintf(szTmp, "%s_%d.log", szFileName, i+1);
		ParserLexicon(pLexicon, pMixSModel, nMix, szTmp);		
	}

	nCRC = 0;nCounter = 0;
	nSize = sizeof(TGrmMdlHdr) + sizeof(TGrmDesc) * (pGrmMdlHdr->nTotalGrm - 1);
	ivMakeCRC((ivPCInt16)((ivUInt32)pGrmMdlHdr+nSize), pGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
	if(nCRC != pGrmMdlHdr->nCRC){
		return ivFalse;
	}

	return ivTrue;
}
#endif

void EsReverserOrder(PLexNode pLexRoot, ivUInt16 nTotalNode)
{
	ivUInt16 i;
	PLexNode pNode1;
	PLexNode pNode2;
	TLexNode tTmpNode;


	/* 将整个网络节点先由倒序存储转正,方便理解和操作 */
	pNode1 = pLexRoot;
	pNode2 = pLexRoot + nTotalNode - 1;
	for(i=0; i<nTotalNode/2; i++, pNode1++, pNode2--){	
		/* ivMemCopy(&tTmpNode, pNode1, sizeof(TLexNode)); */
		tTmpNode.iParent = pNode1->iParent;
        tTmpNode.iSModeIndex = pNode1->iSModeIndex;
        tTmpNode.nStateCount = pNode1->nStateCount;

		/* ivMemCopy(pNode1, pNode2, sizeof(TLexNode)); */
		pNode1->iParent = pNode2->iParent;
        pNode1->iSModeIndex = pNode2->iSModeIndex;
        pNode1->nStateCount = pNode2->nStateCount;
		/* ivMemCopy(pNode2, &tTmpNode, sizeof(TLexNode)); */
		pNode2->iParent = tTmpNode.iParent;
        pNode2->iSModeIndex = tTmpNode.iSModeIndex;
        pNode2->nStateCount = tTmpNode.nStateCount;
	}
}

ivESRStatus EsAddTagToLex(				
					ivPointer	pLexBuf,		/* WorkSpace */		
					ivUInt16	nTagStateNum,	
					PCmdDesc	pTagCmdDesc,								
					ivUInt16	iTagSModelIndex
					)
{
	PLexiconHeader pLexHdr;
	PLexNode pLexRoot;
	PLexNode pSrcNode;
	PLexNode pDstNode;
	PCmdDesc pCmdDesc;
	PCmdDesc pDstCmdDsc, pSrcCmdDsc;
	ivUInt16 i, n, iParent;
	ivUInt16 nTagHead;
	ivUInt16 nTagMid;
	ivUInt16 nTagTail;
	ivUInt16 nOrgHead;
	ivUInt16 nOrgMid;
	ivUInt16 nOrgTail;

	if(ivNull == pLexBuf || ivNull == pTagCmdDesc){
		ivAssert(ivFalse);
		return ivESR_INVARG;
	}

	/*
	语法树结构说明：
	*/
	pLexHdr = (PLexiconHeader)pLexBuf;
	pLexRoot = (PLexNode)((ivAddress)pLexHdr + pLexHdr->nLexRootOffset);
	pCmdDesc = (PCmdDesc)((ivAddress)pLexHdr + pLexHdr->nCmdDescOffset);
	
	/* 修改pCmdDesc位置.不能直接调用ivMemCopy,内存可能有重叠,会覆盖有效信息 */
	pLexHdr->nCmdDescOffset += nTagStateNum * sizeof(TLexNode);
	pSrcCmdDsc = pCmdDesc + pLexHdr->nCmdNum - 1;
   
    pCmdDesc = (PCmdDesc)((ivAddress)pLexHdr + pLexHdr->nCmdDescOffset); 
    pDstCmdDsc = pCmdDesc + pLexHdr->nCmdNum - 1;
    for(n=0; n<pLexHdr->nCmdNum; n++, pSrcCmdDsc--, pDstCmdDsc--)
    {
        pDstCmdDsc->bTag = pSrcCmdDsc->bTag;
        pDstCmdDsc->nCMThresh = pSrcCmdDsc->nCMThresh;
        pDstCmdDsc->nID = pSrcCmdDsc->nID;
    }

	pCmdDesc[pLexHdr->nCmdNum].nID = pTagCmdDesc->nID;
	pCmdDesc[pLexHdr->nCmdNum].nCMThresh = pTagCmdDesc->nCMThresh;
	pCmdDesc[pLexHdr->nCmdNum].bTag = pTagCmdDesc->bTag;

	/* 将整个网络节点先由倒序存储转正,方便理解和操作 */
	EsReverserOrder(pLexRoot, pLexHdr->nTotalNodes);

	/* 头中尾表示:可扩展节点数，中间节点数，叶子节点数 */
	/* 计算首中尾节点个数 */
	nOrgHead = pLexHdr->nExtendNodes;
	nOrgTail = (ivUInt16)pLexHdr->nCmdNum;
	nOrgMid = pLexHdr->nTotalNodes - nOrgHead - nOrgTail;
	nTagHead = ivMin(8, nTagStateNum-1);
	/* nExtendNode < nTotalNode/2. 解码需要 */
	/* while((nTagHead + nOrgHead)*2 > pLexHdr->nTotalNodes + nTagStateNum){ */
	while(pLexHdr->nTotalNodes + nTagStateNum - (nTagHead + nOrgHead)*2 < 0){
		nTagHead --;
	}
	nTagTail = 1;
	nTagMid = nTagStateNum - nTagHead - nTagTail;	

	/* 采用的存储顺序是 OrgHead TagHead TagMid OrgMid OrgTail TagTail */

	/* --------------------------Step1. Org Head 不动--------------------- */

	/* --------------------------Step2. Org Tail-------------------------- */
	n = nTagHead + nTagMid;
	pSrcNode = pLexRoot + pLexHdr->nTotalNodes - 1;
	pDstNode = pSrcNode + n;
	for(i=0; i<pLexHdr->nCmdNum; i++, pSrcNode--, pDstNode--){
        pDstNode->iSModeIndex = pSrcNode->iSModeIndex;
        pDstNode->nStateCount = pSrcNode->nStateCount;
		if(pSrcNode->iParent > nOrgHead)
		{
			/* 父节点是OrgMid */
			pDstNode->iParent = pSrcNode->iParent + n;
		}
		else{
			/* 父节点是OrgHead */
			pDstNode->iParent = pSrcNode->iParent;
		}
	}

	/* --------------------------Step3. Org Mid-------------------------- */
	n = nTagHead + nTagMid;
	pSrcNode = pLexRoot + pLexHdr->nTotalNodes - pLexHdr->nCmdNum - 1;
	pDstNode = pSrcNode + n;
	for(i=0; i<nOrgMid; i++, pSrcNode--, pDstNode--){/* 从后往前,防止将还未搬移的覆盖了 */		
        pDstNode->iSModeIndex = pSrcNode->iSModeIndex;
        pDstNode->nStateCount = pSrcNode->nStateCount;
		if(pSrcNode->iParent > nOrgHead){
			/* 父节点是OrgMid */
			pDstNode->iParent = pSrcNode->iParent + n;
		}
		else{
			/* 父节点是OrgHead */
			pDstNode->iParent = pSrcNode->iParent;
		}
	}

	/* ------------------------Step4. Tag Head,Mid------------------------- */   
	iParent = 0;
	pDstNode = pLexRoot + nOrgHead;
	for(i=0; i<nTagHead+nTagMid; i++, pDstNode++){
		pDstNode->iParent = iParent;
		pDstNode->iSModeIndex = iTagSModelIndex;
        pDstNode->nStateCount = 0;
	
		iParent = nOrgHead + i + 1; /* 是父节点实际索引号+1 */
        iTagSModelIndex ++;
	}

	/* --------------------------Step5. Tag Tail-------------------------- */
	pDstNode = pLexRoot + pLexHdr->nTotalNodes + nTagStateNum - 1;
	pDstNode->iParent = iParent;
	pDstNode->iSModeIndex = iTagSModelIndex;
    pDstNode->nStateCount = nTagStateNum;
	
	/* ------------------------------------------------------------------- */

	pLexHdr->nTotalNodes += nTagStateNum;
	pLexHdr->nExtendNodes += nTagHead;
	pLexHdr->nCmdNum ++;

	/* 将整个网络倒序存储 */
	EsReverserOrder(pLexRoot, pLexHdr->nTotalNodes);

	return ivESR_OK;
}

EsErrID EsDelTagFromLex(PLexiconHeader pLexHdr, ivUInt16 nDelCmdID, ivPUInt16 pnTagStateNum)
{
    PCmdDesc pCmdDesc, pNewCmdDesc;
    PLexNode pLexRoot;
    ivUInt16 i, iTag;
    ivUInt16 iDelNode,iDelNodeParent;
    ivUInt16 nTotalNodes;
    ivUInt16 nTagStateNum;

    /* 该函数内全部内存操作 */
    if(DW_LEXHEADER_CHECK != pLexHdr->dwCheck){
        ivAssert(ivFalse);
        return EsErr_InvRes;
    }

    /* 重定位 */
    pCmdDesc = (PCmdDesc)((ivAddress)pLexHdr + (ivUInt32)(pLexHdr->nCmdDescOffset));
    for(i=0; i<pLexHdr->nCmdNum; i++) {
        if(1 == pCmdDesc[i].bTag && nDelCmdID == pCmdDesc[i].nID) {
            break;
        }
    }
    if(i >= pLexHdr->nCmdNum) {
        /* 没有需要删除的Tag,直接go out */
        *pnTagStateNum = 0; 
        return ivESR_OK;
    }

    iTag = i;

    pLexRoot = (PLexNode)((ivAddress)pLexHdr + (ivUInt32)(pLexHdr->nLexRootOffset));
    nTotalNodes = pLexHdr->nTotalNodes;

    EsReverserOrder(pLexRoot, nTotalNodes);

    /* 找到要删除路径的叶子节点,从尾向头路径逐个删除 */
    nTagStateNum = 0;
    iDelNode = (ivUInt16)(nTotalNodes - pLexHdr->nCmdNum + iTag);
    iDelNodeParent = pLexRoot[iDelNode].iParent;	/* ->iParent从1开始 */
    while(1) {
        for(i=iDelNode+1; i<nTotalNodes; i++) {
            pLexRoot[i-1].iSModeIndex = pLexRoot[i].iSModeIndex;
#if MINI5_USE_NEWCM_METHOD
            pLexRoot[i-1].nStateCount = pLexRoot[i].nStateCount;
#endif
            if(pLexRoot[i].iParent > iDelNode) {
                pLexRoot[i-1].iParent = pLexRoot[i].iParent - 1;
            }
            else {
                pLexRoot[i-1].iParent = pLexRoot[i].iParent;
            }
        }

        nTotalNodes --;
        nTagStateNum ++;

        if(0 == iDelNodeParent) {
            break;
        }
        iDelNode = iDelNodeParent - 1;
        iDelNodeParent = pLexRoot[iDelNode].iParent ;
    }

    *pnTagStateNum = nTagStateNum;

    EsReverserOrder(pLexRoot, nTotalNodes);

    /* 修改PCmdDesc的偏移 */
    pNewCmdDesc = (PCmdDesc)((ivAddress)pCmdDesc - nTagStateNum*sizeof(TLexNode));
    for(i=0; i<pLexHdr->nCmdNum; i++) {
        if(i == iTag) {
            continue;
        }
        ivMemCopy(pNewCmdDesc, pCmdDesc+i, sizeof(TCmdDesc));
        pNewCmdDesc ++;
    }

    pLexHdr->nCmdDescOffset -= nTagStateNum*sizeof(TLexNode);
    pLexHdr->nTotalNodes = nTotalNodes;
    pLexHdr->nCmdNum --;

    return ivESR_OK;	
}

/* 函数编号:50, 2012.5.31完成 */
#if MINI5_SPI_MODE
EsErrID ivCall ESRAddTagToGrm(
			ivPointer	pTagObj, 
			ivCPointer	pOrgRes, 
			ivPointer	pNewRes, 
			ivPUInt32	pnNewResSize, /* 单位为平台最小访问单位 */ 
			ivPUInt16	pGrmIDLst, 
			ivUInt16	nGrmIDNum, 
			ivUInt16	nTagThresh
			)
{
	PEsrEngine pEngine;
	PTagCmd pTagCmdSPI;
	ivUInt16 nTagStateNum;
	ivPointer pWorkSpace;
	PGrmMdlHdr pOrgGrmMdlHdr;
	ivUInt16 nGrmMdlHdrSize;
	PResHeader pNewModelHdr;
	PGrmDesc pOrgGrmDesc;
	ivUInt16 i, j;
	ivUInt32 nCRC;
	ivUInt32 nCounter;
	ivUInt32 nWorkSpaceSize;
	ivBool bValidGrmID;
	ivUInt32 nOffset;
	ivUInt32 nGrmSize;
	EsErrID err;
	ivCPointer pOrgModelHdrSPI;
	ivUInt16 nMixSModelWSize;
	ivUInt16 nMixSModelNum;
	TCmdDesc tTagCmdDesc;
	
	ivUInt32 nModelCRC;
	ivUInt32 nModelCnt;

    ivUInt32 nOrgSilSModelOffset;
	ivUInt32 nNewLexSize;

    ivUInt32 nSize;
    ivPCInt16 pData;
	ivUInt32 nTagModelSize;

    ivPointer pRamBuffer;
    ivPointer pSPIBuffer;
    PSPICache pSPICache;

	if(ivNull == pTagObj || ivNull == pOrgRes  || ivNull == pNewRes || ivNull == pGrmIDLst || ivNull == pnNewResSize){
		ivAssert(ivFalse);
		return EsErr_InvArg;
	}

	if(nTagThresh > 100){
		return ivESR_INVCAL;
	}

	/* Memory align */
	pTagObj = (ivPointer)(((ivUInt32)pTagObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));
	if(DW_OBJ_CHECK != *(ivPUInt32)pTagObj){
		return EsErr_InvCal;
	}

	pEngine = (PEsrEngine)((ivPUInt32)pTagObj + 1);

	/* 复用pTagObj对象内存 */	
    nSize = ivGridSize(sizeof(TEsrEngine) + IV_PTR_GRID);
	pWorkSpace = (ivPointer)((ivAddress)pEngine + nSize);

	/* 将OrgRes的资源头从SPI中拷贝到内存中 */
	pOrgGrmMdlHdr = (PGrmMdlHdr)pWorkSpace;
	nGrmMdlHdrSize = sizeof(TGrmMdlHdr);

    err = EsSPIReadBuf((ivPointer)pOrgGrmMdlHdr, pOrgRes, nGrmMdlHdrSize);
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }

	if(DW_MINI_RES_CHECK != pOrgGrmMdlHdr->dwCheck) {
		return EsErr_InvRes;
	}
	if(pOrgGrmMdlHdr->nTotalGrm > 1) {
		/* 从SPI Copy */
        pSPIBuffer = (ivPointer)((ivAddress)pOrgRes + nGrmMdlHdrSize);
        pRamBuffer = (ivPointer)((ivAddress)pOrgGrmMdlHdr + nGrmMdlHdrSize);
        err = EsSPIReadBuf(pRamBuffer, pSPIBuffer, sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1));
        ivAssert(EsErrID_OK == err);
        if (EsErrID_OK != err) {
            return EsErr_Failed;
        }
        nGrmMdlHdrSize += sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1);
	}
    /* 复用pTagObj从老资源读语法 */
    ivAssert(MINI5_SPI_PAGE_SIZE >= nGrmMdlHdrSize);
	pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(MINI5_SPI_PAGE_SIZE));

    /* 复用pTagObj对象读TagCmd的SPI */
    /* SPI Buf */
    pTagCmdSPI = (PTagCmd)pWorkSpace;
    if(ivNull == pTagCmdSPI) {
        ivAssert(ivFalse);
        return EsErr_InvCal;
    }
    err = EsSPIReadBuf(pWorkSpace, (ivPointer)pEngine->pTagCmdBuf, sizeof(TTagCmd));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }

    nTagStateNum = pTagCmdSPI->nState;
	if(DW_TAGCMD_CHECK != pTagCmdSPI->dwCheck) {
		ivAssert(ivFalse);
		return EsErr_InvCal;
	}
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TTagCmd)));

    /* 复用pTagObj做为SPICache */
    pSPICache = (PSPICache)pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TSPICache)));
    pSPICache->iCacheSPIWrite = 0;
    pSPICache->pCacheSPI = pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(MINI5_SPI_PAGE_SIZE));

    /* 复用pTagObj做为模型资源头 */
    pNewModelHdr = (PResHeader)pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TResHeader)));

    pOrgGrmDesc = (PGrmDesc)pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TGrmDesc)));

	nWorkSpaceSize = (ivAddress)pEngine->pBufferEnd - (ivAddress)pWorkSpace;	

	/* 对SPI中的OrgRes进行CRC */
	nCRC = 0;
	nCounter = 0;
    pSPIBuffer = (ivPointer)((ivAddress)pOrgRes + nGrmMdlHdrSize);
    err = EsSPICalcCRC(pWorkSpace, pSPIBuffer, pOrgGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
    ivAssert(EsErrID_OK == err);
    if(nCRC != pOrgGrmMdlHdr->nCRC) {
		return EsErr_InvRes;
	}

	/* 内存操作.看是否有需要添加Tag的 */
	bValidGrmID = ivFalse;
    ivMemCopy(pOrgGrmDesc, pOrgGrmMdlHdr->tGrmDesc, sizeof(TGrmDesc));
    pSPIBuffer = (ivPointer)((ivAddress)pOrgRes + (sizeof(TGrmMdlHdr) - sizeof(TGrmDesc)));

	tTagCmdDesc.nID = pEngine->nTagID;
	tTagCmdDesc.nCMThresh = nTagThresh;
	tTagCmdDesc.bTag = 1;

	/* 读SPI中的模型信息，用于添加tag时的iSModelOffset计算 */
	pOrgModelHdrSPI = (ivCPointer)((ivAddress)pOrgRes + pOrgGrmMdlHdr->nModelOffset * sizeof(ivInt16));
    err = EsSPIReadBuf((ivPointer)pNewModelHdr, pOrgModelHdrSPI, sizeof(TResHeader));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
    nOrgSilSModelOffset = pNewModelHdr->nSilSModelOffset;

	nMixSModelNum = (ivUInt16)(pNewModelHdr->nMixSModelNum); /* 用于后面tag加入时pLexNode->iSModeOffset */
	nMixSModelWSize = (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel)); /* word */
	
    pSPICache->pSPIBufBase = (ivPointer)((ivAddress)pNewRes + MINI5_SPI_PAGE_SIZE);
    pSPICache->pSPIBuf = pSPICache->pSPIBufBase;
    pSPICache->pSPIBufEnd = (ivPointer)((ivAddress)pNewRes + *pnNewResSize);

	nCRC =0; 
	nCounter = 0;
	nOffset = nGrmMdlHdrSize;
	/* pNewRes留出GrmMdlHdr头信息,由于有nCRC需要全部处理完才得到. 最后回头填写 */
	for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) {
		ivPointer pOrgLex;
		bValidGrmID = ivFalse;
		j = 0;
		while(j < nGrmIDNum) {
			if(pOrgGrmDesc->nGrmID == pGrmIDLst[j]) {
				/* Need to add tag cmd */
				bValidGrmID = ivTrue;
				break;
			}
			j++;
		} /* while(j < nGrmIDNum) */

		pOrgLex = (ivPointer)((ivAddress)pOrgRes + pOrgGrmDesc->nGrmOffset * sizeof(ivInt16));
		nGrmSize = pOrgGrmDesc->nGrmSize * sizeof(ivInt16);
		pOrgGrmDesc->nGrmOffset = nOffset / sizeof(ivInt16);

		if(nWorkSpaceSize < nGrmSize) {
			ivAssert(ivFalse);
			return EsErr_OutOfMemory;
		}

		/* 将SPI中的语法网络拷贝到内存中进行添加tag的操作 */
        err = EsSPIReadBuf(pWorkSpace, pOrgLex, nGrmSize);
        ivAssert(EsErrID_OK == err);
        if (EsErrID_OK != err) {
            return EsErr_Failed;
        }

		if(bValidGrmID) {
			/* 向网络中添加tag */
			nNewLexSize = nGrmSize + nTagStateNum * sizeof(TLexNode) + sizeof(TCmdDesc);
			if(nWorkSpaceSize < nNewLexSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}

			if(nOffset + nNewLexSize > *pnNewResSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}

			err = EsAddTagToLex(pWorkSpace, nTagStateNum, &tTagCmdDesc, (ivUInt16)(nMixSModelNum - 2 - pNewModelHdr->nOldFillerCnt)); /* -2:sil,filler */
			if(ivESR_OK != err) {
				ivAssert(ivFalse);
				return err;
			}

			pOrgGrmDesc->nGrmSize = nNewLexSize / sizeof(ivInt16);

			nGrmSize = nNewLexSize;			
		} /* if(bValidGrmID) */
		else {
			if(nOffset + nGrmSize > *pnNewResSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}			
		} /* else */

		ivMakeCRC(pWorkSpace, nGrmSize, &nCRC, &nCounter);		
		nOffset += nGrmSize;

        nSize = 0;
        ivAssert(nGrmMdlHdrSize  < MINI5_SPI_PAGE_SIZE);
        if (nGrmMdlHdrSize < MINI5_SPI_PAGE_SIZE) {
            pRamBuffer = (ivPointer)((ivAddress)pOrgGrmMdlHdr + nGrmMdlHdrSize);
            nSize = MINI5_SPI_PAGE_SIZE - nGrmMdlHdrSize;
            if (nGrmSize < nSize) {
                nSize = nGrmSize;
            }

            nGrmSize = nGrmSize - nSize;
            nGrmMdlHdrSize = MINI5_SPI_PAGE_SIZE;
            ivMemCopy(pRamBuffer, pWorkSpace, nSize);
        }
        EsSPIWriteData(pSPICache, (ivPointer)((ivAddress)pWorkSpace + nSize), nGrmSize);
        pRamBuffer = (ivPointer)((ivAddress)pOrgGrmMdlHdr + (sizeof(TGrmMdlHdr) - sizeof(TGrmDesc)) + i * sizeof(TGrmDesc));
        ivMemCopy(pRamBuffer, pOrgGrmDesc, sizeof(TGrmDesc));

        pSPIBuffer = (ivPointer)((ivAddress)pSPIBuffer + sizeof(TGrmDesc));
        EsSPIReadBuf((ivPointer)pOrgGrmDesc, pSPIBuffer, sizeof(TGrmDesc));
	} /* for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) */

	pOrgGrmMdlHdr->nModelOffset = nOffset / sizeof(ivInt16);
	pNewModelHdr->nMixSModelNum += nTagStateNum;

    /* 新的tag模型放在sil和filler模型前面,因而要修改sil和filler模型的偏移值 */
    nTagModelSize = (nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel))) / sizeof(ivInt16);
    pNewModelHdr->nSilSModelOffset += nTagModelSize;
    pNewModelHdr->nFillerSModelOffset += nTagModelSize;
    if(pNewModelHdr->nOldFillerCnt > 0) {
        pNewModelHdr->nOldFillerSModelOffset += nTagModelSize;
    }

    /* ------------------------重新计算模型CRC.Begin-------------------------- */ 
	nModelCRC = 0;
	nModelCnt = 0;

    pData = (ivPInt16)((ivAddress)pNewModelHdr + sizeof(ivUInt32)*2);
    nSize = sizeof(TResHeader) - sizeof(ivUInt32)*2;
	ivMakeCRC(pData, nSize, &nModelCRC, &nModelCnt);

    pData = (ivPInt16)((ivAddress)pOrgModelHdrSPI+sizeof(TResHeader));
    nSize = (pNewModelHdr->nMixSModelNum - nTagStateNum - 2 - pNewModelHdr->nOldFillerCnt) * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
	EsSPICalcCRC(pWorkSpace,pData, nSize, &nModelCRC, &nModelCnt);
	
    pData = (ivCPointer)((ivAddress)pEngine->pTagCmdBuf + (sizeof(TTagCmd) - sizeof(TMixStateModel)));
    nSize = nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    EsSPICalcCRC(pWorkSpace, pData, nSize, &nModelCRC, &nModelCnt);

    ivAssert(pNewModelHdr->nSilSModelOffset < pNewModelHdr->nFillerSModelOffset);
    pData = (ivPInt16)((ivAddress)pOrgModelHdrSPI + nOrgSilSModelOffset * sizeof(ivInt16));
    nSize = (pNewModelHdr->nSilMixture + pNewModelHdr->nFillerMixture + pNewModelHdr->nOldFillerCnt * pNewModelHdr->nOldFillerMix) * sizeof(TGModel);
    EsSPICalcCRC(pWorkSpace, pData, nSize, &nModelCRC, &nModelCnt);	

    /* Model Info */
    nSize = sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1) + sizeof(TGrmMdlHdr);
    nSize += (nModelCnt + nCounter)*sizeof(ivInt16) + sizeof(TResHeader);
    if( nSize> *pnNewResSize) {
        ivAssert(ivFalse);
        /* 实际上SPI的最小字节为 (int)(nSize/MINI5_SPI_PAGE_SIZE + 1) * MINI5_SPI_PAGE_SIZE, 比返回的新资源的字节数稍大些*/
        *pnNewResSize = nSize;
        return EsErr_OutOfMemory;
    }

    pNewModelHdr->nCRC = nModelCRC;
	pNewModelHdr->nCRCCnt = nModelCnt;
    /* ------------------------重新计算模型CRC.End-------------------------- */

    /* ------------------------将调整后的模型写入SPI中.Begin-------------------------- */ 
    pData = (ivPCInt16)pNewModelHdr;
    nSize = sizeof(TResHeader);
	ivMakeCRC(pData, nSize, &nCRC, &nCounter);
    EsSPIWriteData(pSPICache, pData, nSize);

    pData = (ivPCInt16)((ivAddress)pOrgModelHdrSPI+sizeof(TResHeader));
    nSize = (pNewModelHdr->nMixSModelNum - nTagStateNum - 2 - pNewModelHdr->nOldFillerCnt) * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    EsSPIWriteSPIData(pSPICache, pWorkSpace, pData, nSize,&nCRC, &nCounter);

    pData = (ivCPointer)((ivAddress)pEngine->pTagCmdBuf + sizeof(TTagCmd) - sizeof(TMixStateModel));
    nSize = nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    EsSPIWriteSPIData(pSPICache, pWorkSpace, pData, nSize,&nCRC, &nCounter);

    ivAssert(pNewModelHdr->nSilSModelOffset < pNewModelHdr->nFillerSModelOffset);
    pData = (ivPCInt16)((ivAddress)pOrgModelHdrSPI + nOrgSilSModelOffset * sizeof(ivInt16));
    nSize = (pNewModelHdr->nSilMixture + pNewModelHdr->nFillerMixture + pNewModelHdr->nOldFillerCnt * pNewModelHdr->nOldFillerMix) * sizeof(TGModel);
    EsSPIWriteSPIData(pSPICache, pWorkSpace, pData, nSize, &nCRC, &nCounter);

    err = EsSPIWriteFlush(pSPICache);
    ivAssert(EsErrID_OK == err);

	pOrgGrmMdlHdr->nCRC = nCRC;
	pOrgGrmMdlHdr->nCRCCnt = nCounter;
	pOrgGrmMdlHdr->nModelSize += nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture-1) * sizeof(TGModel));

    pSPICache->pSPIBufBase = (ivPointer)pNewRes;
    pSPICache->pSPIBuf = pSPICache->pSPIBufBase;
    pSPICache->pSPIBufEnd = (ivPointer)((ivAddress)pNewRes + MINI5_SPI_PAGE_SIZE);
    EsSPIWriteData(pSPICache, pOrgGrmMdlHdr, nGrmMdlHdrSize);
    ivAssert(0 == pSPICache->iCacheSPIWrite);

    /* 输出新资源的大小 */
	*pnNewResSize = nGrmMdlHdrSize + nCounter*sizeof(ivInt16);

#ifdef TAG_LOG
	ParserGrm(pOrgRes, "ParserOrgLex");
	ParserGrm(pNewRes, "ParserTagLex");
#endif

	return EsErrID_OK;
}

/* 函数编号 */
EsErrID ivCall ESRDelTagFromGrm(
                                ivPointer      pTagObj,            /* Tag Object */
                                ivSize ivPtr   pnTagObjSize,       /* [In/Out] Size of Tag object */
                                ivCPointer     pOrgRes,            /* [In] Original Resource */
                                ivPointer      pNewRes,            /* [In/Out] Original Resource */
                                ivPUInt32      pnNewResSize,       /* [In/Out]Size of pNewRes */ 
                                ivUInt16       nDelGrmID,          /* [In] Grammar ID to Delete VoiceTag */
                                ivUInt16       nDelTagID           /* [In] VoiceTag ID to Delete */
                                )
{
    PGrmMdlHdr pOrgGrmMdlHdr;
    ivSize nGrmMdlHdrSize;
    ivPointer pNewLexSPI;
    PGrmDesc pOrgGrmDesc;
    ivUInt16 i;

    ivUInt32 nCRC;
    ivUInt32 nCounter;

    ivPointer pWorkSpace;
    ivSize nWorkSpaceSize;
    ivUInt16 nGrmSize;
    ivUInt16 nSize;

    ivUInt16 nTagStateNum = 0;

    ivUInt32 nOffset;
    EsErrID err;
    ivCPointer pModelHdrSPI;
    PSPICache pSPICache;

    ivPointer pSPIBuffer;
    ivPointer pRamBuffer;

    if(ivNull == pTagObj || ivNull == pnTagObjSize || ivNull == pOrgRes || ivNull == pNewRes || ivNull == pnNewResSize){
        ivAssert(ivFalse);
        return ivESR_INVARG;
    }

    /* Memory align */
    pWorkSpace = (ivPointer)(((ivUInt32)pTagObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));
    nWorkSpaceSize = *pnTagObjSize;

    /* -----------将OrgRes的资源头从SPI中拷贝到内存中.Begin------------------- */
    pOrgGrmMdlHdr = (PGrmMdlHdr)pWorkSpace;	
    err = EsSPIReadBuf((ivPointer)pOrgGrmMdlHdr, pOrgRes, sizeof(TGrmMdlHdr));
    ivAssert(EsErrID_OK == err);
    if (EsErrID_OK != err) {
        return EsErr_Failed;
    }
    if(DW_MINI_RES_CHECK != pOrgGrmMdlHdr->dwCheck){
        return ivESR_INVRESOURCE;
    }

    nGrmMdlHdrSize = sizeof(TGrmMdlHdr) + (pOrgGrmMdlHdr->nTotalGrm-1)*sizeof(TGrmDesc);
    if(nGrmMdlHdrSize > nWorkSpaceSize){
        *pnTagObjSize = nGrmMdlHdrSize;
        return ivESR_OUTOFMEMORY;
    }

    ivAssert(MINI5_SPI_PAGE_SIZE > nGrmMdlHdrSize);
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(MINI5_SPI_PAGE_SIZE));
    nWorkSpaceSize -= nGrmMdlHdrSize;

    pOrgGrmDesc = (PGrmDesc)pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TGrmDesc)));

    /* 复用pTagObj做为SPICache */
    pSPICache = (PSPICache)pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(sizeof(TSPICache)));
    pSPICache->iCacheSPIWrite = 0;
    pSPICache->pCacheSPI = pWorkSpace;
    pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + ivGridSize(MINI5_SPI_PAGE_SIZE));
    /* -----------将OrgRes的资源头从SPI中拷贝到内存中.End------------------- */

    /* ----------------------对SPI中的OrgRes进行CRC------------------------- */
    nCRC = 0;
    nCounter = 0;
    pSPIBuffer = (ivPointer)((ivAddress)pOrgRes+nGrmMdlHdrSize);
    err = EsSPICalcCRC(pWorkSpace, pSPIBuffer, pOrgGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
    ivAssert(EsErrID_OK == err);
    if(nCRC != pOrgGrmMdlHdr->nCRC){
        return ivESR_INVRESOURCE;
    }

    pSPICache->pSPIBufBase = (ivPointer)((ivAddress)pNewRes + MINI5_SPI_PAGE_SIZE);
    pSPICache->pSPIBuf = pSPICache->pSPIBufBase;
    pSPICache->pSPIBufEnd = (ivPointer)((ivAddress)pNewRes + *pnNewResSize);

    /* 针对每个语法网络加载到内存中进行操作 */
    nCRC =0; 
    nCounter = 0;
    nOffset = nGrmMdlHdrSize;
    /* pNewRes留出GrmMdlHdr头信息,由于有nCRC需要全部处理完才得到. 最后回头填写 */
    pNewLexSPI = (ivPointer)((ivAddress)pNewRes + nOffset);
    ivMemCopy(pOrgGrmDesc, pOrgGrmMdlHdr->tGrmDesc, sizeof(TGrmDesc));

    pSPIBuffer = (ivPointer)((ivAddress)pOrgRes + (sizeof(TGrmMdlHdr) - sizeof(TGrmDesc)));
    for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) {
        ivCPointer pOrgLex;

        pOrgLex = (ivCPointer)((ivAddress)pOrgRes + pOrgGrmDesc->nGrmOffset * sizeof(ivInt16));
        nGrmSize = (ivUInt16)(pOrgGrmDesc->nGrmSize * sizeof(ivInt16));
        pOrgGrmDesc->nGrmOffset = nOffset / sizeof(ivInt16);

        if(nGrmSize > nWorkSpaceSize){
            ivAssert(ivFalse);
            *pnTagObjSize = nGrmMdlHdrSize + nGrmSize;
            return ivESR_OUTOFMEMORY;
        }

        /* 将SPI中的语法网络拷贝到内存中进行删除tag的操作 */
        err= EsSPIReadBuf((ivPointer)pWorkSpace, pOrgLex, nGrmSize);
        ivAssert(EsErrID_OK == err);
        if (EsErrID_OK != err) {
            return EsErr_Failed;
        }

        /* ----------删除一条Tag------------ */
        if(pOrgGrmDesc->nGrmID == nDelGrmID){

            /* 可能存在多个nDelTagID的tag.全部删除 */
            while(1){
                err = EsDelTagFromLex(pWorkSpace, nDelTagID, &nTagStateNum);
                if(ivESR_OK != err){
                    ivAssert(ivFalse);
                    return err;
                }

                if(0 == nTagStateNum){ /* 该网络已经没有要删的Tag了 */
                    break;
                }

                nGrmSize -= (nTagStateNum*sizeof(TLexNode) + sizeof(TCmdDesc));				
            }

            if(nOffset + nGrmSize > *pnNewResSize) {
                ivAssert(ivFalse);
                return ivESR_OUTOFMEMORY;
            }


            pOrgGrmDesc->nGrmSize = nGrmSize / sizeof(ivInt16);	
        }

        ivMakeCRC(pWorkSpace, nGrmSize, &nCRC, &nCounter);
        nOffset += nGrmSize;

        nSize = 0;
        ivAssert(nGrmMdlHdrSize  < MINI5_SPI_PAGE_SIZE);
        if (nGrmMdlHdrSize < MINI5_SPI_PAGE_SIZE) {
            pRamBuffer = (ivPointer)((ivAddress)pOrgGrmMdlHdr + nGrmMdlHdrSize);
            nSize = MINI5_SPI_PAGE_SIZE - nGrmMdlHdrSize;
            if (nGrmSize < nSize) {
                nSize = nGrmSize;
            }

            nGrmSize = nGrmSize - nSize;
            nGrmMdlHdrSize = MINI5_SPI_PAGE_SIZE;
            ivMemCopy(pRamBuffer, pWorkSpace, nSize);
        }
        EsSPIWriteData(pSPICache, (ivPointer)((ivAddress)pWorkSpace + nSize), nGrmSize);
        pRamBuffer = (ivPointer)((ivAddress)pOrgGrmMdlHdr + (sizeof(TGrmMdlHdr) - sizeof(TGrmDesc)) + i * sizeof(TGrmDesc));
        ivMemCopy(pRamBuffer, pOrgGrmDesc, sizeof(TGrmDesc));

        pSPIBuffer = (ivPointer)((ivAddress)pSPIBuffer + sizeof(TGrmDesc));
        EsSPIReadBuf((ivPointer)pOrgGrmDesc, pSPIBuffer, sizeof(TGrmDesc));

    } /* for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) */

    /* Model Info */
    if(nOffset + pOrgGrmMdlHdr->nModelSize > *pnNewResSize){
        ivAssert(ivFalse);
        return ivESR_OUTOFMEMORY;
    }

    pModelHdrSPI = (ivCPointer)((ivAddress)pOrgRes + pOrgGrmMdlHdr->nModelOffset * sizeof(ivInt16));
    EsSPIWriteSPIData(pSPICache, pWorkSpace, pModelHdrSPI, pOrgGrmMdlHdr->nModelSize, &nCRC, &nCounter);

    err = EsSPIWriteFlush(pSPICache);
    ivAssert(EsErrID_OK == err);

    pOrgGrmMdlHdr->nModelOffset = nOffset / sizeof(ivInt16);
    pOrgGrmMdlHdr->nCRC = nCRC;
    pOrgGrmMdlHdr->nCRCCnt = nCounter;	

    pSPICache->pSPIBufBase = (ivPointer)pNewRes;
    pSPICache->pSPIBuf = pSPICache->pSPIBufBase;
    pSPICache->pSPIBufEnd = (ivPointer)((ivAddress)pNewRes + MINI5_SPI_PAGE_SIZE);
    EsSPIWriteData(pSPICache, pOrgGrmMdlHdr, nGrmMdlHdrSize);
    ivAssert(0 == pSPICache->iCacheSPIWrite);

    *pnNewResSize = nGrmMdlHdrSize + nCounter * sizeof(ivInt16);

#ifdef TAG_LOG
    ParserGrm(pOrgRes, "ParserOrgLex");
    ParserGrm(pNewRes, "ParserTagLex");
#endif

    return ivESR_OK;
}

#else /* MINI5_SPI_MODE */

EsErrID ivCall ESRAddTagToGrm(
			ivPointer	pTagObj, 
			ivCPointer	pOrgRes, 
			ivPointer	pNewRes, 
			ivPUInt32	pnNewResSize, /* 单位为平台最小访问单位 */ 
			ivPUInt16	pGrmIDLst, 
			ivUInt16	nGrmIDNum, 
			ivUInt16	nTagThresh
			)
{
	PEsrEngine pEngine;
	PTagCmd pTagCmdSPI;
	ivUInt16 nTagStateNum;
	ivPointer pWorkSpace;
	PGrmMdlHdr pOrgGrmMdlHdr;
	ivUInt16 nGrmMdlHdrSize;
	PResHeader pNewModelHdr;
	ivPointer pNewLexSPI;
	PGrmDesc pOrgGrmDesc;
	ivUInt16 i, j;
	ivUInt32 nCRC;
	ivUInt32 nCounter;
	ivUInt32 nWorkSpaceSize;
	ivBool bValidGrmID;
	ivUInt32 nOffset;
	ivUInt32 nGrmSize;
	EsErrID err;
	PResHeader pOrgModelHdrSPI;
	ivUInt16 nMixSModelWSize;
	ivUInt16 nMixSModelNum;
	TCmdDesc tTagCmdDesc;
	
	ivUInt32 nModelCRC;
	ivUInt32 nModelCnt;

	ivUInt32 nNewLexSize;

    ivUInt32 nSize;
    ivPCInt16 pData;
	ivUInt32 nTagModelSize;
    ivPointer pSPIBuffer;

	if(ivNull == pTagObj || ivNull == pOrgRes  || ivNull == pNewRes || ivNull == pGrmIDLst || ivNull == pnNewResSize){
		ivAssert(ivFalse);
		return EsErr_InvArg;
	}

	if(nTagThresh > 100){
		return ivESR_INVCAL;
	}

	/* Memory align */
	pTagObj = (ivPointer)(((ivUInt32)pTagObj+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));
	if(DW_OBJ_CHECK != *(ivPUInt32)pTagObj){
		return EsErr_InvCal;
	}

	pEngine = (PEsrEngine)((ivPUInt32)pTagObj + 1);
	pTagCmdSPI = pEngine->pTagCmdBuf;	 /* 存储TagCmd的SPIBuf地址 */
    /* SPI Buf */
	if(ivNull == pTagCmdSPI) {
		ivAssert(ivFalse);
		return EsErr_InvCal;
	}

	if(DW_TAGCMD_CHECK != pTagCmdSPI->dwCheck) {
		ivAssert(ivFalse);
		return EsErr_InvCal;
	}

	nTagStateNum = pTagCmdSPI->nState;

	/* 复用pTagObj对象内存 */	
	pWorkSpace = (ivPointer)((ivAddress)pEngine + sizeof(TEsrEngine));

	/* 将OrgRes的资源头从SPI中拷贝到内存中 */
	pOrgGrmMdlHdr = (PGrmMdlHdr)pWorkSpace;
	nGrmMdlHdrSize = sizeof(TGrmMdlHdr);

	ivMemCopy(pOrgGrmMdlHdr, pOrgRes, nGrmMdlHdrSize); /* 从SPI Copy */

	if(DW_MINI_RES_CHECK != pOrgGrmMdlHdr->dwCheck) {
		return EsErr_InvRes;
	}
	if(pOrgGrmMdlHdr->nTotalGrm > 1) {
		/* 从SPI Copy */
        ivMemCopy((ivPointer)(pOrgGrmMdlHdr + 1), (ivPointer)((ivAddress)pOrgRes + nGrmMdlHdrSize), sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1)); /* 接着上次的SPI位置拷贝 */
        nGrmMdlHdrSize += sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1);
	}
	pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + nGrmMdlHdrSize);
	
	pNewModelHdr = (PResHeader)pWorkSpace;
	pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + sizeof(TResHeader));
	nWorkSpaceSize = (ivAddress)pEngine->pBufferEnd - (ivAddress)pWorkSpace;	

	/* 对SPI中的OrgRes进行CRC */
	nCRC = 0;
	nCounter = 0;
    ivMakeCRC((ivPCInt16)((ivAddress)pOrgRes+nGrmMdlHdrSize), pOrgGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
    if(nCRC != pOrgGrmMdlHdr->nCRC) {
		return (ivESRStatus)EsErr_InvRes;
	}

	/* 内存操作.看是否有需要添加Tag的 */
	bValidGrmID = ivFalse;
	pOrgGrmDesc = pOrgGrmMdlHdr->tGrmDesc;
	for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) {
		j = 0;
		while(j<nGrmIDNum) {
			if(pOrgGrmDesc[i].nGrmID == pGrmIDLst[j]) {
				/* Need to add tag cmd */
				bValidGrmID = ivTrue;
				break;
			} /* if */
			j++;
		} /* while(j<nGrmIDNum) */
		if(bValidGrmID) {
			break;	
		}
	} /* for (i = 0;...);*/
	
	/* 没有需要添加Tag的语法网络,则直接拷贝退出 */
	if(!bValidGrmID) {
		*pnNewResSize = 0;
		return EsErr_InvCal;
	}

	tTagCmdDesc.nID = pEngine->nTagID;
	tTagCmdDesc.nCMThresh = nTagThresh;
	tTagCmdDesc.bTag = 1;

	/* 读SPI中的模型信息，用于添加tag时的iSModelOffset计算 */
	pOrgModelHdrSPI = (PResHeader)((ivAddress)pOrgRes + pOrgGrmMdlHdr->nModelOffset * sizeof(ivInt16));
	ivMemCopy(pNewModelHdr, pOrgModelHdrSPI, sizeof(TResHeader));

	nMixSModelNum = (ivUInt16)(pNewModelHdr->nMixSModelNum); /* 用于后面tag加入时pLexNode->iSModeOffset */
	nMixSModelWSize = (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel)); /* word */
	
	nCRC =0; 
	nCounter = 0;
	nOffset = nGrmMdlHdrSize;
	/* pNewRes留出GrmMdlHdr头信息,由于有nCRC需要全部处理完才得到. 最后回头填写 */
	pNewLexSPI = (ivPointer)((ivAddress)pNewRes + nOffset);
	for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) {
		ivPointer pOrgLex;
		bValidGrmID = ivFalse;
		j = 0;
		while(j < nGrmIDNum) {
			if(pOrgGrmDesc->nGrmID == pGrmIDLst[j]) {
				/* Need to add tag cmd */
				bValidGrmID = ivTrue;
				break;
			}
			j++;
		} /* while(j < nGrmIDNum) */

		pOrgLex = (ivPointer)((ivAddress)pOrgRes + pOrgGrmDesc->nGrmOffset * sizeof(ivInt16));
		nGrmSize = pOrgGrmDesc->nGrmSize * sizeof(ivInt16);
		pOrgGrmDesc->nGrmOffset = nOffset / sizeof(ivInt16);

		if(nWorkSpaceSize < nGrmSize) {
			ivAssert(ivFalse);
			return EsErr_OutOfMemory;
		}

		/* 将SPI中的语法网络拷贝到内存中进行添加tag的操作 */
		ivMemCopy(pWorkSpace, pOrgLex, nGrmSize);

		if(bValidGrmID) {
			/* 向网络中添加tag */
			nNewLexSize = nGrmSize + nTagStateNum * sizeof(TLexNode) + sizeof(TCmdDesc);
			if(nWorkSpaceSize < nNewLexSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}

			if(nOffset + nNewLexSize > *pnNewResSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}

			err = EsAddTagToLex(pWorkSpace, nTagStateNum, &tTagCmdDesc, (ivUInt16)(nMixSModelNum - 2 - pNewModelHdr->nOldFillerCnt)); /* -2:sil,filler */
			if(ivESR_OK != err) {
				ivAssert(ivFalse);
				return err;
			}

			pOrgGrmDesc->nGrmSize = nNewLexSize / sizeof(ivInt16);

			nGrmSize = nNewLexSize;			
		} /* if(bValidGrmID) */
		else {
			if(nOffset + nGrmSize > *pnNewResSize) {
				ivAssert(ivFalse);
				return EsErr_OutOfMemory;
			}			
		} /* else */

		ivMakeCRC(pWorkSpace, nGrmSize, &nCRC, &nCounter);		
		ivMemCopy(pNewLexSPI, pWorkSpace, nGrmSize); /* nGrmSize单位:word */

		nOffset += nGrmSize;

		pNewLexSPI = (ivPointer)((ivAddress)pNewRes + nOffset);

		pOrgGrmDesc++;
	} /* for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) */

	pOrgGrmMdlHdr->nModelOffset = nOffset / sizeof(ivInt16);
	pNewModelHdr->nMixSModelNum += nTagStateNum;

    /* 新的tag模型放在sil和filler模型前面,因而要修改sil和filler模型的偏移值 */
    nTagModelSize = (nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel))) / sizeof(ivInt16);
    pNewModelHdr->nSilSModelOffset += nTagModelSize;
    pNewModelHdr->nFillerSModelOffset += nTagModelSize;
    if(pNewModelHdr->nOldFillerCnt > 0) {
        pNewModelHdr->nOldFillerSModelOffset += nTagModelSize;
    }

    /* ------------------------重新计算模型CRC.Begin-------------------------- */ 
	nModelCRC = 0;
	nModelCnt = 0;

    pData = (ivPCInt16)((ivUInt32)pNewModelHdr + sizeof(ivUInt32)*2);
    nSize = sizeof(TResHeader) - sizeof(ivUInt32)*2;
	ivMakeCRC(pData, nSize, &nModelCRC, &nModelCnt);

    pData = (ivPCInt16)((ivUInt32)pOrgModelHdrSPI+sizeof(TResHeader));
    nSize = (pNewModelHdr->nMixSModelNum - nTagStateNum - 2 - pNewModelHdr->nOldFillerCnt) * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
	ivMakeCRC(pData, nSize, &nModelCRC, &nModelCnt);
	
    pData = (ivPCInt16)pTagCmdSPI->pMixSModel;
    nSize = nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    ivMakeCRC(pData, nSize, &nModelCRC, &nModelCnt);	

    ivAssert(pNewModelHdr->nSilSModelOffset < pNewModelHdr->nFillerSModelOffset);
    pData = (ivPCInt16)((ivUInt32)pOrgModelHdrSPI + ((PResHeader)pOrgModelHdrSPI)->nSilSModelOffset * sizeof(ivInt16));
    nSize = (pNewModelHdr->nSilMixture + pNewModelHdr->nFillerMixture + pNewModelHdr->nOldFillerCnt * pNewModelHdr->nOldFillerMix) * sizeof(TGModel);
    ivMakeCRC(pData, nSize, &nModelCRC, &nModelCnt);	

    /* Model Info */
    nSize = sizeof(TGrmDesc)*(pOrgGrmMdlHdr->nTotalGrm-1) + sizeof(TGrmMdlHdr);
    nSize += (nModelCnt + nCounter)*sizeof(ivInt16) + sizeof(TResHeader);
    if( nSize> *pnNewResSize) {
        ivAssert(ivFalse);
        *pnNewResSize = nSize;
        return EsErr_OutOfMemory;
    }

    pNewModelHdr->nCRC = nModelCRC;
	pNewModelHdr->nCRCCnt = nModelCnt;
    /* ------------------------重新计算模型CRC.End-------------------------- */

    /* ------------------------将调整后的模型写入SPI中.Begin-------------------------- */ 
    pData = (ivPCInt16)pNewModelHdr;
    nSize = sizeof(TResHeader);
	ivMakeCRC(pData, nSize, &nCRC, &nCounter);
	ivMemCopy(pNewLexSPI, pData, nSize);
	nOffset += nSize;
	pNewLexSPI = (ivPointer)((ivUInt32)pNewRes + nOffset);

    pData = (ivPCInt16)((ivUInt32)pOrgModelHdrSPI+sizeof(TResHeader));
    nSize = (pNewModelHdr->nMixSModelNum - nTagStateNum - 2 - pNewModelHdr->nOldFillerCnt) * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    ivMakeCRC(pData, nSize, &nCRC, &nCounter);
    ivMemCopy(pNewLexSPI, pData, nSize);
    nOffset += nSize;
    pNewLexSPI = (ivPointer)((ivUInt32)pNewRes + nOffset);

    pData = (ivPCInt16)pTagCmdSPI->pMixSModel;
    nSize = nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture - 1) * sizeof(TGModel));
    ivMakeCRC(pData, nSize, &nCRC, &nCounter);
    ivMemCopy(pNewLexSPI, pData, nSize);
    nOffset += nSize;
    pNewLexSPI = (ivPointer)((ivUInt32)pNewRes + nOffset);

    ivAssert(pNewModelHdr->nSilSModelOffset < pNewModelHdr->nFillerSModelOffset);
    pData = (ivPCInt16)((ivUInt32)pOrgModelHdrSPI + ((PResHeader)pOrgModelHdrSPI)->nSilSModelOffset * sizeof(ivInt16));
    nSize = (pNewModelHdr->nSilMixture + pNewModelHdr->nFillerMixture + pNewModelHdr->nOldFillerCnt * pNewModelHdr->nOldFillerMix) * sizeof(TGModel);
    ivMakeCRC(pData, nSize, &nCRC, &nCounter);
    ivMemCopy(pNewLexSPI, pData, nSize);
    nOffset += nSize;
    pNewLexSPI = (ivPointer)((ivUInt32)pNewRes + nOffset);

	pOrgGrmMdlHdr->nCRC = nCRC;
	pOrgGrmMdlHdr->nCRCCnt = nCounter;
	pOrgGrmMdlHdr->nModelSize += nTagStateNum * (sizeof(TMixStateModel) + (pNewModelHdr->nMixture-1) * sizeof(TGModel));

	ivMemCopy(pNewRes, pOrgGrmMdlHdr, nGrmMdlHdrSize);

	*pnNewResSize = nGrmMdlHdrSize + nCounter*sizeof(ivInt16);

#ifdef TAG_LOG
	ParserGrm(pOrgRes, "ParserOrgLex");
	ParserGrm(pNewRes, "ParserTagLex");
#endif

	return EsErrID_OK;
}

/* 函数编号 */
EsErrID ivCall ESRDelTagFromGrm(
								ivPointer      pTagObj,            /* Tag Object */
								ivSize ivPtr   pnTagObjSize,       /* [In/Out] Size of Tag object */
								ivCPointer     pOrgRes,            /* [In] Original Resource */
								ivPointer      pNewRes,            /* [In/Out] Original Resource */
								ivPUInt32      pnNewResSize,       /* [In/Out]Size of pNewRes */ 
								ivUInt16       nDelGrmID,          /* [In] Grammar ID to Delete VoiceTag */
								ivUInt16       nDelTagID           /* [In] VoiceTag ID to Delete */
								)
{
	PGrmMdlHdr pOrgGrmMdlHdr;
	ivSize nGrmMdlHdrSize;
	ivPointer pNewLexSPI;
	PGrmDesc pOrgGrmDesc;
	ivUInt16 i;

	ivUInt32 nCRC;
	ivUInt32 nCounter;

    ivPointer pWorkSpace;
	ivSize nWorkSpaceSize;
	ivUInt16 nGrmSize;

	ivUInt16 nTagStateNum = 0;

	ivUInt32 nOffset;
	EsErrID err;
	PResHeader pModelHdrSPI;

	if(ivNull == pTagObj || ivNull == pnTagObjSize || ivNull == pOrgRes || ivNull == pNewRes || ivNull == pnNewResSize){
		ivAssert(ivFalse);
		return ivESR_INVARG;
	}

	/* Memory align */
	pWorkSpace = (ivPointer)(((ivUInt32)pWorkSpace+IV_PTR_GRID-1)&(~(IV_PTR_GRID-1)));
	nWorkSpaceSize = *pnTagObjSize;

	/* -----------将OrgRes的资源头从SPI中拷贝到内存中.Begin------------------- */
	pOrgGrmMdlHdr = (PGrmMdlHdr)pWorkSpace;	
	ivMemCopy(pOrgGrmMdlHdr, pOrgRes, sizeof(TGrmMdlHdr)); /* 从SPI Copy */
	if(DW_MINI_RES_CHECK != pOrgGrmMdlHdr->dwCheck){
		return ivESR_INVRESOURCE;
	}

    nGrmMdlHdrSize = sizeof(TGrmMdlHdr) + (pOrgGrmMdlHdr->nTotalGrm-1)*sizeof(TGrmDesc);

    if(nGrmMdlHdrSize > nWorkSpaceSize){
        *pnTagObjSize = nGrmMdlHdrSize;
        return ivESR_OUTOFMEMORY;
    }

	pWorkSpace = (ivPointer)((ivAddress)pWorkSpace + nGrmMdlHdrSize);
	nWorkSpaceSize -= nGrmMdlHdrSize;
	/* -----------将OrgRes的资源头从SPI中拷贝到内存中.End------------------- */

	/* ----------------------对SPI中的OrgRes进行CRC------------------------- */
	nCRC = 0;
	nCounter = 0;
	ivMakeCRC((ivPCInt16)((ivAddress)pOrgRes+nGrmMdlHdrSize), pOrgGrmMdlHdr->nCRCCnt * sizeof(ivInt16), &nCRC, &nCounter);
	if(nCRC != pOrgGrmMdlHdr->nCRC){
		return ivESR_INVRESOURCE;
	}

	/* 针对每个语法网络加载到内存中进行操作 */
	nCRC =0; 
	nCounter = 0;
	nOffset = nGrmMdlHdrSize;
	/* pNewRes留出GrmMdlHdr头信息,由于有nCRC需要全部处理完才得到. 最后回头填写 */
	pNewLexSPI = (ivPointer)((ivAddress)pNewRes + nOffset);
	pOrgGrmDesc = pOrgGrmMdlHdr->tGrmDesc;
	for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++, pOrgGrmDesc++)
	{
		ivCPointer pOrgLex;
	
		pOrgLex = (ivCPointer)((ivAddress)pOrgRes + pOrgGrmDesc->nGrmOffset * sizeof(ivInt16));
		nGrmSize = (ivUInt16)(pOrgGrmDesc->nGrmSize * sizeof(ivInt16));
		pOrgGrmDesc->nGrmOffset = nOffset / sizeof(ivInt16);

		if(nGrmSize > nWorkSpaceSize){
			ivAssert(ivFalse);
			*pnTagObjSize = nGrmMdlHdrSize + nGrmSize;
			return ivESR_OUTOFMEMORY;
		}

		/* 将SPI中的语法网络拷贝到内存中进行删除tag的操作 */
		ivMemCopy(pWorkSpace, pOrgLex, nGrmSize);
		
		/* ----------删除一条Tag------------ */
		if(pOrgGrmDesc->nGrmID == nDelGrmID){

			/* 可能存在多个nDelTagID的tag.全部删除 */
			while(1){
				err = EsDelTagFromLex(pWorkSpace, nDelTagID, &nTagStateNum);
				if(ivESR_OK != err){
					ivAssert(ivFalse);
					return err;
				}

				if(0 == nTagStateNum){ /* 该网络已经没有要删的Tag了 */
					break;
				}

				nGrmSize -= (nTagStateNum*sizeof(TLexNode) + sizeof(TCmdDesc));				
			}

			if(nOffset + nGrmSize > *pnNewResSize) {
				ivAssert(ivFalse);
				return ivESR_OUTOFMEMORY;
			}
			

			pOrgGrmDesc->nGrmSize = nGrmSize / sizeof(ivInt16);	
		}

		ivMakeCRC(pWorkSpace, nGrmSize, &nCRC, &nCounter);			
		ivMemCopy(pNewLexSPI, pWorkSpace, nGrmSize); /* nGrmSize单位:word */

		nOffset += nGrmSize;
		pNewLexSPI = (ivPointer)((ivAddress)pNewRes + nOffset);
	} /* for(i=0; i<pOrgGrmMdlHdr->nTotalGrm; i++) */

	/* Model Info */
	if(nOffset + pOrgGrmMdlHdr->nModelSize > *pnNewResSize){
		ivAssert(ivFalse);
		return ivESR_OUTOFMEMORY;
	}

	pModelHdrSPI = (PResHeader)((ivAddress)pOrgRes + pOrgGrmMdlHdr->nModelOffset * sizeof(ivInt16));
	ivMakeCRC((ivPCInt16)pModelHdrSPI, pOrgGrmMdlHdr->nModelSize, &nCRC, &nCounter);
	ivMemCopy(pNewLexSPI, (ivCPointer)pModelHdrSPI, pOrgGrmMdlHdr->nModelSize);

	pOrgGrmMdlHdr->nModelOffset = nOffset / sizeof(ivInt16);
	pOrgGrmMdlHdr->nCRC = nCRC;
	pOrgGrmMdlHdr->nCRCCnt = nCounter;	

	ivMemCopy(pNewRes, pOrgGrmMdlHdr, nGrmMdlHdrSize);

	*pnNewResSize = nGrmMdlHdrSize + nCounter * sizeof(ivInt16);

#ifdef TAG_LOG
	ParserGrm(pOrgRes, "ParserOrgLex");
	ParserGrm(pNewRes, "ParserTagLex");
#endif

	return ivESR_OK;
}
#endif /* MINI5_SPI_MODE */

#endif /* #if MINI5_SPI_MODE && MINI5_TAG_SUPPORT */
