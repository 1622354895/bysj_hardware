#ifndef __LED_H
#define __LED_H

#include "sys.h"

/******************************************************************************************/
/* 引脚 定义 */

#define LED0_GPIO_PORT                  GPIOB
#define LED0_GPIO_PIN                   SYS_GPIO_PIN5
#define LED0_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 3; }while(0)   /* PB口时钟使能 */

#define LED1_GPIO_PORT                  GPIOE
#define LED1_GPIO_PIN                   SYS_GPIO_PIN5
#define LED1_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE口时钟使能 */

/******************************************************************************************/



/* LED端口定义 */
#define LED0(x)         sys_gpio_pin_set(LED0_GPIO_PORT, LED0_GPIO_PIN, x)      /* LED0 */
#define LED1(x)         sys_gpio_pin_set(LED1_GPIO_PORT, LED1_GPIO_PIN, x)      /* LED1 */

/* LED取反定义 */
#define LED0_TOGGLE()   do{ LED0_GPIO_PORT->ODR ^= LED0_GPIO_PIN; }while(0)      /* LED0 = !LED0 */
#define LED1_TOGGLE()   do{ LED1_GPIO_PORT->ODR ^= LED1_GPIO_PIN; }while(0)      /* LED1 = !LED1 */


void led_init(void);    /* 初始化 */

#endif

















