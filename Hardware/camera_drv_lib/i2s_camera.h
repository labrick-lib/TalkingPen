/**
 * @FILENAME:   i2s_camera.h
 * @BRIEF       declare all the function for the OID
 * Copyright (C) 2013 Gaitek (GuangZhou) Technology Co., Ltd.
 * @DATE 2013-08-12
 * @VERSION 1.0
 */



#ifndef _I2S_CAMRA_H_
#define _I2S_CAMRA_H_

#include "new_type.h"
#include "DrvPDMA.h"


/**
 * @brief:   to initialize the camera
 * @author:  WangGuotian
 * @data:    2013-8-12
 * @param:   I2S_PDMA_channel
 *              The index of PDMA channel used for I2S interface.
 * @param[in]  buf		  the point to temp buf,the buf size must be 1600
 * @param[in]  size		  buf size, it must be 1600 
 * @return   T_BOOL
 * @retval:  AK_TRUE     success  to initialize the camera
 *           AK_FALSE    fail  to initialize the camera  
 */
T_BOOL  cam_init(E_DRVPDMA_CHANNEL_INDEX  I2S_PDMA_channel,void * buf, T_S16 size);


/**
 * @brief   Get a camera frame
 * @author  wangguotian
 * @date    2013-08-12
 * @param:  wbuf_1
 *              the temporary buffer 1 for the data sampling.the start address
 *              of the buffer must be aligned by 4 bytes.the buffer size 
 *              must be 304 bytes;
 *              the buffer can be released after exiting this function.
 * @param:  wbuf_1_len
 *              the length of buf_1, in bytes. the value must be 304;
 *
 * @param:  bbuf_2
 *              the temporary buffer 2 for the data sampling.the buffer size 
 *              must be 50 bytes;
 *              the buffer can be released after exiting this function.
 * @param:  bbuf_2_len
 *              the length of buf_2, in bytes. the value must be 50;
 *
 * @param:  bbuf_3
 *              the temporary buffer 3 for the data sampling.the buffer size 
 *              must be 50 bytes;
 *              the buffer can be released after exiting this function.
 * @param:  bbuf_3_len
 *              the length of buf_3, in bytes. the value must be 50; 
 * @return  T_U32
 * @retval  the number of 1 bit in a frame
 *
 * @remark  the temporary buffer(buf_1, buf_2 and buf_3) can be released after 
 *          exiting this function. 
 */
T_U32   cam_get_frame(T_U32 *wbuf_1, T_U16 wbuf_1_len, T_U8 *bbuf_2, T_U8 bbuf_2_len, 
                      T_U8 *bbuf_3, T_U8 bbuf_3_len);


/**
 * @brief   Get a code from the latest frame.
 * @author  wangguotian
 * @date    2013-08-12
 * @param[in] T_VOID
 * @return  T_S32
 * @retval  the code of  a frame, -1 means invalid code, other means valid code
 */
T_S32   cam_get_code(void);


/**
 * @brief   Retrieve the settings for adjusting the camera.The return value
 *          can be used by function cam_set_adj_param
 * @author  wangguotian
 * @date    2013-8-12
 * @param[in]  wbuf
 *              the temporary buffer for the data sampling.the start address
 *              of the buffer must be aligned by 4 bytes.the buffer size 
 *              must be 3K bytes;
 *              the buffer can be released after exiting this function.
 * @param[in]  wbuf_len
 *              the length of wbuf, in bytes. the value must be 3072 (3K bytes);
 *
 * @param[out] pParam1
 *                  Pointer to a T_S32 variable that receives the 
 *                  settings information.
 * @param[out] pParam2
 *                  Pointer to a T_S32 variable that receives the 
 *                  settings information.
 * @param[out] pParam3
 *                  Pointer to a T_S32 variable that receives the 
 *                  settings information.
 * @param[out] pParam4
 *                  Pointer to a T_S32 variable that receives the 
 *                  settings information.
 * @param[out] pParam5
 *                  Pointer to a T_S32 variable that receives the 
 *                  settings information.
 * @return  T_BOOL
 * @retval  AK_TRUE   successful
 * @retval  AK_FALSE  failed, means there is trouble with the pen point.
 *
 * @remark  If this function will be called, spi_cam_init must be called first.
 *          The temporary buffer(wbuf) can be released after exiting this function. 
 *
 */
T_BOOL  cam_get_adj_param(T_U32 *wbuf, T_U16 wbuf_len, T_S32 *pParam1, 
                          T_S32 *pParam2, T_S32 *pParam3, 
                          T_S32 *pParam4,T_S32 *pParam5);



/**
 * @brief   Adjust the camera
 * @author  wangguotian
 * @date    2013-8-12
 * @return  T_VOID
 *
 */
T_VOID  cam_set_adj_param(T_S32 param1, T_S32 param2, T_S32 param3, T_S32 param4,T_S32 param5);


/**
 * @brief   register the callback function which will be running when sampling.
 *          and all the time consumed on the function can't exceed 9ms. 
 * @author  wangguotian
 * @date    2013-8-12
 * @param[in]  audio_codec
 *              the address of the callback funciton.
 * @return  T_VOID
 *
 * @remark  the callback function can't consume more than 9ms.
 */
T_VOID  cam_reg_audio_cb(void (*audio_codec)(void));


#endif  //_I2S_CAMRA_H_




