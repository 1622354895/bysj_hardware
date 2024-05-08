#ifndef _ESP8266_H_
#define _ESP8266_H_





#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志


void ESP8266_Init(void);

void ESP8266_Clear(void);

void ESP8266_SendData(double temp,double humi,int adcx,int PM);
void ESP8266_SendData3(int flag_system,int pm_bj,int tp_bj,int hm_bj);
void ESP8266_SendData4(int pmmax,int humimax,int tempmax);
void ESP8266_SendData_2(unsigned char *data, unsigned short len);

unsigned char *ESP8266_Get(unsigned short timeOut);
void extractUUID(const char *input, char *output)  ;

#endif
