
#include "hw.h"
#include "new_type.h"
#include "i2c_drv.h"
#include "DrvGPIO.h"
#include "ISD9xx.h"
				
#define akerror

#define I2C_DELAY_UNIT  10	//10

#define GPIO_I2C_SCLK   11	//0
#define GPIO_I2C_SDA    10	//1


static T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level )
{
    if(level)
    {
        DrvGPIO_SetBit(GPA, pin);
    }
    else
    {
        DrvGPIO_ClrBit(GPA, pin);
    }
}


static T_U8 gpio_get_pin_level( T_U32 pin )
{
    return DrvGPIO_GetBit(GPA, pin);
}


T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir )
{
    DrvGPIO_Open(GPA, pin, (DRVGPIO_IO)dir);
}



static T_VOID i2c_delay(T_U32 us)
{
    
		//us = us * 10 ;//>> 2;//5;       // time = time/32
		us = us >> 1;
        
        while(us --);
        {
            *(volatile T_U32 *)0;
        }
}


static T_VOID set_i2c_pin(T_U32 pin)
{
     gpio_set_pin_level(pin, 1);
}

static T_VOID clr_i2c_pin(T_U32 pin)
{
    gpio_set_pin_level(pin, 0);
}

static T_U8 get_i2c_pin(T_U32 pin)
{
    return gpio_get_pin_level(pin);
}



static T_VOID i2c_init(void)
{ 
    #if 0
    gpio_int_control(GPIO_I2C_SCLK,0);
    gpio_int_control(GPIO_I2C_SDA,0);
    gpio_set_pullup_pulldown(GPIO_I2C_SCLK,0);
    gpio_set_pullup_pulldown(GPIO_I2C_SDA,0);
    #endif

    SYS->GPA_ALT.GPA0 	= 0;
    SYS->GPA_ALT.GPA1 	= 0;

    DrvGPIO_EnableDebounce(GPA, GPIO_I2C_SCLK);
    DrvGPIO_EnableDebounce(GPA, GPIO_I2C_SDA);

    DrvGPIO_ClrBitMask(GPA, GPIO_I2C_SCLK);
    DrvGPIO_ClrBitMask(GPA, GPIO_I2C_SDA);

    
    gpio_set_pin_dir(GPIO_I2C_SCLK, 1);
    gpio_set_pin_dir(GPIO_I2C_SDA, 1);
    gpio_set_pin_level(GPIO_I2C_SCLK, 1);
    gpio_set_pin_level(GPIO_I2C_SDA, 1);
}


static T_VOID i2c_write_ask(T_U8    flag)
{
    if(flag)
        set_i2c_pin(GPIO_I2C_SDA);
    else
        clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
}



static T_BOOL   i2c_read_ack(void)
{
    T_BOOL ret;
    set_i2c_pin(GPIO_I2C_SDA);
    gpio_set_pin_dir( GPIO_I2C_SDA, 0 );
    i2c_delay(I2C_DELAY_UNIT << 3);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);
    if (!get_i2c_pin(GPIO_I2C_SDA))
    {
        ret = AK_TRUE;
    }
    else
    {
        ret = AK_FALSE;
    }

    i2c_delay(I2C_DELAY_UNIT << 2);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);
    gpio_set_pin_dir( GPIO_I2C_SDA, 1 );
    i2c_delay(I2C_DELAY_UNIT << 2);
    return ret;
}




static T_VOID i2c_begin(void)
{
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 4);
}

static T_VOID i2c_end(void)
{
    i2c_delay(I2C_DELAY_UNIT << 2);
    clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 4);
}



static T_VOID i2c_free(void)
{
    //退出时拉高引脚,以节省功耗
    gpio_set_pin_level(GPIO_I2C_SCLK, 1);
    gpio_set_pin_level(GPIO_I2C_SDA, 1);
    return;
}



static T_BOOL i2c_write_byte(T_U8 data)
{
    T_U8 i;

    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        if (data & 0x80)
            set_i2c_pin(GPIO_I2C_SDA);
        else
            clr_i2c_pin(GPIO_I2C_SDA);
        data <<= 1;

        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 3);
        clr_i2c_pin(GPIO_I2C_SCLK);
    }
    return i2c_read_ack();
}


static T_U8 i2c_read_byte(void)
{
    T_U8 i;
    T_U8 ret;

    ret = 0;
    gpio_set_pin_dir( GPIO_I2C_SDA, 0 );
    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 2);
        ret = ret<<1;
        if (get_i2c_pin(GPIO_I2C_SDA))
            ret |= 1;
        i2c_delay(I2C_DELAY_UNIT << 2);
        clr_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 1);
        if (i==7)
            gpio_set_pin_dir(GPIO_I2C_SDA, 1 );
        i2c_delay(I2C_DELAY_UNIT << 1);
    }

    return ret;
}



T_BOOL i2c_read_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{
    T_U32 i;     
 
  	i2c_init();
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
			i2c_free();
            return AK_FALSE;
        }
    }

    i2c_end();
    
    // restart transmite
    i2c_begin();
    
    // send message to I2C device to transmite data
    if (!i2c_write_byte((T_U8)(addr[0] | 1)))
    {
        i2c_end();
		i2c_free();
        return AK_FALSE;
    }
    
    // transmite data
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1) ? i2c_write_ask(0) : i2c_write_ask(1);
    }
    
    // stop transmite
    i2c_end();
	i2c_free();
    return AK_TRUE;
}



T_BOOL i2c_write_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{
    T_U32 i;     
      
	i2c_init();
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
			i2c_free();
            //akerror("i2c write addr fail",0,1);
            return AK_FALSE;
        }
    }   

    // transmite data
    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();
			i2c_free();
            //akerror("i2c write data fail",0,1);
            return AK_FALSE;
        }
    }

    // stop transmited
    i2c_end();
	i2c_free();
    return AK_TRUE;
}


T_BOOL i2c_read_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_read_bytes(addr, 2, data, size);
}





T_BOOL i2c_write_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_write_bytes(addr, 2, data, size);
}


T_BOOL i2c_write_data(T_U8 dab, T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte(dab))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }


    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}



T_BOOL i2c_read_data(T_U8 dab, T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte((T_U8)(dab | 1)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}


