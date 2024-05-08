#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

/******************************************************************************************/
/* ���� �� �жϱ�� & �жϷ����� ���� */ 

#define KEY0_INT_GPIO_PORT              GPIOE
#define KEY0_INT_GPIO_PIN               SYS_GPIO_PIN4
#define KEY0_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE��ʱ��ʹ�� */
#define KEY0_INT_IRQn                   EXTI4_IRQn
#define KEY0_INT_IRQHandler             EXTI4_IRQHandler

#define KEY1_INT_GPIO_PORT              GPIOE
#define KEY1_INT_GPIO_PIN               SYS_GPIO_PIN3
#define KEY1_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 6; }while(0)   /* PE��ʱ��ʹ�� */
#define KEY1_INT_IRQn                   EXTI3_IRQn
#define KEY1_INT_IRQHandler             EXTI3_IRQHandler

#define WKUP_INT_GPIO_PORT              GPIOA
#define WKUP_INT_GPIO_PIN               SYS_GPIO_PIN0
#define WKUP_INT_GPIO_CLK_ENABLE()      do{ RCC->APB2ENR |= 1 << 2; }while(0)   /* PA��ʱ��ʹ�� */
#define WKUP_INT_IRQn                   EXTI0_IRQn
#define WKUP_INT_IRQHandler             EXTI0_IRQHandler

/******************************************************************************************/


void extix_init(void);  /* �ⲿ�жϳ�ʼ�� */
extern int flag; //��־ϵͳ����
extern int temp_max; //�¶�
extern int humi_max;//ʪ��
extern uint8_t light_min;//����
extern int pm_max;//p�۳�
//������flagλ
extern int pm_bj;
extern int tp_bj;
extern int hm_bj;
extern int offsc;
extern int display;


#endif

























