#include "hw.h"

extern uint8_t g_nState;

//static void Esr_ADC_Config(adc_start_argv *args);
//static uint32_t ADC_Open(S_DRVADC_PARAM *sParam);
// 从Esr_Adc.c 拷贝过来 并进行了修改


/*static void Esr_delay(int32_t i32TimeOut)
{
	while(1)
	{
		if(i32TimeOut-- <= 0)
		{
			break;
		}
	}
}*/
uint32_t ADC_Open(S_DRVADC_PARAM *sParam)
{
    /* reset ADC */
    SYS->IPRSTC2.ADC_RST = 1;
    SYS->IPRSTC2.ADC_RST = 0;

	/* Set ADC divisor */
	SYSCLK->CLKDIV.ADC_N = 0;

	/* ADC engine clock enable */
    SYSCLK->APBCLK.ADC_EN = 1;

	/* ADC enable */
    SDADC->EN = 1;
	
	SDADC->CLK_DIV = sParam->u8SDAdcDivisor;;
	SDADC->DEC.OSR = eDRVADC_OSR_128;

	/* Set FIFO interrupt level */
	SDADC->INT.FIFO_IE_LEV = ADC_DATA_COUNT;

    /* Set BIQ to default state */
	BIQ->BIQ_CTRL.RSTn = 1;

    /* Set adc input channel */
	//DrvADC_SetAdcChannel(sParam->eInputSrc, sParam->eInputMode);
   ANA->AMUX.EN       = 1;
   ANA->AMUX.MIC_SEL  = 1;
   ANA->AMUX.TEMP_SEL = 0;
   ANA->AMUX.MUXP_SEL = 0;
   ANA->AMUX.MUXN_SEL = 0;

	return 0;		
}
/*---------------------------------------------------------------------------------------------------------*/
/* InitialADC                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_Init1(void)
{
	S_DRVADC_PARAM sParam;
	
	/* Open Analog block */
	DrvADC_AnaOpen();

	/* Power control */
	DrvADC_SetPower( 
		eDRVADC_PU_MOD_ON, 
		eDRVADC_PU_IBGEN_ON, 
		eDRVADC_PU_BUFADC_ON, 
		eDRVADC_PU_BUFPGA_ON, 
		eDRVADC_PU_ZCD_OFF);

	/* PGA Setting */
	DrvADC_PGAMute(eDRVADC_MUTE_PGA);
	DrvADC_PGAUnMute(eDRVADC_MUTE_IPBOOST);
	DrvADC_SetPGA(	
	    eDRVADC_REF_SEL_VMID,
	    eDRVADC_PU_PGA_ON,
	    eDRVADC_PU_BOOST_ON,
	    eDRVADC_BOOSTGAIN_26DB);

	/* MIC circuit configuration */
	DrvADC_SetVMID(
		eDRVADC_PULLDOWN_VMID_RELEASE,
		eDRVADC_PDLORES_CONNECTED,
		eDRVADC_PDHIRES_DISCONNECTED);
	DrvADC_SetMIC(TRUE, eDRVADC_MIC_BIAS_SEL_2);	

	sParam.u8SDAdcDivisor = 24;
	sParam.eOSR		  = eDRVADC_OSR_128;
	//sParam.eInputSrc  = eDRVADC_MIC;
	//sParam.eInputMode = eDRVADC_DIFFERENTIAL;
	//sParam.u8ADCFifoIntLevel = ADC_DATA_COUNT;
	//DrvADC_Open(&sParam);
	ADC_Open(&sParam);
}
void ADC_Init(void)
{
	S_DRVADC_PARAM sParam;
	
	// b0,b1,b2,a1,a2,b0,b1,b2,a1,a2,b0,b1,b2,a1,a2
//	uint32_t u32BiqCoeff[15] = {0x058b0, 0x7a856, 0x058af, 0x6e75d, 0x07258,
//								0x0abbe, 0x77394, 0x0abbe, 0x6f2bb, 0x0d8e3,
//								0x04cc2, 0x02a8b, 0x04c0d, 0x6f69a, 0x0f9da};
	
	/* Open Analog block */
	DrvADC_AnaOpen();
	
	/* Power control */
	DrvADC_SetPower( 
	    eDRVADC_PU_MOD_ON, 
	    eDRVADC_PU_IBGEN_ON, 
	    eDRVADC_PU_BUFADC_ON, 
	    eDRVADC_PU_BUFPGA_ON, 
	    eDRVADC_PU_ZCD_OFF);
	
	/* PGA Setting */   
	DrvADC_PGAMute(eDRVADC_MUTE_PGA);
	DrvADC_PGAUnMute(eDRVADC_MUTE_IPBOOST);
	DrvADC_SetPGA(  
	    eDRVADC_REF_SEL_VMID,
	    eDRVADC_PU_PGA_ON,
	    eDRVADC_PU_BOOST_ON,
	    eDRVADC_BOOSTGAIN_26DB);
	
	DrvADC_SetPGAGaindB(1200);   // 2750 dB
	
	/* MIC circuit configuration */
	DrvADC_SetVMID(
	    eDRVADC_PULLDOWN_VMID_RELEASE,
	    eDRVADC_PDLORES_CONNECTED,
	    eDRVADC_PDHIRES_DISCONNECTED);
	DrvADC_SetMIC(TRUE, eDRVADC_MIC_BIAS_75_VCCA);
	
	/* ALC Setting */   
	ALC->ALC_CTRL.NGTH		= 3;
	ALC->ALC_CTRL.NGEN		= 0;
	ALC->ALC_CTRL.ALCATK	= 2;
	ALC->ALC_CTRL.ALCDCY	= 6;
	ALC->ALC_CTRL.ALCMODE	= 0;
	ALC->ALC_CTRL.ALCLVL	= 15;
	ALC->ALC_CTRL.ALCHLD	= 0;		//original 0 modified to 10 by YN 2014-01-18：14:30
	ALC->ALC_CTRL.ALCZC		= 0;
	ALC->ALC_CTRL.ALCMIN	= 0;
	ALC->ALC_CTRL.ALCMAX	= 3;		//original 7 modified to 3 by YN 2014-03-14：09:49
	ALC->ALC_CTRL.ALCPKLIM	= 1;
	ALC->ALC_CTRL.ALCSEL	= 1;
	
	/* Open ADC block */
	sParam.u8AdcDivisor		= 0;
	sParam.u8SDAdcDivisor	= 24;
//	sParam.u8SDAdcDivisor	= 48;
	sParam.eOSR				= eDRVADC_OSR_128;
	sParam.eInputSrc		= eDRVADC_MIC;
	sParam.eInputMode		= eDRVADC_DIFFERENTIAL;
	sParam.u8ADCFifoIntLevel= 7;
	DrvADC_Open(&sParam);
	
	/* Change Decimation and FIFO Setting */
	//DrvADC_SetAdcOverSamplingClockDivisor(u8SDAdcDivisor);
	//DrvADC_SetOverSamplingRatio(eOSR);
	//DrvADC_SetCICGain(u8CICGain);
	//DrvADC_SetFIFOIntLevel(u8ADCFifoIntLevel);
	
	/* Change BIQ Setting */            
	SYSCLK->APBCLK.BIQ_EN = 0;
	SYS->IPRSTC2.BIQ_RST = 1;
	SYS->IPRSTC2.BIQ_RST = 0;
	//DrvADC_SetBIQ(1023, 1, eDRVADC_BIQ_IN_ADC, u32BiqCoeff);
	
	/* Interrupt Setting */
	//DrvADC_EnableAdcInt(DRVADC_ADC_CALLBACK Callback, uint32_t u32UserData);
	
	DrvADC_PGAUnMute(eDRVADC_MUTE_PGA);
}

void ADC_Term(void)
{
	DrvADC_Close();
}

//void ADC_Start(adc_start_argv *args)
//{
//	Esr_ADC_Config(args);
//	DrvADC_EnableAdcInt(args->cb, args->cbparam);
//	DrvADC_StartConvert();
//}


void ADC_Stop()
{

	DrvADC_StopConvert();
	DrvADC_DisableAdcInt();
	DrvADC_Close();
	//DrvADC_AnaClose();
}

void ADC_Read(uint16_t *pdata)
{
	pdata[0] = (SDADC->ADCOUT);
	pdata[1] = (SDADC->ADCOUT);
	pdata[2] = (SDADC->ADCOUT);
	pdata[3] = (SDADC->ADCOUT);
}

//static void Esr_Agc_config(int8_t Agc_Max_level, int8_t Agc_Min_level)
//{
//
//	SYSCLK->APBCLK.BIQ_EN = 1;
//	BIQ->BIQ_CTRL.RSTn = 1;
//	//BIQ->BIQ_CTRL.RSTn = 0; // not use biq
//	//(*((volatile unsigned long*)0x400B0048)) = 0xFF01A360;
//
//	/* Set ALC Noise gate threshold */
//	DrvALC_SetNGTH(0x3);						  //
//
//	/* Set ALC Noise get Enable */
//	DrvALC_SetNGEN(0x1);
//
//	/* Set ALC attack time */
//	DrvALC_SetAttackTime(2);					  //2ms
//
//	/* Set ALC decay time */
//	DrvALC_SetDecayTime(6);						  //0~10: 125*2^N = 8ms
//
//	/* Set ALC mode */
//	DrvALC_SetMode(0);							  // ALC normal operation mode
//
//	/* Set ALC target level */
//	DrvALC_SetTargetLevel(0xf);					  // -6dB
//
//	/* Set ALC hold time */
//	DrvALC_SetHoldTime(10);						  // 1S
//
//	/* Set ALC zero crossing */
//	DrvALC_SetZeroCrossing(0);					  // Disabled
//
//	/* Set ALC minimum gain */
//	DrvALC_SetMinGain(Agc_Min_level);						  // -12dB
//
//	/* Set ALC maximum gain */
//	DrvALC_SetMaxGain(Agc_Max_level);						  // 
//
//	/* Set ALC Noise Gete Enable */
//	DrvALC_EnableNoiseGate(0,0);	   				  // Disable
//
//	/* Set ALC peak limiter */
//	DrvALC_SetALCpeakLimiter(1);				  // Enable
//
//	/* Set ALC gain peak detector select */
//												  // absolute peak value for training
//	/* ALC gain peak detector select */
//	DrvALC_SetALCselect(1);		 				  // Enable




	/* ALC Setting */
//	SYSCLK->APBCLK.BIQ_EN = 1;
//	BIQ->BIQ_CTRL.RSTn = 1;
//	(*((volatile unsigned long*)0x400B0048)) = 0x3E01E82F;/* 0x7ec02360; 0xFF01E360; target level */
//	ALC->ALC_CTRL.NGTH = (0x3 & 0x07);	/* Set ALC Noise gate threshold = -63dB*/
//	ALC->ALC_CTRL.NGEN = (0x1 & 0x01);	/* Set ALC Noise gate Enable */
//	ALC->ALC_CTRL.ALCATK = 2;			/* Set ALC attack time Normal mode :  Time = 500us * 2^2 = 2ms */
//	ALC->ALC_CTRL.ALCDCY = 7;			/* Set ALC decay time Normal mode :  Time = 125us * 2^7 = 16ms */
//	ALC->ALC_CTRL.ALCMODE = (0 & 0x01);	/* Set ALC mode = Normal mode*/
//    ALC->ALC_CTRL.ALCLVL = (0xf);		/* Set ALC target level = -28.5 + 1.5*N (dB) = -6dB */
//	ALC->ALC_CTRL.ALCHLD = (0);			/* Set ALC hold time  0ms*/	
//	ALC->ALC_CTRL.ALCZC	= 0;			/* Set ALC Zero Crossing disable */
//	ALC->ALC_CTRL.ALCMIN = Agc_Min_level;	/* Set ALC minimum gain level*/
//	ALC->ALC_CTRL.ALCMAX = Agc_Max_level;	/* Set ALC maximum gain level*/
//	ALC->ALC_CTRL.ALCPKLIM = (0 & 0x01);/* Set ALC peak limiter Enable*/
//	ALC->ALC_CTRL.ALCSEL = (1 & 0x01);	/* Set ALC gain peak detector select Enable */
//}

/*----------------------------------------------------------------------------*/
/* Initial ADC for ESR                                                        */
/*----------------------------------------------------------------------------*/
//static void Esr_ADC_Config(adc_start_argv *args)
//{
//	S_DRVADC_PARAM sParam;
//
//	DrvADC_SetPGAGaindB(args->gain_agc_off);	  // 12 dB	  3525
//	
// 	/* Open ADC block */
//	//sParam.u8SDAdcDivisor = 16;	//OSR64  :16 for 48K
//	//sParam.u8SDAdcDivisor = 16;	//OSR128 :48 for 8K, 24 for 16K
//	//sParam.u8SDAdcDivisor = 16	//OSR192 :32 for 8K, 16 for 16K
//	sParam.u8AdcDivisor   = 0;
//	if (args->sample_rate == 16000) {
//		sParam.u8SDAdcDivisor = 24;
//	} else {
//		sParam.u8SDAdcDivisor = 48;
//	}
//	//sParam.eOSR		  = eDRVADC_OSR_128;
//	//sParam.eInputSrc  = eDRVADC_MIC;
//	//sParam.eInputMode = eDRVADC_DIFFERENTIAL;
//	//sParam.u8ADCFifoIntLevel = ADC_DATA_COUNT;
//	//DrvADC_Open(&sParam);
//	ADC_Open(&sParam);//代替原DrvADC.c中的DrvADC_Open，减少代码量
//	
//	//Esr_delay(0x20000);
//
//	if(args->agc_enable)
//	{
//		Esr_Agc_config(args->agc_max_level, args->agc_min_level);
//	}
//	DrvADC_PGAUnMute(eDRVADC_MUTE_PGA);
//}

//uint32_t ADC_Open(S_DRVADC_PARAM *sParam)
//{
//    /* reset ADC */
//    SYS->IPRSTC2.ADC_RST = 1;
//    SYS->IPRSTC2.ADC_RST = 0;
//
//	/* Set ADC divisor */
//	SYSCLK->CLKDIV.ADC_N = 0;
//
//	/* ADC engine clock enable */
//    SYSCLK->APBCLK.ADC_EN = 1;
//
//	/* ADC enable */
//    SDADC->EN = 1;
//	
//	SDADC->CLK_DIV = sParam->u8SDAdcDivisor;;
//	SDADC->DEC.OSR = eDRVADC_OSR_128;
//
//	/* Set FIFO interrupt level */
//	SDADC->INT.FIFO_IE_LEV = ADC_DATA_COUNT;
//
//    /* Set BIQ to default state */
//	BIQ->BIQ_CTRL.RSTn = 1;
//
//    /* Set adc input channel */
//	//DrvADC_SetAdcChannel(sParam->eInputSrc, sParam->eInputMode);
//   ANA->AMUX.EN       = 1;
//   ANA->AMUX.MIC_SEL  = 1;
//   ANA->AMUX.TEMP_SEL = 0;
//   ANA->AMUX.MUXP_SEL = 0;
//   ANA->AMUX.MUXN_SEL = 0;
//
//	return 0;		
//}
