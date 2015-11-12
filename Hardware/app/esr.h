#ifndef PRP_ESR_H__YN
#define PRP_ESR_H__YN

//#include "DrvTimer.h"
//#include "DrvI2S.h"
//#include "DrvPDMA.h"
//#include "DrvDPWM.h"
 
#include "ivADPCM.h"
#include "redirect.h"

#include "ivESR.h"
#include "ESRRes.h"	
#include "ESRResForVAD.h"
#include "play.h"
//#include "timer.h"
//#include "icSystem.h"
//#include "icADC.h"


#define AUDIO_START		19
#define AUDIO_REJECT	20

typedef enum{
	REC_PURE,
	REC_MAVO,
}REC_MODE;

typedef enum{
	ESR_MAJOR_NET,
	ESR_MINOR_NET,
}ESR_NET;

enum MINIOR_AUIO{
	ESR_MINOR_AUDIO1 = 	99821,
	ESR_MINOR_AUDIO2,
	ESR_MINOR_AUDIO3
};

int32_t GetEsrCode(ESR_NET EsrNet);
void RecordBegin(int32_t dwRecCode,REC_MODE RecMode);
void PdmaForAdc(void);
uint8_t CodeIsExist(int32_t dwRecCode, uint16_t* wCodeIndex);
void ClrESRMemory(void);

//-------------------------------------录音
typedef struct tagRECHEADER
{
	short	 rec_max_value;
	uint32_t rec_audio_size;	
	struct tagADPCMEncoder rec_encoder; 
}RECHEADER,*PRECHEADER;

#define FLASH_PAGE_SIZE			(256)
#define FLASH_SECTOR_SIZE		(0x1000)		//4K
#define VALUE_4K				(0x1000)
#define VALUE_64K				(0x10000)
//#define RECORD_AUDIO 			(RECORD_START+FLASH_SECTOR_SIZE)

#define REC_TIME				(60)		//s
#define PAGE_NUM				(REC_TIME * 16000 / 4 / 256)

#define REC_CODE_SIZE			(1*1024)		//1K的索引地址
#define REC_INDEX_SIZE			(2*4*1024)		//索引的大小

//--------------------------NEW REC
#define REC_CODE_START	(0)		//(10000)				//从code一万开始支持录音
#define SUPPORT_REC_CNT	(5*4*1024/2 - 1)	//所支持的录音数
#define REC_CODE_END	(REC_CODE_START + 100001) //SUPPORT_REC_CNT)

#define SPI_FLASH_SIZE	(SPI_FLASH_MSIZE * 0x100000)

#endif
