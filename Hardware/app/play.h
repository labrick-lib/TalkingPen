#ifndef PRP_APP_PLAY_H_
#define PRP_APP_PLAY_H_


#include "DrvPDMA.h"
#include "DrvDPWM.h"
#include "icSPI.h"
#include "DrvGPIO.h"
#include "icSystem.h"
#include "stat.h"

#define	Q16_MAX				(32767)
#define	Q16_MIN				(-32768)
#define SIREN7AUDIO			(0x7C003E80)
#define WAVAUDIO			(0x46464952)
#define SIREN7FRAME			(320 * 2)

#define PCMBUFFER_SIZE 		(1260)
#define ENCBUFFER_SIZE 		(20)
#define CYCLEBUFFER_SIZE 	(1600)

#define S7BANDWIDTH			(7000)
#define VP_GET_SR(arg)		(arg & 0x0FFFF)

#define BITRATE2SIZE(br)	(((br) / 400 + 1) & ~1) // ((((br) - 8000) / 4000) * 10 + 20) 对齐到2
#define RECORD_START		(0x10000)
#define CODE_START			(RECORD_START + 4)
#define MAX_AUDIO_NUM		(0x1869F+10)

#define SAMPLE_RATE16K		(16000)

#define	GAIN_COF			(1200)		//(1200)			// 8~12:100%~150%
#define	Q10					(1024)
#define	Q16_MAX				(32767)
#define	Q16_MIN				(-32768)
#define	SAMPLE_MAX_COF		(0x7FFF*GAIN_COF)

#define ADPCM_BUFFER_SIZE		(32)
#define	MVAPPEND_BUFFER_SIZE	(128)
#define	MVPCM_BUFFER_SIZE		(700)
#define MVWORK_SPACE_SIZE		(7200)

#define MVUSEDCODE				(100000)
#define RECUSEDCODE				(100001)

int8_t PlayWork(uint32_t CurAddr_s,uint32_t EndAddr_s,uint32_t ParaData_s);
void PlayStart(uint32_t audio_start,uint32_t end);	//根据索引进行具体的播音
void PlayBegin(int32_t code);						//先找到音频的索引
void PlayNextAudio(void);
void PDMAStart(void);
void DPWM_Init(int32_t sample_rate);
int8_t PlayChange(int32_t code);
void SysTimerDelay(uint32_t us);
void EsrBegin(void);
void PlayService(void);
void MagicVoice(uint32_t dwBaseRate,uint32_t dwRecDataStartAddr, uint16_t wRecFirstClusterIndex, int32_t dwPcmCount);
void StopPlayAudio(void);
void PlayPwrDownAduio(PEN_STATE PenState);
void PlayMagicVoice(void);

#endif
