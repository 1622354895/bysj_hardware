#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

/******************************************************************************************/
/* 引脚 定义 */

#define KEY0_GPIO_PORT                  GPIOE
#define KEY0_GPIO_PIN                   SYS_GPIO_PIN4
#define KEY0_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE口时钟使能 */

#define KEY1_GPIO_PORT                  GPIOE
#define KEY1_GPIO_PIN                   SYS_GPIO_PIN3
#define KEY1_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE口时钟使能 */

#define WKUP_GPIO_PORT                  GPIOA
#define WKUP_GPIO_PIN                   SYS_GPIO_PIN0
#define WKUP_GPIO_CLK_ENABLE()          do{ RCC->APB2ENR |= 1 << 2; }while(0)   /* PA口时钟使能 */

/******************************************************************************************/

#define KEY0        sys_gpio_pin_get(KEY0_GPIO_PORT, KEY0_GPIO_PIN)     /* 读取KEY0引脚 */
#define KEY1        sys_gpio_pin_get(KEY1_GPIO_PORT, KEY1_GPIO_PIN)     /* 读取KEY1引脚 */
#define WK_UP       sys_gpio_pin_get(WKUP_GPIO_PORT, WKUP_GPIO_PIN)     /* 读取WKUP引脚 */


#define KEY0_PRES    1      /* KEY0按下 */
#define KEY1_PRES    2      /* KEY1按下 */
#define WKUP_PRES    3      /* KEY_UP按下(即WK_UP) */

void key_init(void);        /* 按键初始化函数 */
uint8_t key_scan(uint8_t mode);/* 按键扫描函数 */

#endif


















