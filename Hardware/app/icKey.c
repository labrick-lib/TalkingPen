#include "icKey.h"
/*----------------------------------------------------------------------------------------
函数名: InitKey
参数:
		None
返回值:
		None
描述:
		按键GPIO初始化
----------------------------------------------------------------------------------------*/
void InitKey(void)
{
	DrvGPIO_Open(GPB,2,IO_QUASI);
	DrvGPIO_Open(GPB,3,IO_QUASI);
	DrvGPIO_Open(GPB,5,IO_QUASI);
	GPIOB->DOUT |= (1 << 2);
	GPIOB->DOUT |= (1 << 3);
	GPIOB->DOUT |= (1 << 5);

	DrvGPIO_Open(GPA,7,IO_QUASI);	//初始化充电检测引脚
	DrvGPIO_Open(GPA,15,IO_INPUT); 	//初始化USB电源连接
	GPIOA->DOUT |= (1 << 7);
}
/*----------------------------------------------------------------------------------------
函数名: InitKey
参数:
		None
返回值:
		None
描述:
		获取按键状态
----------------------------------------------------------------------------------------*/
uint16_t GetKeyStatus(void)
{
	uint8_t key = 0;
	key |= ((~DrvGPIO_GetBit(GPB,2) & 0x01) << 0);		//开关机/播放暂停
	key |= ((~DrvGPIO_GetBit(GPB,3) & 0x01) << 1);		//录音/播录音
	key |= ((~DrvGPIO_GetBit(GPB,5) & 0x01) << 2);		//魔音->播魔音/识别
	key &= 0x07;
	return key;
}
/*----------------------------------------------------------------------------------------
函数名: InitKey
参数:
		None
返回值:
		None
描述:
		获取按键消息(有很大的缺陷，没有使用)
----------------------------------------------------------------------------------------*/
//int* Get_Key_Message()
//{
//	int code, i;
//	static int last_code=0, last_valid_code=0,count[KEY_COUNT],Key_Flag[KEY_COUNT];
////	Key_Flag[0] = 0;
//	code = GetKeyStatus();
//	
//	if(code == last_code){
//		for(i=0; i<KEY_COUNT; i++){
//			if(code & (1<<i)){
//				if(last_valid_code & (1<<i)){
//					count[i]++;
////					if(count[i] == 50)
////						Key_Flag[i] = 2;
//					if((count[i]) % 50 == 0)	//按键DOWN HOLD
//						Key_Flag[i] = 2;
//				}else{
//					count[i] = 0;
//					Key_Flag[i] = 1;		//按键DOWN
//					last_valid_code |= (1<<i);
//				}	
//			}else{
//				if(last_valid_code & (1<<i)){
//					Key_Flag[i] = 3;		//按键UP
//					last_valid_code &= ~(1<<i);	
//				}
//			}
//		}
//	}else{
//		for(i=0; i<KEY_COUNT; i++){
//			if(last_valid_code & (1<<i)){
//				count[i]++;
//				if(count[i] % 25 == 0)	//按键DOWN HOLD
//					Key_Flag[i] = 2;
//			}	
//		}			
//	}
//	last_code = code;
//
//	return Key_Flag;
//}
