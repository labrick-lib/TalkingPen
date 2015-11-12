#ifndef __ADC_H__N572_iFLYTEK
#define __ADC_H__N572_iFLYTEK

#include "stdint.h"

#ifdef __cplusplus
extern "C"{
#endif
typedef struct adc_start_argv adc_start_argv;
/* ”…FIFO»∑∂® */
#define ADC_DATA_COUNT 4

struct adc_start_argv
{
	uint32_t sample_rate;	 
//	DRVADC_ADC_CALLBACK *cb;
	uint32_t cbparam;	  
	int16_t gain_agc_off;
	int8_t agc_enable;
	int8_t agc_max_level;
	int8_t agc_min_level;	  
};

//typedef struct PDMA_Start_argv PDMA_Start_argv;
//struct PDMA_Start_argv
//{
//    S_DRVPDMA_CH_ADDR_SETTING 	sSrcAddr;    		/* Source Address */
//	S_DRVPDMA_CH_ADDR_SETTING 	sDestAddr;			/* Destination Address */
//    E_DRVPDMA_TRANSFER_WIDTH  	u8TransWidth;		/* Transfer Width */
//    E_DRVPDMA_MODE 			  	u8Mode;				/* Operation Mode */
//	E_DRVPDMA_WRA_INT_EN	  	u8WrapBcr;			/* Wrap Mode */
//	int32_t              	  	i32ByteCnt;			/* Byte Count */
//	PFN_DRVPDMA_CALLBACK		CB;					/* CallBack */
//	E_DRVPDMA_CHANNEL_INDEX		e_Channel;			/* Channel Select */
//};


/*
 * Function declarations
 */
void ADC_Init(void);
void ADC_Term(void);
void ADC_Start(adc_start_argv *args);
//void PDMA0ForADC(PDMA_Start_argv *sPDMA);
void ADC_Stop(void);
void ADC_Read(uint16_t *pdata);


#ifdef  __cplusplus
}
#endif


#endif	/* !__ADC_H__N572_iFLYTEK */
