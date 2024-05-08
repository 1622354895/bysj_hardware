#include "beep.h"

/**
 * @brief       ��ʼ��BEEP���IO��, ��ʹ��ʱ��
 * @param       ��
 * @retval      ��
 */
void beep_init(void)
{
    BEEP_GPIO_CLK_ENABLE(); /* BEEPʱ��ʹ�� */

    sys_gpio_set(BEEP_GPIO_PORT, BEEP_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* BEEP����ģʽ���� */

    BEEP(0);    /* �رշ����� */
}






