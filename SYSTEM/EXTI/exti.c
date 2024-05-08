#include "sys.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "exti.h"
#include "lcd.h"

/**
 * @brief       KEY0 �ⲿ�жϷ������
 * @param       ��
 * @retval      ��
 */
 
int flag=0;
int temp_max=35; //�¶�
int humi_max=85;//ʪ��
int pm_max=300;//p�۳�
int pm_bj=0;
int tp_bj=0;
int hm_bj=0;
int offsc=0;
int display=1;
 
void KEY0_INT_IRQHandler(void)
{
    delay_ms(20);                   /* ���� */
    EXTI->PR = KEY0_INT_GPIO_PIN;   /* ���KEY0�����ж��� ���жϱ�־λ */

    if (KEY0 == 0)
    {
        if(flag){
        if(pm_max-10>=0)
            pm_max -= 10;
       LED0(0);       
       delay_ms(100);   
        LED0(1);
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);      
        }
    }
}

/**
 * @brief       KEY1 �ⲿ�жϷ������
 * @param       ��
 * @retval      ��
 */
void KEY1_INT_IRQHandler(void)
{ 
    delay_ms(20);                   /* ���� */
    EXTI->PR = KEY1_INT_GPIO_PIN;   /* ���KEY1�����ж��� ���жϱ�־λ */

    if (KEY1 == 0)
    {
        if(flag){
        pm_max += 10;
         LED1(0);       
       delay_ms(100);   
        LED1(1);
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);     
        }
    }
}

/**
 * @brief       WK_UP �ⲿ�жϷ������
 * @param       ��
 * @retval      ��
 */
void WKUP_INT_IRQHandler(void)
{ 
    delay_ms(20);                   /* ���� */
    EXTI->PR = WKUP_INT_GPIO_PIN;   /* ���WKUP�����ж��� ���жϱ�־λ */

    if (WK_UP == 1)
    {
        flag=!flag;     //ȡ��
    }
        delay_ms(10);
    lcd_clear(WHITE);      //��ʾ����

    
    //������ʪ�ȼ��۳����ϵͳ
    Chinese_Show_one(30,25,2,16,0,GREEN);
    Chinese_Show_one(45,25,3,16,0,GREEN);
    Chinese_Show_one(60,25,4,16,0,GREEN);
    Chinese_Show_one(75,25,5,16,0,GREEN);
    Chinese_Show_one(90,25,6,16,0,GREEN);
    Chinese_Show_one(105,25,7,16,0,GREEN);
    Chinese_Show_one(120,25,8,16,0,GREEN);
    Chinese_Show_one(135,25,9,16,0,GREEN);
    Chinese_Show_one(150,25,10,16,0,GREEN);
    Chinese_Show_one(165,25,11,16,0,GREEN);
    Chinese_Show_one(180,25,12,16,0,GREEN);
    Chinese_Show_one(195,25,13,16,0,GREEN);
    
    //��ΰ
    Chinese_Show_one(100,50,0,16,0,GREEN);
    Chinese_Show_one(128,50,1,16,0,GREEN);

    //ϵͳ��
    if (flag) { 
        //lcd_clear(WHITE);
        //Chinese_Show_one(80,70,14,16,0,RED);
       // Chinese_Show_one(100,70,14,16,0,RED);
        lcd_show_string(40, 70, 200, 16, 16, "Condition : Online", GREEN);

    }
    //�ر�
    else {
        //Chinese_Show_one(80,70,14,16,0,GREEN);
        //Chinese_Show_one(100,70,14,16,0,GREEN);
        lcd_show_string(40, 70, 250, 16, 16, "Condition : Offline  ", GREEN);
        
    }
    
}

/**
 * @brief       �ⲿ�жϳ�ʼ������
 * @param       ��
 * @retval      ��
 */
void extix_init(void)
{
    key_init();
    sys_nvic_ex_config(KEY0_INT_GPIO_PORT, KEY0_INT_GPIO_PIN, SYS_GPIO_FTIR);   /* KEY0����Ϊ�½��ش����ж� */
    sys_nvic_ex_config(KEY1_INT_GPIO_PORT, KEY1_INT_GPIO_PIN, SYS_GPIO_FTIR);   /* KEY1����Ϊ�½��ش����ж� */
    sys_nvic_ex_config(WKUP_INT_GPIO_PORT, WKUP_INT_GPIO_PIN, SYS_GPIO_RTIR);   /* WKUP����Ϊ�����ش����ж� */

    sys_nvic_init( 0, 2, KEY0_INT_IRQn, 2); /* ��ռ0�������ȼ�2����2 */
    sys_nvic_init( 1, 2, KEY1_INT_IRQn, 2); /* ��ռ1�������ȼ�2����2 */
    sys_nvic_init( 3, 2, WKUP_INT_IRQn, 2); /* ��ռ3�������ȼ�2����2 */
}












