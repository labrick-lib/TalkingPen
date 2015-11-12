
#include "hw.h"
extern uint16_t QUEUE_Num;
extern uint8_t g_nState;

extern uint8_t g_bLowPower,g_bServerLowPwr;
extern uint8_t g_bHaveKeyAction;
//extern uint32_t g_cntStartTime;
uint8_t g_bFirst = 1;
uint8_t g_bStartFlag = 0;

/*
 * GPIO复用
 * PA0/SPI_MOSI0/MCLK
 * PA1/SPI_SCLK/I2C_SCL
 * PA2/SPI_SSB0
 * PA3/SPI_MISO0/I2C_SDA
 * PA4/I2S_FS
 * PA5/I2S_BCLK
 * PA6/I2S_SDI
 * PA7/I2S_SDO
 * PA8/UART_TX/I2S_FS
 * PA9/UART_RX/I2S_BCLK
 * PA10/UART_RTSn/I2S_SDI/I2C_SDA
 * PA11/UART_CTSn/I2S_SDO/I2C_SCL
 * PA12/I2S_FS/SPKP/PWM0
 * PA13/I2S_BCLK/SPKM/PWM1
 * PA14/TM0/SDCLK/SDCLKn
 * PA15/TM1/SDIN
 *
 * PB0/SPI_SSB0/CMP0/SPI_SSB1
 * PB1/SPI_SSB1/CMP1/MCLK
 * PB2/SPI_SCLK/CMP2/I2C_SCL
 * PB3/SPI_MISO0/CMP3/I2C_SDA
 * PB4/SPI_MOSI0/CMP4/PWM0B
 * PB5/SPI_MISO1/CMP5/PWM1B
 * PB6/SPI_MOSI1/CMP6/I2S_SDI
 * PB7/CMP7/I2S_SDO
 * WAKEUP 唤醒线
 *
 */

/*----------------------------------------------------------------------------------------
函数名: SystemClockInit
参数:
		None
返回值:
		None
描述:
		初始化系统时钟，打开LDO
----------------------------------------------------------------------------------------*/
static void SystemClockInit(void)
{
    /* Unlock the protected registers */	
	UNLOCKREG();

	/* HCLK clock source. */
	//DrvSYS_SetHCLKSource(0);
	SYSCLK->PWRCON.OSC49M_EN = 1;
	SYSCLK->PWRCON.OSC10K_EN = 1;
//	SYSCLK->PWRCON.XTL32K_EN = 1;
	SYSCLK->CLKSEL0.STCLK_S = 3; /* Use internal HCLK */

	SYSCLK->APBCLK.ANA_EN = 1;
	ANA->LDOSET = 3;
	ANA->LDOPD.PD = 0;
	 		
	SYSCLK->CLKSEL0.HCLK_S = 0; /* Select HCLK source as 48MHz */ 
	SYSCLK->CLKDIV.HCLK_N  = 0;	/* Select no division          */	
	DrvSYS_SetIPClockSource(E_SYS_TMR0_CLKSRC, 4);
	DrvSYS_SetIPClockSource(E_SYS_TMR1_CLKSRC, 4);

	LOCKREG();

	/* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */
	//DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); 
}
/*----------------------------------------------------------------------------------------
函数名: IsStart
参数:
		None
返回值:
		None
描述:
		判断是否满足开机的条件，满足就直接开机，不满足在判断有没有USB，有就显示充电状态，无关机
----------------------------------------------------------------------------------------*/
void IsStart(void)
{
	KEYMSG msg;
	uint8_t bHaveUsb = 0;

	while(1){
		g_bHaveKeyAction = 1;
		if(KEY_MsgGet(&msg)){			
			if(msg.Key_MsgValue == KEY_ON_OFF){
				if(msg.Key_MsgType == KEY_TYPE_LP){
					g_bStartFlag = 1;
					GPIOA->DOUT |= (1 << 14);
					PlayBegin(PEN_START_AUDIO);
				}else if(msg.Key_MsgType == KEY_TYPE_UP && g_bStartFlag == 1){
					g_nState = PLAY_STATE;
					QUEUE_Num = 0;
					AdapterDisconnect();
					break;
				}
			}
		}
		if(g_bStartFlag){
			AdapterDisconnect();
			PlayWork(0,0,0);			
		}else{			
			g_bStartFlag = 0;
			if(DrvGPIO_GetBit(GPA,15))		//remove vbus
				bHaveUsb = 1;
			else{
				bHaveUsb = 0;
				DrvGPIO_ClrBit(GPA,14);
			}
	
			if(bHaveUsb == 1)
			{
				if(DrvGPIO_GetBit(GPA,7))		//charging
					LedFlashForChargeover();
				else
					LedFlashForCharge();	
			}else{
				AdapterDisconnect();
			}
		}
	}
	CameraStart();
	g_bStartFlag = 0;
}
/*----------------------------------------------------------------------------------------
函数名: HW_Init
参数:
		None
返回值:
		None
描述:
		整个系统的初始化程序
----------------------------------------------------------------------------------------*/
void HW_Init(void)
{
	SystemClockInit();
	DrvGPIO_Open(GPA,14,IO_OUTPUT);
	GPIOA->DOUT |= (1 << 14);
	memset((int32_t *)0x20000000,0,0x2300);
	icTimerInit();
	TimerInit(); 
	KEY_Init();
	SysTimerDelay(100000);
	DrvPDMA_Init();
	Uart_Init();
	SPI_Init();
	LedInit();
	InitialFMC();
	TouchSenseInit();
	ClrESRMemory();
	ADC_Init();
	ADC_Term();
//	{
//		uint8_t i;
//		for(i=0; i<255; i++){
//			LOG(("%d ",(rand()%100)));
//		}
//	}
//	{
//		int test;
//		for(test = 0x2000273C; test < 0x20002e00; test++)
//		{
//			*(int8_t *)test = 0x55;
//		}
//	}
	if(g_bFirst){
		IsStart();
		g_bFirst = 0;
	}
}
/*----------------------------------------------------------------------------------------
函数名: HW_Term
参数:
		None
返回值:
		None
描述:
		整个系统的终止程序，方便休眠使用
----------------------------------------------------------------------------------------*/
void HW_Term(void)
{
	/* 关闭GPIO中断 */
	SPI_Term();
	LedUninit();
	DrvDPWM_Close();
	DrvPDMA_Close();
	DrvTIMER_Close(TMR1);
}
/*----------------------------------------------------------------------------------------
函数名: gpab_irq
参数:
		None
返回值:
		None
描述:
		步子到！！！！！！！
----------------------------------------------------------------------------------------*/
GPIO_GPAB_CALLBACK gpab_irq(uint32_t p, uint32_t q)
{
	return 0;
}
/*----------------------------------------------------------------------------------------
函数名: SetWakeupPin
参数:
		None
返回值:
		None
描述:
		设置休眠唤醒引脚，并规定GPB2下降沿唤醒中断
----------------------------------------------------------------------------------------*/
void SetWakeupPin(void)
{
	DrvGPIO_SetIntCallback(gpab_irq(0, 4));
//	DrvGPIO_EnableInt(GPA, 10, IO_FALLING, MODE_EDGE);	
//	GPIOA->IEN |= (1 << (10 + 16));//PA10 上升沿
	GPIOA->IEN |= (1 << 10);//PA10下降沿中断
	GPIOB->IEN |= (1 << 16); //PB0 上升沿沿中断
	GPIOB->IEN |= (1 << 17); //PB1 上升沿中断
	GPIOB->IEN |= (1 << 2);


	NVIC_SetPriority (GPAB_IRQn, (1<<__NVIC_PRIO_BITS) - 2);
	NVIC_EnableIRQ(GPAB_IRQn);
	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT1_IRQn);
}
/*----------------------------------------------------------------------------------------
函数名: halt
参数:
		None
返回值:
		None
描述:
		进入休眠的程序
----------------------------------------------------------------------------------------*/
void halt(void) //进入stop模式 电流：< 10uA
{			
//	SYS->GPB_ALT.GPB0 = 0;
	SYS->GPA_ALT.GPA4 = 0;
	SYS->GPA_ALT.GPA5 = 0;
	SYS->GPA_ALT.GPA6 = 0;
	SYS->GPA_ALT.GPA10 = 0;
	SYS->GPA_ALT.GPA11 = 0;
	SYS->GPA_ALT.GPA12 = 0;
	SYS->GPA_ALT.GPA13 = 0;
	SYS->GPA_ALT.GPA14 = 0;

	DrvGPIO_Open(GPA, 4, IO_OUTPUT);
	DrvGPIO_Open(GPA, 5, IO_OUTPUT);
	DrvGPIO_Open(GPA, 6, IO_OUTPUT);
	DrvGPIO_Open(GPA, 10, IO_OUTPUT);
	DrvGPIO_Open(GPA, 11, IO_OUTPUT);
	DrvGPIO_Open(GPA, 12, IO_OUTPUT);
	DrvGPIO_Open(GPA, 13, IO_OUTPUT);
	DrvGPIO_Open(GPA, 14, IO_OUTPUT);

	DrvGPIO_ClrBit(GPA,4);
	DrvGPIO_ClrBit(GPA,5);
	DrvGPIO_ClrBit(GPA,6);
	DrvGPIO_ClrBit(GPA,10);
	DrvGPIO_ClrBit(GPA,11);
	DrvGPIO_ClrBit(GPA,12);
	DrvGPIO_ClrBit(GPA,13);
	DrvGPIO_ClrBit(GPA,14);

	SYS->GPA_ALT.GPA8 = 0;
	SYS->GPA_ALT.GPA9 = 0;
	GPIOA->PMD.PMD8 = IO_INPUT;
	GPIOA->PMD.PMD9 = IO_INPUT;

	UNLOCKREG();
	ANA->LDOPD.PD = 1;
	SYSCLK->APBCLK.ANA_EN = 0;
	LOCKREG();

	SYS->WAKECR.WAKE_TRI = 0; //使能wakeup引脚内部上拉
	
	ANA->ISRC.EN = 0; //如果不设置这两个，则电流为16uA
	ACMP->CMPCR[0].CMPEN = 0;
	DrvADC_AnaClose(); //LDO

	SetWakeupPin();		
	UNLOCKREG();
	SYSCLK->PWRCON.STOP = 1; //这一位设成1,Stop模式电流才能降到10uA
	SYSCLK->PWRCON.STANDBY_PD = 0;
	SYSCLK->PWRCON.DEEP_PD = 0;
	SCB->SCR = 1 << 2; //设置sleep为deep sleep
	LOCKREG();	
//RTC时钟开启时，休眠状态下要使能RTC模块的时钟
//使用触摸唤醒的时候也要使用RTC
	*(uint32_t*)(&SYSCLK->CLKSLEEP) = 1;	
	*(uint32_t*)(&SYSCLK->APBCLK) = 1;
//	sleep = 1;
	__wfi();
}
/*----------------------------------------------------------------------------------------
函数名: HW_EnterPowerDown
参数:
		None
返回值:
		None
描述:
		判断是否进入休眠，不进入则显示充电状态
----------------------------------------------------------------------------------------*/
void HW_EnterPowerDown(PEN_STATE PenState)
{
	// Uninitialize all IPs
	KEYMSG msg;
	int16_t count = 0,temp = 0,bHaveUsb = 0;
	if(DrvGPIO_GetBit(GPA,15))
		bHaveUsb = 1;		//有USB
	else bHaveUsb = 0;
	HW_Term();
HALT:
	if(!bHaveUsb){
//		NVIC_SystemReset();
		halt();
		count = 0;
		while(DrvGPIO_GetBit(GPB,0))
		{
			SysTimerDelay(100000);
			count++;
			if(count > 13)
			{
				goto WAKE_UP;
			}
		}
		goto HALT;
	}
WAKE_UP:	
	// Re-initialize
	QUEUE_Num = 0;
	g_nState = PLAY_STATE;
	HW_Init();
	if(!bHaveUsb)	temp = KEY_TYPE_DOWN;
	else		temp = KEY_TYPE_LP;

	while(1){
		g_bHaveKeyAction = 1;
		if(KEY_MsgGet(&msg)){
//		LOG("hello --- %d | %d\r\n",msg.Key_MsgValue,msg.Key_MsgType);
			if(msg.Key_MsgValue == KEY_ON_OFF){
				if(msg.Key_MsgType == temp){
					PlayBegin(PEN_START_AUDIO);
					g_bStartFlag = 1;
				}else if(msg.Key_MsgType == KEY_TYPE_UP && g_bStartFlag){		//long press KEY_ON_OFF	
					g_nState = PLAY_STATE;
					QUEUE_Num = 0;
					AdapterDisconnect();
					break;
				}
			}
		}
		if(g_bStartFlag){
			PlayWork(0,0,0);
			AdapterDisconnect();
		}else{
			if(DrvGPIO_GetBit(GPA,15))		//remove vbus
				bHaveUsb = 1;
			else{
				DrvGPIO_ClrBit(GPA,14);
				bHaveUsb = 0;
			}
	
			if(bHaveUsb)
			{
				if(DrvGPIO_GetBit(GPA,7))
					LedFlashForChargeover();
				else
					LedFlashForCharge();	
			}else{
				AdapterDisconnect();
			}
		}
	}
	g_bStartFlag = 0;
	CameraStart();	
}

int16_t adc_res;

typedef struct {
    BOOL                   bOpenFlag;
    DRVADC_ADC_CALLBACK    *g_ptADCCallBack;
    DRVADC_ADCMP0_CALLBACK *g_ptADCMP0CallBack;
    DRVADC_ADCMP1_CALLBACK *g_ptADCMP1CallBack;
    uint32_t g_pu32UserData[3];
} S_DRVADC_TABLE;

extern S_DRVADC_TABLE gsAdcTable;
/*----------------------------------------------------------------------------------------
函数名: ADCCBForPwrCheck
参数:
		None
返回值:
		None
描述:
		ADC用于电量检测的回调函数
----------------------------------------------------------------------------------------*/
void ADCCBForPwrCheck(uint32_t parm)
{
	uint8_t i;
	for(i = 0; i < 4; i++)
	{
		adc_res = SDADC->ADCOUT;
	}	
}
/*----------------------------------------------------------------------------------------
函数名: ADCCBForPwrCheck
参数:
		wSample：ADC所采集出的数值
返回值:
		dwAdcVal：转后得到ADC采集的真实值
描述:
		用于电量检测值的转换
其他：
		adc结果：0V对应162  3.3V对应值为-293 1.2V为0
		把采样结果转换成0到455之间的值
		0对应0V 455对应3.3V => 转到[0:256]区间		
----------------------------------------------------------------------------------------*/
int16_t AdcValueConvert(int16_t wSample)
{
	int32_t dwAdcVal;	
	dwAdcVal = wSample / 100;
	if(wSample > 0)
	{
		dwAdcVal = 163 - dwAdcVal;
		wSample = -wSample;
		if(wSample % 100 > 50)
		{
			dwAdcVal--;
		}			
	}
	else
	{
		dwAdcVal = -dwAdcVal;
		dwAdcVal += 163;
		if(wSample % 100 > 50)
		{
			dwAdcVal++;
		}			
	}
	//dwAdcVal:0对应0V 455对应3.3V
	//转到[0:255]
//	dwAdcVal = 256 * dwAdcVal / 455;
//	if(dwAdcVal > 255)
//	{
//		dwAdcVal = 255; 
//	}		
	return dwAdcVal;	
}
/*----------------------------------------------------------------------------------------
函数名: AdcPwrCheck
参数:
		None
返回值:
		wResult：采集出的样值
描述:
		ADC用于电量检测的初始化与采集样值
----------------------------------------------------------------------------------------*/
uint32_t AdcPwrCheck(void)
{
	uint32_t i;
	uint16_t wResult;
	SYS->IPRSTC2.ADC_RST = 1;
	SYS->IPRSTC2.ADC_RST = 0;
	SYSCLK->APBCLK.ADC_EN = 1;

//ADC set
	SDADC->INT.IE = 0;		
	SDADC->CLK_DIV = 15;//49.152M / (15 + 1) = 3.072MHz	
	SDADC->DEC.OSR = 3;	//3.072MHz / 384 = 8KHz 0:64 1:128 2:192 3:384
	SDADC->DEC.GAIN = 0;
	SDADC->ADCPDMA.RxDmaEn = 0;
	SDADC->ADCMPR[0].CMPEN = 0;
	SDADC->ADCMPR[0].CMPIE = 0;
	gsAdcTable.g_ptADCCallBack = ADCCBForPwrCheck;
	SDADC->INT.FIFO_IE_LEV = 4;	
	SDADC->INT.IE = 1;
	NVIC_EnableIRQ(ADC_IRQn);
	
		
//channel mux
	GPIOB->PMD.PMD6 = 0;     /* configure GPB6 as input mode */
	//DrvADC_SetAMUX(AMUX_GPIO, eDRVADC_MUXP_NONE, eDRVADC_MUXN_GPB6_SEL);		 
	ANA->AMUX.MIC_SEL = 0;
	ANA->AMUX.TEMP_SEL = 0;
	ANA->AMUX.MUXP_SEL = 0;
	ANA->AMUX.MUXN_SEL = 1 << 6; //PB6
	ANA->AMUX.EN = 1;

//PGA set	
	ANA->PGA_GAIN.GAIN = 8;//-6db

	ANA->PGAEN.BOOSTGAIN = 0;
	ANA->PGAEN.PU_IPBOOST = 1;
	ANA->PGAEN.PU_PGA = 1;
	ANA->PGAEN.REF_SEL = 1; //ref voltage:1.2V 

	ANA->SIGCTRL.MUTE_IPBOOST = 0;
	ANA->SIGCTRL.MUTE_PGA = 0;
	ANA->SIGCTRL.PU_MOD = 1;
	ANA->SIGCTRL.PU_IBGEN = 1;
	ANA->SIGCTRL.PU_BUFADC = 1;
	ANA->SIGCTRL.PU_BUFPGA = 1;

//ALC
	ALC->ALC_CTRL.ALCSEL = 0;		
	SDADC->EN = 1;
	i = 20000;//等待一会，让ADC采样结束
	while(i-- >0) ;
    SDADC->EN = 0;
	SYSCLK->APBCLK.ADC_EN = 0;
	gsAdcTable.g_ptADCCallBack = NULL;
	NVIC_DisableIRQ(ADC_IRQn);
	wResult = AdcValueConvert(adc_res);
//	LOG("sys volt: %d %d\n",adc_res, wResult);
	return wResult; 	
}

#define LOW_POWER_CHECK_ENABLE	(1)
#define POWER_CHECK_USE_ADC		(1)

//把[0:455]之间的电压值转换成mV
#define convert_2_mv(v) ((v - 37) * 100 / 13 + 108)

#define UP_RES		(4700)
#define DOWN_RES	(4700)
#define IN_RES		(107000)

#define DOWN_ALL_RES (DOWN_RES*IN_RES/(DOWN_RES + IN_RES))

#define CONVERT_IN_INRES(x)	(x*(UP_RES+DOWN_ALL_RES)/DOWN_ALL_RES + 100)		//100为和实际测量之间的差距
/*----------------------------------------------------------------------------------------
函数名: HW_PowerCheck
参数:
		None
返回值:
		wResult：采集出的样值
描述:
		ADC用于电量检测的初始化与采集样值
----------------------------------------------------------------------------------------*/
uint16_t HW_PowerCheck()
{
#if LOW_POWER_CHECK_ENABLE == 1	
#if POWER_CHECK_USE_ADC == 1 //使用ADC来检测系统电压，检测的引脚是PB6
	uint16_t wSysVol;
	wSysVol = AdcPwrCheck();

//	LOG("wSysVol:%d",CONVERT_IN_INRES(convert_2_mv(wSysVol)));

	return CONVERT_IN_INRES(convert_2_mv(wSysVol));
	
		
#else //CHECK_USE_ADC 使用内部的比较器来做电量检测，比较的值是1.2V
    SYS->GPB_ALT.GPB6 = 2; //设置PB6为比较器输入口
	SYSCLK->APBCLK.ANA_EN   = 1;/* Turn on analog peripheral clock */
	SYSCLK->APBCLK.ACMP_EN  = 1;
	//DrvACMP_Ctrl(CMP1, CMPCR_CN1_VBG, CMPCR_CMPIE_DIS, CMPCR_CMPEN_EN);
	ACMP->CMPCR[1].CMPCN = CMPCR_CN1_VBG;	//使用VBG 1.2V作为参考电压
	ACMP->CMPCR[1].CMPIE = CMPCR_CMPIE_DIS;	//关中断
	ACMP->CMPCR[1].CMPEN = CMPCR_CMPEN_EN;	//开始比较
	if(ACMP->CMPSR.CO1 == 0)
	{
		g_power_state = POWER_SHUTDOWN;	
		return 0;
	}
	else
	{
		g_power_state = POWER_SAFE;
		return 255;
	}
#endif //CHECK_USE_ADC
#else
	g_power_state = POWER_SAFE;
	return 255;
#endif
}
/*----------------------------------------------------------------------------------------
函数名: PowerCheck
参数:
		None
返回值:
		2/1/0：低电/严重低电/正常
描述:
		判断系统处于何种状态
----------------------------------------------------------------------------------------*/
uint8_t PowerCheck(void)
{
	int16_t sys_vol = 0;
	sys_vol = HW_PowerCheck();
	if(sys_vol < SERVER_VOL){
		g_bLowPower = 0;
		g_bServerLowPwr = 1;
		LedFlashForLowPwr(LED_ON);
		return POWER_SHUTDOWN;
	}else if(sys_vol < SAFE_VOL){
		g_bLowPower = 1;
		g_bServerLowPwr = 0;
		return POWER_LOW;
	}else{
		g_bLowPower = 0;
		g_bServerLowPwr = 0;
		LedFlashForLowPwr(LED_OFF);
		return POWER_SAFE;
	}
}
