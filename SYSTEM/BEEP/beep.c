#include "beep.h"

/**
 * @brief       初始化BEEP相关IO口, 并使能时钟
 * @param       无
 * @retval      无
 */
void beep_init(void)
{
    BEEP_GPIO_CLK_ENABLE(); /* BEEP时钟使能 */

    sys_gpio_set(BEEP_GPIO_PORT, BEEP_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* BEEP引脚模式设置 */

    BEEP(0);    /* 关闭蜂鸣器 */
}






