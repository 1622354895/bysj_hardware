//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸����
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Huawei\",\"24196836\"\r\n"

#define ESP8266_ONENET_INFO		"AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,0\r\n" //��������Ϊ0 ��ģ��1

#define ESP8266_USERCFG_INFO  "AT+MQTTUSERCFG=0,1,\"mqtt1\",\"i4g9oLr6AU\",\"version=2018-10-31&res=products%2Fi4g9oLr6AU%2Fdevices%2Fmqtt1&et=2344492222&method=md5&sign=%2BgKorQQGcIl4UDUn35kIQg%3D%3D\",0,0,\"\"\r\n"
//����������ģ�͵�ת����Ҫ��9����һ��Ϊһ����+������������ʼ��

//�������ģ�����������Ǹ���Ӧ�õ�ͬ�������������·��Ǹ����������ͬ��������Ҫдһ����ȡ����
//#define ESP8266_SCRIP_INFO "AT+MQTTSUB=0,\"$sys/6R0OoL20pr/matt1/thing/property/set\",0\r\n"

//��ģ��
//matt1
//6R0OoL20pr
//version=2018-10-31&res=products%2F6R0OoL20pr%2Fdevices%2Fmatt1&et=2344492222&method=md5&sign=FJUfyOlqgOHSQ5mk1j0OaA%3D%3D

//�������Ǹ�
//mqtt1   
//i4g9oLr6AU  
//version=2018-10-31&res=products%2Fi4g9oLr6AU%2Fdevices%2Fmqtt1&et=2344492222&method=md5&sign=%2BgKorQQGcIl4UDUn35kIQg%3D%3D

////�������Ķ��ģ���ģ�Ͳ������ ���$sys/584938/onenet_mqtt_dev1/dp/post/json/+
#define ESP8266_SCRIP_INFO "AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n" 
//���Ū�˲��ܽ��������������
#define SCRIP "AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/cmd/#\",0\r\n"

//const char* pubtopic="$sys/6R0OoL20pr/matt1/thing/property/post"; //����ģ��
const char* pubtopic="$sys/i4g9oLr6AU/mqtt1/dp/post/json";   //������ 
const char* hf="$sys/i4g9oLr6AU/mqtt1/cmd/response/";
unsigned char esp8266_buf[256];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;


extern u8 humidityH;	  //ʪ����������

//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;
    
    //ÿ�η��Ͷ�Ҫ���ģ�
    //Usart_SendString(USART3,"AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n",strlen("AT+MQTTSUB=0,\"$sys/i4g9oLr6AU/mqtt1/dp/post/json/accepted\",0\r\n"));
	Usart_SendString(USART3, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
		{
            //printf("%s",esp8266_buf);
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				ESP8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	temp���¶�ֵ
//				      humi��ʪ��ֵ
//				      adcx�����ն�
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_SendData(double temp,double humi,int adcx,int PM)
{
	char cmdBuf[512];
	
	ESP8266_Clear();								//��ս��ջ���
	
	//�ȷ���Ҫ�������ݵ�ָ����׼��
	
    //��ģ�ͷ�ʽ
    //��������ΪSUB  ԭ��PUB  params
	//sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"temp\\\":{\\\"value\\\":%lf\\}\\,\\\"humi\\\":{\\\"value\\\":%lf\\}\\,\\\"adcx\\\":{\\\"value\\\":%d\\}}}\",0,0\r\n",pubtopic,temp,humi,adcx);		//��������
    
    //��������ʽ
    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"temp\\\":[{\\\"v\\\":%lf}\\]\\,\\\"PM\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi\\\":[{\\\"v\\\":%lf}]\\,\\\"adcx\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,temp,PM,humi,adcx);		//��������
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);
}


void ESP8266_SendData3(int flag_system,int pm_bj,int tp_bj,int hm_bj){
    	char cmdBuf[512];
	
	ESP8266_Clear();								//��ս��ջ���
	

    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"system\\\":[{\\\"v\\\":%d}\\]\\,\\\"pm_warning\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi_warning\\\":[{\\\"v\\\":%d}]\\,\\\"temp_warning\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,flag_system ,pm_bj,hm_bj,tp_bj);		//��������
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);

}

void ESP8266_SendData4(int pmmax,int humimax,int tempmax){
    	char cmdBuf[512];
	
	ESP8266_Clear();								//��ս��ջ���
	

    sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"pmmax\\\":[{\\\"v\\\":%d}\\]\\,\\\"humimax\\\":[{\\\"v\\\":%d}\\]\\,\\\"tempmax\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic ,pmmax,humimax,tempmax);		//��������
	
    while(ESP8266_SendCmd(cmdBuf, "OK"))
		delay_ms(500);
	memset(cmdBuf,0,sizeof(cmdBuf));
	delay_ms(100);

}



//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
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
    
    //����������,�����ϱ�   //��Ϊ��ģ��ʱ����������ע�ͻ���
    printf( "7. MQTTscrip\r\n");
	while(ESP8266_SendCmd(ESP8266_SCRIP_INFO, "OK"))
		delay_ms(500);
    
    //����������Ķ��� ���ĺ���ܻ�ȡ����Ա�bվ������Ƶ��mqtt�ɴ������㣩
    //��ģ��ɾ�����
    printf( "8. scrip\r\n");
    
	while(ESP8266_SendCmd(SCRIP, "OK"))
		delay_ms(500);
	
	printf(" ESP8266 Init OK\r\n");

}

//==========================================================
//	�������ƣ�	USART3_IRQHandler
//
//	�������ܣ�	����3�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void USART3_IRQHandler(void)
{

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�����ж�
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
		esp8266_buf[esp8266_cnt++] = USART3->DR;
		 //printf("%s",esp8266_buf);
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
    //������д�����ǲ��ǰٷְ��յ���

}
//==========================================================
//	�������ƣ�	ESP8266_Get
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//==========================================================
unsigned char *ESP8266_Get(unsigned short timeOut)
{
    //printf("����");
	char *ptrIPD = NULL;
    char cmdBuf[512];
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
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

                sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"OK\",0,0\r\n", a); // ��������
               // printf("cmdbuf: %s\r\n", cmdBuf);  
                
 //sprintf(cmdBuf, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":123\\\,\\\"dp\\\":{\\\"temp\\\":[{\\\"v\\\":%lf}\\]\\,\\\"PM\\\":[{\\\"v\\\":%d}\\]\\,\\\"humi\\\":[{\\\"v\\\":%lf}]\\,\\\"adcx\\\":[{\\\"v\\\":%d}]}}\",0,0\r\n",pubtopic,temp,PM,humi,adcx);		//��������

                
//                $sys/584938/onenet_mqtt_dev1/cmd/response/c5a7e27e-05bb-41eb-972b-abce424ade8e
                while(ESP8266_SendCmd(cmdBuf, "OK"))
                    delay_ms(500);                

                
                return input;   //Ϊʲôʧ�ܣ�����ȥ�ͱ������� �Ǹ�send����

            }                
		}
		
		delay_ms(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}
void ESP8266_SendData_2(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//��ս��ջ���
	
	//�ȷ���Ҫ�������ݵ�ָ����׼��
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		//��Ȼ׼����ϼ��ɿ�ʼ��������
		Usart_SendString(USART3, data, len);		//�����豸������������
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