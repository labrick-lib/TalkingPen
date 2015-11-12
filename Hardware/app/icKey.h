#ifndef PRP_APP_KEY_H_
#define PRP_APP_KEY_H_

#include "Driver\DrvGPIO.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvTimer.h"

#define KEY_COUNT 2

/*按键初始化--------------------------------*/
void InitKey(void);

/*获取按键状态-------------------------------*/
uint16_t GetKeyStatus(void);

/*获取按键消息-------------------------------*/
//int* Get_Key_Message(void);

#endif
