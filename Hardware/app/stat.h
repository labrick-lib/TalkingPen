#ifndef	PRP_APP_STAT_H_
#define PRP_APP_STAT_H_

#define __DEBUG__

//#ifdef __DEBUG__
//#define LOG	printf
//#else
//#define LOG /\
///LOG
//#endif

#ifdef __DEBUG__
#define LOG(A)	printf A
#else
#define LOG(A)
#endif

#define SAFE_VOL	(3600) 		//mv
#define	SERVER_VOL	(3400)		//mv

#define POWER_SAFE		(0)
#define POWER_LOW		(1)
#define POWER_SHUTDOWN	(2)

typedef enum {
	PLAY_STATE,
	ESR_STATE,
	PWRDOWN_STATE,
	SLEEP_STATE,
	RECORD_STATE,
	MVREC_STATE,
}PEN_STATE;

#define REMINDER_TIME	(60)
#define SHUTDOWN_TIME	(120)
#define	REC_EXIT_TIME	(10)
#define ESR_DENY_TIME	(10)
#define LOW_POWER_TIME	(300)

enum PEN_AUDIO{
	PEN_START_AUDIO = 99600,		//开机
	PEN_TIMEOUT_AUDIO,				//限制
	PEN_SHUTDOWN_AUDIO,				//关机		无用回头删除
	PEN_RECTIP_AUDIO,				//录音
	PEN_LOWPWR_AUDIO,				//低电
	PEN_ESRTIP_AUDIO,				//识别
	PEN_NORECTIP_AUDIO,				//无录音
	PEN_SLEEP_AUDIO,				//休眠
	
	PEN_HEAD_AUDIO1 = 99608,		//摸脑袋1
	PEN_HEAD_AUDIO2,				//摸脑袋2
	PEN_HEAD_AUDIO3,				//摸脑袋3
};

#define PEN_FIRST_MUSIC			(60000)
#define PEN_LAST_MUSIC			(60012)

#define PLAY_MUSIC_CODE			(47)
#define PLAY_STOP_CODE			(49)
#define PLAY_SUSPEND_CODE		(48)
#define PLAY_RESUME_CODE		(46)
#define PLAY_NEXT_CODE			(51)
#define PLAY_PREV_CODE			(50)

#define VOLUME_PLUS_CODE		(7)
#define VOLUME_MINUS_CODE		(8)
#define VOLUME_LEVEL0_CODE		(10)
#define VOLUME_LEVEL1_CODE		(11)
#define VOLUME_LEVEL2_CODE		(12)
#define VOLUME_LEVEL3_CODE		(13)
#define VOLUME_LEVEL4_CODE		(14)
#define VOLUME_LEVEL5_CODE		(15)
#define VOLUME_LEVEL6_CODE		(16)
#define VOLUME_LEVEL7_CODE		(17)

#define ROLE_CODE		(-1)

#define LOWPWR_REMINDER_TIME	(60)		//低电之后多久提醒一次低电

#define ESR_CODE_START 			(99800)
#define ESR_CODE_END			(99999)

#define DATAF_BASE_ADDR			(0x20000)

#define	SPI_FLASH_MSIZE			(4)					//单位M
#define FLASH_MIN_ERASE_KSIZE	(64)					//单位是K
#define FLASH_MIN_ERASE_SIZE	(FLASH_MIN_ERASE_KSIZE*0x400)

#endif
