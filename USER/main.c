/*环境监测系统
戴伟
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

u8 humidityH;	  //湿度整数部分
u8 humidityL;	  //湿度小数部分
u8 temperatureH;   //温度整数部分
u8 temperatureL;   //温度小数部分
DHT11_Data_TypeDef DHT11_Data;

//控制命令的beep LED0标记、报警标记
int flag_2=0;
int flag_3=0;
int flag_bj=0;
int enable_bj=1;  //阈值报警允许位


//控制名和值
typedef struct {
    char *control_name;
    int number;
} ControlInfo;


//环境检测子线程 -- 温度湿度、粉尘、光线
void environment_monitoring_thread();

 //系统状态打开or 关闭显示
void display_system_status();

//空气质量及其报警函数
void air_quality();

//命令处理函数
ControlInfo extract_control_info(const char *str);

// 控制硬件的函数
void control_hardware(const ControlInfo *info);

//获取云平台命令
void Get_onenet(void);

//系统状态关闭
void Off(void);

//初始化
void init();

int main(void)
{
    //初始化
    init();
    
    //系统显示
    display_system_status();
    
        
    while(1)
  {
      //系统关闭后通过云平台命令开启
      Get_onenet(); 

    if(flag) 
    {
       
      environment_monitoring_thread();    //开始检测

    }
    

    else{
        Off();
        }
    }

}



 //系统状态打开or 关闭显示
void display_system_status() {
    delay_ms(5);
    lcd_clear(WHITE);
    //环境温湿度及粉尘监测系统
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
    
    //戴伟
    Chinese_Show_one(100,50,0,16,0,GREEN);
    Chinese_Show_one(128,50,1,16,0,GREEN);
    
    //lcd_show_string(45, 30, 200, 16, 16, "Observation System", GREEN);
    //lcd_show_string(90, 50, 200, 16, 16, "Dai Wei", GREEN);
    //系统打开
    if (flag) { 
        //lcd_clear(WHITE);
        lcd_show_string(40, 70, 200, 16, 16, "Condition : Online", GREEN);

    }
    //关闭
    else {
        lcd_show_string(40, 70, 250, 16, 16, "Condition : Offline  ", GREEN);
        
    }
}

//环境检测子线程
//标志位为0则退出
void environment_monitoring_thread() {
    display_system_status();
        offsc=0;//恢复标志位
        display=1;
    while (flag) {
        DHT11_Init();
        // 循环检查标志位，为1执行
//        if (DHT11_Init()) {
//            lcd_show_string(30, 110, 200, 16, 16, "DHT11 Error", RED);
//            delay_ms(200);
//            lcd_fill(30, 110, 239, 130 + 16, WHITE);
//            delay_ms(200);
//        } 
       // else {    //dht11获取成功
            //lcd_show_string(30, 110, 200, 16, 16, "DHT11 OK", RED);
            lcd_show_string(30, 130, 200, 16, 16, "Temp:  C", BLUE);
            lcd_show_string(30, 150, 200, 16, 16, "Humi:  %", BLUE);
            lcd_show_string(30, 167, 200, 16, 16, "Light:", BLUE);
            lcd_show_string(30+110, 130, 200, 16, 16, "  PM:", BLUE);
            lcd_show_string(30+130, 150, 200, 16, 16, "ug/m3", BLUE);
            
            //显示数据监测
            Chinese_Show_one(30,110,35,16,0,BLUE);
            Chinese_Show_one(45,110,36,16,0,BLUE);
            Chinese_Show_one(60,110,37,16,0,BLUE);
            Chinese_Show_one(75,110,38,16,0,BLUE);
            lcd_show_string(90, 110, 200, 16, 16, ":", BLUE);
            
             //浓度阈值调节  放在while外面的时候命令更改值时不会显示，因为就不运行这里了
        Chinese_Show_one(10,210,32,16,0,BLUE);
        Chinese_Show_one(25,210,33,16,0,BLUE);
        lcd_show_string(40, 210, 200, 16, 16, ":PM     Temp    Humi", BLUE);
        lcd_show_num(65, 210, pm_max, 4, 16, BLUE);     
        lcd_show_num(140, 210, temp_max, 3, 16, BLUE); 
        lcd_show_num(205, 210, humi_max, 3, 16, BLUE);   
            
            
            int t = 0;
            int pmman=0;
            // 循环检查标志位,并且获值,每0.5秒获取一次（任务书要求）
            while (flag) { 
                if (t % 4 == 0) {
                    
                    Read_DHT11(&DHT11_Data);
                    humidityH=DHT11_Data.humi_int;	  //湿度整数部分
                    humidityL=DHT11_Data.humi_deci;	  //湿度小数部分
                    humidity = humidityH+humidityL/10.0;
                    
                    temperatureH=DHT11_Data.temp_int;   //温度整数部分
                    temperatureL=DHT11_Data.temp_deci;   //温度小数部分
                    temperature = temperatureH+temperatureL/10.0;
                    
                    light = lsens_get_val();         //光照
                    
                    pmman=GetGP2YSingleValue();
                    if(pmman<9999)  //需要改进
                        PM = pmman;       //得到pm2.5值 
                    
                    
                    if(timecount>18)//3.6s上传一次,（下面8266get（0））
                        {   
                            timecount = 0;
                    //发送数据到云平台
                    printf("humi =%f ,temp =%f, PM =%d, light value =%d\r\n",humidity,temperature,PM,light);	
                    ESP8266_SendData(temperature,humidity,light,PM);//发送数据
                    ESP8266_SendData3(flag,pm_bj,tp_bj,hm_bj);
                    ESP8266_SendData4(pm_max,humi_max,temp_max);
                    //ESP8266_Clear(); 发送命令已经清空了
                        }
                    
                            
                            //显示获取的环境值
                    lcd_show_num(30 + 40, 130, temperature, 2, 16, BLUE);
                    lcd_show_num(30 + 40, 150, humidity, 2, 16, BLUE);
                    lcd_show_num(30 + 80, 150, PM, 5, 16, BLUE);
                    lcd_show_xnum(30 + 45, 167, light, 3, 16, 0, BLUE); /* 显示光照的值 */
                }
                
                
                //空气质量及报警；
                air_quality();
                
                 //接收云平台消息,放在外面时刻获取，现在获取的精度高多了,因为以前0.5获取一次，现在感觉有90%获取率
                //每次云平台发送失败就是在数据上传的一瞬间发送的，重发就行了
                    Get_onenet();  ///放在空气质量后面可以保证下发命令关闭系统时屏幕清空，但是中断那个就不知道了

                
                
                t++;
                timecount++;
                //没用，先留着,起个延时检测的作用（延时0.5s）
                if (t == 4) {
                    t = 0;
                    // LED0_TOGGLE();  /* LED0闪烁 */
                }
               // ESP8266_Clear();
                delay_ms(100);
                
            }
        //}
    }
}

//空气质量及其报警函数
void air_quality()
{
    
    
    
    //超过阈值报警
    if(PM >= pm_max)
    {
        if(PM<9999)
        {   pm_bj=1;
     if(enable_bj){ //报警允许位默认开启，直到发命令让它关闭
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
    lcd_show_string(30, 235, 200, 16, 16, "    ", RED); //清除对应PM
    }
    
    //温度超标
     if(temperature >= temp_max)
    {
        tp_bj=1;
     if(enable_bj){ //报警允许位默认开启，直到发命令让它关闭
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
        lcd_show_string(70, 235, 200, 16, 16, "    ", RED); }//清除对应信息
    //湿度超标
     if(humidity >= humi_max)
    {
        hm_bj=1;
     if(enable_bj){ //报警允许位默认开启，直到发命令让它关闭
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
    //光照超标报警，但是应该这个没有必要
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
       if(!flag_3 &!flag_bj)   //没有通过控制命令打开
            BEEP(0);
       if(!flag_2 &!flag_bj) 
            LED0(1);
       if(!flag_3 &!flag_bj) 
            lcd_show_string(90, 260, 200, 16, 16, "             ", RED);
       //恢复正常后吧允许打开，防止关了警报后忘了开
      enable_bj=1;  
       
     lcd_show_string(30, 235, 200, 16, 16, "                       ", RED);//清空
   }
    //空气质量
   Chinese_Show_one(65,187,23,16,0,BLUE);
   Chinese_Show_one(80,187,24,16,0,BLUE);
   Chinese_Show_one(95,187,25,16,0,BLUE);
   Chinese_Show_one(110,187,26,16,0,BLUE);
   lcd_show_string(130, 187, 200, 16, 16, ": ", BLUE);
   lcd_show_string(160, 187, 200, 16, 16, "     ", BLUE);//覆盖很差的差字
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
        //接收云平台消息
         dataPtr = ESP8266_Get(0); 
    
         if(dataPtr != NULL)   //收到命令
             {
      //printf("已收到消息\r\n");
     // printf("dataPtr == %s\n\r",dataPtr);
      
      //调用函数处理收到的消息
                ControlInfo info = extract_control_info(dataPtr);
    
  //获取到控制命令
      if (info.control_name != NULL) { 
          printf("控制名 : %s\r\n", info.control_name);
          printf("数字:  %d\r\n", info.number);
          
          // 控制硬件
          control_hardware(&info);
          
          // 释放内存
          free(info.control_name);
      } else {
          printf("提取失败\n");
      } 
  //printf("控制名: %s\n", info.control_name);   说明内存已经释放
      
  //BEEP(1);//表示收到命令
  delay_ms(100);
      
  ESP8266_Clear();
         
         }
}

// 函数用于解析控制名和数字  
ControlInfo extract_control_info(const char *str) {
    ControlInfo info = { NULL, 0 };
    const char *control_name_start = strrchr(str, ','); // 找到最后一个逗号的位置
    if (control_name_start != NULL) {
        control_name_start++; // 移动到逗号后的第一个字符位置
        
        // 分配内存
        info.control_name = (char *)malloc(strlen(control_name_start) * sizeof(char));
        
        if (info.control_name != NULL) {
            // 拷贝控制名
            strcpy(info.control_name, control_name_start);
            
            // 找到数字的起始位置并转换为整数
            char *number_start = strchr(info.control_name, ':');
            if (number_start != NULL) {
                *number_start = '\0'; // 将冒号替换为字符串结束符
                number_start++; // 移动到数字的起始位置
                info.number = atoi(number_start);
            }
        } else {
            perror("malloc");
        }
    }
    return info;
}

// 控制硬件函数
void control_hardware(const ControlInfo *info) {
    
       //控制系统开关,只能控制关，因为关了就不进行获取命令操作了，跳出函数了
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
    
       // 阈值报警允许位控制
    else if (strcmp(info->control_name, "warning") == 0 ) {
        if(info->number == 0)
        {
            enable_bj=0;
          LED0(1);//关
            BEEP(0);
        }

        else if(info->number == 1)
            enable_bj=1;   //上面警报解除会自动打开
    } 
    
    
            // 控制LED0
    else if (strcmp(info->control_name, "LED0") == 0) {
        if(info->number == 0)
        {
            LED0(1);//关
            flag_2 =0;
        }
        else if(info->number == 1)
        {
            LED0(0);
        ////!!!! 这个flag是关键 可以列入问题
            flag_2 =1;
        }
    } 
    
            // 控制蜂鸣器
    else if (strcmp(info->control_name, "BEEP") == 0) {
         if(info->number == 0)
        {
            BEEP(0);//关
            flag_3 =0;
        }
        else if(info->number == 1)
        {
            BEEP(1);
            flag_3 =1;
        }
        
    } 
    
            // 控制LED1
    else if (strcmp(info->control_name, "LED1") == 0) {
        if(info->number == 0)
        {
            LED1(1);//关
        }
        else if(info->number == 1)
        {
            LED1(0);
        ////!!!! 这个flag是关键 可以列入问题
        }
    } 
        // 报警 只能控制显示warning 以及蜂鸣器led 不能接除超标报警
    else if (strcmp(info->control_name, "bj") == 0) {
        if(info->number == 0)
        {
            enable_bj=0;
            LED0(1);//关
            BEEP(0);
            flag_bj=0;
        }
        else if(info->number == 1)
        {
            LED0(0);//开
            BEEP(1);
            enable_bj=1;
            lcd_show_string(90, 260, 200, 16, 16, "Warning", RED);
            flag_bj=1;
        }
    } 
    
    //清屏
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
    
            // 调节阈值
    else if (strcmp(info->control_name, "PMmax") == 0||strcmp(info->control_name, "pmmax") == 0) {
        if (0 <= info->number &&info->number <= 5000)  //限定范围
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
        printf("未知控制名：%s\n", info->control_name);
    }
}
//关闭状态
void Off(void)
{
    if(offsc<=20)  //上传3次
    {
        if(offsc%10 == 0)   //保证一定的频率
        {
           //ESP8266_SendData(temperature,humidity,light,PM,flag,pm_bj,tp_bj,hm_bj);//发送数据 
            ESP8266_SendData3(flag,pm_bj,tp_bj,hm_bj);
        }
        offsc++;
    }
    
     BEEP(0);
     LED0(1);
     LED1(1);
    if(display)
    {display_system_status();//再进行一次清屏显示处理，可杜绝因为中断而导致清屏不完全
        display=0;
        }

}

//初始化函数
void init()
{
        //初始化
    sys_stm32_clock_init(9);    /* 设置时钟, 72Mhz */
    delay_init();
    NVIC_Configuration();
    uart_init(115200);
    beep_init();                /* 初始化蜂鸣器 */
    GP2Y_Adc_Init();  //ADC初始化     
    led_init();                 /* 初始化LED */
    lcd_init();                 /* 初始化LCD */
    beep_init();                /* 初始化蜂鸣器 */
    key_init();                 /* 初始化按键 */
    extix_init();               /* 初始化外部中断输入 */
    lsens_init();               /* 初始化光敏传感器 */
    uart3_init(115200);//串口3初始化 
    
   lcd_show_string(85, 70, 200, 16, 24, "System", RED);
    lcd_show_string(20, 110, 200, 16, 24, "initialization...", RED);
	ESP8266_Init();					//初始化ESP8266
    lcd_clear(WHITE);
}
