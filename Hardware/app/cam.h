#ifndef PRP_APP_CAM_H_
#define PRP_APP_CAM_H_

#include "i2s_camera.h"	
#include "icSPI.h"
#include "timer.h"

#define PARAM_ADDR 0x0

int8_t get_param_from_file(int32_t *pParam1, int32_t *pParam2, int32_t *pParam3, int32_t *pParam4,int32_t *pParam5);
void save_param_to_file(int32_t param1, int32_t param2, int32_t param3, int32_t param4, int32_t param5);
T_VOID cam_adjust_check(void);
void CameraStart(void);
int32_t GetCamCode(void);
int32_t GetRecCode(void);

#endif
