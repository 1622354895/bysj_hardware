//单片机头文件
#include "stm32f10x.h"

//网络设备驱动
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Huawei\",\"24196836\"\r\n"

#define ESP8266_ONENET_INFO		"AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,0\r\n" //数据流改为0 物模型1

#define ESP8266_USERCFG_INFO  "AT+MQTTUSERCFG=0,1,\"mqtt1\",\"i4g9oLr6AU\",\"version=2018-10-31&res=products%2Fi4g9oLr6AU%2Fdevices%2Fmqtt1&et=2344492222&method=md5&sign=%2BgKorQQGcIl4UDUn35kIQg%3D%3D\",0,0,\"\"\r\n"
//数据流与物模型的转换需要改9处（一行为一处）+下面那两条初始化

//这个是物模型属性设置那个，应该等同于数据流命令下发那个接收命令的同步？就是要写一个获取函数
//#define ESP8266_SCRIP_INFO "AT+MQTTSUB=0,\"$sys/6R0OoL20pr/matt1/thing/property/set\",0\r\n"

//物模型
//matt1
//6R0OoL20pr
//version=2018-10-31&res=products%2F6R0OoL20pr%2Fdevices%2Fmatt1&et=2344492222&method=md5&sign=FJUfyOlqgOHSQ5mk1j0OaA%3D%3D

//数据流那个
//mqtt1   
//i4g9oLr6AU  
//version=2018-10-31&res=products%2Fi4g9oLr6AU%2Fdevices%2Fmqtt1&et=2344492222&method=md5&sign=%2BgKorQQGcIl4UDUn35kIQg%3D%3D

////数据流的订阅，物模型不用这个 亦可$sys/584938/onenet_mqtt_dev1/dp/post/json/+
#define ESP8266_SCRIP_INFO "AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n" 
//这个弄了才能接收命令，订阅命令
#define SCRIP "AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/cmd/#\",0\r\n"

//const char* pubtopic="$sys/6R0OoL20pr/matt1/thing/property/post"; //物理模型
const char* pubtopic="$sys/i4g9oLr6AU/mqtt1/dp/post/json";   //数据流 
const char* hf="$sys/i4g9oLr6AU/mqtt1/cmd/response/";
unsigned char esp8266_buf[256];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;


extern u8 humidityH;	  //湿度整数部分

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;
    
    //每次发送都要订阅？
    //Usart_SendString(USART3,"AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n",strlen("AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n"));
	Usart_SendString(USART3, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
            //printf("%s",esp8266_buf);
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	temp：温度值
//				      humi：湿度值
//				      adcx：光照度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(double temp,double humi,int adcx,int PM)
{
	char cmdBuf[512];
	
	ESP8266_Clear();								//清空接收缓存
	
	//先发送要发送数据的指令做准备
	
    //物模型方式
    //数据流改为SUB  原本PUB  params
	//sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"temp\\\":{\\\"value\\\":%lf\\}\\,\\\"humi\\\":{\\\"value\\\":%lf\\}\\,\\\"adcx\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",pubtopic,temp,humi,adcx);		//发送命令
    
    //数据流方式
    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"temp\\\":[{\\\"v\\\":%lf}\\]\\,\\\"PM\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi\\\":[{\\\"v\\\":%lf}]\\,\\\"adcx\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,temp,PM,humi,adcx);		//发送命令
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);
}


void ESP8266_SendData3(int flag_system,int pm_bj,int tp_bj,int hm_bj){
    	char cmdBuf[512];
	
	ESP8266_Clear();								//清空接收缓存
	

    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"system\\\":[{\\\"v\\\":%d}\\]\\,\\\"pm_warning\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi_warning\\\":[{\\\"v\\\":%d}]\\,\\\"temp_warning\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,flag_system ,pm_bj,hm_bj,tp_bj);		//发送命令
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);

}

void ESP8266_SendData4(int pmmax,int humimax,int tempmax){
    	char cmdBuf[512];
	
	ESP8266_Clear();								//清空接收缓存
	

    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"pmmax\\\":[{\\\"v\\\":%d}\\]\\,\\\"humimax\\\":[{\\\"v\\\":%d}\\]\\,\\\"tempmax\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic ,pmmax,humimax,tempmax);		//发送命令
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);

}



//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Init(void)
{
	
	
	ESP8266_Clear();
	
	printf("0. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		delay_ms(500);
	
	printf("1. AT+RST\r\n");
	while(ESP8266_SendCmd("AT+RST\r\n", ""))
		delay_ms(500);
	
	
	printf("2. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		delay_ms(500);
	
	printf( "3. AT+CWDHCP\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		delay_ms(500);
	
	printf("4. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		delay_ms(500);
	
	printf( "5. MQTTUSERCFG\r\n");
	while(ESP8266_SendCmd(ESP8266_USERCFG_INFO, "OK"))
		delay_ms(500);

	
	printf( "6. MQTTCONN\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "OK"))
		delay_ms(500);
    
    //数据流订阅,才能上报   //改为物模型时吧上面两个注释互换
    printf( "7. MQTTscrip\r\n");
	while(ESP8266_SendCmd(ESP8266_SCRIP_INFO, "OK"))
		delay_ms(500);
    
    //这个数据流的订阅 订阅后才能获取命令（对比b站两个视频和mqtt旧代码来搞）
    //物模型删掉这个
    printf( "8. scrip\r\n");
    
	while(ESP8266_SendCmd(SCRIP, "OK"))
		delay_ms(500);
	
	printf(" ESP8266 Init OK\r\n");

}

//==========================================================
//	函数名称：	USART3_IRQHandler
//
//	函数功能：	串口3收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void USART3_IRQHandler(void)
{

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = USART3->DR;
		 //printf("%s",esp8266_buf);
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
    //在这里写接收是不是百分百收到？

}
//==========================================================
//	函数名称：	ESP8266_Get
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//==========================================================
unsigned char *ESP8266_Get(unsigned short timeOut)
{
    //printf("这里");
	char *ptrIPD = NULL;
    char cmdBuf[512];
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
            if(strstr((const char *)esp8266_buf, "request") != NULL)
            {       
                 char input[256] ;
                char uuid[50]; 
                char a[256];
                 strcpy(input, esp8266_buf);

                extractUUID(input, uuid); 
                    strcpy(a, hf);
                
               // printf("Modified: %s\r\n", uuid);  
                strcat(a, uuid);
               // printf("a=%s\r\n",a);

                sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"OK\",0,0\r\n", a); // 发送命令
               // printf("cmdbuf: %s\r\n", cmdBuf);  
                
 //sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"temp\\\":[{\\\"v\\\":%lf}\\]\\,\\\"PM\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi\\\":[{\\\"v\\\":%lf}]\\,\\\"adcx\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,temp,PM,humi,adcx);		//发送命令

                
//                $sys/584938/onenet_mqtt_dev1/cmd/response/c5a7e27e-05bb-41eb-972b-abce424ade8e
                while(ESP8266_SendCmd(cmdBuf, "OK"))
                    delay_ms(500);                

                
                return input;   //为什么失败！发过去就被清理了 那个send命令

            }                
		}
		
		delay_ms(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}
void ESP8266_SendData_2(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	
	//先发送要发送数据的指令做准备
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		//既然准备完毕即可开始发送数据
		Usart_SendString(USART3, data, len);		//发送设备连接请求数据
	}

}
void extractUUID(const char *input, char *output) {
    const char *start = strstr(input, "/cmd/request/") + strlen("/cmd/request/");
    const char *end = strchr(start, '"');
    if (start != NULL && end != NULL) {
        int length = end - start;
        strncpy(output, start, length);
        output[length] = '\0'; // Null-terminate the string
    } else {
        // Handle case when the pattern is not found
        strcpy(output, "UUID not found");
    }
} 