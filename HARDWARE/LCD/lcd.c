#include "stdlib.h"
#include "lcd.h"
#include "lcdfont.h"
#include "usart.h"


/* lcd_ex.c��Ÿ���LCD����IC�ļĴ�����ʼ�����ִ���,�Լ�lcd.c,��.c�ļ�
 * ��ֱ�Ӽ��뵽��������,ֻ��lcd.c���õ�,����ͨ��include����ʽ���.(��Ҫ��
 * �����ļ��ٰ�����.c�ļ�!!����ᱨ��!)
 */
#include "lcd_ex.c"


/* LCD�Ļ�����ɫ�ͱ���ɫ */
uint32_t g_point_color = 0XF800;    /* ������ɫ */
uint32_t g_back_color  = 0XFFFF;    /* ����ɫ */

/* ����LCD��Ҫ���� */
_lcd_dev lcddev;


/**
 * @brief       LCDд����
 * @param       data: Ҫд�������
 * @retval      ��
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data;            /* ʹ��-O2�Ż���ʱ��,����������ʱ */
    LCD->LCD_RAM = data;
}

/**
 * @brief       LCDд�Ĵ������/��ַ����
 * @param       regno: �Ĵ������/��ַ
 * @retval      ��
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;          /* ʹ��-O2�Ż���ʱ��,����������ʱ */
    LCD->LCD_REG = regno;   /* д��Ҫд�ļĴ������ */
}

/**
 * @brief       LCDд�Ĵ���
 * @param       regno:�Ĵ������/��ַ
 * @param       data:Ҫд�������
 * @retval      ��
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno;   /* д��Ҫд�ļĴ������ */
    LCD->LCD_RAM = data;    /* д������ */
}

/**
 * @brief       LCD��ʱ����,�����ڲ�����mdk -O1ʱ���Ż�ʱ��Ҫ���õĵط�
 * @param       t:��ʱ����ֵ
 * @retval      ��
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--);
}

/**
 * @brief       LCD������
 * @param       ��
 * @retval      ��ȡ��������
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram;  /* ��ֹ���Ż� */
    lcd_opt_delay(2);
    ram = LCD->LCD_RAM;
    return ram;
}

/**
 * @brief       ׼��дGRAM
 * @param       ��
 * @retval      ��
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

/**
 * @brief       ��ȡ��ĳ�����ɫֵ
 * @param       x,y:����
 * @retval      �˵����ɫ(32λ��ɫ,�������LTDC)
 */
uint32_t lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)return 0;   /* �����˷�Χ,ֱ�ӷ��� */

    lcd_set_cursor(x, y);       /* �������� */

    if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2E00);   /* 5510 ���Ͷ�GRAMָ�� */
    }
    else
    {
        lcd_wr_regno(0X2E);     /* 9341/5310/1963/7789/7796/9806 �ȷ��Ͷ�GRAMָ�� */
    }

    r = lcd_rd_data();          /* �ٶ�(dummy read) */

    if (lcddev.id == 0x1963)
    {
        return r;               /* 1963ֱ�Ӷ��Ϳ��� */
    }

    r = lcd_rd_data();          /* ʵ��������ɫ */

    if (lcddev.id == 0x7796)    /* 7796 һ�ζ�ȡһ������ֵ */
    {
        return r;
    }

    /* ILI9341/NT35310/NT35510/ST7789/ILI9806 Ҫ��2�ζ��� */
    b = lcd_rd_data();
    g = r & 0XFF;       /* ���� 9341/5310/5510/7789/9806, ��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ */
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));  /* 9341/5310/5510/7789/9806 ��Ҫ��ʽת��һ�� */
}

/**
 * @brief       LCD������ʾ
 * @param       ��
 * @retval      ��
 */
void lcd_display_on(void)
{
    if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2900);   /* ������ʾ */
    }
    else    /* 9341/5310/1963/7789/7796/9806 �ȷ��Ϳ�����ʾָ�� */
    {
        lcd_wr_regno(0X29);     /* ������ʾ */
    }
}

/**
 * @brief       LCD�ر���ʾ
 * @param       ��
 * @retval      ��
 */
void lcd_display_off(void)
{
    if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(0X2800);   /* �ر���ʾ */
    }
    else    /* 9341/5310/1963/7789/7796/9806 �ȷ��͹ر���ʾָ�� */
    {
        lcd_wr_regno(0X28);     /* �ر���ʾ */
    }
}

/**
 * @brief       ���ù��λ��(��RGB����Ч)
 * @param       x,y: ����
 * @retval      ��
 */
void lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcddev.id == 0X1963)
    {
        if (lcddev.dir == 0)    /* ����ģʽ, x������Ҫ�任 */
        {
            x = lcddev.width - 1 - x;
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0XFF);
        }
        else                    /* ����ģʽ */
        {
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0XFF);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_data((lcddev.width - 1) & 0XFF);
        }

        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0XFF);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0XFF);

    }
    else if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(y & 0XFF);
    }
    else    /* 9341/5310/7789/7796/9806 �� �������� */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_data(x & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0XFF);
    }
}

/**
 * @brief       ����LCD���Զ�ɨ�跽��(��RGB����Ч)
 *   @note
 *              9341/5310/5510/1963/7789/7796/9806��IC�Ѿ�ʵ�ʲ���
 *              ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341),
 *              ����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
 *
 * @param       dir:0~7,����8������(���嶨���lcd.h)
 * @retval      ��
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    /* ����ʱ����1963���ı�ɨ�跽��, ����IC�ı�ɨ�跽������ʱ1963�ı䷽��, ����IC���ı�ɨ�跽�� */
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) || (lcddev.dir == 0 && lcddev.id == 0X1963))
    {
        switch (dir)   /* ����ת�� */
        {
            case 0:
                dir = 6;
                break;

            case 1:
                dir = 7;
                break;

            case 2:
                dir = 4;
                break;

            case 3:
                dir = 5;
                break;

            case 4:
                dir = 1;
                break;

            case 5:
                dir = 0;
                break;

            case 6:
                dir = 3;
                break;

            case 7:
                dir = 2;
                break;
        }
    }
 
    /* ����ɨ�跽ʽ ���� 0X36/0X3600 �Ĵ��� bit 5,6,7 λ��ֵ */
    switch (dir)
    {
        case L2R_U2D:/* ������,���ϵ��� */
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U:/* ������,���µ��� */
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D:/* ���ҵ���,���ϵ��� */
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U:/* ���ҵ���,���µ��� */
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R:/* ���ϵ���,������ */
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L:/* ���ϵ���,���ҵ��� */
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R:/* ���µ���,������ */
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L:/* ���µ���,���ҵ��� */
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    dirreg = 0X36;  /* �Ծ��󲿷�����IC, ��0X36�Ĵ������� */

    if (lcddev.id == 0X5510)
    {
        dirreg = 0X3600;    /* ����5510, ����������ic�ļĴ����в��� */
    }

    /* 9341 & 7789 & 7796 Ҫ����BGRλ */
    if (lcddev.id == 0X9341 || lcddev.id == 0X7789 || lcddev.id == 0x7796)
    {
        regval |= 0X08;
    }

    lcd_write_reg(dirreg, regval);

    if (lcddev.id != 0X1963)   /* 1963�������괦�� */
    {
        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   /* ����X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   /* ����X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    /* ������ʾ����(����)��С */
    if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data((lcddev.width - 1) & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data((lcddev.height - 1) & 0XFF);
    }
    else
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_data((lcddev.width - 1) & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0);
        lcd_wr_data(0);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0XFF);
    }
}

/**
 * @brief       ����
 * @param       x,y: ����
 * @param       color: �����ɫ(32λ��ɫ,�������LTDC)
 * @retval      ��
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    lcd_set_cursor(x, y);       /* ���ù��λ�� */
    lcd_write_ram_prepare();    /* ��ʼд��GRAM */
    LCD->LCD_RAM = color;
}

/**
 * @brief       SSD1963�����������ú���
 * @param       pwm: ����ȼ�,0~100.Խ��Խ��.
 * @retval      ��
 */
void lcd_ssd_backlight_set(uint8_t pwm)
{
    lcd_wr_regno(0xBE);         /* ����PWM��� */
    lcd_wr_data(0x05);          /* 1����PWMƵ�� */
    lcd_wr_data(pwm * 2.55);    /* 2����PWMռ�ձ� */
    lcd_wr_data(0x01);          /* 3����C */
    lcd_wr_data(0xFF);          /* 4����D */
    lcd_wr_data(0x00);          /* 5����E */
    lcd_wr_data(0x00);          /* 6����F */
}

/**
 * @brief       ����LCD��ʾ����
 * @param       dir:0,����; 1,����
 * @retval      ��
 */
void lcd_display_dir(uint8_t dir)
{
    lcddev.dir = dir;   /* ����/���� */

    if (dir == 0)       /* ���� */
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0X1963)
        {
            lcddev.wramcmd = 0X2C;  /* ����д��GRAM��ָ�� */
            lcddev.setxcmd = 0X2B;  /* ����дX����ָ�� */
            lcddev.setycmd = 0X2A;  /* ����дY����ָ�� */
            lcddev.width = 480;     /* ���ÿ��480 */
            lcddev.height = 800;    /* ���ø߶�800 */
        }
        else   /* ����IC, ����: 9341 / 5310 / 7789 / 7796 / 9806 ��IC */
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310 || lcddev.id == 0x7796)     /* �����5310/7796 ���ʾ�� 320*480�ֱ��� */
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }

        if (lcddev.id == 0X9806)    /* �����9806 ���ʾ�� 480*800 �ֱ��� */
        {
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }
    else                /* ���� */
    {
        lcddev.width = 320;         /* Ĭ�Ͽ�� */
        lcddev.height = 240;        /* Ĭ�ϸ߶� */

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0X1963 || lcddev.id == 0x9806)
        {
            lcddev.wramcmd = 0X2C;  /* ����д��GRAM��ָ�� */
            lcddev.setxcmd = 0X2A;  /* ����дX����ָ�� */
            lcddev.setycmd = 0X2B;  /* ����дY����ָ�� */
            lcddev.width = 800;     /* ���ÿ��800 */
            lcddev.height = 480;    /* ���ø߶�480 */
        }
        else   /* ����IC, ����: 9341 / 5310 / 7789 / 7796 ��IC */
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310 || lcddev.id == 0x7796)     /* �����5310/7796 ���ʾ�� 320*480�ֱ��� */
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR);     /* Ĭ��ɨ�跽�� */
}

/**
 * @brief       ���ô���(��RGB����Ч),���Զ����û������굽�������Ͻ�(sx,sy).
 * @param       sx,sy:������ʼ����(���Ͻ�)
 * @param       width,height:���ڿ�Ⱥ͸߶�,�������0!!
 *   @note      �����С:width*height.
 *
 * @retval      ��
 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0X1963 && lcddev.dir != 1)    /* 1963�������⴦�� */
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0XFF);
        lcd_wr_data((sx + width - 1) >> 8);
        lcd_wr_data((sx + width - 1) & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0XFF);
        lcd_wr_data(height >> 8);
        lcd_wr_data(height & 0XFF);
    }
    else if (lcddev.id == 0X5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(sx & 0XFF);
        lcd_wr_regno(lcddev.setxcmd + 2);
        lcd_wr_data(twidth >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3);
        lcd_wr_data(twidth & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(sy & 0XFF);
        lcd_wr_regno(lcddev.setycmd + 2);
        lcd_wr_data(theight >> 8);
        lcd_wr_regno(lcddev.setycmd + 3);
        lcd_wr_data(theight & 0XFF);
    }
    else    /* 9341/5310/7789/1963/7796/9806���� �� ���ô��� */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0XFF);
        lcd_wr_data(twidth >> 8);
        lcd_wr_data(twidth & 0XFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0XFF);
        lcd_wr_data(theight >> 8);
        lcd_wr_data(theight & 0XFF);
    }
}

/**
 * @brief       ��ʼ��LCD
 *   @note      �ó�ʼ���������Գ�ʼ�������ͺŵ�LCD(�����.c�ļ���ǰ�������)
 *
 * @param       ��
 * @retval      ��
 */
void lcd_init(void)
{
    LCD_CS_GPIO_CLK_ENABLE();   /* LCD_CS��ʱ��ʹ�� */
    LCD_WR_GPIO_CLK_ENABLE();   /* LCD_WR��ʱ��ʹ�� */
    LCD_RD_GPIO_CLK_ENABLE();   /* LCD_RD��ʱ��ʹ�� */
    LCD_RS_GPIO_CLK_ENABLE();   /* LCD_RS��ʱ��ʹ�� */
    LCD_BL_GPIO_CLK_ENABLE();   /* LCD_BL��ʱ��ʹ�� */

    RCC->APB2ENR |= 3 << 5;     /* ʹ��PD,PE */
    RCC->AHBENR |= 1 << 8;      /* ʹ��FSMCʱ�� */

    sys_gpio_set(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LCD_CS����ģʽ���� */

    sys_gpio_set(LCD_WR_GPIO_PORT, LCD_WR_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LCD_WR����ģʽ���� */

    sys_gpio_set(LCD_RD_GPIO_PORT, LCD_RD_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LCD_RD����ģʽ���� */

    sys_gpio_set(LCD_RS_GPIO_PORT, LCD_RS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* LCD_RS����ģʽ���� */

    sys_gpio_set(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* LCD_BL����ģʽ����(�������) */

    /* LCD_D0~LCD_D15 IO�ڳ�ʼ�� */
    sys_gpio_set(GPIOD, (3 << 0) | (7 << 8) | (3 << 14), SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* PD0,1,8,9,10,14,15   AF OUT */
    sys_gpio_set(GPIOE, (0X1FF << 7), SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);                      /* PE7~15  AF OUT */

    /* FSMCʱ������HCLK, Ƶ��Ϊ72Mhz
     * �Ĵ�������
     * bank1��NE1~4,ÿһ����һ��BCR+TCR�������ܹ��˸��Ĵ�����
     * ��������ʹ��NE4 ��Ҳ�Ͷ�ӦBTCR[6], [7]
     */
    LCD_FSMC_BCRX = 0X00000000; /* BCR�Ĵ������� */
    LCD_FSMC_BTRX = 0X00000000; /* BTR�Ĵ������� */
    LCD_FSMC_BWTRX = 0X00000000;/* BWTR�Ĵ������� */

    /* ����BCR�Ĵ��� ʹ���첽ģʽ */
    LCD_FSMC_BCRX |= 1 << 12;   /* �洢��дʹ�� */
    LCD_FSMC_BCRX |= 1 << 14;   /* ��дʹ�ò�ͬ��ʱ�� */
    LCD_FSMC_BCRX |= 1 << 4;    /* �洢�����ݿ��Ϊ16bit */

    /* ����BTR�Ĵ���, ��ʱ����ƼĴ��� */
    LCD_FSMC_BTRX |= 0 << 28;   /* ģʽA */
    LCD_FSMC_BTRX |= 0 << 0;    /* ��ַ����ʱ��(ADDSET)Ϊ1��HCLK 1/72M = 13.9ns (ʵ�� > 200ns) */

    /* ��ΪҺ������IC�Ķ����ݵ�ʱ���ٶȲ���̫��,�����Ǹ�������оƬ */
    LCD_FSMC_BTRX |= 15 << 8;   /* ���ݱ���ʱ��(DATAST)Ϊ16��HCLK = 13.9 * 16 = 222.4ns */

    /* дʱ����ƼĴ��� */
    LCD_FSMC_BWTRX |= 0 << 28;  /* ģʽA */
    LCD_FSMC_BWTRX |= 0 << 0;   /* ��ַ����ʱ��(ADDSET)Ϊ1��HCLK = 13.9ns */

    /* ĳЩҺ������IC��д�ź���������Ҳ��50ns�� */
    LCD_FSMC_BWTRX |= 1 << 8;   /* ���ݱ���ʱ��(DATAST)Ϊ2��HCLK = 13.9 * 2 = 27.8ns (ʵ�� > 200ns) */

    /* ʹ��BANK1,����1 */
    LCD_FSMC_BCRX |= 1 << 0;    /* ʹ��BANK1������1 */
    LCD_FSMC_BCRX |= (uint32_t)1 << 31; /* ʹ��FSMC */

    lcd_opt_delay(0X1FFFF);     /* ��ʼ��FSMC��,����ȴ�һ��ʱ����ܿ�ʼ��ʼ�� */

    /* ����9341 ID�Ķ�ȡ */
    lcd_wr_regno(0XD3);
    lcddev.id = lcd_rd_data();  /* dummy read */
    lcddev.id = lcd_rd_data();  /* ����0X00 */
    lcddev.id = lcd_rd_data();  /* ��ȡ0X93 */
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); /* ��ȡ0X41 */

    if (lcddev.id != 0X9341)    /* ���� 9341 , ���Կ����ǲ��� ST7789 */
    {
        lcd_wr_regno(0X04);
        lcddev.id = lcd_rd_data();      /* dummy read */
        lcddev.id = lcd_rd_data();      /* ����0X85 */
        lcddev.id = lcd_rd_data();      /* ��ȡ0X85 */
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data();     /* ��ȡ0X52 */
        
        if (lcddev.id == 0X8552)        /* ��8552��IDת����7789 */
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789)        /* Ҳ����ST7789, �����ǲ��� NT35310 */
        {
            lcd_wr_regno(0xD4);
            lcddev.id = lcd_rd_data();  /* dummy read */
            lcddev.id = lcd_rd_data();  /* ����0x01 */
            lcddev.id = lcd_rd_data();  /* ����0x53 */
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); /* �������0x10 */

            if (lcddev.id != 0x5310)    /* Ҳ����NT35310,���Կ����ǲ���ST7796 */
            {
                lcd_wr_regno(0XD3);
                lcddev.id = lcd_rd_data();  /* dummy read */
                lcddev.id = lcd_rd_data();  /* ����0X00 */
                lcddev.id = lcd_rd_data();  /* ��ȡ0X77 */
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); /* ��ȡ0X96 */
                
                if (lcddev.id != 0x7796)    /* Ҳ����ST7796,���Կ����ǲ���NT35510 */
                {
                    /* ������Կ�������ṩ�� */
                    lcd_write_reg(0xF000, 0x0055);
                    lcd_write_reg(0xF001, 0x00AA);
                    lcd_write_reg(0xF002, 0x0052);
                    lcd_write_reg(0xF003, 0x0008);
                    lcd_write_reg(0xF004, 0x0001);

                    lcd_wr_regno(0xC500);       /* ��ȡID�Ͱ�λ */
                    lcddev.id = lcd_rd_data();  /* ����0x55 */
                    lcddev.id <<= 8;

                    lcd_wr_regno(0xC501);       /* ��ȡID�߰�λ */
                    lcddev.id |= lcd_rd_data(); /* ����0x10 */
                    
                    delay_ms(5);                /* �ȴ�5ms, ��Ϊ0XC501ָ���1963��˵���������λָ��, �ȴ�5ms��1963��λ����ٲ��� */

                    if (lcddev.id != 0x5510)    /* Ҳ����NT5510,���Կ����ǲ���ILI9806 */
                    {
                        lcd_wr_regno(0XD3);
                        lcddev.id = lcd_rd_data();  /* dummy read */
                        lcddev.id = lcd_rd_data();  /* ����0X00 */
                        lcddev.id = lcd_rd_data();  /* ����0X98 */
                        lcddev.id <<= 8;
                        lcddev.id |= lcd_rd_data(); /* ����0X06 */
                        
                        if (lcddev.id != 0x9806)    /* Ҳ����ILI9806,���Կ����ǲ���SSD1963 */
                        {
                            lcd_wr_regno(0xA1);
                            lcddev.id = lcd_rd_data();
                            lcddev.id = lcd_rd_data();  /* ����0x57 */
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); /* ����0x61 */

                            if (lcddev.id == 0x5761) lcddev.id = 0x1963; /* SSD1963���ص�ID��5761H,Ϊ��������,����ǿ������Ϊ1963 */
                        }
                    }
                }
            }
        }
    }

    /* �ر�ע��, �����main�����������δ���1��ʼ��, ��Ῠ����printf
     * ����(������f_putc����), ����, �����ʼ������1, �������ε�����
     * ���� printf ��� !!!!!!!
     */
    //printf("LCD ID:%x\r\n", lcddev.id); /* ��ӡLCD ID */

    if (lcddev.id == 0X7789)
    {
        lcd_ex_st7789_reginit();    /* ִ��ST7789��ʼ�� */
    }
    else if (lcddev.id == 0X9341)
    {
        lcd_ex_ili9341_reginit();   /* ִ��ILI9341��ʼ�� */
    }
    else if (lcddev.id == 0x5310)
    {
        lcd_ex_nt35310_reginit();   /* ִ��NT35310��ʼ�� */
    }
    else if (lcddev.id == 0x7796)
    {
        lcd_ex_st7796_reginit();    /* ִ��ST7796��ʼ�� */
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_ex_nt35510_reginit();   /* ִ��NT35510��ʼ�� */
    }
    else if (lcddev.id == 0x9806)
    {
        lcd_ex_ili9806_reginit();   /* ִ��ILI9806��ʼ�� */
    }
    else if (lcddev.id == 0x1963)
    {
        lcd_ex_ssd1963_reginit();   /* ִ��SSD1963��ʼ�� */
        lcd_ssd_backlight_set(100); /* ��������Ϊ���� */
    }

    lcd_display_dir(0); /* Ĭ��Ϊ���� */
    LCD_BL(1);          /* �������� */
    lcd_clear(WHITE);
}

/**
 * @brief       ��������
 * @param       color: Ҫ��������ɫ
 * @retval      ��
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;    /* �õ��ܵ��� */
    lcd_set_cursor(0x00, 0x0000);   /* ���ù��λ�� */
    lcd_write_ram_prepare();        /* ��ʼд��GRAM */

    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
   }
}

/**
 * @brief       ��ָ����������䵥����ɫ
 * @param       (sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex - sx + 1) * (ey - sy + 1)
 * @param       color:Ҫ������ɫ(32λ��ɫ,�������LTDC)
 * @retval      ��
 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;
    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        lcd_set_cursor(sx, i);      /* ���ù��λ�� */
        lcd_write_ram_prepare();    /* ��ʼд��GRAM */

        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM = color;   /* ��ʾ��ɫ */
        }
    }
}

/**
 * @brief       ��ָ�����������ָ����ɫ��
 * @param       (sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: Ҫ������ɫ�����׵�ַ
 * @retval      ��
 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;            /* �õ����Ŀ�� */
    height = ey - sy + 1;           /* �߶� */

    for (i = 0; i < height; i++)
    {
        lcd_set_cursor(sx, sy + i); /* ���ù��λ�� */
        lcd_write_ram_prepare();    /* ��ʼд��GRAM */

        for (j = 0; j < width; j++)
        {
            LCD->LCD_RAM = color[i * width + j]; /* д������ */
        }
    }
}

/**
 * @brief       ����
 * @param       x1,y1: �������
 * @param       x2,y2: �յ�����
 * @param       color: �ߵ���ɫ
 * @retval      ��
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1;          /* ������������ */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)incx = 1;   /* ���õ������� */
    else if (delta_x == 0)incx = 0; /* ��ֱ�� */
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; /* ˮƽ�� */
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if ( delta_x > delta_y)distance = delta_x;  /* ѡȡ�������������� */
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++ )   /* ������� */
    {
        lcd_draw_point(row, col, color); /* ���� */
        xerr += delta_x ;
        yerr += delta_y ;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       ��ˮƽ��
 * @param       x0,y0: �������
 * @param       len  : �߳���
 * @param       color: ���ε���ɫ
 * @retval      ��
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > lcddev.width) || (y > lcddev.height))return;

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief       ������
 * @param       x1,y1: �������
 * @param       x2,y2: �յ�����
 * @param       color: ���ε���ɫ
 * @retval      ��
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       ��Բ
 * @param       x,y  : Բ��������
 * @param       r    : �뾶
 * @param       color: Բ����ɫ
 * @retval      ��
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);       /* �ж��¸���λ�õı�־ */

    while (a <= b)
    {
        lcd_draw_point(x0 + a, y0 - b, color);  /* 5 */
        lcd_draw_point(x0 + b, y0 - a, color);  /* 0 */
        lcd_draw_point(x0 + b, y0 + a, color);  /* 4 */
        lcd_draw_point(x0 + a, y0 + b, color);  /* 6 */
        lcd_draw_point(x0 - a, y0 + b, color);  /* 1 */
        lcd_draw_point(x0 - b, y0 + a, color);
        lcd_draw_point(x0 - a, y0 - b, color);  /* 2 */
        lcd_draw_point(x0 - b, y0 - a, color);  /* 7 */
        a++;

        /* ʹ��Bresenham�㷨��Բ */
        if (di < 0)
        {
            di += 4 * a + 6;
        }
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/**
 * @brief       ���ʵ��Բ
 * @param       x,y: Բ��������
 * @param       r    : �뾶
 * @param       color: Բ����ɫ
 * @retval      ��
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + xr * xr) > sqmax)
        {
            /* draw lines from outside */
            if (xr > imax)
            {
                lcd_draw_hline (x - i + 1, y + xr, 2 * (i - 1), color);
                lcd_draw_hline (x - i + 1, y - xr, 2 * (i - 1), color);
            }

            xr--;
        }

        /* draw lines from inside (center) */
        lcd_draw_hline(x - xr, y + i, 2 * xr, color);
        lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}

/**
 * @brief       ��ָ��λ����ʾһ���ַ�
 * @param       x,y  : ����
 * @param       chr  : Ҫ��ʾ���ַ�:" "--->"~"
 * @param       size : �����С 12/16/24/32
 * @param       mode : ���ӷ�ʽ(1); �ǵ��ӷ�ʽ(0);
 * @param       color : �ַ�����ɫ;
 * @retval      ��
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); /* �õ�����һ���ַ���Ӧ������ռ���ֽ��� */
    chr = chr - ' ';    /* �õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩ */

    switch (size)
    {
        case 12:
            pfont = (uint8_t *)asc2_1206[chr];  /* ����1206���� */
            break;

        case 16:
            pfont = (uint8_t *)asc2_1608[chr];  /* ����1608���� */
            break;

        case 24:
            pfont = (uint8_t *)asc2_2412[chr];  /* ����2412���� */
            break;

        case 32:
            pfont = (uint8_t *)asc2_3216[chr];  /* ����3216���� */
            break;

        default:
            return ;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];    /* ��ȡ�ַ��ĵ������� */

        for (t1 = 0; t1 < 8; t1++)   /* һ���ֽ�8���� */
        {
            if (temp & 0x80)        /* ��Ч��,��Ҫ��ʾ */
            {
                lcd_draw_point(x, y, color);        /* �������,Ҫ��ʾ����� */
            }
            else if (mode == 0)     /* ��Ч��,����ʾ */
            {
                lcd_draw_point(x, y, g_back_color); /* ������ɫ,�൱������㲻��ʾ(ע�ⱳ��ɫ��ȫ�ֱ�������) */
            }

            temp <<= 1; /* ��λ, �Ա��ȡ��һ��λ��״̬ */
            y++;

            if (y >= lcddev.height)return;  /* �������� */

            if ((y - y0) == size)   /* ��ʾ��һ����? */
            {
                y = y0; /* y���긴λ */
                x++;    /* x������� */

                if (x >= lcddev.width)return;   /* x���곬������ */

                break;
            }
        }
    }
}

/**
 * @brief       ƽ������, m^n
 * @param       m: ����
 * @param       n: ָ��
 * @retval      m��n�η�
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)result *= m;

    return result;
}

/**
 * @brief       ��ʾlen������
 * @param       x,y : ��ʼ����
 * @param       num : ��ֵ(0 ~ 2^32)
 * @param       len : ��ʾ���ֵ�λ��
 * @param       size: ѡ������ 12/16/24/32
 * @param       color : ���ֵ���ɫ;
 * @retval      ��
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)   /* ������ʾλ��ѭ�� */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;   /* ��ȡ��Ӧλ������ */

        if (enshow == 0 && t < (len - 1))   /* û��ʹ����ʾ,�һ���λҪ��ʾ */
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2)*t, y, ' ', size, 0, color);/* ��ʾ�ո�,ռλ */
                continue;   /* �����¸�һλ */
            }
            else
            {
                enshow = 1; /* ʹ����ʾ */
            }

        }

        lcd_show_char(x + (size / 2)*t, y, temp + '0', size, 0, color); /* ��ʾ�ַ� */
    }
}

/**
 * @brief       ��չ��ʾlen������(��λ��0Ҳ��ʾ)
 * @param       x,y : ��ʼ����
 * @param       num : ��ֵ(0 ~ 2^32)
 * @param       len : ��ʾ���ֵ�λ��
 * @param       size: ѡ������ 12/16/24/32
 * @param       mode: ��ʾģʽ
 *              [7]:0,�����;1,���0.
 *              [6:1]:����
 *              [0]:0,�ǵ�����ʾ;1,������ʾ.
 * @param       color : ���ֵ���ɫ;
 * @retval      ��
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)   /* ������ʾλ��ѭ�� */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;    /* ��ȡ��Ӧλ������ */

        if (enshow == 0 && t < (len - 1))   /* û��ʹ����ʾ,�һ���λҪ��ʾ */
        {
            if (temp == 0)
            {
                if (mode & 0X80)   /* ��λ��Ҫ���0 */
                {
                    lcd_show_char(x + (size / 2)*t, y, '0', size, mode & 0X01, color);  /* ��0ռλ */
                }
                else
                {
                    lcd_show_char(x + (size / 2)*t, y, ' ', size, mode & 0X01, color);  /* �ÿո�ռλ */
                }

                continue;
            }
            else
            {
                enshow = 1; /* ʹ����ʾ */
            }

        }

        lcd_show_char(x + (size / 2)*t, y, temp + '0', size, mode & 0X01, color);
    }
}

/**
 * @brief       ��ʾ�ַ���
 * @param       x,y         : ��ʼ����
 * @param       width,height: �����С
 * @param       size        : ѡ������ 12/16/24/32
 * @param       p           : �ַ����׵�ַ
 * @param       color       : �ַ�������ɫ;
 * @retval      ��
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' '))   /* �ж��ǲ��ǷǷ��ַ�! */
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }

        if (y >= height)break;  

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}

//����
const unsigned char chinese [100][36]={
{0x10,0x04,0x57,0xD5,0x55,0x7E,0x55,0x54,0xF7,0xD4,0x55,0x54,0x55,0x7E,0x57,0xD5,0x10,0x05,0x10,0x02,0xFF,0xE4,0x10,0x18,0x50,0x64,0x33,0x82,0x10,0x0F,0x00,0x00},/*"��",0*/

{0x00,0x80,0x01,0x00,0x06,0x00,0x1F,0xFF,0xE0,0x00,0x10,0x40,0x12,0x40,0x12,0x40,0x12,0x40,0xFF,0xFF,0x12,0x40,0x12,0x40,0x12,0x48,0x12,0x44,0x10,0x78,0x00,0x00},/*"ΰ",1*/
    
{0x20,0x04,0x21,0x06,0x21,0x04,0x3F,0xF8,0x21,0x08,0x21,0x08,0x00,0x20,0x20,0x40,0x20,0x80,0x21,0x00,0x27,0xFF,0x38,0x00,0x21,0x00,0x20,0x80,0x20,0x60,0x00,0x00},/*"��",2*/

{0x08,0x08,0x08,0x0C,0xFF,0xF8,0x08,0x10,0x08,0x11,0x04,0x01,0x25,0xF2,0x35,0x5C,0xAD,0x50,0x65,0x50,0x2D,0x5E,0x35,0x51,0x25,0xF1,0x04,0x01,0x04,0x07,0x00,0x00},/*"��",3*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x02,0x00,0x7E,0x7F,0x42,0x49,0x42,0x49,0x7E,0x49,0x42,0x49,0x7E,0x49,0x42,0x7F,0x42,0x00,0x7E,0x00,0x02,0x00,0x00},/*"��",4*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x22,0x7F,0x12,0x49,0x0A,0x49,0xFE,0x49,0x02,0x49,0x02,0x49,0xFE,0x49,0x0A,0x7F,0x12,0x00,0x22,0x00,0x02,0x00,0x00},/*"ʪ",5*/

{0x00,0x02,0x00,0x0C,0x3F,0xF1,0x24,0x01,0x24,0x21,0x24,0x32,0x3F,0xAA,0xA4,0xA4,0x64,0xA4,0x24,0xA4,0x3F,0xAA,0x24,0x32,0x24,0x01,0x24,0x01,0x20,0x01,0x00,0x00},/*"��",6*/

{0x00,0x01,0x00,0x02,0x40,0x0C,0x40,0x30,0x7F,0xC1,0x42,0x01,0x41,0x82,0x40,0x62,0x42,0x14,0x4E,0x08,0x72,0x14,0x02,0x62,0x03,0x82,0x00,0x01,0x00,0x01,0x00,0x00},/*"��",7*/

{0x22,0x10,0x1A,0x60,0x03,0x80,0xFF,0xFF,0x0A,0x80,0x32,0x60,0x01,0x01,0x02,0x02,0x0D,0x0C,0x71,0xF0,0x01,0x02,0xF1,0x01,0x0D,0xFE,0x02,0x00,0x01,0x00,0x00,0x00},/*"��",8*/

{0x00,0x02,0x02,0x02,0x04,0x22,0x08,0x22,0x30,0x22,0x00,0x22,0x00,0x22,0xFE,0xFE,0x00,0x22,0x00,0x22,0x20,0x22,0x10,0x22,0x08,0x22,0x06,0x02,0x00,0x02,0x00,0x00},/*"��",9*/

{0x00,0x02,0x00,0x02,0x7E,0x7E,0x00,0x42,0x00,0x42,0xFF,0x7E,0x00,0x42,0x02,0x42,0x0C,0x42,0xF0,0x7E,0x20,0x42,0x28,0x42,0x26,0x7E,0x20,0x02,0x00,0x02,0x00,0x00},/*"��",10*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x01,0x7F,0xE2,0x40,0x0C,0x4F,0xF0,0x40,0x08,0x7F,0xE4,0x00,0x00,0x1F,0xE2,0x00,0x01,0xFF,0xFE,0x00,0x00,0x00,0x00},/*"��",11*/

{0x00,0x00,0x00,0x42,0x44,0x44,0x4C,0xC8,0x54,0xD0,0x65,0x42,0x45,0x41,0x46,0x7E,0x84,0x40,0x88,0x40,0x90,0x50,0x81,0x48,0x80,0xC4,0x00,0x62,0x00,0x00,0x00,0x00},/*"ϵ",12*/

{0x04,0x44,0x0C,0xE6,0x35,0x44,0xC6,0x48,0x0C,0x48,0x00,0x01,0x11,0x02,0x13,0x0C,0x15,0xF0,0x99,0x00,0x71,0x00,0x11,0xFC,0x15,0x02,0x13,0x02,0x11,0x8E,0x00,0x00},/*"ͳ",13*/

{0x00,0x20,0x10,0x20,0x0C,0x40,0x00,0x80,0xFF,0xFF,0x04,0x01,0x04,0x02,0x04,0x0C,0x04,0x70,0xFF,0x80,0x04,0x60,0x04,0x18,0x44,0x04,0x34,0x02,0x04,0x01,0x00,0x00},/*"״",14*/

{0x00,0x82,0x20,0x9C,0x21,0x00,0x21,0x00,0x22,0x3C,0x24,0x02,0x2A,0x02,0xF1,0x42,0x28,0x32,0x24,0x02,0x22,0x02,0x21,0x0E,0x21,0x20,0x20,0x90,0x00,0x8C,0x00,0x00},/*"̬",15*/

{0x01,0x00,0x41,0x01,0x41,0x02,0x41,0x0C,0x7F,0xF0,0x41,0x00,0x41,0x00,0x41,0x00,0x41,0x00,0x41,0x00,0x7F,0xFF,0x41,0x00,0x41,0x00,0x41,0x00,0x01,0x00,0x00,0x00},/*"��",16*/

{0x00,0x81,0x00,0x81,0x08,0x82,0x88,0x82,0x68,0x84,0x08,0x88,0x08,0xB0,0x0F,0xC0,0x08,0xB0,0x08,0x88,0x28,0x84,0xC8,0x82,0x08,0x82,0x00,0x81,0x00,0x81,0x00,0x00},/*"��",17*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x02,0x00,0x7E,0x7F,0x42,0x49,0x42,0x49,0x7E,0x49,0x42,0x49,0x7E,0x49,0x42,0x7F,0x42,0x00,0x7E,0x00,0x02,0x00,0x00},/*"��",18*/

{0x00,0x02,0x00,0x0C,0x3F,0xF1,0x24,0x01,0x24,0x21,0x24,0x32,0x3F,0xAA,0xA4,0xA4,0x64,0xA4,0x24,0xA4,0x3F,0xAA,0x24,0x32,0x24,0x01,0x24,0x01,0x20,0x01,0x00,0x00},/*"��",19*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x22,0x7F,0x12,0x49,0x0A,0x49,0xFE,0x49,0x02,0x49,0x02,0x49,0xFE,0x49,0x0A,0x7F,0x12,0x00,0x22,0x00,0x02,0x00,0x00},/*"ʪ",20*/

{0x00,0x02,0x00,0x0C,0x3F,0xF1,0x24,0x01,0x24,0x21,0x24,0x32,0x3F,0xAA,0xA4,0xA4,0x64,0xA4,0x24,0xA4,0x3F,0xAA,0x24,0x32,0x24,0x01,0x24,0x01,0x20,0x01,0x00,0x00},/*"��",21*/

{0x60,0x00,0x90,0x00,0x90,0x00,0x67,0xE0,0x1F,0xF8,0x30,0x0C,0x20,0x04,0x40,0x02,0x40,0x02,0x40,0x02,0x40,0x02,0x40,0x02,0x20,0x04,0x78,0x08,0x00,0x00,0x00,0x00},/*"��",22*/

{0x08,0x00,0x30,0x02,0x22,0x02,0x24,0x82,0x28,0x82,0x20,0x82,0xA0,0x82,0x60,0xFE,0x20,0x82,0x20,0x82,0x28,0x82,0x24,0x82,0x22,0x02,0x28,0x02,0x30,0x00,0x00,0x00},/*"��",23*/

{0x04,0x00,0x08,0x00,0x32,0x00,0xE2,0x00,0x2A,0x00,0x2A,0x00,0x2A,0x00,0x2A,0x00,0x2A,0x00,0x2A,0x00,0x2A,0x00,0x2B,0xF0,0x20,0x0C,0x20,0x02,0x00,0x0F,0x00,0x00},/*"��",24*/

{0x00,0x01,0x00,0x06,0x7F,0xF8,0x48,0x01,0x48,0x01,0x49,0xFA,0x49,0x02,0x49,0x04,0x49,0x08,0xFF,0x70,0x89,0x08,0x89,0x08,0x89,0x04,0x89,0xFA,0x08,0x01,0x00,0x00},/*"��",25*/

{0x04,0x00,0x04,0x01,0x04,0x01,0x7D,0xF5,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0xFF,0x55,0x55,0x55,0x55,0x55,0x55,0x7D,0xF5,0x04,0x01,0x04,0x01,0x04,0x00,0x00,0x00},/*"��",26*/

{0x00,0x80,0x01,0x00,0x06,0x00,0x1F,0xFF,0xE0,0x00,0x08,0x01,0x08,0x06,0x08,0x38,0xFF,0xC0,0x08,0x00,0x0F,0xFC,0x88,0x02,0x68,0x02,0x08,0x02,0x08,0x1E,0x00,0x00},/*"��",27*/

{0x00,0x00,0x00,0x00,0x3F,0xFF,0x24,0x81,0x24,0x82,0x24,0x84,0xA4,0xC0,0x64,0xA0,0x24,0x90,0x24,0x88,0x24,0x94,0x3F,0xA2,0x00,0x42,0x00,0x01,0x00,0x01,0x00,0x00},/*"��",28*/

{0x00,0x00,0x00,0x00,0x0F,0xF0,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0xFF,0xFF,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x0F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00},/*"��",29*/

{0x00,0x40,0x08,0x80,0x11,0x00,0x23,0xFF,0xCC,0x00,0x00,0x00,0x7F,0xFF,0x49,0x02,0x49,0x04,0x49,0xC0,0x49,0x30,0x49,0x28,0x7F,0x44,0x00,0x82,0x00,0x02,0x00,0x00},/*"��",30*/

{0x00,0x82,0x20,0x84,0x24,0x88,0x24,0x91,0xA4,0xA1,0x64,0xD1,0x27,0x91,0x3C,0x91,0x24,0x9F,0x64,0x91,0xA4,0x91,0x24,0x91,0x24,0x91,0x20,0x81,0x00,0x80,0x00,0x00},/*"��",31*/

{0x00,0x00,0x1F,0xFF,0x80,0x00,0x44,0x08,0x05,0xC8,0x45,0x50,0x45,0xD2,0x44,0x04,0x7F,0xE8,0x44,0x30,0x55,0xC8,0x4C,0x1C,0x40,0x01,0x7F,0xFF,0x00,0x00,0x00,0x00},/*"��",32*/

{0x00,0x80,0x01,0x00,0x06,0x00,0x1F,0xFF,0xE0,0x02,0x20,0x02,0x27,0xFE,0x25,0x52,0x25,0x52,0xFD,0x52,0x25,0x52,0x25,0x52,0x27,0xFE,0x20,0x02,0x00,0x02,0x00,0x00},/*"ֵ",33*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"��",34*/

{0x09,0x41,0x4A,0x59,0x2C,0x6A,0x08,0xC6,0xFF,0x44,0x08,0x4A,0x2C,0x71,0x4A,0x00,0x01,0x01,0x0E,0x02,0xF1,0xCC,0x10,0x30,0x10,0xCC,0x1F,0x02,0x10,0x01,0x00,0x00},/*"��" 35*/

{0x08,0x42,0x08,0x41,0xFF,0xFE,0x08,0x80,0x09,0x01,0x00,0x06,0x7F,0xF8,0x49,0x00,0x49,0x3F,0x49,0x22,0x4F,0xE2,0x49,0x22,0x49,0x22,0x79,0x3F,0x01,0x00,0x00,0x00},/*"��",36*/

{0x00,0x02,0x00,0x02,0x7E,0x7E,0x00,0x42,0x00,0x42,0xFF,0x7E,0x00,0x42,0x02,0x42,0x0C,0x42,0xF0,0x7E,0x20,0x42,0x28,0x42,0x26,0x7E,0x20,0x02,0x00,0x02,0x00,0x00},/*"��",37*/

{0x08,0x20,0x06,0x20,0x40,0x7E,0x31,0x80,0x00,0x01,0x7F,0xE2,0x40,0x0C,0x4F,0xF0,0x40,0x08,0x7F,0xE4,0x00,0x00,0x1F,0xE2,0x00,0x01,0xFF,0xFE,0x00,0x00,0x00,0x00},/*"��",38*/

{0x02,0x01,0x12,0x06,0x12,0xF8,0x12,0x04,0xFF,0xFE,0x12,0x22,0x12,0x22,0x42,0x02,0x45,0xF2,0x79,0x12,0x41,0x12,0x45,0x12,0x43,0x12,0x7D,0xF2,0x00,0x02,0x00,0x00},/*"��",39*/

{0x08,0x20,0x08,0xC0,0x0B,0x00,0xFF,0xFF,0x09,0x00,0x08,0xC8,0x04,0x30,0x44,0xC0,0x44,0x02,0x44,0x01,0x47,0xFE,0x44,0x00,0x44,0x80,0x44,0x60,0x04,0x18,0x00,0x00},/*"��",40*/
};



void Chinese_Show_one(u8 x, u8 y, u8 num, u8 size, u8 mode,uint32_t color)
{
    u8 temp,t,t1;
    u8 y0=y;
    u8 csize=(size/8 + ((size%8)?1:0)) * size;    
    for(t=0;t<csize;t++)
    {  
        if(size==16)     
					temp = chinese[num][t];     
        else 
					return;      
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80) 
							lcd_draw_point(x,y,color);
            else 
							lcd_draw_point(x,y,WHITE);
            temp<<=1;
            y++;
            if((y-y0)==size)
            {
                y=y0;
                x++;
                break;
            } 
         }  
     }  
}













