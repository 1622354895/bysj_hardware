#include "adc3.h"
#include "lsens.h"
#include "delay.h"


/**
 * @brief       ��ʼ������������
 * @param       ��
 * @retval      ��
 */
void lsens_init(void)
{
    LSENS_ADC3_CHX_GPIO_CLK_ENABLE();   /* IO��ʱ��ʹ�� */

    sys_gpio_set(LSENS_ADC3_CHX_GPIO_PORT, LSENS_ADC3_CHX_GPIO_PIN,
                 SYS_GPIO_MODE_AIN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* AD�ɼ�����ģʽ����,ģ������ */

    adc3_init();    /* ��ʼ��ADC */
}

/**
 * @brief       ��ȡ����������ֵ
 * @param       ��
 * @retval      0~100:0,�;100,����
 */
uint8_t lsens_get_val(void)
{
    uint32_t temp_val = 0;
    temp_val = adc3_get_result_average(LSENS_ADC3_CHX, 10); /* ��ȡƽ��ֵ */
    temp_val /= 40;

    if (temp_val > 100)temp_val = 100;

    return (uint8_t)(100 - temp_val);
}












