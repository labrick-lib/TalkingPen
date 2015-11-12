#ifndef __IIC_H__N572_iFLYTEK
#define __IIC_H__N572_iFLYTEK

#include "new_type.h"

/*
void i2c_init(void);
void i2c_term(void);

int8_t i2c_send(uint8_t addr, uint8_t data[], uint32_t size);
int8_t i2c_read(uint8_t addr, uint8_t data[], uint32_t size);

int8_t i2c_write_data(uint8_t addr, uint8_t data);
int8_t i2c_read_data(uint8_t addr, uint8_t *pdata);
*/

T_BOOL i2c_write_data(T_U8 dab, T_U8 *data, T_U8 size);
T_BOOL i2c_read_data(T_U8 dab, T_U8 *data, T_U8 size);

T_BOOL i2c_read_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);
T_BOOL i2c_write_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);

#endif
