
/*---------------------------------------------------------------------------------------------------------*/
/* This touch sensing method is only adopted on ISD-9160_TOUCH board.                                      */
/* Please also make sure the jumper(JP23) short															   */
/*---------------------------------------------------------------------------------------------------------*/
#include	"ivTouchSense.h"
//#include "icSystem.h"

#define TouchPad_Num				8
#define	TouchScanningPad			7		// PB.x for TouchPad

#define KEY_TEST_COUT				1

#define	BASE_COEF_Q					16
#define	BASE_UPDATE_COEF			((int32_t)(0.005*(1<<BASE_COEF_Q)))
#define	BASE_INIT_COEF				((int32_t)(1.05*(1<<BASE_COEF_Q)))
#define BASE_SUPRESS				((int32_t)(2.5*(1<<BASE_COEF_Q)))


// 跟喇叭距离比较近的按键的上门限设置一般为300，下门限一般为250
// 跟喇叭距离比较远的触控按键上门限一般为150，下门限一般为100
static const uint16_t g_PressHold_Def[TouchPad_Num][2] = 
{
	{150,100},				// Key1-PB0
	{150,100},				// Key2-PB1
	{150,100},				// Key3-PB2
	{150,100},				// Key4-PB3
	{150,100},				// Key5-PB4
	{150,100},				// Key6-PB5（脸部按键，距喇叭较近）
	{150,100},				// Key7-PB6（头部按键，距喇叭较近）
	{150,100}				// Key8-PB7
};


uint16_t i32CurTouchValue[8];
uint8_t i32TestCount[8];

uint32_t i32BaseTouchValue[8];	/* Q by BASE_COEF_Q */
uint32_t i32KeyStatus;
volatile uint32_t u8PadIndex;

//extern uint8_t temp_tick;
uint8_t scan_key;

/*-----------------------------------------------------------------------------

Start TouchSenseTrigger:
1. Reset Counter;
2. Enable Interrupt;
3. Enable Counter;

-----------------------------------------------------------------------------*/
void TouchSenseTrigger(void)
{
	DrvCAPSENSE_ResetCnt();
	DrvCAPSENSE_INT_Enable();
	DrvCAPSENSE_CntEnable();
}

/*---------------------------------------------------------------------------------------------------------*/
/* interrupt routines                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/  
#define		FILTER_COEF_Q		16
#define		FILTER_UPDATA_COF	((int32_t)(0.25*(1<<FILTER_COEF_Q)))

void CAPS_IRQHandler (void)
{
	int32_t f; 
//	int16_t tmp;
	DrvCAPSENSE_INT_Disable();
//	scan_key = 1;
//	GPIOB->PMD.PMD6 = IO_QUASI;
//	switch(u8PadIndex)
//	{
//		case 1:
//			sys.ptc_down = (GPIOB->PIN & (1 << 6)) ? 0 : 1;
//			break;
//		case 2:
//			sys.ptc_up = (GPIOB->PIN & (1 << 6)) ? 0 : 1;
//			break;
//		case 5:
//			GPIOB->PMD.PMD6 = IO_INPUT;
//			//sys.temp_cool = adc_pwr_test();
//			break;
//		case 0:
//			sys.open = (GPIOB->PIN & (1 << 6)) ? 1 : 0;
//			break;
//		case 7:
//			GPIOB->PMD.PMD6 = IO_INPUT;
//			//sys.temp_heat = adc_pwr_test();
//			break;
//		case 4:
//			sys.water = (GPIOB->PIN & (1 << 6)) ? 0 : 1;
//			break;
//		default:
//			break;
//	}

	f = DrvCAPSENSE_GetCnt()-i32CurTouchValue[u8PadIndex];
	f *= FILTER_UPDATA_COF;
	f >>= FILTER_COEF_Q;
	i32CurTouchValue[u8PadIndex] += f;

	//i32CurTouchValue[u8PadIndex] = DrvCAPSENSE_GetCnt();
	
	u8PadIndex++;
	
	if(u8PadIndex < TouchPad_Num)
	{

//		tmp = DrvGPIO_GetPortBits(GPA);
//		tmp = tmp & 0xf1ff;
//		tmp |= u8PadIndex << 9;
//		DrvGPIO_SetPortBits(GPA, tmp);
		
		TouchSenseTrigger();		
	}
	else
	{
//		temp_tick = 9;
//		scan_key = 0;
	}
}

/*-----------------------------------------------------------------------------

Init TouchSense:
1. Init GPIOB Input and Output;
2. Init Peripheral Module;
3. Get the Base Capacitance;

-----------------------------------------------------------------------------*/
void TouchSenseInit(void)			  	
{	
	int32_t i;
//	int16_t tmp;

	/* Set GPIOA 9~11*/
//	DrvGPIO_Open(GPA, 9, IO_OUTPUT);
//	DrvGPIO_Open(GPA, 10, IO_OUTPUT);
//	DrvGPIO_Open(GPA, 11, IO_OUTPUT);
	
//	GPIOB->PMD.PMD6 = IO_INPUT;
//	GPIOB->DOUT |= 1 << 6;
	
	// Set GPIOB to CapSense Model !!!
	//outpw(0x5000003C,0x0000AAAA);		
	//DrvGPIO_InitFunction(FUNC_ACMP0);
	//SYS->GPB_ALT.GPB0 	=2;
	//SYS->GPB_ALT.GPB1 	=2;
	//SYS->GPB_ALT.GPB2 	=2;
	//SYS->GPB_ALT.GPB3 	=2;
	//SYS->GPB_ALT.GPB4 	=2;
	//SYS->GPB_ALT.GPB5 	=2;
	//SYS->GPB_ALT.GPB6 	=2;
	SYS->GPB_ALT.GPB7 	=2;

	SYSCLK->APBCLK.ANA_EN   = 1;               /* Turn on analog peripheral clock */
	SYSCLK->APBCLK.ACMP_EN  = 1;
	
	// Set GPIOB to Input Model !!!
	//outpw(0x50004040,0xFFFF0000);		
	/*
	for(u8PadIndex=0;u8PadIndex<8;u8PadIndex++)
	{
		DrvGPIO_Open(GPB, u8PadIndex, IO_INPUT);
	}
	*/
	GPIOB->PMD.PMD0 = IO_OUTPUT;
	GPIOB->PMD.PMD1 = IO_OUTPUT;
	GPIOB->PMD.PMD2 = IO_OUTPUT;
	GPIOB->PMD.PMD3 = IO_OUTPUT;
	GPIOB->PMD.PMD4 = IO_OUTPUT;
	GPIOB->PMD.PMD5 = IO_OUTPUT;
	GPIOB->PMD.PMD6 = IO_OUTPUT;
	GPIOB->PMD.PMD7 = IO_INPUT;
	
	
    NVIC_EnableIRQ(CAPS_IRQn);

	// Set Analog Comapartor0 Enable!!!
	//ACMP->CMPCR[0].CMPEN = 1;
	//DrvACMP_ComparatorEn(CMP0);

	// Select Comparator0 negative input!!
	// 1=VMID reference voltage = VCC/2
	// 0=VBG, Bandgap reference voltage = 1.2V
	// Comparator0, Negative input is VBG=1.2V, 
	// Interrupt Disable, Comparator Enable
	DrvACMP_Ctrl(CMP0, CMPCR_CN0_VBG, CMPCR_CMPIE_DIS, CMPCR_CMPEN_EN);

	// Select master current for source generation!! 
	// 3=5uA, 2=2.5uA, 1=1uA, 0=0.5uA
	DrvCAPSENSE_ISRC_Val(3);

	// Set Number of PCLK cycles to discharge external capacitor.
	// 0=1cycle, 1=2cycles, 2=8cycles, 3=16cycles.
	// Set Peripheral performs 2^(CYCLE_CNT) relaxation cycles before generating
	// interrupt.
	DrvCAPSENSE_Ctrl(LOW_TIME_3,CYCLE_CNT_5);	
	
	u8PadIndex = 0;

//	tmp = DrvGPIO_GetPortBits(GPA);
//	tmp = tmp & 0xf1ff;
//	tmp |= u8PadIndex << 9;
//	DrvGPIO_SetPortBits(GPA, tmp);

	// Set the active analog GPIO input selected to Comparator 0 positive
	// input
	//ACMP->CMPSEL = u8PadIndex;	   		// Set up analog comapartor.
	ACMP->CMPSEL = TouchScanningPad;

	// Individually enable current source to GPIOB pins. 
	// Each GPIOB has a separate current source.
	// 1:Enable current source to pin GPIOB[x], 0:Disable;
	DrvCAPSENSE_ISRC_En(1<<TouchScanningPad);
	
	TouchSenseTrigger();

	for(i=0; i<10; i++)
	{
		while (u8PadIndex < TouchPad_Num);
		// Prepare Next Timer TouchScanning
		u8PadIndex = 0;	
		
//		tmp = DrvGPIO_GetPortBits(GPA);
//		tmp = tmp & 0xf1ff;
//		tmp |= u8PadIndex << 9;
//		DrvGPIO_SetPortBits(GPA, tmp);		

		TouchSenseTrigger();		
	}
	
	for(i=0; i<8; i++)	
	{
		i32BaseTouchValue[i] = i32CurTouchValue[i]*BASE_INIT_COEF;
//		LOG("i32BaseTouchValue[%d] is %d\r\n", i, i32CurTouchValue[i]);
	}
	
}

uint32_t DrvTouchScanning(void)
{
	int32_t i;
	uint32_t KeyValue = i32KeyStatus;
//	int16_t tmp;

	int bitMask = 1;
//	for(i=0; i<8; i++,bitMask<<=1){
		int32_t f;
		i = 7;
		f = i32BaseTouchValue[i] >> BASE_COEF_Q;
		if(0 == (KeyValue&bitMask)) /* No Touch */{
			if(i32CurTouchValue[i] > f+g_PressHold_Def[i][0]){
				if(i32TestCount[i]++ >= KEY_TEST_COUT){
					i32TestCount[i] = 0;
					KeyValue ^= bitMask;
//					LOG("Key %d Pressed!!\r\n", i);
				}
			}else{
				i32TestCount[i] = 0;
				// updata BaseTouchValue!! 
				// Base = Base*(1-a) + Cur*a!!
				f = i32CurTouchValue[i] - f;
				i32BaseTouchValue[i] += f*BASE_UPDATE_COEF;	
			}
		}else{							/* Touch */
			if(i32CurTouchValue[i] < f+g_PressHold_Def[i][1]){
				if(i32TestCount[i]++ >= KEY_TEST_COUT){
					i32TestCount[i] = 0;
					//i32BaseTouchValue[i] -= BASE_SUPRESS;
					KeyValue ^= bitMask;
//					LOG("Key %d Released!!\r\n", i);
				}
			}else{
				i32TestCount[i] = 0;
			}			
		}
//	}

	// Prepare Next Timer TouchScanning
	u8PadIndex = 0;	
//	tmp = DrvGPIO_GetPortBits(GPA);
//	tmp = tmp & 0xf1ff;
//	tmp |= u8PadIndex << 9;
//	DrvGPIO_SetPortBits(GPA, tmp);	
	TouchSenseTrigger();
	i32KeyStatus = KeyValue;
//	LOG("i32KeyStatus:%d\r\n",i32KeyStatus);
	return i32KeyStatus;
}
