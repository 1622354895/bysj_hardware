#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

/******************************************************************************************/
/* 引脚 和 中断编号 & 中断服务函数 定义 */ 

#define KEY0_INT_GPIO_PORT              GPIOE
#define KEY0_INT_GPIO_PIN               SYS_GPIO_PIN4
#define KEY0_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE口时钟使能 */
#define KEY0_INT_IRQn                   EXTI4_IRQn
#define KEY0_INT_IRQHandler             EXTI4_IRQHandler

#define KEY1_INT_GPIO_PORT              GPIOE
#define KEY1_INT_GPIO_PIN               SYS_GPIO_PIN3
#define KEY1_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE口时钟使能 */
#define KEY1_INT_IRQn                   EXTI3_IRQn
#define KEY1_INT_IRQHandler             EXTI3_IRQHandler

#define WKUP_INT_GPIO_PORT              GPIOA
#define WKUP_INT_GPIO_PIN               SYS_GPIO_PIN0
#define WKUP_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 2; }while(0)   /* PA口时钟使能 */
#define WKUP_INT_IRQn                   EXTI0_IRQn
#define WKUP_INT_IRQHandler             EXTI0_IRQHandler

/******************************************************************************************/


void extix_init(void);  /* 外部中断初始化 */
extern int flag; //标志系统开关
extern int temp_max; //温度
extern int humi_max;//湿度
extern uint8_t light_min;//光照
extern int pm_max;//p粉尘
//数据流flag位
extern int pm_bj;
extern int tp_bj;
extern int hm_bj;
extern int offsc;
extern int display;


#endif

























