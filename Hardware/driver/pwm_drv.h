
#ifndef __pwm_drv_h_
#define __pwm_drv_h_

uint32_t pwm_init(uint8_t channel, uint32_t freq, uint32_t duty);
uint32_t pwm_ctrl(uint8_t channel, uint8_t duty, uint8_t off_stat);
void pwm_led_init(void);
void pwm_led_set(uint8_t led0_duty, uint8_t led1_duty);
	   
#endif
