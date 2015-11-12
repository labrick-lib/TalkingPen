#include "esr.h"
#include "stat.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define OBJ_RAM_SIZE		(0x2300)
__align(4) int8_t objram[OBJ_RAM_SIZE] __attribute__ ((section("esrvars"), zero_init));		//识别所用Ram

#define OBJ_RAM_SIZE_FOR_VAD		(6*1024)
__align(4) int8_t objramForVAD[OBJ_RAM_SIZE_FOR_VAD] __attribute__ ((section("recvars"), zero_init));		//识别所用Ram

#define RESIDENT_RAM_SIZE	(40)
__align(4) int8_t residentram[RESIDENT_RAM_SIZE];

#define	MAX_FRAME_SIZE		(64)
__align(4) int16_t g_waAdcSwitchBuffer[2][MAX_FRAME_SIZE] __attribute__ ((section("swhvars"), zero_init));
uint8_t g_bCurBuffer = 0;

#define MAX_SPIBUFFERSIZE	(256)
#define MAX_SPIBUFFERCOUNT	(7)
__align(4) int8_t g_waToFlashBuffer[MAX_SPIBUFFERCOUNT][MAX_SPIBUFFERSIZE] __attribute__ ((section("recvars"), zero_init));
int32_t g_dwaEncodeHeader[MAX_SPIBUFFERCOUNT] __attribute__ ((section("recvars"), zero_init));

RECHEADER g_sHeader __attribute__ ((section("recvars"), zero_init));
uint32_t g_waRecHeader[64] __attribute__ ((section("recvars"), zero_init));
uint8_t  g_nRecHeader = 0;

//播音要在上述BUFFER使用之前

extern uint8_t g_bHaveKeyAction;
extern uint8_t g_bHaveEsrAction;
extern uint8_t g_bHaveRecAction;
extern uint8_t g_nState;

#define  SAMPLE_RATE_SET		(16000)  /* 8000 */
#define  VAD_CHKBACK_FRMNUM		(70)

//--------------------------------------
uint16_t g_wSpiCurPosition = 0;
uint8_t  g_nSpiIndex = 0;
uint16_t g_cntHaveData = 0;
int16_t *g_pWriteBuffer = 0;

struct tagADPCMEncoder tagAdpcmEncoder;
int16_t g_wRecCodeIndex = -1;

uint32_t g_dwRecWriteAddr = 0;
uint32_t g_dwRecWriteHeaderAddr = 0;
uint32_t g_dwRecCodeAddr = 0;
uint32_t g_dwRecBackAddr = 0;

uint32_t g_dwRecDataStartAddr = 0;
uint32_t g_dwRecIndexAddr = 0;

uint32_t g_dwRecIndexIdleStartAddr = 0;
uint32_t g_dwRecIndexIdleEndAddr = 0;

uint8_t	 g_bNoEsr = 0;
uint8_t	 g_bEsrPure = 1;
uint8_t	 g_bHadRecord = 0;
uint8_t	 g_bFirstAreaChecked = 0;

ivPointer pEsrObj;
void ClrESRMemory(void)
{
	memset(residentram,0,sizeof(residentram));
}

/*----------------------------------------------------------------------------------------
函数名: InitEsr
参数:
		None
返回值:
		ivFalse/ivTrue: 初始化识别是否成功
描述:
		创建识别模块，开启ADC和PDMA
----------------------------------------------------------------------------------------*/
static uint8_t InitEsr(int8_t* _objram, ivSize obj_ram_size, ivCPointer _g_pu8ResData)
{
	EsErrID err;
	ivSize	nESRObjSize;
	ivPointer pResidentRAM;
	ivUInt16 nResidentRAMSize;

	LOG(("INIT ESR...\r\n"));

	pEsrObj = _objram;
	nESRObjSize = obj_ram_size;
	pResidentRAM = residentram;
	nResidentRAMSize = RESIDENT_RAM_SIZE;

	memset(_objram,0,obj_ram_size);
	memset(g_waAdcSwitchBuffer,0,sizeof(g_waAdcSwitchBuffer));
//	err = ESRCreate(pEsrObj, &nESRObjSize, pResidentRAM, &nResidentRAMSize, (ivCPointer)g_pu8ResData, 0);
	err = ESRCreate(pEsrObj, &nESRObjSize, pResidentRAM, &nResidentRAMSize, (ivCPointer)_g_pu8ResData, 0,SAMPLE_RATE_SET, VAD_CHKBACK_FRMNUM);
    if(ivESR_OK != err){
        LOG(("ESRCreate failed! err=%d\r\n", (int16_t)err));
        return ivFalse;
    }
	err = ESRSetParam(pEsrObj, ES_PARAM_SPEECH_FRM_MAX, 400);
	if(err != ivESR_OK)
	{
		LOG(("ESR set ES_PARAM_SPEECH_FRM_MAX Err\n"));
		return ivFalse;
	}
    err = ESRSetParam(pEsrObj, ES_PARAM_SPEECH_FRM_MIN, 20);
//	err = ESRSetParam(pEsrObj, ES_PARAM_AMBIENTNOISE, 5000);
	if(err != ivESR_OK)
	{
		LOG(("ESR set ES_PARAM_SPEECH_FRM_MIN Err\n"));
		return ivFalse;
	}
	ADC_Init();
	PdmaForAdc();
	LOG(("INIT ESR OK!\r\n"));

	return ivTrue; 
}
/*----------------------------------------------------------------------------------------
函数名: IsContinueEsr
参数:
		None
返回值:
		ivFalse/ivTrue: 继续执行/退出识别
描述:
		判断有没有退出识别的条件
----------------------------------------------------------------------------------------*/
static uint8_t IsContinueEsr(void)
{
	KEYMSG msg;

	if(KEY_MsgGet(&msg)){
		g_bHaveKeyAction = 1;
		if(msg.Key_MsgValue == KEY_ON_OFF){
			if(msg.Key_MsgType == KEY_TYPE_SP){
				g_nState = PLAY_STATE;
				return ivFalse;
			}else if(msg.Key_MsgType == KEY_TYPE_LP){
				g_nState = PWRDOWN_STATE;
				return ivFalse;
			}
		}else if(msg.Key_MsgValue == KEY_RECORD){
			if(msg.Key_MsgType == KEY_TYPE_DOWN){
				g_nState = RECORD_STATE;
				return ivFalse;
			}
		}
	}
	if(GetSysTicks() >= ESR_DENY_TIME)
	{
		g_bHaveEsrAction = 1;
		g_nState = PLAY_STATE;
		return ivFalse;
	}
	return ivTrue;
}
/*----------------------------------------------------------------------------------------
函数名: GetEsrCode
参数:
		None
返回值:
		(nResultID+ESR_CODE_START)/ivTrue: 识别的结果/识别到环境噪音或者拒识
描述:
		获得识别的结果
----------------------------------------------------------------------------------------*/
int32_t GetEsrCode(ESR_NET EsrNet)
{
	ivESRStatus nStatus = 0;
	PCEsrResult pResult = ivNull;
	ivUInt32 dwMsg = 0;
	EsErrID err;

	LOG(("ESR STARTING...\r\n"));
	if(EsrNet == ESR_MAJOR_NET){
		if(!InitEsr(objram, OBJ_RAM_SIZE, g_pu8ResData)){
			LOG(("Esr Init Failed!\r\n"));
			return ivFalse;
		}
	}else if(EsrNet == ESR_MINOR_NET){
		if(!InitEsr(objramForVAD, OBJ_RAM_SIZE_FOR_VAD, g_pu8ResDataForVAD)){
			LOG(("Esr Init Failed!\r\n"));
			return ivFalse;
		}
	}
    while(1) {
		if(!IsContinueEsr()){
			LOG(("err\n"));
			ADC_Term();
			return ivFalse ;	
		}
		err = ESRRunStep(pEsrObj, dwMsg, &nStatus, &pResult);
		dwMsg = 0;
		if((ivESR_OK != err) && (EsErr_BufferEmpty != err)){
			LOG(("ESRRunStep failed! err=%d\r\n", (int16_t)err));
			break;
		}
        if (ES_STATUS_FINDSTART == nStatus) {
            LOG(("vad\r\n"));
			dwMsg = ES_MSG_RESERTSEARCHR;
        }
        if (ES_STATUS_RESULT == nStatus) {
			LOG(("RESULT OK\r\n"));
            if(ES_TYPE_RESULT == pResult->nResultType) {
				g_bHaveEsrAction = 1;
                LOG(("----------------nResultID=%d, fConfidence=%d\r\n",pResult->nResultID, pResult->nConfidenceScore));
				g_nState = PLAY_STATE;
                break;
            }
            else if ( ES_TYPE_FORCERESULT == pResult->nResultType) {
				g_bHaveEsrAction = 1;
                LOG(("----------------nResultID=%d, fConfidence=%d.语音超时\r\n",pResult->nResultID, pResult->nConfidenceScore));
                break;
            }
            else if (ES_TYPE_AMBIENTNOISE == pResult->nResultType) {
                LOG(("----------------环境噪音\r\n"));
                break;
            }
            else if (ES_TYPE_RESULTNONE == pResult->nResultType) {
                LOG(("----------------拒识\r\n"));
                break;
            }
        }
    }
//	DrvPDMA_Close();
	LOG(("EXIT!!\r\n"));
	ADC_Term();
	if (ES_STATUS_RESULT == nStatus) {
	    if(ES_TYPE_RESULT == pResult->nResultType){
	        return (pResult->nResultID + ESR_CODE_START);		//+ESR_CODE_START区分是否是正常退出
	    }
	}
	return ivTrue;
}
///*----------------------------------------------------------------------------------------
//函数名: ErarseRecord
//参数:
//		dwRecCode: 录音所对应的码值
//返回值:
//		1/0： 准备成功/准备失败
//描述:
//		读取录音索引、确定录音地址、找到录音码值索引、初始化该索引、播放提示音、初始化VAD
//----------------------------------------------------------------------------------------*/
static void ErarseRecord(int16_t wCodeIndex)
{
	uint32_t dwRecFirstClusterIndex = 1;
	uint32_t dwRecClusterIndex = 0;
	uint32_t dwRecClusterJumpAddr = 0;
	uint32_t dwRecClusterJumpEndAddr = 0;
	uint32_t dwRecCodeIndexAddr = g_dwRecCodeAddr + wCodeIndex*8;
	
	LOG(("ERASING...\r\n"));
	
	LOG(("dwReadAudioIndexAddr:%x, dwRecCode:%d\r\n",dwRecCodeIndexAddr,wCodeIndex));
	FMC_ReadBuffer(dwRecCodeIndexAddr+4, &dwRecFirstClusterIndex, sizeof(dwRecFirstClusterIndex));
	LOG(("dwRecFirstClusterIndex:%d\r\n",dwRecFirstClusterIndex));
//	FMD_Write2Byte(&wNoRecFlag, (g_dwRecStartAddr + (dwRecCode-REC_CODE_START)*2));		//删CODE结构---------------这里不正确后面修改
	dwRecClusterJumpAddr = g_dwRecDataStartAddr + dwRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE + sizeof(RECHEADER);
	dwRecClusterJumpEndAddr = g_dwRecDataStartAddr + dwRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE + FLASH_PAGE_SIZE;
	
	dwRecClusterIndex = dwRecFirstClusterIndex;
	
	do{
		LOG(("g_dwRecIndexIdleEndAddr:%x,wRecClusterIndex:%x\r\n",g_dwRecIndexIdleEndAddr,dwRecClusterIndex));
		
		FMC_WriteBuffer(g_dwRecIndexIdleEndAddr, &dwRecClusterIndex, sizeof(dwRecClusterIndex));
		g_dwRecIndexIdleEndAddr += 4;
		
		if(((g_dwRecIndexIdleEndAddr - g_dwRecIndexAddr) % FMC_SECTOR_SIZE) == 0){
			if(g_dwRecIndexIdleEndAddr >= (g_dwRecIndexAddr + REC_INDEX_SIZE))
				g_dwRecIndexIdleEndAddr = g_dwRecIndexAddr;
			FMC_Erase(g_dwRecIndexIdleEndAddr);
		}
		FMD_ReadData(&dwRecClusterIndex, dwRecClusterJumpAddr, sizeof(dwRecClusterIndex));
		dwRecClusterJumpAddr += 4;
		if(dwRecClusterJumpAddr == dwRecClusterJumpEndAddr){			//一页用完，暂时不管
//			dwRecClusterJumpAddr = g_dwRecDataStartAddr + wRecClusterIndex[(~bSwitchFlag) & 0xFF]*FLASH_MIN_ERASE_SIZE;
//			dwRecClusterJumpEndAddr = dwRecClusterJumpAddr + FLASH_PAGE_SIZE;
		}
	}while(dwRecClusterIndex != 0xFFFFFFFF);	
	LOG(("ERASE OVER!\r\n"));
}
//*----------------------------------------------------------------------------------------
//函数名: FindIdleCluster
//参数:
//		wcntUsedArea：最多使用过的片区数
//返回值:
//		iFindIdle: 找到空闲片区的位置（索引），没有找到空闲位置，即出现错误返回-1
//描述:
//		根据所给的最大片区使用数，给其在已使用过的片区中寻找空白片区
//----------------------------------------------------------------------------------------*
static int32_t FindIdleCluster(){
	uint32_t dwRecIndexIdleStart = 0;
	uint32_t dwRecClusterAddr = 0;
//	uint16_t dwRecIndexIdleEnd = 0;
//	while(dwRecIndexIdleStart == 0)
	LOG(("ALLOCATE A CLUSTER...\r\n"));
	
	FMC_ReadBuffer(g_dwRecIndexIdleStartAddr,&dwRecIndexIdleStart,sizeof(dwRecIndexIdleStart));
	g_dwRecIndexIdleStartAddr += 4;
	if(g_dwRecIndexIdleStartAddr >= g_dwRecIndexAddr + REC_INDEX_SIZE){
		g_dwRecIndexIdleStartAddr = g_dwRecIndexAddr;
	}
//	FMD_ReadData(&dwRecIndexIdleEnd,g_dwRecIndexIdleEndAddr,2);
	
//	
//	if(dwRecIndexIdleStart <= dwRecIndexIdleEnd)
	LOG(("ALLOCATE A CLUSTER OK! \r\ndwRecIndexIdleStart:%d\t%x\r\n",dwRecIndexIdleStart,g_dwRecIndexIdleStartAddr));
	dwRecClusterAddr = g_dwRecDataStartAddr + dwRecIndexIdleStart * FLASH_MIN_ERASE_SIZE;
	if(dwRecClusterAddr >= SPI_FLASH_SIZE)
	{	
		LOG(("Error:memory is not enough, please change bigger memory"));
		return -1;
	}else{
		return dwRecIndexIdleStart;
	}
}
///*----------------------------------------------------------------------------------------
//函数名: InitIndex
//参数:
//		dwRecCode: 录音所对应的码值
//返回值:
//		1/0： 准备成功/准备失败
//描述:
//		读取录音索引、确定录音地址、找到录音码值索引、初始化该索引、播放提示音、初始化VAD
//----------------------------------------------------------------------------------------*/
void InitIndex()
{	
	uint32_t dwcntRemainCluster = (SPI_FLASH_SIZE - g_dwRecDataStartAddr)/FLASH_MIN_ERASE_SIZE;
	uint16_t iInitIndex;
	uint32_t dwUSEDFlag = 0x44455355;
	uint32_t waInitIndex[64];
	memset(waInitIndex,0xFF,sizeof(waInitIndex));
	
	LOG(("FIRST USED THIS FLASH, INITAL...\r\n"));
	
	FMC_Erase(DATAF_BASE_ADDR);
	FMC_WriteBuffer(DATAF_BASE_ADDR, &dwUSEDFlag, sizeof(dwUSEDFlag));
	
	for(iInitIndex=0; iInitIndex < REC_INDEX_SIZE; iInitIndex+=FMC_SECTOR_SIZE)		//操控1024个BLOCK
		FMC_Erase(g_dwRecIndexAddr + iInitIndex*FMC_SECTOR_SIZE);

	for(iInitIndex = 0; iInitIndex < dwcntRemainCluster; iInitIndex++){
		if((iInitIndex%64 == 0) && (iInitIndex != 0)){
			FMC_WriteBuffer(g_dwRecIndexAddr + (iInitIndex/64 - 1)*64*2,(uint32_t*)waInitIndex,sizeof(waInitIndex));
			memset(waInitIndex,0xFF,sizeof(waInitIndex));
		}
		waInitIndex[iInitIndex%64] = iInitIndex;
	}
	FMC_WriteBuffer(g_dwRecIndexAddr + (iInitIndex/64)*64*2,(uint32_t*)waInitIndex,sizeof(waInitIndex));
	LOG(("INITAL THIS FLASH OK!\r\n"));//dwcntRemainCluster:%d, iInitIndex:%d\r\n",dwcntRemainCluster,iInitIndex);
	g_dwRecIndexIdleStartAddr = g_dwRecIndexAddr;
	g_dwRecIndexIdleEndAddr = g_dwRecIndexAddr + dwcntRemainCluster*4;
}
/////*----------------------------------------------------------------------------------------
////函数名: GetRecCodeAddr
////参数:
////		dwRecCode: 录音所对应的码值
////返回值:
////		1/0： 准备成功/准备失败
////描述:
////		读取录音索引、确定录音地址、找到录音码值索引、初始化该索引、播放提示音、初始化VAD
////----------------------------------------------------------------------------------------*/
//uint32_t GetRecCodeAddr(int32_t dwRecCode)
//{
//	uint32_t dwcntRecCode = 0;
//	uint32_t dwTempAddr = 0;
//	uint32_t dwaTranferBuffer[2] = {0};
//	
//	FMC_ReadBuffer(g_dwRecCodeValidAddr,&dwcntRecCode,sizeof(dwcntRecCode));
//	if(dwcntRecCode == 0xFFFFFFFF){		//初次使用
//		FMC_Erase(g_dwRecCodeValidAddr);
//		dwcntRecCode = 1;
//		FMC_WriteBuffer(g_dwRecCodeValidAddr,&dwcntRecCode,sizeof(dwcntRecCode));
//		memset(dwaTranferBuffer,0xFF,sizeof(dwaTranferBuffer));
//		dwaTranferBuffer[0] = dwRecCode;
//		dwaTranferBuffer[1] = (uint32_t)FindIdleCluster;
//		FMC_WriteBuffer(g_dwRecCodeValidAddr,dwaTranferBuffer,sizeof(dwaTranferBuffer));
//		return (g_dwRecCodeValidAddr+8);
//	}else{
//		if(g_dwRecCodeValidAddr == g_dwRecCodeAddr)
//			dwRecCodeUnvalidAddr = g_dwRecBackAddr;
//		else
//			dwRecCodeUnvalidAddr = g_dwRecCodeAddr;
//		
//		FMC_Erase(dwRecCodeUnvalidAddr);
//		dwcntRecCode++;
//		FMC_WriteBuffer(dwRecCodeUnvalidAddr,&dwcntRecCode,sizeof(dwcntRecCode));
//		for(dwTempAddr=(g_dwRecCodeValidAddr+8); dwTempAddr < g_dwRecCodeValidAddr+(dwcntRecCode+1)*8; dwTempAddr+=8){
//			FMC_ReadBuffer(dwTempAddr,dwaTranferBuffer,sizeof(dwaTranferBuffer));
//			if(dwaTranferBuffer[0] == dwRecCode)
//				continue;
//			FMC_WriteBuffer(dwRecCodeUnvalidAddr,dwaTranferBuffer,sizeof(dwaTranferBuffer));	
//		}
//		dwaTranferBuffer[0] = dwRecCode;
//		dwaTranferBuffer[1] = (uint32_t)FindIdleCluster;
//		FMC_WriteBuffer(g_dwRecCodeValidAddr,dwaTranferBuffer,sizeof(dwaTranferBuffer));
//		return (g_dwRecCodeValidAddr + (dwcntRecCode+1)*8);
//	}
//	
//}
uint8_t CodeIsExist(int32_t dwRecCode, uint16_t* wCodeIndex)		//返回特性（有和无该CODE），传递数值
{
	uint32_t dwTempCode;
	uint16_t iFindCode = 1;
	LOG(("JUDGE %d IS EXIST?\r\n",dwRecCode));
	//第一个8字节留出来
	FMC_ReadBuffer(DATAF_BASE_ADDR+iFindCode*8,&dwTempCode,sizeof(dwTempCode));
	while(dwTempCode != 0xFFFFFFFF){
		if(dwRecCode == dwTempCode){
			*wCodeIndex = iFindCode;
			LOG(("EXIST!\r\n"));
			return iFindCode;
		}
		iFindCode++;
		FMC_ReadBuffer(DATAF_BASE_ADDR+iFindCode*8,&dwTempCode,sizeof(dwTempCode));
	}
	*wCodeIndex = iFindCode;
	LOG(("Not EXIST!\r\n"));
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: PrepareForRecord
参数:
		dwRecCode: 录音所对应的码值
返回值:
		1/0： 准备成功/准备失败
描述:
		读取录音索引、确定录音地址、找到录音码值索引、初始化该索引、播放提示音、初始化VAD
----------------------------------------------------------------------------------------*/
static int8_t PrepareForRecord(int32_t dwRecCode, uint32_t* dwRecIndexIdleStartAddrBack)
{
	uint32_t dwSourceSize = 0;
	uint32_t dwUSEDFlag = 0;
	int32_t  dwIdleCluster = 0;
	uint16_t wCodeIndex = 0;
//	uint32_t waInitIndex[64];
//	uint16_t iInitIndex = 0;
	
	//准备各种全局变量
	FMD_ReadData(&dwSourceSize, RECORD_START, sizeof(dwSourceSize));				//0x10000处为总资源大小
	g_dwRecDataStartAddr = (dwSourceSize/FLASH_MIN_ERASE_SIZE + 1) * FLASH_MIN_ERASE_SIZE;	//索引放入内部FLASH，所以此处为数据块的开始
	
	//FMC操作
	g_dwRecCodeAddr = DATAF_BASE_ADDR;
	g_dwRecBackAddr = g_dwRecCodeAddr + REC_CODE_SIZE;
	g_dwRecIndexAddr = g_dwRecBackAddr + REC_CODE_SIZE;
	
	LOG(("g_dwAddr:%x\r\n",g_dwRecDataStartAddr));
	
//	FMC_ReadBuffer(g_dwRecIndexAddr,waInitIndex,sizeof(waInitIndex));
//	for(iInitIndex = 0; iInitIndex < 64; iInitIndex++)
//		LOG("waInitIndex:%x\r\n",waInitIndex[iInitIndex]);
		
	FMC_ReadBuffer((g_dwRecBackAddr-8),&g_dwRecIndexIdleStartAddr,4);
	FMC_ReadBuffer((g_dwRecBackAddr-4),&g_dwRecIndexIdleEndAddr,4);
	
	LOG(("ALLOCATE CLUSTER ADDR:\r\ng_dwRecIndexIdleStartAddr:%d\tg_dwRecIndexIdleEndAddr:%d\r\n",g_dwRecIndexIdleStartAddr,g_dwRecIndexIdleEndAddr));
	
	FMC_ReadBuffer(g_dwRecCodeAddr,&dwUSEDFlag,4);
	if(dwUSEDFlag != 0x44455355){		//USED的ASCII码
		InitIndex();
	}
//	FMC_ReadBuffer(g_dwRecIndexAddr,waInitIndex,sizeof(waInitIndex));
//	for(iInitIndex = 0; iInitIndex < 64; iInitIndex++)
//		LOG("waInitIndex:%x\r\n",waInitIndex[iInitIndex]);
	
//	if((g_dwRecIndexIdleStartAddr == 0xFFFFFFFF) || (g_dwRecIndexIdleEndAddr == 0xFFFFFFFF)){
//		FMC_ReadBuffer((g_dwRecIndexAddr-8),&g_dwRecIndexIdleStartAddr,4);
//		FMC_ReadBuffer((g_dwRecIndexAddr-4),&g_dwRecIndexIdleEndAddr,4);
//		if((g_dwRecIndexIdleStartAddr == 0xFFFFFFFF) || (g_dwRecIndexIdleEndAddr == 0xFFFFFFFF)){
//			InitIndex();						//地址是系统初始值，初始化索引表
//			g_dwRecCodeValidAddr = g_dwRecCodeAddr;
//			g_dwRecCodeUnvalidAddr = g_dwRecBackAddr;
//		}else{
//			g_dwRecCodeValidAddr = g_dwRecBackAddr;
//			g_dwRecCodeUnvalidAddr = g_dwRecCodeAddr;
//		}
//	}else{
//		g_dwRecCodeValidAddr = g_dwRecCodeAddr;
//		g_dwRecCodeValidAddr = g_dwRecBackAddr;
//	}
//	FMC_Erase(g_dwRecCodeUnvalidAddr);
	if(CodeIsExist(dwRecCode, &wCodeIndex)){
		ErarseRecord(wCodeIndex);			//仅擦除外部FLASH，不变动内部FLASH
	}
	*dwRecIndexIdleStartAddrBack = g_dwRecIndexIdleStartAddr; 
	dwIdleCluster = FindIdleCluster();				//分配第一簇
	if(dwIdleCluster == -1)
		return 0;
	g_dwRecWriteAddr = g_dwRecDataStartAddr + dwIdleCluster*FLASH_MIN_ERASE_SIZE;			//这个索引表最后修改
	g_dwRecWriteHeaderAddr = g_dwRecWriteAddr;	//头也写在这里
	
	PlayBegin(PEN_RECTIP_AUDIO);				//注意这里会毁坏“esrvars”中的变量，特别是g_sHeader
	while(!PlayWork(0,0,0));
	
	g_sHeader.rec_audio_size = 0xFFFFFFFF;		//清除上一次的大小
	g_sHeader.rec_max_value = 0xFFFF;		   	//清除上一次的录音最大值
	memset(g_waRecHeader,0xFFFF,sizeof(g_waRecHeader));
	g_nRecHeader = (uint8_t)sizeof(g_sHeader)/4;
//	g_waRecHeader[g_nRecHeader++] = wIdleCluster;
	
	g_cntHaveData = 0;
	g_wSpiCurPosition = 0;
	g_nSpiIndex = 0;
	g_bFirstAreaChecked = 0;
	ivADPCM_InitCoder(&tagAdpcmEncoder);	

	if(!InitEsr(objramForVAD, OBJ_RAM_SIZE_FOR_VAD, g_pu8ResDataForVAD)){
		LOG(("Esr Init Failed!\r\n"));
		return 0;
	}
	FMD_EraseBlock(g_dwRecWriteAddr);
	g_dwRecWriteAddr += FLASH_PAGE_SIZE;
	return 1;	
}
//*----------------------------------------------------------------------------------------
//函数名: WriteRecHeader
//参数:
//		dwRecCode: 录音所对应的码值
//返回值:
//		i: 其为当前码值在录音索引当中的位置，没有找到码值并且没有额外空间创建返回-1
//描述:
//		根据码值找到其在索引中的位置，没有时就新创建一个位置
//----------------------------------------------------------------------------------------*
void WriteRecHeader(uint8_t bFirstHeader)
{
	int32_t *pHeader = (int32_t *)&g_sHeader;
	uint8_t iTranfer; 

	if(bFirstHeader){
		for(iTranfer=2; iTranfer<sizeof(RECHEADER)/4; iTranfer++){
			g_waRecHeader[iTranfer] = *(pHeader + iTranfer);
		}
	}
	FMD_WriteData(g_waRecHeader,g_dwRecWriteHeaderAddr);
}
//*----------------------------------------------------------------------------------------
//函数名: ModifyCodeFirstIndex
//参数:
//		dwRecCode: 录音所对应的码值
//返回值:
//		i: 其为当前码值在录音索引当中的位置，没有找到码值并且没有额外空间创建返回-1
//描述:
//		根据码值找到其在索引中的位置，没有时就新创建一个位置
//----------------------------------------------------------------------------------------*
void ModifyCodeFirstIndex(int32_t dwRecCode, int32_t dwRecFirstClusterIndex)
{
	int32_t aTranferBuffer[2];
	uint8_t iRead,iWrite = 0;
	uint8_t bHaveCode = 0;
	uint16_t wTranferBufferSize = sizeof(aTranferBuffer);
	uint16_t wCodeIndex = 0;
	
	bHaveCode = CodeIsExist(dwRecCode,&wCodeIndex);
	
	FMC_Erase(g_dwRecBackAddr);
	for(iRead=0; iRead<127; iRead++){		//16?256?4K
		FMC_ReadBuffer((g_dwRecCodeAddr + iRead*wTranferBufferSize), (uint32_t*)aTranferBuffer, wTranferBufferSize);
		if(bHaveCode && iRead == wCodeIndex){
			continue;
		}
		FMC_WriteBuffer(g_dwRecBackAddr+(iWrite++)*wTranferBufferSize, (uint32_t*)aTranferBuffer, wTranferBufferSize);
	}
	aTranferBuffer[0] = g_dwRecIndexIdleStartAddr;		//修改索引的起始和结束地址
	aTranferBuffer[1] = g_dwRecIndexIdleEndAddr;
	FMC_WriteBuffer(g_dwRecBackAddr+127*wTranferBufferSize, (uint32_t*)aTranferBuffer, wTranferBufferSize);
	FMC_Erase(g_dwRecCodeAddr);
	for(iRead=0; iRead<128; iRead++){
		FMC_ReadBuffer((g_dwRecBackAddr+iRead*wTranferBufferSize), (uint32_t*)aTranferBuffer, wTranferBufferSize);
		FMC_WriteBuffer(g_dwRecCodeAddr+iRead*wTranferBufferSize, (uint32_t*)aTranferBuffer, wTranferBufferSize);
	}
	
	if(dwRecFirstClusterIndex != 0xFFFFFFFF){
		if(!CodeIsExist(dwRecCode,&wCodeIndex)){
			LOG(("wCodeIndex:%d\r\n",wCodeIndex));
			aTranferBuffer[0] = dwRecCode;
			aTranferBuffer[1] = dwRecFirstClusterIndex;
			FMC_WriteBuffer(g_dwRecCodeAddr+wCodeIndex*8, (uint32_t*)aTranferBuffer, wTranferBufferSize);
			FMC_ReadBuffer(g_dwRecCodeAddr+wCodeIndex*8, (uint32_t*)aTranferBuffer,4);
			LOG(("aTranferBuffer:%x\r\n",aTranferBuffer[0]));
		}else{
			LOG(("modify first index error!!"));
		}
	}
}
/*----------------------------------------------------------------------------------------
函数名: RecordBegin
参数:
		dwRecCode: 录音所对应的码值
返回值:
		None
描述:
		对所传入的码值进行录音
----------------------------------------------------------------------------------------*/
void RecordBegin(int32_t dwRecCode, REC_MODE RecMode)
{
	ivESRStatus nStatus = 0;
	PCEsrResult pResult = ivNull;
	ivUInt32 dwMsg = 0;
	KEYMSG 	 msg;
	uint16_t  ncntPage = 0;
	uint16_t cntWritedPage = 0;
	int32_t dwIdleCluster = 0, dwRecFirstClusterIndex = 0;
	uint8_t bEraseFlag = 1,bFirstHeader = 1;
	uint32_t dwRecIndexIdleStartAddrBack = 0;
	uint8_t iTranfer, cntWritedBlock = 1;
	int32_t *pHeader = 0;
	g_bEsrPure = 0;

	if((dwRecCode < REC_CODE_START) || (dwRecCode > REC_CODE_END)){		//码值不在录音范围内
		LOG(("Failed code for record!"));
		g_nState = PLAY_STATE;
		return ;
	}
	if(!PrepareForRecord(dwRecCode, &dwRecIndexIdleStartAddrBack)){
		LOG(("Prepare for record failed!"));
		g_nState = PLAY_STATE;
		return ;
	}	
	
	if(RecMode == REC_MAVO)
		g_nState = RECORD_STATE;

    while(g_nState == RECORD_STATE && cntWritedPage < PAGE_NUM){
		g_bHaveRecAction = 1;
		if(dwMsg == 0){
			ESRRunStep(pEsrObj, dwMsg, &nStatus, &pResult);
		}

        if(ES_STATUS_FINDSTART == nStatus) {
			nStatus = 5;														//随便赋值一个不影响结果的值
			LedFlashForEsr(LED_ON);
			if(g_cntHaveData > MAX_SPIBUFFERCOUNT){
				g_wSpiCurPosition = ((g_cntHaveData + 2) % MAX_SPIBUFFERCOUNT);	//正在写第spi_wirte+1个BUFFER可能会操作第g_cntHaveData+2个BUFFER
				g_cntHaveData = MAX_SPIBUFFERCOUNT - 2;							//一个正在使用，一个可能在使用
			}
			g_sHeader.rec_encoder.m_nPrevVal = (g_dwaEncodeHeader[g_wSpiCurPosition]>>16) & 0xFFFF;
			g_sHeader.rec_encoder.m_nIndex 	 =	g_dwaEncodeHeader[g_wSpiCurPosition] & 0xFFFF; 
            dwMsg = ES_MSG_RESERTSEARCHR;
			g_bNoEsr = 1;
			ncntPage = 1;			//第一个4K的第一个页被4K属性占用
			bEraseFlag = 0;
			LedFlashForEsr(LED_ON);
        }
		if(dwMsg == ES_MSG_RESERTSEARCHR){		//VAD检测到开始		
			if (g_cntHaveData > 0)
			{
				FMD_WriteData(g_waToFlashBuffer[g_wSpiCurPosition],g_dwRecWriteAddr);
				g_dwRecWriteAddr += MAX_SPIBUFFERSIZE;
				g_wSpiCurPosition = (g_wSpiCurPosition + 1)	% MAX_SPIBUFFERCOUNT;
				g_cntHaveData--;
				cntWritedPage++;		//总共写的page数
	
				ncntPage++;		//当前block中的page数
				if(ncntPage >= 256){
					g_bHaveRecAction = 1;
					if(g_nRecHeader == 63){
						cntWritedBlock++;		//总共占用多少个block
						dwIdleCluster = FindIdleCluster();
						if(dwIdleCluster == -1)
							break;
						g_waRecHeader[g_nRecHeader] = dwIdleCluster;
						WriteRecHeader(bFirstHeader);				//写入第一个头
						bFirstHeader = 0;
						g_dwRecWriteAddr = g_dwRecDataStartAddr + dwIdleCluster*FLASH_MIN_ERASE_SIZE;
						FMD_EraseBlock(g_dwRecWriteAddr);
						g_dwRecWriteHeaderAddr = g_dwRecWriteAddr;
						g_dwRecWriteAddr += FLASH_PAGE_SIZE;
						memset(g_waRecHeader,0xFF,sizeof(g_waRecHeader));
						g_nRecHeader = 0;
						ncntPage = 1;
						
						LOG(("RecHeader full, allocate new memory!!"));			//后面完成溢出的处理
					}else{
						ncntPage = 0;
						dwIdleCluster = FindIdleCluster();
						if(dwIdleCluster == -1)
							break;
						g_dwRecWriteAddr = g_dwRecDataStartAddr + dwIdleCluster*FLASH_MIN_ERASE_SIZE;
						FMD_EraseBlock(g_dwRecWriteAddr);
						g_waRecHeader[g_nRecHeader++] = dwIdleCluster;
					}
				}
			}		
		}
		if(KEY_MsgGet(&msg)){
			g_bHaveKeyAction = 1;
			if(RecMode == REC_PURE){
				if(msg.Key_MsgValue == KEY_RECORD){
					if(msg.Key_MsgType == KEY_TYPE_UP){
						LOG(("RECORD END!!"));
						g_nState = PLAY_STATE;
					}
				}
			}else if(RecMode == REC_MAVO){
				if(msg.Key_MsgValue == KEY_MACESR){
					if(msg.Key_MsgType == KEY_TYPE_UP){
						LOG(("MACREC END!!"));
						g_nState = PLAY_STATE;
					}
				}
			}
			if(msg.Key_MsgValue == KEY_ON_OFF){
//			 	if(msg.Key_MsgType == KEY_TYPE_SP){	//擦除片区
//					ErarseRecord(dwRecCode);
//					bEraseFlag = 1;
//					g_nState	= PLAY_STATE;
//				}else 
				if(msg.Key_MsgType == KEY_TYPE_LP){
					g_nState = PWRDOWN_STATE;	
				}
			}else if(msg.Key_MsgValue == KEY_TOUCH){
				if(RecMode == REC_MAVO){
					if(msg.Key_MsgType == KEY_TYPE_UP){
						g_nState = PLAY_STATE;
					}
				}
			}
		}
    }
//	DrvPDMA_Close();
	ADC_Term();
	g_bNoEsr = 0;
	g_sHeader.rec_audio_size = cntWritedPage * FLASH_PAGE_SIZE - cntWritedBlock*FLASH_PAGE_SIZE;

	if(bEraseFlag){
		LedFlashForEsr(LED_ON);
		g_dwRecIndexIdleStartAddr = dwRecIndexIdleStartAddrBack;	//如果是擦除，就把分配地址还原到擦除之前的状态；
		ModifyCodeFirstIndex(dwRecCode, (int32_t)0xFFFFFFFF);
	}else{
		WriteRecHeader(bFirstHeader);
		FMC_ReadBuffer(dwRecIndexIdleStartAddrBack, (uint32_t*)&dwRecFirstClusterIndex,4);
		ModifyCodeFirstIndex(dwRecCode,dwRecFirstClusterIndex);
		
		pHeader = (int32_t*)&g_sHeader;
		memset(g_waRecHeader,0xFF,sizeof(g_waRecHeader));
		for(iTranfer=0; iTranfer<2; iTranfer++){
			g_waRecHeader[iTranfer] = *(pHeader + iTranfer);
		}
		FMD_WriteData(g_waRecHeader,g_dwRecDataStartAddr + dwRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE);
	}
//	ModifyIndexPointer();		//擦除时修改索引结束地址
	LedFlashForEsr(LED_OFF);
}
/*----------------------------------------------------------------------------------------
函数名: CBPdmdForAdc
参数:
		None
返回值:
		None
描述:
		ADC的回调函数，开始时给识别数据，后面压缩数据并写到g_pWriteBuffer
----------------------------------------------------------------------------------------*/
void CBPdmdForAdc(void)
{
	EsErrID err;
	int32_t dwTemp = 0;
	uint8_t iFindMax = 0;
	uint8_t bCurBufferBak = g_bCurBuffer;
	static uint8_t	cntDiscardStart = 0;
//	static int16_t Test = 0,TestFlag = 0;
//	int16_t i = 0;
	
	g_bCurBuffer ^= 1;
	PDMA->channel[eDRVPDMA_CHANNEL_0].DAR = (uint32_t)&g_waAdcSwitchBuffer[g_bCurBuffer][0];
	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_0);
	
	if(cntDiscardStart > 0)	cntDiscardStart--; 	//丢弃前15个页120ms，减去魔音开头出现杂波的影响；
	else
	{
		if(!g_bNoEsr){
			err = ESRAppendData(pEsrObj, g_waAdcSwitchBuffer[bCurBufferBak], MAX_FRAME_SIZE);
			if (err != ivESR_OK)
			{
				LOG(("!"));
			}
		}
		else
		for(iFindMax = 0; iFindMax < MAX_FRAME_SIZE; iFindMax++)
		{
//			if(!TestFlag)
//				g_waAdcSwitchBuffer[bCurBufferBak][iFindMax] = Test++;
//			else
//				g_waAdcSwitchBuffer[bCurBufferBak][iFindMax] = Test--;	
//			if(Test == 30000)	TestFlag = 1;
//			if(Test == -30000)	TestFlag = 0;
			dwTemp = g_waAdcSwitchBuffer[bCurBufferBak][iFindMax];
			dwTemp = dwTemp>0 ? dwTemp : -dwTemp;
			if(dwTemp > g_sHeader.rec_max_value)	
				g_sHeader.rec_max_value = dwTemp;
		}
	
		if(!g_bEsrPure){
			ivADPCM_Encode(&tagAdpcmEncoder, g_waAdcSwitchBuffer[bCurBufferBak], MAX_FRAME_SIZE, (uint8_t *)g_pWriteBuffer);
			g_pWriteBuffer += MAX_FRAME_SIZE / 4;
			g_nSpiIndex++;
			if(g_nSpiIndex == (MAX_SPIBUFFERSIZE/2) / (MAX_FRAME_SIZE/4)){
				g_nSpiIndex = 0;
				g_cntHaveData++;
				g_dwaEncodeHeader[(g_wSpiCurPosition + g_cntHaveData) % MAX_SPIBUFFERCOUNT] = (int32_t)tagAdpcmEncoder.m_nPrevVal << 16;
				g_dwaEncodeHeader[(g_wSpiCurPosition + g_cntHaveData) % MAX_SPIBUFFERCOUNT] += tagAdpcmEncoder.m_nIndex;
				g_pWriteBuffer = (int16_t *)g_waToFlashBuffer[(g_wSpiCurPosition + g_cntHaveData) % MAX_SPIBUFFERCOUNT];
	//			LOG("buffer:%d,%d,%d\r\n",(g_wSpiCurPosition + g_cntHaveData) % MAX_SPIBUFFERCOUNT,g_wSpiCurPosition,g_cntHaveData);
			}
		}
	}
}
/*----------------------------------------------------------------------------------------
函数名: CBPdmdForAdc
参数:
		None
返回值:
		None
描述:
		ADC的回调函数，开始时给识别数据，后面压缩数据并写到g_pWriteBuffer
----------------------------------------------------------------------------------------*/
void PdmaForAdc(void)
{
	STR_PDMA_T sPDMA;

	DrvPDMA_Init();
	g_bCurBuffer = 0;
	g_pWriteBuffer = (int16_t *)g_waToFlashBuffer[(g_wSpiCurPosition + g_cntHaveData) % MAX_SPIBUFFERCOUNT];
	
	// CH2 ADC RX Setting 
	sPDMA.sSrcAddr.u32Addr          = (uint32_t)&SDADC->ADCOUT; 
	sPDMA.sDestAddr.u32Addr         = (uint32_t)&g_waAdcSwitchBuffer[g_bCurBuffer][0];
	sPDMA.u8Mode                    = eDRVPDMA_MODE_APB2MEM;
	sPDMA.u8TransWidth              = eDRVPDMA_WIDTH_16BITS;
	sPDMA.sSrcAddr.eAddrDirection   = eDRVPDMA_DIRECTION_FIXED; 
	sPDMA.sDestAddr.eAddrDirection  = eDRVPDMA_DIRECTION_INCREMENTED;   
	sPDMA.i32ByteCnt				= MAX_FRAME_SIZE * sizeof(int16_t);
	DrvPDMA_Open(eDRVPDMA_CHANNEL_0, &sPDMA);
	
	// PDMA Setting 
	DrvPDMA_SetCHForAPBDevice(
	    eDRVPDMA_CHANNEL_0, 
	    eDRVPDMA_ADC,
	    eDRVPDMA_READ_APB    
	);
	
	// Enable INT 
	DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD );
	    
	// Install Callback function    
	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD, (PFN_DRVPDMA_CALLBACK)CBPdmdForAdc ); 
	
	// Enable ADC PDMA and Trigger PDMA specified Channel 
	DrvADC_PdmaEnable();
	
	// Start A/D conversion 
	DrvADC_StartConvert();
	
	// start ADC PDMA transfer
	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_0);
}
