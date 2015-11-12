#include "timer.h"

#define USE_TIMER	0
uint32_t g_cntSysTicks = 0; //没有动作时进行计时；
uint32_t g_cntWaitForRecTicks = 0;		//按住录音键，10s之内获取不到正确的码值结束录音
uint32_t g_cntPwrCheckTicks = 10;
uint8_t g_bRecordStart = 0;

uint8_t g_bHaveKeyAction = 0;
uint8_t g_bHavePlayAction = 0;
uint8_t g_bHaveEsrAction = 0;
uint8_t g_bHaveRecAction = 0;
uint8_t g_bHaveCamAction = 0;

uint8_t g_bContinueTick = 0;

uint8_t g_bLowPower = 0;
uint8_t g_bServerLowPwr = 0;
uint32_t g_cntLowPwrTicks = 0;

uint8_t g_bReminderAudio = 0;
uint8_t g_bShutDown = 0;
uint8_t g_bCheckPower = 0;
uint8_t g_bLowPwrAudio = 0;

extern uint8_t g_nState;

//int16_t abc = 0,abc1 = 0;
/*----------------------------------------------------------------------------------------
函数名: icTimerInit
参数:
		None
返回值:
		None
描述:
		对所选择的定时器进行硬件初始化
----------------------------------------------------------------------------------------*/
void icTimerInit(void)
{
#if USE_TIMER == 0
	// 开定时器0
	SYSCLK->CLKSEL1.TMR0_S = 4; // 选择48M计时频率
	SYSCLK->APBCLK.TMR0_EN = 1; // 开时钟
	outpw((uint32_t)&TIMER0->TCSR ,0 ); // 关闭定时器
	TIMER0->TISR.TIF = 1; // 清除中断标志
	TIMER0->TCMPR = 49152; // 计数10 频率49.152MHz 得1KHZ
	TIMER0->TCSR.PRESCALE = 0;
	outpw((uint32_t)&TIMER0->TCSR,  (uint32_t)((1 << 30) | (1 << 29) | (1 << 27))); // 使能定时器，使能中断，模式01周期重复计数
	NVIC_EnableIRQ(TMR0_IRQn);
#else
	// 开定时器1
	SYSCLK->CLKSEL1.TMR1_S = 0; // 选择10K计时频率
	SYSCLK->APBCLK.TMR1_EN =1; // 开时钟
	outpw((uint32_t)&TIMER1->TCSR ,0 ); // 关闭定时器
	TIMER1->TISR.TIF = 1; // 清除中断标志
	TIMER1->TCMPR = 10; // 计数10 频率10KHZ 得1KHZ
	TIMER1->TCSR.PRESCALE = 0;
	outpw((uint32_t)&TIMER1->TCSR,  (uint32_t)((1 << 30) | (1 << 29) | (1 << 27))); // 使能定时器，使能中断，模式01周期重复计数
	NVIC_EnableIRQ(TMR1_IRQn);
#endif

}
/*----------------------------------------------------------------------------------------
函数名: icTimerTerm
参数:
		None
返回值:
		None
描述:
		对所选择的定时器进行硬件逆初始化
----------------------------------------------------------------------------------------*/
void icTimerTerm(void)
{
#if USE_TIMER == 0
	// 关闭定时器0
	TIMER0->TCSR.IE = 0; // 关中断
	TIMER0->TCSR.CRST = 1; //重置定时计数 停用定时器
	SYSCLK->APBCLK.TMR0_EN = 0; // 关时钟
	NVIC_DisableIRQ(TMR0_IRQn);
#else
	// 关闭定时器1
	TIMER1->TCSR.IE = 0; // 关中断
	TIMER1->TCSR.CRST = 1; //重置定时计数 停用定时器
	SYSCLK->APBCLK.TMR1_EN = 0; // 关时钟
	NVIC_DisableIRQ(TMR1_IRQn);
#endif
}

#if USE_TIMER == 0
void TMR0_IRQHandler(void)
#else
void TMR1_IRQHandler(void)
#endif
{
#if USE_TIMER == 0
	TIMER0->TISR.TIF =1; // 清除中断标志
#else
	TIMER1->TISR.TIF =1; // 清除中断标志
#endif		
}

/*----------------------------------------------------------------------------------------
函数名: TMRCBForSysTick
参数:
		None
返回值:
		None
描述:
		定时器回调函数，用于空闲计时、录音等待计时、识别计时、电量检测计时等等计时操作
----------------------------------------------------------------------------------------*/
void TMRCBForSysTick(void)
{
	g_cntSysTicks++;
	if(g_bHavePlayAction && !g_bContinueTick){
		g_cntSysTicks = 0;
		g_bHavePlayAction = 0;
	}
	if(g_bHaveKeyAction || g_bHaveEsrAction || g_bHaveRecAction || g_bHaveCamAction){
		g_cntSysTicks = 0;
		g_bHaveKeyAction = 0;
		g_bHaveEsrAction = 0;
		g_bHaveRecAction = 0;
		g_bHaveCamAction = 0;
		g_bContinueTick = 0;
	}

	//------------------------------
	if(g_bRecordStart)
		g_cntWaitForRecTicks++;
	else
		g_cntWaitForRecTicks = 0;

	//-----------------------------10s钟检测一次电量
	g_cntPwrCheckTicks++;
	if(g_cntPwrCheckTicks >= 10){
		g_bCheckPower = 1;
		g_cntPwrCheckTicks = 0;
	}
	//-----------------------------
	if(!(DrvGPIO_GetBit(GPA,15))){
		if(g_bLowPower){
			RED_LED_ON;
			if(g_cntLowPwrTicks % LOWPWR_REMINDER_TIME == 0)	g_bLowPwrAudio = 1;
			g_cntLowPwrTicks++;
		}else if(g_bServerLowPwr){
			g_cntLowPwrTicks = LOW_POWER_TIME;
		}else
			g_cntLowPwrTicks = 0;
	}

//	LOG("--%d\t%d\r\n",abc,abc1);
//	abc = 0;
//	abc1 = 0;
}
/*----------------------------------------------------------------------------------------
函数名: GetSysTicks
参数:
		None
返回值:
		g_cntSysTicks：系统定时器的时间
描述:
		用于获取系统定时器的空闲时间（也可以是等待时间）
----------------------------------------------------------------------------------------*/
int16_t GetSysTicks(void)
{
	return g_cntSysTicks;
}
/*----------------------------------------------------------------------------------------
函数名: GetWaitForRecTicks
参数:
		None
返回值:
		g_cntWaitForRecTicks：系统定时器的时间
描述:
		获取从进入录音模式到开始录音的时间
----------------------------------------------------------------------------------------*/
int16_t GetWaitForRecTicks(void)
{
	return g_cntWaitForRecTicks;
}
/*----------------------------------------------------------------------------------------
函数名: TimerInit
参数:
		None
返回值:
		None
描述:
		初始化定时器，并创建第一个虚拟定时器：1s为周期
----------------------------------------------------------------------------------------*/
void TimerInit(void)
{
	DrvTIMER_Init();
	DrvTIMER_Open(TMR1, 1000, PERIODIC_MODE);
	DrvTIMER_SetTimerEvent(TMR1,1000, (TIMER_CALLBACK)TMRCBForSysTick,0);
	DrvTIMER_EnableInt(TMR1);
	DrvTIMER_Ioctl(TMR1, TIMER_IOC_START_COUNT, 0);
}
/*----------------------------------------------------------------------------------------
函数名: CommonCheck
参数:
		None
返回值:
		None
描述:
		进行一些常规的检测：提示音、关机、电量检测、低电处理等
----------------------------------------------------------------------------------------*/
void CommonCheck(void)
{
//	if(g_cntSysTicks == REMINDER_TIME){
//		g_cntSysTicks = REMINDER_TIME + 1;
//		g_bReminderAudio = 0;
//		g_bContinueTick = 1;
//		PlayBegin(PEN_TIMEOUT_AUDIO);
//	}
	if(g_cntSysTicks >= SHUTDOWN_TIME){
		g_bShutDown = 0;
		g_bContinueTick = 0;
		g_nState = SLEEP_STATE;
		g_cntSysTicks = 0;
	}

	if(g_bCheckPower){
		g_bCheckPower = 0;
		PowerCheck();
	}
	if(g_bLowPwrAudio)
	{
		g_bLowPwrAudio = 0;
		PlayBegin(PEN_LOWPWR_AUDIO);
		g_nState = PWRDOWN_STATE;		
	}
//	if(g_cntLowPwrTicks >= LOW_POWER_TIME)		//这个暂时不需要（电压更低）
//	{
//		g_nState = SLEEP_STATE;
//	}
}

