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
u8 QoS;
u32 messageid;


int main(int argc, char *argv[]) {
	MQTT_Init();	
	
	processpublishack();



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
	
	printf("%d\r\n",MQTT_UNSUBACK(outbuff, res));			//返回的是正确（1）与否（-1） 
} 

void ping(void)
{
	MQTT_PINGREQ(); 
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void pingresp_test(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	
	printf("%d\r\n",MQTT_PINGRESP(outbuff, res));			//返回的是正确（1）与否（-1） 
} 

void publish0_test(void)
{
	MQTT_PUBLISH0(1, "USER001","123", 3);
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void publish_test(void)
{
	MQTT_PUBLISH(1, 2, 1, "USER001","123", 3);
	for(i = 0; i < mqtt.length; i++)
	{
		printf("%02x ",mqtt.buff[i]);
	}
	printf("\r\n");
}

void processpublish_test(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBLISH(outbuff, res, &QoS, &messageid));
	printf("QoS = %d\r\n",QoS);
	printf("messageid = %x\r\n",messageid);
	printf("topic_len = %d\r\n",mqtt.topic[0]*256 + mqtt.topic[1]);
	printf("topic = %s\r\n",&mqtt.topic[2]);
	
	printf("data_len = %d\r\n",mqtt.data[0]*256 + mqtt.data[1]);	
	printf("data = %s\r\n",&mqtt.data[2]);
}

void publishack_test(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBLISH(outbuff, res, &QoS, &messageid));
	if(QoS = 1)
	{
		MQTT_PUBACK(messageid);
		for(i = 0; i < mqtt.length; i++)
		{
			printf("%02x ",mqtt.buff[i]);
		}
	}
}

void processpublishack(void)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPublish(outbuff, res, &messageid));
	printf("%x\r\n", messageid);
}




