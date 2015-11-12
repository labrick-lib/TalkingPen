//-------------------------和图像编码有关的程序
#include "cam.h"

//PARAM_ADDR 是参数在spiflash中的起始地址

/*
引脚使用：
SPI:
PA0--MOSI0
PA1--SCLK
PA2--SSB0
PA3--MISO0

IIC:
PB2--SCK
PB3--SDA

PWM:
PA12--0通道
PA13--1通道
*/
extern uint8_t g_nState;
extern uint8_t g_bRecordStart;
extern uint8_t g_bHaveKeyAction;
extern uint8_t g_bHaveRecAction;

int8_t get_param_from_file(int32_t *pParam1, int32_t *pParam2, int32_t *pParam3, int32_t *pParam4,int32_t *pParam5)
{
	int32_t buf[5];

	FMD_ReadData(buf, PARAM_ADDR, 5 * 4);//5个int型数据
	*pParam1 = buf[0];
	*pParam2 = buf[1];
	*pParam3 = buf[2];
	*pParam4 = buf[3];
	*pParam5 = buf[4];
	return 1;	
}

void save_param_to_file(int32_t param1, int32_t param2, int32_t param3, int32_t param4, int32_t param5)
{
	uint8_t buf[256];
	uint32_t i;
	for(i = 0; i < 256; i++)
	{
		buf[i] = 0;
	}
	*(int32_t*)&buf[0] = param1;
	*(int32_t*)&buf[4] = param2;
	*(int32_t*)&buf[8] = param3;
	*(int32_t*)&buf[12] = param4;
	*(int32_t*)&buf[16] = param5;
	
	FMD_EraseBlock(PARAM_ADDR); //擦除PARAM_ADDR所在的Sector(4K)
	FMD_WriteData(buf, PARAM_ADDR); //一次写一页 		
}


  
T_VOID cam_adjust_check(void)
{
    int32_t param1, param2, param3, param4, param5;

    if(get_param_from_file(&param1, &param2, &param3, &param4, &param5)){
		if(param5==0xAA){
	        cam_set_adj_param(param1, param2, param3, param4, param5);            
			LOG(("Get param OK\n"));
			return;
		}          
    }else
//	{
//		LOG("Get param NG\n");  
//    }
//    
//    LOG("Begin to adjust...\n");
    //play_beep(200);
    
    if(cam_get_adj_param((T_U32 *)0x20000000, 1024*3, &param1, &param2, &param3, &param4,&param5)){
        //save the data in file
        LOG(("Camera adjusting OK\n"));
        save_param_to_file(param1, param2, param3, param4, param5);	
        LOG(("Sava param OK\n"));
        cam_set_adj_param(param1, param2, param3, param4, param5);
        //play_beep(500);
        return ;       
    }else{
        LOG(("Camera adjust NG\n"));           
    }

}

//__align(4) uint8_t cam_buf[3 * 1024];
T_VOID us_delay(T_U32 us)
{
    
		us = us * 10 ;//>> 2;//5;       // time = time/32
        
        while(us--);
        {
            *(volatile T_U32 *)0;
        }
}
static void audio_codec(void)
{
	us_delay(1000*9); 
}
/*----------------------------------------------------------------------------------------
函数名: CameraStart
参数:
		None
返回值:
		None
描述:
		启动点读笔的摄像头，并自校正笔头
----------------------------------------------------------------------------------------*/
unsigned char buf[1600] __attribute__ ((section("camvars"), zero_init));
void InitCam(void)
{
	uint8_t	bRet;
//	unsigned char buf[1600];
//	memset(buf,0,sizeof(buf));
    bRet = cam_init(eDRVPDMA_CHANNEL_3,buf,1600);	//笔头初始化参数
    if(!bRet)
    {
		LOG(("camera init fail\n"));
		while(1);
    }
	else
	{
		LOG(("camera init ok \n"));
	}
}
void CameraStart(void)
{
	LOG(("CAMERA INIT...\r\n"));
	InitCam();
	cam_adjust_check();		   //笔头自校正程序
	cam_reg_audio_cb(audio_codec);
	LOG(("CAMERA INIT OK!\r\n"));
}
/*----------------------------------------------------------------------------------------
函数名: GetCamCode
参数:
		None
返回值:
		dwCode
描述:
		获取摄像头的码值，并返回
----------------------------------------------------------------------------------------*/
//T_U8  *show_log_buf = (T_U8 *)(0x20000000 + 1024*5);
int32_t GetCamCode(void)
{
	int32_t dwCode;
//	extern int16_t abc;

	DrvPDMA_DisableInt(eDRVPDMA_CHANNEL_2, eDRVPDMA_WAR);
//	__disable_irq();
	DrvTIMER_Ioctl(TMR0,TIMER_IOC_STOP_COUNT,0);
//	while(1){						 
	cam_get_frame((T_U32 *)0x20000000, 304, (T_U8*)(0x20000000+308), 50,
	(T_U8 *)(0x20000000+308 + 50), 50);
	dwCode = cam_get_code();
//	abc++;
//	LOG(("dwCode=%d\r\n",dwCode));
//	}
//	__enable_irq();
	DrvTIMER_Ioctl(TMR0,TIMER_IOC_START_COUNT,0);
	DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_2,eDRVPDMA_WAR);
	
	GPIOToSpi();	
	return dwCode;
}
/*----------------------------------------------------------------------------------------
函数名: GetRecCode
参数:
		None
返回值:
		dwCode：录音所使用的码值
描述:
		获取摄像头的码值，并返回用于录音，但在获取到码值之前有按键检测或者超时检测
----------------------------------------------------------------------------------------*/
int32_t GetRecCode(void)
{
	KEYMSG msg;
	int32_t dwRecCode = 0;
	uint8_t bReturnFlag = 0;

//	CameraStart();
	g_bRecordStart = 1;
	LedFlashBeforeRec(LED_ON);

//	LOG("start by selecting a code, end by bouncing key\r\n");
	while( (dwRecCode = GetCamCode()) == -1 )		//获得外界码值
	{
	 	if(GetWaitForRecTicks() >= REC_EXIT_TIME){
			bReturnFlag = 1;	
		}
		if(KEY_MsgGet(&msg)){
			g_bHaveKeyAction = 1;
			if(msg.Key_MsgValue == KEY_RECORD){
				if(msg.Key_MsgType == KEY_TYPE_UP){
//					LOG("record end!!");
					bReturnFlag = 1;
				}
			}
		}

		if(bReturnFlag)
		{
			bReturnFlag = 0;
			g_nState = PLAY_STATE;
			break;
		}
	}
	LOG(("dwRecCode:%d",dwRecCode));
	g_bRecordStart = 0;
	g_bHaveRecAction = 1;
	LedFlashBeforeRec(LED_OFF);
	return dwRecCode;
}

