#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "MQTT.h"
#include "cc.h"
#include <string.h>
#include "test.h"




void data_test(MQTT_TCB *m)
{
	printf("%s\r\n",m->param.ClientID);
	printf("%s\r\n",m->param.UserName);
	printf("%s\r\n",m->param.Passward);
}

void connect_test(MQTT_TCB *m)
{
	int ret = MQTT_CONNECT(m, 100);
	if(ret < 0) {
		printf("Failed to pack MQTT CONNECT message\r\n");
		return;
	}else {
		for(i = 0; i < m->length.totallength; i++)
		{
			printf("%02x ",m->buff[i]);
		}
	}
	
	printf("\r\n");
}


void connectack_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_CONNACK(m, outbuff, res));			//صǷȼ 
}

void disconnect_test(MQTT_TCB *m)
{
	MQTT_DISCONNECT(m); 
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
	printf("\r\n");
}

void subscribe_test(MQTT_TCB *m)
{
	res = MQTT_SUBSCRIBE(m, "TEST", 0);
	if(res < 0) {
		printf("Failed to pack MQTT SUBSCRIBE message\r\n");
		return;
	}
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
}

void suback_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff); 
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_SUBACK(m, outbuff, res));			//صǷȼ
} 

void unsubscribe_test(MQTT_TCB *m)
{
	res = MQTT_UNSUBSCRIBE(m, "TEST");
	if(res < 0) {
		printf("Failed to pack MQTT UNSUBSCRIBE message\r\n");
		return;
	}
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
}

void unsuback_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	
	printf("%d\r\n",MQTT_UNSUBACK(m, outbuff, res));			//صȷ1-1 
} 

void ping(MQTT_TCB *m)
{
	MQTT_PINGREQ(m); 
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
	printf("\r\n");
}

void pingresp_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	
	printf("%d\r\n",MQTT_PINGRESP(m, outbuff, res));			//صȷ1-1 
} 

void publish0_test(MQTT_TCB *m)
{
	MQTT_PUBLISH0(m, 1, "USER001","123", 3);
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
	printf("\r\n");
}

void publish_test(MQTT_TCB *m)
{
	MQTT_PUBLISH(m, 1, 2, 1, "USER001","123", 3);
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
	printf("\r\n");
}

void processpublish_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBLISH(m, outbuff, res, &QoS, &messageid));
	printf("QoS = %d\r\n",QoS);
	printf("messageid = %x\r\n",messageid);
	printf("topic_len = %d\r\n",m->topic[0]*256 + m->topic[1]);
	printf("topic = %s\r\n",&m->topic[2]);
	
	printf("data_len = %d\r\n",m->data[0]*256 + m->data[1]);	
	printf("data = %s\r\n",&m->data[2]);
}

void publishack_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBLISH(m, outbuff, res, &QoS, &messageid));
	if(QoS = 1)
	{
		MQTT_PUBACK(m,messageid);
		for(i = 0; i < m->length.totallength; i++)
		{
			printf("%02x ",m->buff[i]);
		}
	}
}

void processpublishack(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPublish(m, outbuff, res, &messageid));
	printf("%x\r\n", messageid);
}


void pubrec_test(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBLISH(m, outbuff, res, &QoS, &messageid));
	if(QoS == 2)
	{
		MQTT_PUBREC(m, messageid);
		for(i = 0; i < m->length.totallength; i++)
		{
			printf("%02x ",m->buff[i]);
		}
	}
}

void processpubrec(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBREC(m, outbuff, res, &messageid));
	printf("%x\r\n", messageid);
}

void pubrel_test(MQTT_TCB *m)
{
	processpubrec(m);
	MQTT_PUBREL(m, messageid);
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
}

void processpubrel(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBREL(m, outbuff, res, &messageid));
	printf("%x\r\n", messageid);
}

void pubcomp_test(MQTT_TCB *m)
{
	processpubrel(m);
	MQTT_PUBCOMP(m, messageid);
	for(i = 0; i < m->length.totallength; i++)
	{
		printf("%02x ",m->buff[i]);
	}
}

void processpubcomp(MQTT_TCB *m)
{
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	for(i = 0; i < res; i++)
	{
		printf("%02x ",outbuff[i]);
	}
	printf("\r\n");
	printf("%d\r\n",MQTT_ProcessPUBCOMP(m, outbuff, res, &messageid));
	printf("%x\r\n", messageid);
}



