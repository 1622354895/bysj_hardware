/*�������ϵͳ
��ΰ
*/

#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stdio.h"
#include "oled.h"
#include "gy2y1014au.h"
#include "dht11.h"
#include "beep.h"
#include "lcd.h"
#include "led.h"
#include "exti.h"
#include "key.h"
#include "lsens.h"
#include "esp8266.h"
#include "onenet.h"
#include <string.h>  
#include <stdlib.h> 


double temperature;
double humidity;
int PM ;
int light;
const char *dataPtr = NULL;
int timecount=0;

u8 humidityH;	  //ʪ����������
u8 humidityL;	  //ʪ��С������
u8 temperatureH;   //�¶���������
u8 temperatureL;   //�¶�С������
DHT11_Data_TypeDef DHT11_Data;

//���������beep LED0��ǡ��������
int flag_2=0;
int flag_3=0;
int flag_bj=0;
int enable_bj=1;  //��ֵ��������λ


//��������ֵ
typedef struct {
    char *control_name;
    int number;
} ControlInfo;


//����������߳� -- �¶�ʪ�ȡ��۳�������
void environment_monitoring_thread();

 //ϵͳ״̬��or �ر���ʾ
void display_system_status();

//�����������䱨������
void air_quality();

//�������
ControlInfo extract_control_info(const char *str);

// ����Ӳ���ĺ���
void control_hardware(const ControlInfo *info);

//��ȡ��ƽ̨����
void Get_onenet(void);

//ϵͳ״̬�ر�
void Off(void);

//��ʼ��
void init();

int main(void)
{
    //��ʼ��
    init();
    
    //ϵͳ��ʾ
    display_system_status();
    
        
    while(1)
  {
      //ϵͳ�رպ�ͨ����ƽ̨�����
      Get_onenet(); 

    if(flag) 
    {
       
      environment_monitoring_thread();    //��ʼ���

    }
    

    else{
        Off();
        }
    }

}



 //ϵͳ״̬��or �ر���ʾ
void display_system_status() {
    delay_ms(5);
    lcd_clear(WHITE);
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
    
    //lcd_show_string(45, 30, 200, 16, 16, "Observation System", GREEN);
    //lcd_show_string(90, 50, 200, 16, 16, "Dai Wei", GREEN);
    //ϵͳ��
    if (flag) { 
        //lcd_clear(WHITE);
        lcd_show_string(40, 70, 200, 16, 16, "Condition : Online", GREEN);

    }
    //�ر�
    else {
        lcd_show_string(40, 70, 250, 16, 16, "Condition : Offline  ", GREEN);
        
    }
}

//����������߳�
//��־λΪ0���˳�
void environment_monitoring_thread() {
    display_system_status();
        offsc=0;//�ָ���־λ
        display=1;
    while (flag) {
        DHT11_Init();
        // ѭ������־λ��Ϊ1ִ��
//        if (DHT11_Init()) {
//            lcd_show_string(30, 110, 200, 16, 16, "DHT11 Error", RED);
//            delay_ms(200);
//            lcd_fill(30, 110, 239, 130 + 16, WHITE);
//            delay_ms(200);
//        } 
       // else {    //dht11��ȡ�ɹ�
            //lcd_show_string(30, 110, 200, 16, 16, "DHT11 OK", RED);
            lcd_show_string(30, 130, 200, 16, 16, "Temp:  C", BLUE);
            lcd_show_string(30, 150, 200, 16, 16, "Humi:  %", BLUE);
            lcd_show_string(30, 167, 200, 16, 16, "Light:", BLUE);
            lcd_show_string(30+110, 130, 200, 16, 16, "  PM:", BLUE);
            lcd_show_string(30+130, 150, 200, 16, 16, "ug/m3", BLUE);
            
            //��ʾ���ݼ��
            Chinese_Show_one(30,110,35,16,0,BLUE);
            Chinese_Show_one(45,110,36,16,0,BLUE);
            Chinese_Show_one(60,110,37,16,0,BLUE);
            Chinese_Show_one(75,110,38,16,0,BLUE);
            lcd_show_string(90, 110, 200, 16, 16, ":", BLUE);
            
             //Ũ����ֵ����  ����while�����ʱ���������ֵʱ������ʾ����Ϊ�Ͳ�����������
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);   
            
            
            int t = 0;
            int pmman=0;
            // ѭ������־λ,���һ�ֵ,ÿ0.5���ȡһ�Σ�������Ҫ��
            while (flag) { 
                if (t % 4 == 0) {
                    
                    Read_DHT11(&DHT11_Data);
                    humidityH=DHT11_Data.humi_int;	  //ʪ����������
                    humidityL=DHT11_Data.humi_deci;	  //ʪ��С������
                    humidity = humidityH+humidityL/10.0;
                    
                    temperatureH=DHT11_Data.temp_int;   //�¶���������
                    temperatureL=DHT11_Data.temp_deci;   //�¶�С������
                    temperature = temperatureH+temperatureL/10.0;
                    
                    light = lsens_get_val();         //����
                    
                    pmman=GetGP2YSingleValue();
                    if(pmman<9999)  //��Ҫ�Ľ�
                        PM = pmman;       //�õ�pm2.5ֵ 
                    
                    
                    if(timecount>18)//3.6s�ϴ�һ��,������8266get��0����
                        {   
                            timecount = 0;
                    //�������ݵ���ƽ̨
                    printf("humi =%f ,temp =%f, PM =%d, light value =%d\r\n",humidity,temperature,PM,light);	
                    ESP8266_SendData(temperature,humidity,light,PM);//��������
                    ESP8266_SendData3(flag,pm_bj,tp_bj,hm_bj);
                    ESP8266_SendData4(pm_max,humi_max,temp_max);
                    //ESP8266_Clear(); ���������Ѿ������
                        }
                    
                            
                            //��ʾ��ȡ�Ļ���ֵ
                    lcd_show_num(30 + 40, 130, temperature, 2, 16, BLUE);
                    lcd_show_num(30 + 40, 150, humidity, 2, 16, BLUE);
                    lcd_show_num(30 + 80, 150, PM, 5, 16, BLUE);
                    lcd_show_xnum(30 + 45, 167, light, 3, 16, 0, BLUE); /* ��ʾ���յ�ֵ */
                }
                
                
                //����������������
                air_quality();
                
                 //������ƽ̨��Ϣ,��������ʱ�̻�ȡ�����ڻ�ȡ�ľ��ȸ߶���,��Ϊ��ǰ0.5��ȡһ�Σ����ڸо���90%��ȡ��
                //ÿ����ƽ̨����ʧ�ܾ����������ϴ���һ˲�䷢�͵ģ��ط�������
                    Get_onenet();  ///���ڿ�������������Ա�֤�·�����ر�ϵͳʱ��Ļ��գ������ж��Ǹ��Ͳ�֪����

                
                
                t++;
                timecount++;
                //û�ã�������,�����ʱ�������ã���ʱ0.5s��
                if (t == 4) {
                    t = 0;
                    // LED0_TOGGLE();  /* LED0��˸ */
                }
               // ESP8266_Clear();
                delay_ms(100);
                
            }
        //}
    }
}

//�����������䱨������
void air_quality()
{
    
    
    
    //������ֵ����
    if(PM >= pm_max)
    {
        if(PM<9999)
        {   pm_bj=1;
     if(enable_bj){ //��������λĬ�Ͽ�����ֱ�������������ر�
    BEEP(1);     
    LED0(0);}
    Chinese_Show_one(30,235,8,16,0,RED);
    Chinese_Show_one(45,235,9,16,0,RED);
    Chinese_Show_one(160,235,39,16,0,RED);
    Chinese_Show_one(175,235,40,16,0,RED);
    Chinese_Show_one(190,235,34,16,0,RED);
    Chinese_Show_one(205,235,34,16,0,RED);
    lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
    //Chinese_Show_one(150,230,34,16,0,RED);
    //Chinese_Show_one(160,230,34,16,0,RED);  
    }
    }
    else{
        pm_bj=0;
    lcd_show_string(30, 235, 200, 16, 16, "    ", RED); //�����ӦPM
    }
    
    //�¶ȳ���
     if(temperature >= temp_max)
    {
        tp_bj=1;
     if(enable_bj){ //��������λĬ�Ͽ�����ֱ�������������ر�
    BEEP(1);     
    LED0(0);}
    Chinese_Show_one(70,235,18,16,0,RED);
    Chinese_Show_one(85,235,19,16,0,RED);
    Chinese_Show_one(160,235,39,16,0,RED);
    Chinese_Show_one(175,235,40,16,0,RED);
    Chinese_Show_one(190,235,34,16,0,RED);
    Chinese_Show_one(205,235,34,16,0,RED);
    lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
    //Chinese_Show_one(150,230,34,16,0,RED);
    //Chinese_Show_one(160,230,34,16,0,RED);
    }
    else{
        tp_bj=0;
        lcd_show_string(70, 235, 200, 16, 16, "    ", RED); }//�����Ӧ��Ϣ
    //ʪ�ȳ���
     if(humidity >= humi_max)
    {
        hm_bj=1;
     if(enable_bj){ //��������λĬ�Ͽ�����ֱ�������������ر�
    BEEP(1);     
    LED0(0);}
        Chinese_Show_one(110,235,20,16,0,RED);
    Chinese_Show_one(125,235,21,16,0,RED);
    Chinese_Show_one(160,235,39,16,0,RED);
    Chinese_Show_one(175,235,40,16,0,RED);
    Chinese_Show_one(190,235,34,16,0,RED);
    Chinese_Show_one(205,235,34,16,0,RED);
    lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
    //Chinese_Show_one(150,230,34,16,0,RED);
    //Chinese_Show_one(160,230,34,16,0,RED);
    
    }
    else{
        hm_bj=0;
        lcd_show_string(110, 235, 200, 16, 16, "    ", RED);}
    //���ճ��걨��������Ӧ�����û�б�Ҫ
//    else if(light <= light_min)
//         {
//    BEEP(1);     
//    LED0(0);
//        Chinese_Show_one(70,235,20,16,0,RED);
//    Chinese_Show_one(85,235,21,16,0,RED);
//    Chinese_Show_one(100,235,39,16,0,RED);
//    Chinese_Show_one(115,235,40,16,0,RED);
//    Chinese_Show_one(130,235,34,16,0,RED);
//    Chinese_Show_one(145,235,34,16,0,RED);
//    lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
//    //Chinese_Show_one(150,230,34,16,0,RED);
//    //Chinese_Show_one(160,230,34,16,0,RED);
//    
//    }   
    if(PM<pm_max && temperature<temp_max &&humidity<humi_max){
       if(!flag_3 &!flag_bj)   //û��ͨ�����������
            BEEP(0);
       if(!flag_2 &!flag_bj) 
            LED0(1);
       if(!flag_3 &!flag_bj) 
            lcd_show_string(90, 260, 200, 16, 16, "             ", RED);
       //�ָ������������򿪣���ֹ���˾��������˿�
      enable_bj=1;  
       
     lcd_show_string(30, 235, 200, 16, 16, "                       ", RED);//���
   }
    //��������
   Chinese_Show_one(65,187,23,16,0,BLUE);
   Chinese_Show_one(80,187,24,16,0,BLUE);
   Chinese_Show_one(95,187,25,16,0,BLUE);
   Chinese_Show_one(110,187,26,16,0,BLUE);
   lcd_show_string(130, 187, 200, 16, 16, ": ", BLUE);
   lcd_show_string(160, 187, 200, 16, 16, "     ", BLUE);//���Ǻܲ�Ĳ���
    if(PM<75)
        Chinese_Show_one(145,187,27,16,0,BLUE);
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:A ", BLUE);
    else if(PM<150)
        Chinese_Show_one(145,187,28,16,0,BLUE);
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:B ", BLUE);
    else if(PM<300)
        Chinese_Show_one(145,187,29,16,0,BLUE);
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:C ", BLUE);
    else if(PM<1050)
        Chinese_Show_one(145,187,31,16,0,RED);
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:D ", BLUE);
    //else if(PM<3000)
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:F ", BLUE);
    else
    {   Chinese_Show_one(145,187,30,16,0,RED);
        Chinese_Show_one(165,187,31,16,0,RED);}
        //lcd_show_string(65, 180, 200, 16, 16, "air_quality:G ", BLUE);
}


void Get_onenet()

{
        //������ƽ̨��Ϣ
         dataPtr = ESP8266_Get(0); 
    
         if(dataPtr != NULL)   //�յ�����
             {
      //printf("���յ���Ϣ\r\n");
     // printf("dataPtr == %s\n\r",dataPtr);
      
      //���ú��������յ�����Ϣ
                ControlInfo info = extract_control_info(dataPtr);
    
  //��ȡ����������
      if (info.control_name != NULL) { 
          printf("������ : %s\r\n", info.control_name);
          printf("����:  %d\r\n", info.number);
          
          // ����Ӳ��
          control_hardware(&info);
          
          // �ͷ��ڴ�
          free(info.control_name);
      } else {
          printf("��ȡʧ��\n");
      } 
  //printf("������: %s\n", info.control_name);   ˵���ڴ��Ѿ��ͷ�
      
  //BEEP(1);//��ʾ�յ�����
  delay_ms(100);
      
  ESP8266_Clear();
         
         }
}

// �������ڽ���������������  
ControlInfo extract_control_info(const char *str) {
    ControlInfo info = { NULL, 0 };
    const char *control_name_start = strrchr(str, ','); // �ҵ����һ�����ŵ�λ��
    if (control_name_start != NULL) {
        control_name_start++; // �ƶ������ź�ĵ�һ���ַ�λ��
        
        // �����ڴ�
        info.control_name = (char *)malloc(strlen(control_name_start) * sizeof(char));
        
        if (info.control_name != NULL) {
            // ����������
            strcpy(info.control_name, control_name_start);
            
            // �ҵ����ֵ���ʼλ�ò�ת��Ϊ����
            char *number_start = strchr(info.control_name, ':');
            if (number_start != NULL) {
                *number_start = '\0'; // ��ð���滻Ϊ�ַ���������
                number_start++; // �ƶ������ֵ���ʼλ��
                info.number = atoi(number_start);
            }
        } else {
            perror("malloc");
        }
    }
    return info;
}

// ����Ӳ������
void control_hardware(const ControlInfo *info) {
    
       //����ϵͳ����,ֻ�ܿ��ƹأ���Ϊ���˾Ͳ����л�ȡ��������ˣ�����������
    if (strcmp(info->control_name, "SYSTEM") == 0  ||strcmp(info->control_name, "system") == 0 ||strcmp(info->control_name, "System") == 0) {
        if(info->number == 0)
        {
            flag = 0;
            delay_ms(10);
            
            display_system_status();
        }

        else if(info->number == 1)
            flag=1;  
    } 
    
       // ��ֵ��������λ����
    else if (strcmp(info->control_name, "warning") == 0 ) {
        if(info->number == 0)
        {
            enable_bj=0;
          LED0(1);//��
            BEEP(0);
        }

        else if(info->number == 1)
            enable_bj=1;   //���澯��������Զ���
    } 
    
    
            // ����LED0
    else if (strcmp(info->control_name, "LED0") == 0) {
        if(info->number == 0)
        {
            LED0(1);//��
            flag_2 =0;
        }
        else if(info->number == 1)
        {
            LED0(0);
        ////!!!! ���flag�ǹؼ� ������������
            flag_2 =1;
        }
    } 
    
            // ���Ʒ�����
    else if (strcmp(info->control_name, "BEEP") == 0) {
         if(info->number == 0)
        {
            BEEP(0);//��
            flag_3 =0;
        }
        else if(info->number == 1)
        {
            BEEP(1);
            flag_3 =1;
        }
        
    } 
    
            // ����LED1
    else if (strcmp(info->control_name, "LED1") == 0) {
        if(info->number == 0)
        {
            LED1(1);//��
        }
        else if(info->number == 1)
        {
            LED1(0);
        ////!!!! ���flag�ǹؼ� ������������
        }
    } 
        // ���� ֻ�ܿ�����ʾwarning �Լ�������led ���ܽӳ����걨��
    else if (strcmp(info->control_name, "bj") == 0) {
        if(info->number == 0)
        {
            enable_bj=0;
            LED0(1);//��
            BEEP(0);
            flag_bj=0;
        }
        else if(info->number == 1)
        {
            LED0(0);//��
            BEEP(1);
            enable_bj=1;
            lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
            flag_bj=1;
        }
    } 
    
    //����
    else if (strcmp(info->control_name, "lcdclear") == 0 || strcmp(info->control_name, "clear") == 0) {
        if(info->number == 0)
        {
            //display_system_status();
        }
        else if(info->number == 1)
        {
            display_system_status();
        }
    }
    
            // ������ֵ
    else if (strcmp(info->control_name, "PMmax") == 0||strcmp(info->control_name, "pmmax") == 0) {
        if (0 <= info->number &&info->number <= 5000)  //�޶���Χ
        {       pm_max=info->number;
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);    
        }
    } 
    else if (strcmp(info->control_name, "HUMImax") == 0||strcmp(info->control_name, "humimax") == 0) {
       if (0 <= info->number && info->number <= 100) 
        {   humi_max=info->number;
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);     
        }
    } 
    else if (strcmp(info->control_name, "TEMPmax") == 0||strcmp(info->control_name, "tempmax") == 0) {
       if (0 <= info->number&&info->number <= 100) 
        {   temp_max=info->number;
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);      
        }
    }     
    else {
        printf("δ֪��������%s\n", info->control_name);
    }
}
//�ر�״̬
void Off(void)
{
    if(offsc<=20)  //�ϴ�3��
    {
        if(offsc%10 == 0)   //��֤һ����Ƶ��
        {
           //ESP8266_SendData(temperature,humidity,light,PM,flag,pm_bj,tp_bj,hm_bj);//�������� 
            ESP8266_SendData3(flag,pm_bj,tp_bj,hm_bj);
        }
        offsc++;
    }
    
     BEEP(0);
     LED0(1);
     LED1(1);
    if(display)
    {display_system_status();//�ٽ���һ��������ʾ�����ɶž���Ϊ�ж϶�������������ȫ
        display=0;
        }

}

//��ʼ������
void init()
{
        //��ʼ��
    sys_stm32_clock_init(9);    /* ����ʱ��, 72Mhz */
    delay_init();
    NVIC_Configuration();
    uart_init(115200);
    beep_init();                /* ��ʼ�������� */
    GP2Y_Adc_Init();  //ADC��ʼ��     
    led_init();                 /* ��ʼ��LED */
    lcd_init();                 /* ��ʼ��LCD */
    beep_init();                /* ��ʼ�������� */
    key_init();                 /* ��ʼ������ */
    extix_init();               /* ��ʼ���ⲿ�ж����� */
    lsens_init();               /* ��ʼ������������ */
    uart3_init(115200);//����3��ʼ�� 
    
   lcd_show_string(85, 70, 200, 16, 24, "System", RED);
    lcd_show_string(20, 110, 200, 16, 24, "initialization...", RED);
	ESP8266_Init();					//��ʼ��ESP8266
    lcd_clear(WHITE);
}
