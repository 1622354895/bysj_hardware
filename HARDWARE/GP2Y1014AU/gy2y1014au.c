#include "gy2y1014au.h"

#include "lcd.h"

//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��0~3 
void GP2Y_Adc_Init(void)  //ADCͨ����ʼ��
{
	ADC_InitTypeDef ADC_InitStructure; 

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1, ENABLE );        //ʹ��GPIOA,ADC1ͨ��ʱ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //��Ƶ����6ʱ��Ϊ72M/6=12MHz

	//PA1��Ϊģ��ͨ����������  ��//û���Ҳ����                       
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;         //ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);        

	//PA5 ��Ϊ�����������    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;           
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GP2Y_High;//npn�ߵ�ƽ����ͨ

	ADC_DeInit(ADC1);  //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode= ADC_Mode_Independent;        //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode= DISABLE;   //ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode= DISABLE;        //ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv= ADC_ExternalTrigConv_None;      //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign= ADC_DataAlign_Right;       //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel= 1; //˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1,&ADC_InitStructure);  //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	ADC_Cmd(ADC1,ENABLE);    //ʹ��ָ����ADC1

	ADC_ResetCalibration(ADC1);       //����ָ����ADC1�ĸ�λ�Ĵ���

	while(ADC_GetResetCalibrationStatus(ADC1));        //��ȡADC1����У׼�Ĵ�����״̬,����״̬��ȴ�

	ADC_StartCalibration(ADC1);          //����ADУ׼

	while(ADC_GetCalibrationStatus(ADC1));              //��ȡָ��ADC1��У׼����,����״̬��ȴ�
}

//���ADCֵ
//ch:ͨ��ֵ 0~3
u16 GP2Y_Get_Adc(u8 ch)   
{
	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1,ch, 1, ADC_SampleTime_239Cycles5 );  //ADC1,ADCͨ��3,��һ��ת��,����ʱ��Ϊ239.5����                               

	ADC_SoftwareStartConvCmd(ADC1,ENABLE);             //ʹ��ָ����ADC1�����ת����������

	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);      //�������һ��ADC1�������ת�����
}


u16 GetGP2YSingleValue(void)
{
	uint16_t ADCVal;
	int dustVal = 0;
	float Voltage;

	//GPIO_SetBits(GPIOA, GPIO_Pin_0);//��1  �����ڲ�LED
	//delay_us(280); 			 // ����LED���280us�ĵȴ�ʱ��
	//ADCVal = GP2Y_Get_Adc(ADC_Channel_1);  //PA1 ��������ȡADֵ
    ADCVal = GPY2_Get_Adc_Aveurage(ADC_Channel_1,5);  //PA1 ��������ȡADֵ,ȡƽ�� ���������ȷ��
	//delay_us(19);			  //��ʱ19us����Ϊ����AD����������Ϊ239.5������ADת��һ�����ʱ21us��19��21�ټ�280�պ���320us
	//GPIO_ResetBits(GPIOA, GPIO_Pin_0);//��0
	//delay_us(9680);			//��Ҫ�����0.32ms/10ms��PWM�ź������������е�LED
	
	Voltage = 3.3f * ADCVal / 4096.f * 2; //���AO����ڵĵ�ѹֵ
    //Voltage = (float)ADCVal / 4095 * 5.0; //���AO����ڵĵ�ѹֵ
	
	dustVal = (0.17*Voltage-0.1)*1000;  //����1000��λ����ug/m3//

	//if (dustVal < 0)
		//dustVal = 0;            //��λ//

	//if (dustVal>500)        
		//dustVal=500;


    //lcd_show_num(120 , 180, Voltage*1000, 4, 16, RED); 
	
	return dustVal;
}

//��ȡͨ��ch��ת��ֵ
//ȡtimes��,Ȼ��ƽ��

//��ǰû�����������ֱ�ӻ�ȡ�رգ���λ��Ҳ������ǰ
u16 GPY2_Get_Adc_Aveurage(u8 ch,u8 times)
{
	u32 pm_val=0;
	u8 t;

	for(t=0;t<times;t++)
	{
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);//��1  �����ڲ�LED
        delay_us(280);
		pm_val+=GP2Y_Get_Adc(ch);
        delay_us(40);
         GPIO_SetBits(GPIOA, GPIO_Pin_5);//��1  �����ڲ�LED
       delay_us(9680);
		delay_us(5);
	}
	
	return pm_val/times;
}         











