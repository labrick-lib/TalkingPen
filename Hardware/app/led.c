#include "led.h"
#include "play.h"
#include "timer.h"

uint8_t g_bLedFlashForCam = 0;
uint8_t g_bLedFlashForCharging = 0;
uint8_t g_bAdapterConnected = 0;
uint8_t g_bLedFlashForNoEsring = 0;
uint8_t g_bLedFlashBeforeRec = 0;
uint8_t g_bLowPwrFlag = 0;

int16_t g_wMixDuty = 0,g_wMixCycle = 0;
/*----------------------------------------------------------------------------------------
函数名: TMRCBForLedCommon
参数:
		None
返回值:
		None
描述:
		LED闪烁通用的程序，去除充电时的呼吸灯
----------------------------------------------------------------------------------------*/ 
void TMRCBForLedCommon(void)
{
//	static int8_t bReverse = 0;
	static int16_t wCountForCam = 0,wCountForNoEsr = 0,wCountForBeforeRec = 0;//,wCountForCharge = 0
	static uint8_t cntFlashNum = 0;

	if( g_bLedFlashForCam ){				//cam
		if(!g_bLowPwrFlag)		RED_LED_OFF;
		else					RED_LED_ON;
		if(wCountForCam < 3*2)
			GREEN_LED_ON;
		else if(wCountForCam == 3*2)
			GREEN_LED_OFF;
		else if(wCountForCam >= 53*2){
			GREEN_LED_OFF;
			wCountForCam = -1;
		}
		wCountForCam++;
	}

	if( g_bLedFlashForNoEsring ){			//nocam:led flash 2s
		if(!g_bLowPwrFlag)		RED_LED_OFF;
		else					RED_LED_ON;
		if(wCountForNoEsr < 5){
			GREEN_LED_OFF;
		}else if( wCountForNoEsr == 5){
			GREEN_LED_ON;
		}else if( wCountForNoEsr > 10){
			GREEN_LED_OFF;
			wCountForNoEsr = -1;
			cntFlashNum++;
		}
		if(cntFlashNum == 2){
			GREEN_LED_ON;
			g_bLedFlashForNoEsring = 0;
			cntFlashNum = 0;
		}
		wCountForNoEsr++;
	}

	if( g_bLedFlashBeforeRec ){
		if(!g_bLowPwrFlag)		RED_LED_OFF;
		else					RED_LED_ON;
		if( wCountForBeforeRec < 5*2){
			GREEN_LED_ON;
		}else if( wCountForBeforeRec == 5*2 ){
			GREEN_LED_OFF;
		}else if( wCountForBeforeRec >= 10*2 ){
			GREEN_LED_OFF;
			wCountForBeforeRec = -1;
		}
		wCountForBeforeRec++;		
	}else wCountForBeforeRec = 0;

//	if( g_bAdapterConnected ){
//		if( g_bLedFlashForCharging ){		//charging
//			g_wMixCycle = 2*10;
//			if(!bReverse){
//				if(g_wMixDuty == g_wMixCycle)
//					bReverse = 1;
//				g_wMixDuty++;
//			}else{
//				if(g_wMixDuty == -1){
//					bReverse = 0;
//					g_bSwitchLedFlag = ~g_bSwitchLedFlag;
//				}
//				g_wMixDuty--;
//			}	
//		}else if( g_bLedFlashForCharging == 0 ){ 		//chargeover
//			g_wMixCycle = 2*10;
//			g_bSwitchLedFlag = 0;
//			if(!bReverse){
//				if(g_wMixDuty == g_wMixCycle)
//					bReverse = 1;
//				g_wMixDuty++;
//			}else{
//				if(g_wMixDuty == 0)
//					bReverse = 0;
//				g_wMixDuty--;
//			}	
//		}
//	}else{
//		g_wMixCycle = 0;
//	}
}
/*----------------------------------------------------------------------------------------
函数名: TMRCBForLedBreath
参数:
		None
返回值:
		None
描述:
		充电和充电完成时的呼吸灯效果
----------------------------------------------------------------------------------------*/
void TMRCBForLedBreath(void)
{
	static uint8_t nMixCount = 0;
	static uint8_t bReverse = 0;
	static uint16_t wCountForCharge = 0;
	static uint8_t g_bSwitchLedFlag = 0;

#define BREATH_CYCLE	100
	
	if( g_bAdapterConnected ){
		wCountForCharge++;
		if(wCountForCharge == BREATH_CYCLE){
			wCountForCharge = 0;
			if( g_bLedFlashForCharging ){		//charging
				g_wMixCycle = 10;
				if(!bReverse){
					if(g_wMixDuty >= g_wMixCycle)
						bReverse = 1;
					else if(g_wMixDuty >= 10)
						g_wMixDuty += 2;
					else
						g_wMixDuty++;
				}else{
					if(g_wMixDuty <= 0){
						bReverse = 0;
						g_bSwitchLedFlag = ~g_bSwitchLedFlag;
					}
					else if(g_wMixDuty >= 10)
						g_wMixDuty -= 2;
					else
						g_wMixDuty--;
				}
			}else if( g_bLedFlashForCharging == 0 ){ 		//chargeover
				g_wMixCycle = 10;
				g_bSwitchLedFlag = 0;
				if(!bReverse){
					if(g_wMixDuty >= g_wMixCycle)
						bReverse = 1;
					else if(g_wMixDuty > 10)
						g_wMixDuty += 2;
					else
						g_wMixDuty++;
				}else{
					if(g_wMixDuty <= 0)
						bReverse = 0;
					else if(g_wMixDuty > 10)
						g_wMixDuty -= 2;
					else if(g_wMixDuty > 0)
						g_wMixDuty--;
				}	
			}
		}
	}else{
		g_wMixCycle = 0;
	}

	if(g_wMixCycle){
		nMixCount++;
		if(nMixCount < g_wMixDuty){
			if(!g_bSwitchLedFlag){
				GREEN_LED_ON;
				RED_LED_OFF;
			}else{
				GREEN_LED_OFF;
				RED_LED_ON;
			}
		}else if(nMixCount == g_wMixDuty){
			GREEN_LED_OFF;
			RED_LED_OFF;
		}else if(nMixCount >= g_wMixCycle){		//规定为一个LED的明亮周期；
			GREEN_LED_OFF;
			RED_LED_OFF;
			nMixCount = 0;
		}
	}
}
/*----------------------------------------------------------------------------------------
函数名: LedInit
参数:
		None
返回值:
		None
描述:
		初始化GPIO、初始变量、定时器
----------------------------------------------------------------------------------------*/
void LedInit(void)
{
	DrvGPIO_Open(GPB,GREEN_LED,IO_OUTPUT); 			//绿色灯
	GREEN_LED_OFF;

	DrvGPIO_Open(GPB,RED_LED,IO_OUTPUT);		   	//红色灯
	RED_LED_OFF;

	DrvGPIO_Open(GPA,9,IO_OUTPUT);		   	//测试使用
	GPIOA->DOUT &= ~(1 << 9);

	g_bLedFlashForCam = 0;
	g_bLedFlashForCharging = 0;
	g_bAdapterConnected = 0;
	g_bLedFlashForNoEsring = 0;
	g_bLedFlashBeforeRec = 0;

	g_wMixDuty = 0;
	g_wMixCycle = 0;

	DrvTIMER_SetTimerEvent(TMR1,50, (TIMER_CALLBACK)TMRCBForLedCommon,0);
	DrvTIMER_SetTimerEvent(TMR1,1, (TIMER_CALLBACK)TMRCBForLedBreath,0);
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForCam
参数:
		bFlag：摄像头LED开关 LED_ON/LED_OFF 开/关
返回值:
		None
描述:
		摄像头可操作：green0.3->green5s闪烁，不可操作关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashForCam(uint8_t bFlag) 		//green:0.3s--5s flash_on
{
	if(!bFlag){
		g_bLedFlashForCam = 0;
		RED_LED_OFF;
		GREEN_LED_OFF;
	}else if(bFlag){
		g_bLedFlashForCam = 1;
//		wCountForCam = 0;
	}
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForEsr
参数:
		bFlag：Esr状态LED开关（同时也用作了录音状态提示） LED_ON/LED_OFF 开/关
返回值:
		None
描述:
		Esr and record状态：green常亮，结束关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashForEsr(uint8_t bFlag)		//green:long
{
	if(!bFlag)
		GREEN_LED_OFF;
	else if(bFlag)
		GREEN_LED_ON;
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForNoEsr
参数:
		None
返回值:
		None
描述:
		拒识之后：Green0.25s->0.25s闪烁两次
----------------------------------------------------------------------------------------*/
void LedFlashForNoEsr()		//拒识	green:0.25s-0.25s -> 0.25s--0.25s
{
	g_bLedFlashForNoEsring = 1;
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForLowPwr
参数:
		bFlag：低电状态LED开关 LED_ON/LED_OFF 开/关
返回值:
		None
描述:
		低电状态：red常亮，结束关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashForLowPwr(uint8_t bFlag) 	//red:long
{
	if(!bFlag)
		RED_LED_OFF;
	else if(bFlag){
		RED_LED_ON;
		g_bLowPwrFlag = 1;
	}
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForCharge
参数:
		None
返回值:
		None
描述:
		充电完成状态：green1s->red1s呼吸闪烁，结束关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashForCharge()	  	//green:1s -> red:1s -> green:1s .... 
{
	g_bAdapterConnected = 1;
	g_bLedFlashForCharging = 1;	
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashForChargeover
参数:
		None
返回值:
		None
描述:
		充电完成状态：green1s->1s呼吸闪烁，结束关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashForChargeover()	//green:1s -> green:1s -> green:1s ....
{
	g_bAdapterConnected = 1;
	g_bLedFlashForCharging = 0;
}
/*----------------------------------------------------------------------------------------
函数名: AdapterDisconnect
参数:
		None
返回值:
		None
描述:
		操作电源适配器是否断开，执行这个函数说明适配器已经断开
----------------------------------------------------------------------------------------*/
void AdapterDisconnect(void)
{
	g_bAdapterConnected = 0;
	GREEN_LED_OFF;
	RED_LED_OFF;
}
/*----------------------------------------------------------------------------------------
函数名: LedFlashBeforeRec
参数:
		bFlag：状态LED开关 LED_ON/LED_OFF 开/关
返回值:
		None
描述:
		录音之前的一段闪烁状态：green0.5s->0.5s闪烁，结束关闭LED
----------------------------------------------------------------------------------------*/
void LedFlashBeforeRec(uint8_t bFlag)
{
	if(bFlag){
		g_bLedFlashBeforeRec = 1;
//		wCountForBeforeRec = 0;
	}else{ 
		g_bLedFlashBeforeRec = 0;
		RED_LED_OFF;
		GREEN_LED_OFF;
	}

}
/*----------------------------------------------------------------------------------------
函数名: LedUninit
参数:
		None
返回值:
		None
描述:
		LEDGPIO口的逆初始化，关闭LED
----------------------------------------------------------------------------------------*/
void LedUninit()
{
	DrvGPIO_SetBit(GPB,GREEN_LED);
	DrvGPIO_SetBit(GPB,RED_LED);
}


