#ifndef __BEEP_H
#define __BEEP_H

#include "sys.h"

/******************************************************************************************/
/* 引脚 定义 */

#define BEEP_GPIO_PORT                  GPIOB
#define BEEP_GPIO_PIN                   SYS_GPIO_PIN8
#define BEEP_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 3; }while(0)   /* PB口时钟使能 */

/******************************************************************************************/

/* 蜂鸣器控制 */
#define BEEP(x)         sys_gpio_pin_set(BEEP_GPIO_PORT, BEEP_GPIO_PIN, x)

/* BEEP取反定义 */
#define BEEP_TOGGLE()   do{ BEEP_GPIO_PORT->ODR ^= BEEP_GPIO_PIN; }while(0)     /* BEEP = !BEEP */

void beep_init(void);   /* 初始化蜂鸣器 */

#endif

















