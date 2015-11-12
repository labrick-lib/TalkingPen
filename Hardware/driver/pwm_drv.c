
#include "hw.h"
#include "pwm_drv.h"

typedef struct pwm_struct
{
	uint32_t cnr0; //pwm通道0的定时器计数，决定了pwm的周期
	uint32_t duty0;//pwm通道0的占空比
	uint32_t cnr1;
	uint32_t duty1;
} PWM_STRUCT;

PWM_STRUCT pwm_struct;

//初始化pwm，当要修改pwm的频率的时候需要调用该函数，可设置的频率范围为：1-240KHz
//channel:要初始化的通道通道0对应PA12, 通道1对应PA13
//freq   :pwm输出的频率,单位是KHz,最大值是240,因为更高频率的时候占空比无法做到100级的区分度
//duty   :pwm的初始占空比，初始化完成后便会输出这个占空比的pwm信号
//返回值 :最终设置的pwm频率		
uint32_t pwm_init(uint8_t channel, uint32_t freq, uint32_t duty)
{
	uint32_t cnr, cmr;
	SYSCLK->APBCLK.PWM01_EN = 1;
	SYSCLK->CLKSEL1.PWM01_S = 3;//使用48MHz	
	PWMA->PPR.CP01 = 1; //1+1 分频
	if(freq > 240)
		freq = 240;
	cnr = 24000 * 100 / freq;
	cnr = cnr / 100;
	cmr = cnr * duty / 100;
	if(channel == 0)
	{	
		SYS->GPA_ALT.GPA12 = 1; //PWM0
		PWMA->CSR.CSR0 = 4; //1分频 pwm时钟 = 48M / 2 / 1 = 24MHz
		PWMA->PCR.CH0MOD = 1; //Auto-reload mode
		PWMA->PCR.CH0INV = 0; //反相
		PWMA->CNR0 = cnr;
		PWMA->CMR0 = cmr; 			
		PWMA->POE.PWM0 = 1;
		PWMA->PCR.CH0EN = 1;
		pwm_struct.cnr0 = cnr;
		pwm_struct.duty0 = duty;
	}
	else
	{	
		SYS->GPA_ALT.GPA13 = 1; //PWM1
		PWMA->CSR.CSR1 = 4; //1分频 pwm时钟 = 48M / 2 / 1 = 24MHz
		PWMA->PCR.CH1MOD = 1; //Auto-reload mode
		PWMA->PCR.CH1INV = 0; //反相
		PWMA->CNR1 = cnr;
		PWMA->CMR1 = cmr;	
		PWMA->POE.PWM1 = 1;
		PWMA->PCR.CH1EN = 1;
		pwm_struct.cnr1 = cnr;
		pwm_struct.duty1 = duty;		
	}
	return freq;
}

//初始化完pwm后可以调用该函数来调整占空比
//channel : 要设置的通道 0:对应PA12 1:对应PA13
//duty    : 要输出的占空比
//off_stat: 如果设置的占空比为0的时候，对应通道输出该电平，0则输出低电平，1则输出高电平
uint32_t pwm_ctrl(uint8_t channel, uint8_t duty, uint8_t off_stat)
{
	uint32_t cmr;
	if(duty == 0)
	{
		if(channel == 0)
		{
			SYS->GPA_ALT.GPA12 = 0;
			if(off_stat) //占空比为0的时候是关闭pwm，把引脚设置成一个状态
			{
				GPIOA->DOUT |= 1 << 12;	
			}
			else
			{
				GPIOA->DOUT &= ~(1 << 12);
			}	
			GPIOA->PMD.PMD12 = IO_OUTPUT;			
		}
		else
		{
			SYS->GPA_ALT.GPA13 = 0;
			if(off_stat)
			{
				GPIOA->DOUT |= 1 << 13;
			}
			else
			{
				GPIOA->DOUT &= ~(1 << 13);
			}
			GPIOA->PMD.PMD13 = IO_OUTPUT;
		}
	}
	else
	{
		if(channel == 0)
		{
			SYS->GPA_ALT.GPA12 = 1; //PWM0
			cmr = pwm_struct.cnr0 * duty / 100;
			PWMA->CMR0 = cmr;
			pwm_struct.duty0 = duty;
			PWMA->PCR.CH0EN = 1;	
		}
		else
		{
			SYS->GPA_ALT.GPA13 = 1; //PWM1	
			cmr = pwm_struct.cnr1 * duty / 100;
			PWMA->CMR1 = cmr;
			pwm_struct.duty1 = duty;
			PWMA->PCR.CH1EN = 1;
		}
	}
	return duty;
}





//add by wgtbupt

void pwm_led_init(void)
{
	pwm_init(0, 120, 0);
	pwm_init(1, 120, 0);
}


void pwm_led_set(uint8_t led0_duty, uint8_t led1_duty)
{
#if   0
	if(100 == led0_duty)
	{
		pwm_ctrl(0, 0, 1);	
	}

	if(100 == led1_duty)
	{
		pwm_ctrl(1, 0, 1);	
	}
#endif

	pwm_ctrl(0, led0_duty, 0);	
	pwm_ctrl(1, led1_duty, 0);
}

