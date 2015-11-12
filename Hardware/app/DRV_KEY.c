#include "DRV_KEY.h"

#define	QUEUE_SIZE			8
#define	QUEUE_SIZE_MASK		(QUEUE_SIZE-1)

#define	SCANNING_TIME		10

//uint32_t g_cntStartTime = 0;		//利用这里的定时器来产生随机数种子

uint16_t Data_Pre,Key_Lock;


uint8_t KeyMask[KEY_NUM];
uint8_t KeyLevel[KEY_NUM];
uint16_t HoldTime[KEY_NUM];

KEYMSG	QUEUE_Msg[QUEUE_SIZE];
uint16_t QUEUE_Front;
uint16_t QUEUE_Num;

void KEY_Init(void)
{    
	InitKey();
	
    
    KeyMask[0] = KEY_ON_OFF;
	KeyMask[1] = KEY_RECORD;
	KeyMask[2] = KEY_MACESR;
	KeyMask[3] = KEY_TOUCH;
		
	KeyLevel[0] = KEY_ON_OFF;
	KeyLevel[1] = KEY_RECORD;
	KeyLevel[2] = KEY_MACESR;
	KeyLevel[3] = KEY_TOUCH;
	
	QUEUE_Front = 0;
	QUEUE_Num = 0;

	//Set Key Lock
	Data_Pre = 0x0000;
	Key_Lock = Data_Pre;

	DrvTIMER_SetTimerEvent(TMR1, SCANNING_TIME, (TIMER_CALLBACK)Key_Scanning_ISR, 0);
	
}

void KEY_MsgPost(PKEYMSG pSysMsg)
{	
	if(QUEUE_Num < QUEUE_SIZE)
	{
		QUEUE_Msg[(QUEUE_Front + QUEUE_Num) & QUEUE_SIZE_MASK].Key_MsgValue = 
		pSysMsg->Key_MsgValue;
		
		QUEUE_Msg[(QUEUE_Front + QUEUE_Num) & QUEUE_SIZE_MASK].Key_MsgType = 
		pSysMsg->Key_MsgType;
		
		QUEUE_Msg[(QUEUE_Front + QUEUE_Num) & QUEUE_SIZE_MASK].Key_HoldTime = 
		pSysMsg->Key_HoldTime;
		
		QUEUE_Num++;
	}
//	LOG(("POST:%d,%d,%d,%d\r\n",pSysMsg->Key_MsgValue,pSysMsg->Key_MsgType,pSysMsg->Key_HoldTime,QUEUE_Num));
}

uint16_t KEY_MsgGet(PKEYMSG pSysMsg)
{
//	LOG("QUEUE_Num:%d\r\n",QUEUE_Num);
	if(QUEUE_Num)
	{
		pSysMsg->Key_MsgValue = QUEUE_Msg[QUEUE_Front].Key_MsgValue;
		pSysMsg->Key_MsgType = QUEUE_Msg[QUEUE_Front].Key_MsgType;
		pSysMsg->Key_HoldTime = QUEUE_Msg[QUEUE_Front].Key_HoldTime;
		QUEUE_Front = (QUEUE_Front + 1) & QUEUE_SIZE_MASK;
		QUEUE_Num--;
//		LOG("QUEUE_Num:%d\r\n",QUEUE_Num);
		return 1;
	}
	else
	{
		pSysMsg->Key_MsgValue = 0;
		pSysMsg->Key_MsgType = 0;
		pSysMsg->Key_HoldTime = 0;
		return 0;
	}
}

void Key_Scanning_ISR(uint32_t data)
{
	uint16_t Data_Cur;
	uint16_t TouchData;
	uint16_t i;
	KEYMSG pSysMsg;

    Data_Cur = (uint16_t)GetKeyStatus();		//只用了一个
	TouchData = (uint16_t)DrvTouchScanning();
	TouchData <<= (KEY_NUM-1);
	Data_Cur += TouchData;
//	LOG(("world!\t"));

	for(i = 0; i < KEY_NUM; i++)
	{
		if(((~Key_Lock) & KeyMask[i]) == KeyLevel[i])
		{
			if((Data_Pre & KeyMask[i]) == KeyLevel[i])
			{
				if((Data_Cur & KeyMask[i]) != KeyLevel[i])
				{
					if(HoldTime[i] > 60)
					{
						if(HoldTime[i] < 1500)
						{
							pSysMsg.Key_MsgValue = KeyMask[i];
							pSysMsg.Key_MsgType = KEY_TYPE_SP;
							pSysMsg.Key_HoldTime = HoldTime[i];
							KEY_MsgPost(&pSysMsg);
						}
						pSysMsg.Key_MsgValue = KeyMask[i];
						pSysMsg.Key_MsgType = KEY_TYPE_UP;
						pSysMsg.Key_HoldTime = HoldTime[i];
						KEY_MsgPost(&pSysMsg);
					}
					HoldTime[i] = 0x0000;
				}
			    else
			    {
				    HoldTime[i] += SCANNING_TIME;
				    if( HoldTime[i] >= 50000 )
				    {
					    HoldTime[i] = 550;
				    }
				
					if(HoldTime[i] == 50)
					{
					    pSysMsg.Key_MsgValue = KeyMask[i];
					    pSysMsg.Key_MsgType = KEY_TYPE_DOWN;
					    pSysMsg.Key_HoldTime = HoldTime[i];
					    KEY_MsgPost(&pSysMsg);
					}
					else if(HoldTime[i] == 1500)
					{
					    pSysMsg.Key_MsgValue = KeyMask[i];
					    pSysMsg.Key_MsgType = KEY_TYPE_LP;
					    pSysMsg.Key_HoldTime = HoldTime[i];
					    KEY_MsgPost(&pSysMsg);
					}
					if(((HoldTime[i] % 100) == 0)&&(HoldTime[i] >= 100))
					{
					    pSysMsg.Key_MsgValue = KeyMask[i];
					    pSysMsg.Key_MsgType = KEY_TYPE_KH;
					    pSysMsg.Key_HoldTime = HoldTime[i];
					    KEY_MsgPost(&pSysMsg);
				    
					}
			    }
		    }
	    }
	}
	Data_Pre = Data_Cur;
	Key_Lock &= Data_Cur;
	
//	if(g_cntStartTime++ >= 65535)	g_cntStartTime = 0;
	
}

