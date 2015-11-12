#ifndef PRP_APP_DRVKEY_H_
#define PRP_APP_DRVKEY_H_

#include    "DrvSYS.h"
#include    "DrvGPIO.h"
#include    "DrvTimer.h"
#include	"icKey.h"
#include 	"ivTouchSense.h"
#include 	"stat.h"

#define KEY_DEBUG               1

// key message type
#define	KEY_TYPE_DOWN		    0x00U				// key down
#define	KEY_TYPE_SP			    0x20U				// short press
#define	KEY_TYPE_LP			    0x40U				// long press
#define	KEY_TYPE_KH			    0x60U				// key hold
#define	KEY_TYPE_UP			    0x80U				// key up

// key message value
#define KEY_ON_OFF              (1 << 0)
#define KEY_RECORD		   	 	(1 << 1)
#define KEY_MACESR				(1 << 2)
#define KEY_TOUCH				(1 << 3)

#define KEY_NUM                 4

//Key Port Define
#define KEY_PORT                GPIOB

//TimerF Counting cycle
#define COUNT_TIME              6


#define u32IOModeA              DRVGPIO_IOMODE_PIN14_QUASI |	\
                                DRVGPIO_IOMODE_PIN15_QUASI 	


typedef struct sysmsg
{
	uint8_t	Key_MsgValue;			//按键位置
	uint8_t	Key_MsgType;			//键值类型
	uint16_t	Key_HoldTime;			//键盘按下时间
} KEYMSG,*PKEYMSG;

extern KEYMSG msg;

/* 键盘驱动 */
/*-----------------------------------------------------------------------------
    Name:       KEY_Init
    Function:   Init Key
-----------------------------------------------------------------------------*/
void		KEY_Init(void);								//键盘初始化
void		KEY_MsgPost(PKEYMSG pSysMsg);				//向队列中填充消息
uint16_t		KEY_MsgGet(PKEYMSG pSysMsg);				//从队列中获取消息
/*-----------------------------------------------------------------------------
    Name:       TMRF_IRQHandler()
    Function:   The Interrupt of TimerF
-----------------------------------------------------------------------------*/
void 		Key_Scanning_ISR(uint32_t data);

#endif	/* _DRV_KEY_H_ */
