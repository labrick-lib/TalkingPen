//-------------------和播音有关的程序
#include "play.h"
#include "lib/LibSiren7.h"
#include "esr.h"
#include "led.h"
#include "ivMV.h"
#include "stat.h"

__align(4) int16_t g_waPcmBuffer[PCMBUFFER_SIZE] __attribute__ ((section("playvars"), zero_init));
__align(4) int16_t g_waEncBuffer[ENCBUFFER_SIZE] __attribute__ ((section("playvars"), zero_init));
__align(4) int16_t g_waCycBuffer[CYCLEBUFFER_SIZE] __attribute__ ((section("playvars"), zero_init));

sSiren7_CODEC_CTL sEnDeCtl __attribute__ ((section("playvars"), zero_init));
sSiren7_DEC_CTX sS7Dec_Ctx __attribute__ ((section("playvars"), zero_init));

uint32_t g_dwReadPcmAddr = 0,g_dwLastWritedPosition = 0,g_dwLinkAudioAddr=0;
int32_t g_dwLinkNum=0;

#define	Normalized_Cof					(16384) 
#define VOLUME_MAX_LEVEL				(63)
#define VOLUME_MIN_LEVEL				(35)
#define VOLUME_LEVEL_VALUE				(6)
static const unsigned short g_waVolumeCof[VOLUME_MAX_LEVEL+1] = {23,26,29,32,36,41,46,51,58,65,73,82,92,103,116,
130,146,164,184,206,231,260,292,327,367,412,462,519,582,653,733,823,923,1036,
1162,1304,1463,1642,1842,2067,2319,2602,2920,3276,3676,4125,4628,5193,5827,6538,
7335,8230,9235,10362,11626,13045,14636,16422,18426,20675,23197,26028,29204,32768};

const static int16_t ROLE_COF_Q7[7] = {256, 230, 192, 128, 92, 85, 70};
#define 	Q7		128

uint8_t g_nVolumeIndex = 57;
uint8_t g_bStopPlay = 0;
uint8_t g_bNoInterruptedAudio = 0;

//----------------------------

__align(4) int16_t	g_waMVPcmBuffer[MVPCM_BUFFER_SIZE] __attribute__ ((section("magicvars"), zero_init));
__align(4) int8_t	g_naMVWorkSpace[MVWORK_SPACE_SIZE] __attribute__ ((section("magicvars"), zero_init));
__align(4) int16_t 	g_waMVAppendBuf[MVAPPEND_BUFFER_SIZE] __attribute__ ((section("magicvars"), zero_init));
__align(4) int16_t	g_waAdpcmBuffer[ADPCM_BUFFER_SIZE] __attribute__ ((section("magicvars"), zero_init));

uint32_t g_dwHaveWritedPosition;

uint8_t g_nRole = 4;

short g_wRecordMaxValue, g_wAmpCof;
extern uint8_t g_nState;
extern uint8_t g_bHaveKeyAction;
extern uint8_t g_bHavePlayAction;
extern uint8_t g_bHaveCamAction;

//extern uint32_t g_cntStartTime;
ivTADPCMDecoder Decoder;

#define PLAY_LIST_MAX_CNT		(10)
uint16_t g_dwaPlayList[PLAY_LIST_MAX_CNT] = {0};
uint8_t g_cntPlayAudio = 0;
uint8_t g_nCurPlayAudio = 0;
/*----------------------------------------------------------------------------------------
函数名: SysTimerDelay
参数:
		us: 延时的时间，以us为单位
返回值:
		None
描述:
		采用系统定时器来完成延时功能
----------------------------------------------------------------------------------------*/
void SysTimerDelay(uint32_t us)
{
    SysTick->LOAD = us * 48; 
    SysTick->VAL   =  (0x00);
    SysTick->CTRL = (1 << SYSTICK_CLKSOURCE) | (1<<SYSTICK_ENABLE);

    /* Waiting for down-count to zero */
    while((SysTick->CTRL & (1 << 16)) == 0);
}
/*----------------------------------------------------------------------------------------
函数名: PlayVolumeCtrl
参数:
		Buffer: 所需调整音量的数据的起始地址
		Size：	所需调整音量的数据的字数
返回值:
		None
描述:
		把buffer为开始地址，大小为Size字大小空间的音频音量调整为g_nVolumeIndex所规定值
----------------------------------------------------------------------------------------*/
static void PlayVolumeCtrl(int16_t *Buffer, uint32_t Size)
{
	int32_t i;
	int32_t dwPcmTemp;
//	LOG("%d\r\n",Size>>1);
	for(i=0; i<(Size>>1); i++)
	{
		dwPcmTemp = *Buffer;
//		LOG("%x::%x\r\n",Buffer,*Buffer);
		dwPcmTemp = (dwPcmTemp*g_waVolumeCof[g_nVolumeIndex])/Normalized_Cof;
		dwPcmTemp = dwPcmTemp>Q16_MAX ? Q16_MAX:dwPcmTemp;
		dwPcmTemp = dwPcmTemp<Q16_MIN ? Q16_MIN:dwPcmTemp;		
		*Buffer = dwPcmTemp;
		Buffer++;
	}
}
/*----------------------------------------------------------------------------------------
函数名: DPWMPlayUninit
参数:
		None
返回值:
		None
描述:
		停止PDMA操作，关闭DPWM设备
----------------------------------------------------------------------------------------*/
static void DPWMPlayUninit(void)
{
	DrvPDMA_EnableCH(eDRVPDMA_CHANNEL_2, eDRVPDMA_DISABLE);
	while(DrvPDMA_IsCHBusy(eDRVPDMA_CHANNEL_2));
	DrvDPWM_DisablePDMA();  
	DrvDPWM_Close();
}
///*----------------------------------------------------------------------------------------
//函数名: ClearPlayList
//参数:
//		dwPlayCurAddr：音频开始地址
//		dwAudioEndAddr：音频结束的地址
//返回值:
//		None
//描述:
//		初始化一下播音参数，解码数据填满环形缓冲区，开启PDMA和DPWM播音
//----------------------------------------------------------------------------------------*/
void ClearPlayList(void)
{
	g_cntPlayAudio = g_nCurPlayAudio = 0;
	g_bNoInterruptedAudio = 0;
}
///*----------------------------------------------------------------------------------------
//函数名: StopPlayAudio
//参数:
//		None
//返回值:
//		None
//描述:
//		停止播音并关闭设备，从中采用让音量逐渐变小的方式避免"啪"的声音
//----------------------------------------------------------------------------------------*/
void StopPlayAudio(void)
{
	uint8_t i,nVolumeIndexBak = 0;
	nVolumeIndexBak = g_nVolumeIndex;
	for( i=0; i < CYCLEBUFFER_SIZE/320; i++){
		g_nVolumeIndex -= 1;
		if( (g_dwLastWritedPosition + SIREN7FRAME) > ((uint32_t)g_waCycBuffer + CYCLEBUFFER_SIZE * 2) )
			g_dwLastWritedPosition = (uint32_t)g_waCycBuffer;
		PlayVolumeCtrl((int16_t *)g_dwLastWritedPosition,SIREN7FRAME);
		g_dwLastWritedPosition += SIREN7FRAME;
	}
	g_nVolumeIndex = nVolumeIndexBak;
	g_bStopPlay = 1;
	DPWMPlayUninit();
}
///*----------------------------------------------------------------------------------------
//函数名: PlayResume
//参数:
//		dwPlayCurAddr：音频开始地址
//		dwAudioEndAddr：音频结束的地址
//返回值:
//		None
//描述:
//		初始化一下播音参数，解码数据填满环形缓冲区，开启PDMA和DPWM播音
//----------------------------------------------------------------------------------------*/
void PlayResume(void)
{
	if(g_bStopPlay){
		g_bStopPlay = 0;
		memset(g_waCycBuffer, 0xFF, sizeof(g_waCycBuffer));
		PDMAStart();
		PlayWork(0, 0, 0);
	}
}
///*----------------------------------------------------------------------------------------
//函数名: AddToPlayList
//参数:
//		dwPlayCurAddr：音频开始地址
//		dwAudioEndAddr：音频结束的地址
//返回值:
//		None
//描述:
//		初始化一下播音参数，解码数据填满环形缓冲区，开启PDMA和DPWM播音
//----------------------------------------------------------------------------------------*/
void AddToPlayList(uint32_t dwFirstCode)
{
	if(g_cntPlayAudio != PLAY_LIST_MAX_CNT)
		g_dwaPlayList[g_cntPlayAudio++] = (uint16_t)dwFirstCode;
	else
		LOG(("link Audio exceed "));
}
///*----------------------------------------------------------------------------------------
//函数名: PlayList
//参数:
//		dwPlayCurAddr：音频开始地址
//		dwAudioEndAddr：音频结束的地址
//返回值:
//		None
//描述:
//		初始化一下播音参数，解码数据填满环形缓冲区，开启PDMA和DPWM播音
//----------------------------------------------------------------------------------------*/
void PlayList(void)
{	
	if((!g_bNoInterruptedAudio || g_bStopPlay) && (g_nCurPlayAudio != g_cntPlayAudio)){
		PlayBegin((uint32_t)g_dwaPlayList[g_nCurPlayAudio++]);
		g_bNoInterruptedAudio = 1;
	}else if(g_nCurPlayAudio == g_cntPlayAudio){
		ClearPlayList();
	}
}
/*----------------------------------------------------------------------------------------
函数名: PlayWork
参数:
		CurAddr_s：播音当前的SPI地址
		EndAddr_s：整个音频的结束地址
		ParaData_s：播放音频所传递的参数，是Siren7压缩的还是wav文件
返回值:
		1/0: 播音已完成/播音未完成
描述:
		播音所需要的主要函数，负责解码数据并把数据填入环形缓冲区
----------------------------------------------------------------------------------------*/
int8_t PlayWork(uint32_t CurAddr_s,uint32_t EndAddr_s,uint32_t ParaData_s)
{
	static uint32_t	dwPlayCurAddr = 0, dwAudioEndAddr = 0, dwParaData = 0;//,flag_test = 2;
	uint32_t dwCurMemPlayAddr = 0;
	int16_t 	wTxedNum = 0,readbs = 0;
	uint16_t	iMovingData = 0, wReadNum = 0;
//	static uint8_t	s_bhavePlayedAudio = 0;

	if(CurAddr_s != 0)	dwPlayCurAddr = CurAddr_s;
	if(EndAddr_s != 0)		dwAudioEndAddr = EndAddr_s;
	if(ParaData_s != 0)		dwParaData = ParaData_s;

  	GPIOToSpi();   	
   	readbs = 40;//BITRATE2SIZE(sEnDeCtl.bit_rate);
	if(!g_bStopPlay){
		if(dwAudioEndAddr - dwPlayCurAddr > readbs){
			dwCurMemPlayAddr = (uint32_t)DrvPDMA_GetCurrentSourceAddr(eDRVPDMA_CHANNEL_2);
			wTxedNum = dwCurMemPlayAddr - g_dwLastWritedPosition;
			if(wTxedNum < 0){
				wTxedNum = CYCLEBUFFER_SIZE*2 + wTxedNum;		
			}
			while((wTxedNum >= SIREN7FRAME) && (dwAudioEndAddr - dwPlayCurAddr > readbs)){
				if(dwParaData == SIREN7AUDIO){		//Siren7
					if(wTxedNum >= SIREN7FRAME){
						wReadNum = readbs;
						wTxedNum -= SIREN7FRAME;
					}else{
					   	//g_dwLastWritedPosition = g_dwCurPlayAddr;
					   	break;
					}
					FMD_ReadData(g_waEncBuffer,dwPlayCurAddr,wReadNum);
					dwPlayCurAddr += readbs;
					if( (g_dwLastWritedPosition + SIREN7FRAME) > ((uint32_t)g_waCycBuffer + CYCLEBUFFER_SIZE * 2) )
						g_dwLastWritedPosition = (uint32_t)g_waCycBuffer;
					LibS7Decode(&sEnDeCtl, &sS7Dec_Ctx,(signed short *)g_waEncBuffer, (signed short *)g_dwLastWritedPosition);
					PlayVolumeCtrl((int16_t *)g_dwLastWritedPosition,SIREN7FRAME); 
					g_dwLastWritedPosition += SIREN7FRAME;
				}
				else{
//				if(dwParaData == WAVAUDIO){
					if(wTxedNum >= SIREN7FRAME){
						wReadNum = SIREN7FRAME;
						wTxedNum -= wReadNum;
					}else{
						//g_dwLastWritedPosition = g_dwCurPlayAddr;
					   	break;
					}
					FMD_ReadData(g_waPcmBuffer,dwPlayCurAddr,wReadNum);
					dwPlayCurAddr += wReadNum;
					g_dwReadPcmAddr = (uint32_t)g_waPcmBuffer;
					PlayVolumeCtrl((int16_t *)g_waPcmBuffer,sizeof(g_waPcmBuffer));
					for(iMovingData = 0; iMovingData < SIREN7FRAME/2; iMovingData++){
						if(g_dwLastWritedPosition >= ((uint32_t)g_waCycBuffer + CYCLEBUFFER_SIZE * 2))
							g_dwLastWritedPosition = (uint32_t)g_waCycBuffer;
						*(uint16_t *)(g_dwLastWritedPosition) = *(uint16_t *)(g_dwReadPcmAddr);
						g_dwLastWritedPosition += 2;
						g_dwReadPcmAddr += 2;
					}
				}
				dwCurMemPlayAddr = (uint32_t)DrvPDMA_GetCurrentSourceAddr(eDRVPDMA_CHANNEL_2);
				wTxedNum = dwCurMemPlayAddr - g_dwLastWritedPosition;
				if(wTxedNum < 0){
					wTxedNum = CYCLEBUFFER_SIZE*2 + wTxedNum;		
				}
			} 
//			s_bhavePlayedAudio = 1;
			g_bHavePlayAction = 1;
		}else{// if(s_bhavePlayedAudio == 1){
			StopPlayAudio();
			if(g_dwLinkNum-1 > 0){
				g_dwLinkNum--;
				PlayNextAudio();
			}
//			s_bhavePlayedAudio = 0;
			return 1;		//播音完成，可以关机
		}
	}
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: PlayNextAudio
参数:
		None
返回值:
		None
描述:
		播完一个音频后播放下一个音频
----------------------------------------------------------------------------------------*/
void PlayNextAudio(void)
{
	uint32_t dwAudioEndAddr = 0, dwAudioSize = 0,dwAudioStartAddr = 0;

	FMD_ReadData(&dwAudioStartAddr,g_dwLinkAudioAddr,4); 				//找到index
	g_dwLinkAudioAddr += 4;
	FMD_ReadData(&dwAudioSize,g_dwLinkAudioAddr,4);
	g_dwLinkAudioAddr += 4;
	dwAudioEndAddr = dwAudioStartAddr + dwAudioSize;

	PlayStart(dwAudioStartAddr, dwAudioEndAddr);
}
/*----------------------------------------------------------------------------------------
函数名: PlayBegin
参数:
		code：音频所对应的码值
返回值:
		None
描述:
		根据码值查找索引，找到音频所在的位置，并启动播音
----------------------------------------------------------------------------------------*/
void PlayBegin(int32_t code)
{
	uint32_t dwAudioEndAddr = 0, dwAudioSize = 0, dwAudioStartAddr = 0;
	int32_t dwLinkNumBak = 0;

	if(code <= MAX_AUDIO_NUM && code > 0 ){
	   	dwLinkNumBak = g_dwLinkNum;
		FMD_ReadData(&g_dwLinkNum,(CODE_START + code*5),1);   		//通过code得到连接音频的个数,index
		if(g_dwLinkNum > 0)
		{
			FMD_ReadData(&g_dwLinkAudioAddr,(CODE_START + code*5 + 1),4);
			FMD_ReadData(&dwAudioStartAddr,g_dwLinkAudioAddr,4); 				//找到index
			g_dwLinkAudioAddr += 4;
			FMD_ReadData(&dwAudioSize,g_dwLinkAudioAddr,4);
			g_dwLinkAudioAddr += 4;
			dwAudioEndAddr = dwAudioStartAddr + dwAudioSize;
	
			PlayStart(dwAudioStartAddr, dwAudioEndAddr);
		}
		else{
			g_dwLinkNum = dwLinkNumBak;								//采集到的一些无用的码值不会破坏当前的播音（多个）
		}
	}
}
/*----------------------------------------------------------------------------------------
函数名: PlayStart
参数:
		dwPlayCurAddr：音频开始地址
		dwAudioEndAddr：音频结束的地址
返回值:
		None
描述:
		初始化一下播音参数，解码数据填满环形缓冲区，开启PDMA和DPWM播音
----------------------------------------------------------------------------------------*/
void PlayStart(uint32_t dwPlayCurAddr,uint32_t dwAudioEndAddr)
{
	uint32_t dwParaData = 0;
	uint32_t readbs;//, pcm = 0, 
	uint16_t count_i =0;//,count_j = 0;//,flag = 8;

	g_bStopPlay = 0;
	memset(g_waCycBuffer,-1,sizeof(g_waCycBuffer));
	memset(g_waEncBuffer,-1,sizeof(g_waEncBuffer));
	memset(g_waPcmBuffer,-1,sizeof(g_waPcmBuffer));

	g_dwLastWritedPosition = (int32_t)g_waCycBuffer;
	readbs = 40;//BITRATE2SIZE(sEnDeCtl.bit_rate); 

	FMD_ReadData(&dwParaData,dwPlayCurAddr, 4);
	if (dwParaData == SIREN7AUDIO){	//Siren7编码	
		LibS7Init(&sEnDeCtl,VP_GET_SR(dwParaData),S7BANDWIDTH);
		LibS7DeBufReset(sEnDeCtl.frame_size,&sS7Dec_Ctx);
		dwPlayCurAddr += 4;

		FMD_ReadData(g_waEncBuffer,dwPlayCurAddr,readbs);
		LibS7Decode(&sEnDeCtl, &sS7Dec_Ctx, 
					(signed short *)g_waEncBuffer, (signed short *)g_dwLastWritedPosition);
				
		for(count_i = 0; count_i< CYCLEBUFFER_SIZE/320; count_i++){	//对g_waCycBuffer初次赋值，给640 * 6  = 3840 < 4096 	24ms
			FMD_ReadData(g_waEncBuffer,dwPlayCurAddr,readbs); 
			dwPlayCurAddr += readbs;
			if( (g_dwLastWritedPosition + SIREN7FRAME) > ((uint32_t)g_waCycBuffer + CYCLEBUFFER_SIZE * 2) )
				g_dwLastWritedPosition = (uint32_t)g_waCycBuffer;
			LibS7Decode(&sEnDeCtl, &sS7Dec_Ctx,(signed short *)g_waEncBuffer, (signed short *)g_dwLastWritedPosition);
			PlayVolumeCtrl((int16_t *)g_dwLastWritedPosition,SIREN7FRAME);
			g_dwLastWritedPosition += SIREN7FRAME;
		}
	}else
	{  	//wav文件 if (dwParaData == WAVAUDIO)
   		//dwPlayCurAddr += 44;
//		FMD_ReadData(g_waCycBuffer,dwPlayCurAddr,CYCLEBUFFER_SIZE*2);
//		dwPlayCurAddr += PCMBUFFER_SIZE*2;
//		dwParaData = 123;
	} 
	//g_dwLastWritedPosition = (int32_t)g_waCycBuffer;
	PDMAStart();
	PlayWork(dwPlayCurAddr, dwAudioEndAddr, dwParaData);
}
/*----------------------------------------------------------------------------------------
函数名: DPWMInit
参数:
		dwSampleRate：采样率
返回值:
		None
描述:
		初始化DPWM设备
----------------------------------------------------------------------------------------*/
void DPWMInit(uint32_t dwSampleRate)
{
	DrvDPWM_Open();
	DrvDPWM_SetDPWMClk(E_DRVDPWM_DPWMCLK_HCLK);
	DrvDPWM_SetSampleRate(dwSampleRate);
	DrvDPWM_SetFrequency(eDRVDPWM_FREQ_4);
	DrvDPWM_Enable();
}
/*----------------------------------------------------------------------------------------
函数名: DPWMInit
参数:
		None
返回值:
		None
描述:
		设置PDMA参数，并启动PDMA
----------------------------------------------------------------------------------------*/
void PDMAStart(void)
{
	STR_PDMA_T sPDMA; 
	/* PDMA Init */
    DrvPDMA_Init();
	DPWMInit(16000);

	/* CH1 TX Setting */
	sPDMA.sSrcAddr.u32Addr 			= (uint32_t)g_waCycBuffer;
    sPDMA.sDestAddr.u32Addr 		= (uint32_t)&DPWM->FIFO;   
    sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_16BITS;
	sPDMA.u8Mode 					= eDRVPDMA_MODE_MEM2APB;
	sPDMA.sSrcAddr.eAddrDirection 	= eDRVPDMA_DIRECTION_WRAPAROUND; 
	sPDMA.sDestAddr.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED; 
	sPDMA.u8WrapBcr				 	= eDRVPDMA_WRA_NO_INT;  
	sPDMA.i32ByteCnt                = CYCLEBUFFER_SIZE * 2;
	DrvPDMA_Open(eDRVPDMA_CHANNEL_2,&sPDMA);

	/* Enable INT */	 
	DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_2, eDRVPDMA_WAR ); 
	 
 	//DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_2,eDRVPDMA_WAR,	(PFN_DRVPDMA_CALLBACK) PDMA2_Callback ); 
	
	/* PDMA Setting */
	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_2,eDRVPDMA_DPWM,eDRVPDMA_WRITE_APB);
	DrvDPWM_EnablePDMA();
  
	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);	
}
/*----------------------------------------------------------------------------------------
函数名: EsrBegin
参数:
		None
返回值:
		None
描述:
		获取识别所对应的码值，并启动播放对应的音频
----------------------------------------------------------------------------------------*/
void EsrBegin(void)
{
	int32_t dwEsrCode;
	uint8_t iNum = 0;
	uint8_t nAudioIndex = 0;
										//1  2  3  4  5  6  7  8  9 10 11 12 13 14
	const static uint8_t cntEsrAudio[] = {2, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	static uint8_t nAudioInnerIndex[13] = {0};
	LOG(("ESR BEGIN...\r\n"));
	
	PlayBegin(PEN_ESRTIP_AUDIO);				//注意这里会毁坏“esrvars”中的变量，特别是g_sHeader
	while(!PlayWork(0,0,0));
	do{			
		LedFlashForEsr(LED_ON);			
		dwEsrCode = GetEsrCode(ESR_MAJOR_NET);

		if(((dwEsrCode < ESR_CODE_START) && dwEsrCode != 0) || dwEsrCode > ESR_CODE_END){
			LedFlashForNoEsr();
		}
	}while(((dwEsrCode < ESR_CODE_START) && dwEsrCode != 0) || dwEsrCode > ESR_CODE_END);
	LedFlashForEsr(LED_OFF);
	if(dwEsrCode != 0)
	{
		while(iNum < (dwEsrCode - ESR_CODE_START)){
			nAudioIndex += cntEsrAudio[iNum];
			iNum++;
		}
//		LOG(("before HardFault:%d,%d,%d\r\n",iNum,cntEsrAudio[iNum],rand()));
//		LOG(("random:%d\r\n",rand()%cntEsrAudio[iNum]));
//		rand();
		PlayBegin(ESR_CODE_START+nAudioIndex + nAudioInnerIndex[iNum]);
		if(++nAudioInnerIndex[iNum] >= cntEsrAudio[iNum])
			nAudioInnerIndex[iNum] = 0;
		if(dwEsrCode == (ESR_CODE_START + 13)){
			
		}
	}
	LOG(("ESR OK!\r\n"));
}
/*----------------------------------------------------------------------------------------
函数名: IsVolumeCtrl
参数:
		code：传入需要判断的码值
返回值:
		1/0：是音量控制码值/不是音量控制码值
描述:
		判断code值是不是音量控制的码值，如果是就改变音量并返回1，不是直接返回0
----------------------------------------------------------------------------------------*/
int8_t IsVolumeCtrl(int32_t dwCode)
{
	if(dwCode == VOLUME_PLUS_CODE){	
		g_nVolumeIndex += VOLUME_LEVEL_VALUE;
		if(g_nVolumeIndex >= VOLUME_MAX_LEVEL)	g_nVolumeIndex = VOLUME_MAX_LEVEL;
		return 1;
	}else if(dwCode == VOLUME_MINUS_CODE){
		g_nVolumeIndex -= VOLUME_LEVEL_VALUE;
		if(g_nVolumeIndex <= VOLUME_MIN_LEVEL)	g_nVolumeIndex = VOLUME_MIN_LEVEL;
		return 1;
	}
//	else if(dwCode==10 || dwCode==13 || dwCode==16 || dwCode==19 || dwCode==22 || dwCode==25){
//		g_nVolumeIndex = VOLUME_MIN_LEVEL + ((dwCode - 10)/3) * 6;
//		return 1;
	else if(dwCode == VOLUME_LEVEL0_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL;
		return 1;
	}else if(dwCode == VOLUME_LEVEL1_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 4;
		return 1;
	}else if(dwCode == VOLUME_LEVEL2_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 8;
		return 1;
	}else if(dwCode == VOLUME_LEVEL3_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 12;
		return 1;
	}else if(dwCode == VOLUME_LEVEL4_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 16;
		return 1;
	}else if(dwCode == VOLUME_LEVEL5_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 20;
		return 1;
	}else if(dwCode == VOLUME_LEVEL6_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 24;
		return 1;
	}else if(dwCode == VOLUME_LEVEL7_CODE){
		g_nVolumeIndex = VOLUME_MIN_LEVEL + 28;
		return 1;
	}
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: HaveRecord
参数:
		code：传入需要判断的码值
返回值:
		1/0：该码值存在录音/不存在录音
描述:
		判断code值存不存在录音，如果存在就播放录音返回1，不存在直接返回0
----------------------------------------------------------------------------------------*/
int8_t HaveRecord(int32_t dwCode)
{
	uint32_t dwRecFirstClusterIndex;
	uint32_t dwSourceSize,dwRecDataStartAddr;
	uint16_t dwCodeIdle = 0;
	RECHEADER sHeader;
//	uint32_t hello[64];
//	uint8_t i;
	
	if((dwCode < REC_CODE_START) || (dwCode > REC_CODE_END)){
//		LOG("%d is Not had Rec!",dwCode);
		return 0;
	}
	
	FMD_ReadData(&dwSourceSize,0x10000,sizeof(dwSourceSize));				//0x10000处为总资源大小
	dwRecDataStartAddr = (dwSourceSize/FLASH_MIN_ERASE_SIZE + 1) * FLASH_MIN_ERASE_SIZE;	//计算处录音应该的位置 		//4K对齐处
	
//	LOG("dwAddr:%x,%x",dwRecDataStartAddr,dwRecStartAddr);
//	LOG("hello world!!!!");
	
//	FMD_ReadData(&wRecFirstClusterIndex,dwRecStartAddr + (dwCode-REC_CODE_START)*2,2);								//读取片区索引（位置）
	if(CodeIsExist(dwCode,&dwCodeIdle)){
		DrvPDMA_EnableCH(eDRVPDMA_CHANNEL_2, eDRVPDMA_DISABLE);
		g_nRole = 4;		//角色切换
		FMC_ReadBuffer(DATAF_BASE_ADDR+dwCodeIdle*8 + 4,&dwRecFirstClusterIndex,sizeof(dwRecFirstClusterIndex));
		FMD_ReadData(&sHeader,dwRecDataStartAddr + dwRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE, sizeof(RECHEADER));
//		FMD_ReadData(hello,dwRecDataStartAddr+dwRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE,sizeof(hello));
//		for(i=0; i<64; i++)
//			LOG("hello[%d]:%x\r\n",i,hello[i]);
		g_wRecordMaxValue = sHeader.rec_max_value;
		Decoder.m_nPrevVal = sHeader.rec_encoder.m_nPrevVal;
		Decoder.m_nIndex = sHeader.rec_encoder.m_nIndex;
//			LOG("g_wRecordMaxValue:%d\r\n",g_wRecordMaxValue);
		if(sHeader.rec_audio_size > 800)						
			MagicVoice(SAMPLE_RATE16K, dwRecDataStartAddr, dwRecFirstClusterIndex, (sHeader.rec_audio_size - 400));		//去除最后100ms的杂音
		return 1;
	}else{
		PlayBegin(PEN_NORECTIP_AUDIO);
		return 0;
	}
}
/*----------------------------------------------------------------------------------------
函数名: HaveRecord
参数:
		code：传入需要判断的码值
返回值:
		1/0：该码值存在录音/不存在录音
描述:
		判断code值存不存在录音，如果存在就播放录音返回1，不存在直接返回0
----------------------------------------------------------------------------------------*/
void PlayMagicVoice(void)
{
	g_nRole = 2;
	HaveRecord(MVUSEDCODE);
}
/*----------------------------------------------------------------------------------------
函数名: IsChangeRole
参数:
		code：传入需要判断的码值
返回值:
		1/0：是角色控制码值/不是角色控制码值
描述:
		判断code值是不是音量控制的码值，如果是就改变音量并返回1，不是直接返回0
----------------------------------------------------------------------------------------*/
int8_t IsChangeRole(int32_t dwCode)
{
	if(dwCode == ROLE_CODE){
		if(g_nRole == 7)
			g_nRole = 1;
		else
			g_nRole++;

		return 1;
	} 
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: IsAboutMusic
参数:
		code：传入需要判断的码值
返回值:
		1/0：是角色控制码值/不是角色控制码值
描述:
		判断code值是不是音量控制的码值，如果是就改变音量并返回1，不是直接返回0
----------------------------------------------------------------------------------------*/
int8_t IsAboutMusic(int32_t dwCode)
{
	static int32_t dwMusicCode = PEN_FIRST_MUSIC;
	extern uint8_t g_bStartFlag;
//	uint8_t i;
	
	if(g_bStartFlag){
		dwMusicCode = PEN_FIRST_MUSIC;
		g_bStartFlag = 0;
	}
	if(dwCode == PLAY_SUSPEND_CODE || (dwCode == PLAY_STOP_CODE)){
		StopPlayAudio();
		return 1;
	}else if(dwCode == PLAY_RESUME_CODE){
		PlayResume();
		return 1;
	}else if(dwCode == PLAY_MUSIC_CODE){
//		if(g_cntPlayAudio == 0)
//			for(i=0; i<3; i++)
//				AddToPlayList(dwMusicCode+i);
		PlayBegin(dwMusicCode);
		return 1;
	}else if(dwCode == PLAY_NEXT_CODE){
		if(++dwMusicCode > PEN_LAST_MUSIC)
			dwMusicCode = PEN_FIRST_MUSIC;
		PlayBegin(dwMusicCode);
		return 1;
	}else if(dwCode == PLAY_PREV_CODE){
		if(--dwMusicCode < PEN_FIRST_MUSIC)
			dwMusicCode = PEN_LAST_MUSIC;
		PlayBegin(dwMusicCode);
		return 1;
	}
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: PlayChange
参数:
		code：传入需要判断的码值
返回值:
		1/0：连续出现50次-1时返回一次0/其余返回0
描述:
		分析code值所处的作用（音量调节、录音、普通播音），并按照分类分别执行
----------------------------------------------------------------------------------------*/
int8_t PlayChange(int32_t dwCode)
{
	static int32_t dwValidCode;
	static uint8_t cntErr = 0;	    		
	if ((dwCode!=-1 && (dwCode != dwValidCode || cntErr > 5))){
		LOG(("dwCode:%d",dwCode));
		g_bHaveCamAction = 1;
		dwValidCode = dwCode;
		ClearPlayList();
		if(IsAboutMusic(dwCode));
		else if(IsChangeRole(dwCode));
		else if(IsVolumeCtrl(dwCode));
//		else if(HaveRecord(dwCode));
		else	PlayBegin(dwCode);
		cntErr = 0;
	}
	else if(dwCode == -1){
		cntErr++;
	}else if(dwCode == dwValidCode){
		cntErr = 0;
	}
	if(cntErr >= 50){
		cntErr = 0;
		return 0;
	}
	return 1;
}
void ProcessHead(void)
{
	#define HEAD_AUDIO_NUM  3
	static uint8_t nHeadAudioIndex = 0;
	
	if(!g_bStopPlay){
		ClearPlayList();
		StopPlayAudio();
	}else{
		PlayBegin(PEN_HEAD_AUDIO1 + nHeadAudioIndex);
		if((++nHeadAudioIndex) >= HEAD_AUDIO_NUM){
			nHeadAudioIndex = 0;
		}
	}
}
void PlayRecord(void)
{
	HaveRecord(RECUSEDCODE);
}
/*----------------------------------------------------------------------------------------
函数名: PlayService
参数:
		None
返回值:
		None
描述:
		播音服务的主循环，负责按键检测，数据解码并填充缓冲区，电量检测等等
----------------------------------------------------------------------------------------*/
void PlayService(void)
{
	KEYMSG msg;

	LedFlashForCam(LED_ON);
	while(g_nState == PLAY_STATE)
	{		
		if(KEY_MsgGet(&msg)){
//			LOG(("KEY:Value:%d,TYPE:%d,HOLDTIME:%d\r\n",msg.Key_MsgValue,msg.Key_MsgType,msg.Key_HoldTime));
			g_bHaveKeyAction = 1;
			if(msg.Key_MsgValue == KEY_ON_OFF){
				if(msg.Key_MsgType == KEY_TYPE_SP){
					if(!g_bStopPlay)
						StopPlayAudio();
					else
						PlayResume();
				}else if(msg.Key_MsgType == KEY_TYPE_LP){	
					g_nState = PWRDOWN_STATE;
				}
			}else if(msg.Key_MsgValue == KEY_RECORD){
				if(msg.Key_MsgType == KEY_TYPE_LP){
					g_nState = RECORD_STATE;		
				}else if(msg.Key_MsgType == KEY_TYPE_SP){
					PlayRecord();
				}
			}else if(msg.Key_MsgValue == KEY_MACESR){
				if(msg.Key_MsgType == KEY_TYPE_LP){
					g_nState = MVREC_STATE;
				}else if(msg.Key_MsgType == KEY_TYPE_SP){
					g_nState = ESR_STATE;
				}
			}else if(msg.Key_MsgValue == KEY_TOUCH){
				if(msg.Key_MsgType == KEY_TYPE_SP){
					ProcessHead();
				}
			}
		}
		PlayWork(0,0,0);
		PlayList();
		if(!PlayChange(GetCamCode())){
//			cam_init(eDRVPDMA_CHANNEL_3);	//笔头初始化参数
		}
		CommonCheck();
	}
	ClearPlayList();
	StopPlayAudio();
	LedFlashForCam(LED_OFF);
}
void PlayPwrDownAduio(PEN_STATE PenState)
{
	uint32_t dwEsrCode = 0;
	if(SLEEP_STATE == PenState){
		PlayBegin(PEN_SLEEP_AUDIO);
		while(!PlayWork(0,0,0));
	}else if(PWRDOWN_STATE == PenState){
		PlayBegin(ESR_MINOR_AUDIO2);
		while(!PlayWork(0,0,0));
		LedFlashForEsr(LED_ON);
		dwEsrCode = GetEsrCode(ESR_MINOR_NET);
		if(dwEsrCode == ESR_CODE_START){
			PlayBegin(ESR_MINOR_AUDIO1);
			while(!PlayWork(0,0,0));
		}else{
			PlayBegin(ESR_MINOR_AUDIO3);
			while(!PlayWork(0,0,0));
		}
		LOG(("SHUTDOWN ESR CODE: %d\r\n",dwEsrCode));
		LedFlashForEsr(LED_OFF);
	}
}
/*----------------------------------------------------------------------------------------
函数名: PDMACBForMV
参数:
		status
返回值:
		None
描述:
		没有使用
----------------------------------------------------------------------------------------*/
//void PDMACBForMV(uint32_t status)
//{
//
//	PDMA->channel[eDRVPDMA_CHANNEL_2].SAR = (uint32_t)g_waMVPcmBuffer;		
//	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);	
//
//}
/*----------------------------------------------------------------------------------------
函数名: PDMAPlayInitForMV
参数:
		None
返回值:
		None
描述:
		初始化魔音PDMA
----------------------------------------------------------------------------------------*/
void PDMAPlayInitForMV(void)
{
	STR_PDMA_T sPDMA;  

	PDMA_GCR->PDSSR.DPWM_TXSEL = eDRVPDMA_CHANNEL_2;

	sPDMA.sSrcAddr.u32Addr 			= (uint32_t)g_waMVPcmBuffer; 
	sPDMA.sDestAddr.u32Addr 		= (uint32_t)&DPWM->FIFO;
	sPDMA.u8Mode 					= eDRVPDMA_MODE_MEM2APB;;
	sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_16BITS;
	sPDMA.sSrcAddr.eAddrDirection 	= eDRVPDMA_DIRECTION_WRAPAROUND; 
	sPDMA.sDestAddr.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED;
	sPDMA.u8WrapBcr				 	= eDRVPDMA_WRA_NO_INT;  
    sPDMA.i32ByteCnt = MVPCM_BUFFER_SIZE*2;	   	//Full MIC buffer length (byte)
    DrvPDMA_Open(eDRVPDMA_CHANNEL_2, &sPDMA);

	DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_2, eDRVPDMA_BLKD );
//	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_2,eDRVPDMA_DPWM,eDRVPDMA_WRITE_APB);

//	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_2, eDRVPDMA_BLKD,
//		(PFN_DRVPDMA_CALLBACK)PDMACBForMV ); 
}
/*----------------------------------------------------------------------------------------
函数名: CBMVGetData
参数:
		pcmData：pcm数据的首地址
		nDataSize：pcm数据的字节数
返回值:
		None
描述:
		判断已播出的字节数，如果大于nDataSize，则改变音量并向环形缓冲区填充数据
----------------------------------------------------------------------------------------*/
static void ivCall CBMVGetData(ivPInt16 pcmData,ivInt16 nDataSize)
{
	int32_t i, nLen, Temp;
	nDataSize >>= 1;

	while(1)
	{
		nLen = PDMA->channel[eDRVPDMA_CHANNEL_2].CSAR - 
			(uint32_t)&(g_waMVPcmBuffer[g_dwHaveWritedPosition]);
		nLen >>= 1;
		if(0 >= nLen){
			nLen += MVPCM_BUFFER_SIZE;
		}
		if(nDataSize < nLen){
			for(i=0; i< nDataSize; i++){
				Temp = (int32_t)((*pcmData) * g_wAmpCof)/Q10;
				Temp = Temp > Q16_MAX ? Q16_MAX : Temp;
				Temp = Temp < Q16_MIN ? Q16_MIN : Temp;
				g_waMVPcmBuffer[g_dwHaveWritedPosition] = 
					(int16_t)Temp;
				g_dwHaveWritedPosition++;
				if(MVPCM_BUFFER_SIZE <= g_dwHaveWritedPosition){
					g_dwHaveWritedPosition = 0;
				}
				pcmData++;
			}
			break;
		}
	}
}
/*----------------------------------------------------------------------------------------
函数名: MagicVoice
参数:
		dwBaseRate：计算魔音的采样率的基础采样率
		dwSpiCurAddr：音频当前播放位置的SPI地址
		dwPcmCount：需要播音的总的字节数
返回值:
		None
描述:
		魔音的主函数，清零变量、播音控制、解码数据、追加数据
----------------------------------------------------------------------------------------*/
void MagicVoice(uint32_t dwBaseRate,uint32_t dwRecDataStartAddr, uint16_t wRecFirstClusterIndex, int32_t dwPcmCount)
{
	int32_t cntHavePlayed=0;
	KEYMSG	msg;
	uint8_t	i = 0,First=0;
	uint32_t dwRecNextClusterIndex = 0;
	uint32_t dwSpiCurAddr = dwRecDataStartAddr + wRecFirstClusterIndex*FLASH_MIN_ERASE_SIZE;
	uint32_t dwRecClusterJumpAddr = dwSpiCurAddr + sizeof(RECHEADER);		//找到跳转信息
	uint32_t dwRecClusterJumpEndAddr = dwSpiCurAddr + FLASH_PAGE_SIZE;
	uint32_t dwRecClusterEndAddr = dwSpiCurAddr + FLASH_MIN_ERASE_SIZE;

	uint32_t Rate_MV;
	ivStatus err;
	ivHandle hMVObj;
	TUser tUser;
	ivSize nMVObjSize = MVWORK_SPACE_SIZE;//4KB,2K Word;
	
	LOG(("JumpAddr:%x, HEADER:%d\r\n",dwRecClusterJumpAddr,sizeof(RECHEADER)));
	
	dwSpiCurAddr += FLASH_PAGE_SIZE;		//跳过第一页的属性和跳转信息
	hMVObj = g_naMVWorkSpace;
	g_dwHaveWritedPosition = 0;
	memset(g_naMVWorkSpace, 0, sizeof(g_naMVWorkSpace));
	memset(g_waMVPcmBuffer, 0, sizeof(g_waMVPcmBuffer));
	memset(g_waMVAppendBuf, 0, sizeof(g_waMVAppendBuf));
	tUser.lpfnGetRusult = CBMVGetData;

	err = MVCreate(hMVObj, &nMVObjSize,&tUser);
	err = MVSetParam(hMVObj, MV_PARAM_SAMPLE_RATE,(ivCPointer)dwBaseRate);
	err = MVSetParam(hMVObj, MV_PARAM_ROLE,(ivCPointer)g_nRole);
	err = MVStart(hMVObj);
	LOG(("MVStart err: %d \r\n",(int16_t)err));
	
	PDMAPlayInitForMV();
	Rate_MV = (dwBaseRate*ROLE_COF_Q7[g_nRole-1])/Q7;
//	LOG("Rate_MV=%d\r\n", Rate_MV);
	
	DPWMInit(Rate_MV);
	DrvDPWM_EnablePDMA();
	DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);

	g_wAmpCof = (int32_t)(SAMPLE_MAX_COF) / g_wRecordMaxValue;
	g_wAmpCof = (g_wAmpCof*g_waVolumeCof[g_nVolumeIndex])/Normalized_Cof;   // Volume Ctrl!!

	while(dwPcmCount > 0)
	{
		g_bHavePlayAction = 1;
		if(KEY_MsgGet(&msg)){
			g_bHaveKeyAction = 1;
			if(msg.Key_MsgValue == KEY_RECORD){
				if(msg.Key_MsgType == KEY_TYPE_DOWN){
					g_nState = RECORD_STATE;
					break;
				}
			}else if(msg.Key_MsgValue == KEY_ON_OFF){
			 	if(msg.Key_MsgType == KEY_TYPE_SP){
					g_nState = ESR_STATE;
					break;
				}else if(msg.Key_MsgType == KEY_TYPE_LP){
					g_nState = PWRDOWN_STATE;					
					break;	
				}
			}
		}


		cntHavePlayed = PDMA->channel[eDRVPDMA_CHANNEL_2].CSAR - (uint32_t)&(g_waMVPcmBuffer[g_dwHaveWritedPosition]);
		cntHavePlayed >>= 1;
		if(0 > cntHavePlayed)
		{
			cntHavePlayed += MVPCM_BUFFER_SIZE;
		}
		
		if(MVPCM_BUFFER_SIZE/2 < cntHavePlayed)
		{
			LOG(("*"));
		}
		FMD_ReadData(g_waAdpcmBuffer,dwSpiCurAddr,ADPCM_BUFFER_SIZE<<1);
		dwSpiCurAddr += ADPCM_BUFFER_SIZE<<1;
		if(dwSpiCurAddr >= dwRecClusterEndAddr){			//指向一个FLASH_MIN_ERASE_SIZE的最后时就可以跳转了
			g_bHavePlayAction = 1;
			LOG(("JumpAddr:%x,EndAddr:%x\r\n",dwRecClusterJumpAddr,dwRecClusterJumpEndAddr));
			
			FMD_ReadData(&dwRecNextClusterIndex,dwRecClusterJumpAddr,4);
//			if(wRecNextClusterIndex == (uint16_t)0xFFFF){
//				DPWMPlayUninit();
//				return;
//			}
			if((dwRecClusterJumpAddr+4) == dwRecClusterJumpEndAddr){
				dwRecClusterJumpAddr = dwRecDataStartAddr + dwRecNextClusterIndex*FLASH_MIN_ERASE_SIZE;
				dwRecClusterJumpEndAddr = dwRecClusterJumpAddr + FLASH_PAGE_SIZE;
				
				dwSpiCurAddr = dwRecClusterJumpEndAddr;
				dwRecClusterEndAddr = dwRecClusterJumpAddr + FLASH_MIN_ERASE_SIZE;
			}else{
				dwSpiCurAddr = dwRecDataStartAddr + dwRecNextClusterIndex*FLASH_MIN_ERASE_SIZE;
				dwRecClusterEndAddr = dwSpiCurAddr + FLASH_MIN_ERASE_SIZE;
				dwRecClusterJumpAddr += 4;
			}
					
		}
		ivADPCM_Decode(&Decoder, (ivPUInt8)g_waAdpcmBuffer, 
				ADPCM_BUFFER_SIZE<<1, g_waMVAppendBuf);
	
		if(dwPcmCount <= 1280)			//30*64(*4)个字节留作音量渐低结尾
		{
			if(g_nVolumeIndex-i)
				i++;
			g_wAmpCof = (int32_t)(SAMPLE_MAX_COF)/(g_wRecordMaxValue);
			g_wAmpCof = (g_wAmpCof*g_waVolumeCof[g_nVolumeIndex-i])/Normalized_Cof;
		}else{
			g_wAmpCof = (int32_t)(SAMPLE_MAX_COF) / g_wRecordMaxValue;
			g_wAmpCof = (g_wAmpCof*g_waVolumeCof[g_nVolumeIndex])/Normalized_Cof;   // Volume Ctrl!!
		}

		err = MVAppendAudioData(hMVObj, g_waMVAppendBuf,MVAPPEND_BUFFER_SIZE>>1);
	   	err = MVAppendAudioData(hMVObj, &g_waMVAppendBuf[MVAPPEND_BUFFER_SIZE>>1],MVAPPEND_BUFFER_SIZE>>1);
		dwPcmCount = dwPcmCount - (ADPCM_BUFFER_SIZE<<1);
		if(!First)
		{
			First = 1;
			DrvPDMA_CHEnablelTransfer(eDRVPDMA_CHANNEL_2);
		}
	}
	DPWMPlayUninit();	
}


