#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "MQTT.h"
#include "cc.h"
#include <string.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int i;
int res;
u8 databuff[64];
u8 outbuff[64];


int main(int argc, char *argv[]) {
	MQTT_Init();	
	
	unsuback_test();



	printf("\r\n");
	return 0;
}



int Str_to_Hex(s8* indata, u8* outdata)
{
	int num = 0;
	s8 *str;
	s8 *endstr;
	
	str = indata;
	while(*str != '\0')
	{
		outdata[num] = strtol(str, (char**)&endstr, 16);
		num++;
		str = endstr;
	}
	
	return num;
}

void data_test(void)
{
	printf("%s\r\n",mqtt.ClientID);
	printf("%s\r\n",mqtt.UserName);
	printf("%s\r\n",mqtt.Passward);
}

void connect_test(void)
{
	MQTT_CONNECT(100);
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void  connectwill_test(void)
{
	MQTT_CONNECTWILL( 1, 2, 1, 100);
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void connectack_test(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_CONNACK(outbuff, res));			//返回的是服务等级 
}

void disconnect_test(void)
{
	MQTT_DISCONNECT(); 
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void subscribe_test(void)
{
	MQTT_SUBSCRIBE("USER002", 2);
	
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
}

void suback_test()
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff); 
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_SUBACK(outbuff, res));			//返回的是服务等级
} 

void unsubscribe_test(void)
{
	MQTT_UNSUBSCRIBE("USER002");
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
}

void unsuback_test(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	
	printf("%d\r\n",MQTT_UNSUBACK(outbuff, res));			//返回的是正确（0）于否（-1） 
} 







