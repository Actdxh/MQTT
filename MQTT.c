#include "MQTT.h"
#include "string.h"
#include "stdio.h"

MQTT_TCB mqtt;

void MQTT_Init(void)
{
	memset(&mqtt, 0, sizeof(mqtt));
	sprintf(mqtt.ClientID,  "USER001");
	sprintf(mqtt.UserName,  "USER001");
	sprintf(mqtt.Passward,  "USER001");
	sprintf(mqtt.WillTopic, "WILL001");
	sprintf(mqtt.WillData,  "WILL001");
	mqtt.MessageID = 1;
}

/************************无遗嘱版本connect函数*************************/ 
void MQTT_CONNECT(u32 keepalive)														//keepalive是保持时间 
{
	/************************固定报头*************************/ 
	int statue = 0;
	mqtt.Fixedheader_len = 1;
	mqtt.Variableheader_len = 10;
	mqtt.Load_len = 2 + strlen(mqtt.ClientID) +											//前面的2指接下来的数据长度，目前不包含遗嘱
					2 + strlen(mqtt.UserName) +
					2 + strlen(mqtt.Passward);		
	mqtt.Reamining_len = mqtt.Variableheader_len + mqtt.Load_len;	
			
	mqtt.buff[0] = 0x10; 
	do{
		if(mqtt.Reamining_len/128 == 0)													//不需要进位 
		{
			mqtt.buff[mqtt.Fixedheader_len] = mqtt.Reamining_len; 
		}else
		{
			mqtt.buff[mqtt.Fixedheader_len] = (mqtt.Reamining_len % 128) | 0x80; 
		} 
		mqtt.Fixedheader_len++;
		statue++;
		mqtt.Reamining_len = mqtt.Reamining_len/128; 
	}while(mqtt.Reamining_len);
	
	/************************可变报头*************************/ 
	/* 
	* 7.   User Name Flag	1
	* 6.   Password Flag	1
	* 5.   Will Retain		0
	* 4.   Will QoS			0
	* 3.   Will QoS			0
	* 2.   Will Flag		0
	* 1.   Clean Session	1
	* 0.   Reserved			0
	*/
	
	mqtt.buff[mqtt.Fixedheader_len++] = 0x00;											//长度高八位 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x04;											//长度低八位 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x4D;											//"M" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x51;											//"Q" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x54;											//"T" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x54;											//"T" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x04;											//协议级别 
	mqtt.buff[mqtt.Fixedheader_len++] = 0xC2;											//连接标志位：
	mqtt.buff[mqtt.Fixedheader_len++] = keepalive/256;									//保持时间高位 
	mqtt.buff[mqtt.Fixedheader_len++] = keepalive%256;									//保持时间高低位 
	
	/************************有效负载*************************/ 
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(mqtt.ClientID)/256;
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(mqtt.ClientID)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len], mqtt.ClientID, strlen(mqtt.ClientID));
	
	mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID)] = strlen(mqtt.UserName)/256;	
	mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID) + 1] = strlen(mqtt.UserName)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID) + 2], mqtt.UserName, strlen(mqtt.UserName));
	
	mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID) + strlen(mqtt.UserName) +2] = strlen(mqtt.Passward)/256;
	mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID) + strlen(mqtt.UserName) +3] = strlen(mqtt.Passward)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + strlen(mqtt.ClientID) + strlen(mqtt.UserName) + 4], mqtt.Passward, strlen(mqtt.Passward));
	
	mqtt.Fixedheader_len = 1 + statue;													//固定报头最终长度（包括剩余长度）
	
	mqtt.length =  mqtt.Fixedheader_len + mqtt.Variableheader_len + mqtt.Load_len;		//报文总长度 
}

/************************有遗嘱版本connect函数*************************/ 
void MQTT_CONNECTWILL( u8 Will_Retain, u8 Will_QoS, u8 Clean_Session, u32 keepalive)
{
	/************************固定报头*************************/ 
	int statue = 0;
	mqtt.Fixedheader_len = 1;
	mqtt.Variableheader_len = 10;
	mqtt.Load_len = 2 + strlen(mqtt.ClientID) +	
					2 + strlen(mqtt.UserName) +
					2 + strlen(mqtt.WillTopic) +
					2 + strlen(mqtt.WillData) +
					2 + strlen(mqtt.Passward);		
	mqtt.Reamining_len = mqtt.Variableheader_len + mqtt.Load_len;	
			
	mqtt.buff[0] = 0x10; 
	do{
		if(mqtt.Reamining_len/128 == 0)													//不需要进位 
		{
			mqtt.buff[mqtt.Fixedheader_len] = mqtt.Reamining_len; 
		}else
		{
			mqtt.buff[mqtt.Fixedheader_len] = (mqtt.Reamining_len % 128) | 0x80; 
		} 
		mqtt.Fixedheader_len++;
		statue++;
		mqtt.Reamining_len = mqtt.Reamining_len/128; 
	}while(mqtt.Reamining_len);
	
	/************************可变报头*************************/ 
	/* 
	* 7.   User Name Flag	1
	* 6.   Password Flag	1
	* 5.   Will_Retain		
	* 4.   Will_QoS			
	* 3.   Will_QoS			
	* 2.   Will_Flag		1
	* 1.   Clean_Session	
	* 0.   Reserved			0
	*/
	
	mqtt.buff[mqtt.Fixedheader_len++] = 0x00;											//长度高八位 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x04;											//长度低八位 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x4D;											//"M" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x51;											//"Q" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x54;											//"T" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x54;											//"T" 
	mqtt.buff[mqtt.Fixedheader_len++] = 0x04;											//协议级别 
	mqtt.buff[mqtt.Fixedheader_len++] = 0xC4 | (Will_Retain << 5) 						//遗嘱保留 
											 | (Will_QoS << 3) 							//服务等级 
											 | (Clean_Session << 1);					//清理会话功能 
	mqtt.buff[mqtt.Fixedheader_len++] = keepalive/256;									//保持时间高位 
	mqtt.buff[mqtt.Fixedheader_len++] = keepalive%256;									//保持时间高低位 
	
	/************************有效负载*************************/ 
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(mqtt.ClientID)/256;
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(mqtt.ClientID)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len], mqtt.ClientID, strlen(mqtt.ClientID));
	
	mqtt.buff[mqtt.Fixedheader_len + 0 + strlen(mqtt.ClientID)] = strlen(mqtt.WillTopic)/256;	
	mqtt.buff[mqtt.Fixedheader_len + 1 + strlen(mqtt.ClientID)] = strlen(mqtt.WillTopic)%256;
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + 2 + strlen(mqtt.WillTopic)], mqtt.WillTopic, strlen(mqtt.WillTopic));
	
	mqtt.buff[mqtt.Fixedheader_len + 2 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic)] = strlen(mqtt.WillData)/256;	
	mqtt.buff[mqtt.Fixedheader_len + 3 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic)] = strlen(mqtt.WillData)%256;
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + 4 + strlen(mqtt.WillTopic) + strlen(mqtt.WillTopic)], mqtt.WillData, strlen(mqtt.WillData));

	mqtt.buff[mqtt.Fixedheader_len + 4 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic) + strlen(mqtt.WillData)] = strlen(mqtt.UserName)/256;	
	mqtt.buff[mqtt.Fixedheader_len + 5 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic)+ strlen(mqtt.WillData)] = strlen(mqtt.UserName)%256;
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + 6 + strlen(mqtt.WillTopic) + strlen(mqtt.WillTopic)+ strlen(mqtt.WillData)], mqtt.UserName, strlen(mqtt.UserName));
	
	mqtt.buff[mqtt.Fixedheader_len + 6 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic) + strlen(mqtt.WillData) + strlen(mqtt.UserName)] = strlen(mqtt.Passward)/256;	
	mqtt.buff[mqtt.Fixedheader_len + 7 + strlen(mqtt.ClientID) + strlen(mqtt.WillTopic)+ strlen(mqtt.WillData) + strlen(mqtt.UserName)] = strlen(mqtt.Passward)%256;
	memcpy(&mqtt.buff[mqtt.Fixedheader_len + 8 + strlen(mqtt.WillTopic) + strlen(mqtt.WillTopic)+ strlen(mqtt.WillData) + strlen(mqtt.UserName)], mqtt.Passward, strlen(mqtt.Passward));
	
	mqtt.Fixedheader_len = 1 + statue;													//固定报头最终长度（包括剩余长度） 
	
	mqtt.length =  mqtt.Fixedheader_len + mqtt.Variableheader_len + mqtt.Load_len;		//报文总长度 
}

char MQTT_CONNACK(u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x20))
	{
		
	}else
	{
		return -1;
	}
	return rxdata[3];
} 

/************************DISCONNECT函数*************************/ 
void MQTT_DISCONNECT(void)
{
	mqtt.buff[0] = 0xE0;
	mqtt.buff[1] = 0x00;
	mqtt.length = 2;
}

/************************SUBSCRIBE函数*************************/ 
void MQTT_SUBSCRIBE(char* topic, char QS)												//一次制订阅一个主题 
{
	/************************固定报头*************************/ 
	int statue = 0;
	mqtt.Fixedheader_len = 1;
	mqtt.Variableheader_len = 2;
	mqtt.Load_len = 2 + strlen(topic) +	1;												//2:主题长度，1服务等级 
	mqtt.Reamining_len = mqtt.Variableheader_len + mqtt.Load_len;
				
	mqtt.buff[0] = 0x82; 
	do{
		if(mqtt.Reamining_len/128 == 0)													//不需要进位 
		{
			mqtt.buff[mqtt.Fixedheader_len] = mqtt.Reamining_len; 
		}else
		{
			mqtt.buff[mqtt.Fixedheader_len] = (mqtt.Reamining_len % 128) | 0x80; 
		} 
		mqtt.Fixedheader_len++;
		statue++;
		mqtt.Reamining_len = mqtt.Reamining_len/128; 
	}while(mqtt.Reamining_len);
	
	/************************可变报头*************************/ 
	
	mqtt.buff[mqtt.Fixedheader_len++] = mqtt.MessageID/256;								//报文标识符高位 
	mqtt.buff[mqtt.Fixedheader_len++] = mqtt.MessageID%256;								//报文标识符低位 
	mqtt.MessageID++;
	if(mqtt.MessageID == 0)
	{
		mqtt.MessageID = 1;
	} 
	
	/************************有效负载*************************/ 
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(topic)/256;
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(topic)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len], topic, strlen(topic));
	mqtt.buff[mqtt.Fixedheader_len + strlen(topic)] = QS;
	
	mqtt.Fixedheader_len = 1 + statue;													//固定报头最终长度（包括剩余长度）
	
	mqtt.length =  mqtt.Fixedheader_len + mqtt.Variableheader_len + mqtt.Load_len;		//报文总长度 
}

/************************SUBACK函数*************************/ 
char MQTT_SUBACK(u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 5) && (rxdata[0] == 0x90))
	{
	}else
	{
		return -1;
	}
	return rxdata[4];																	//服务等级 
} 

/************************SUBSCRIBE函数*************************/ 
void MQTT_UNSUBSCRIBE(char* topic)
{
	/************************固定报头*************************/ 
	int statue = 0;
	mqtt.Fixedheader_len = 1;
	mqtt.Variableheader_len = 2;
	mqtt.Load_len = 2 + strlen(topic);													//2:主题长度 
	mqtt.Reamining_len = mqtt.Variableheader_len + mqtt.Load_len;
				
	mqtt.buff[0] = 0xA2; 
	do{
		if(mqtt.Reamining_len/128 == 0)													//不需要进位 
		{
			mqtt.buff[mqtt.Fixedheader_len] = mqtt.Reamining_len; 
		}else
		{
			mqtt.buff[mqtt.Fixedheader_len] = (mqtt.Reamining_len % 128) | 0x80; 
		} 
		mqtt.Fixedheader_len++;
		statue++;
		mqtt.Reamining_len = mqtt.Reamining_len/128; 
	}while(mqtt.Reamining_len);
	
	/************************可变报头*************************/ 
	
	mqtt.buff[mqtt.Fixedheader_len++] = mqtt.MessageID/256;								//报文标识符高位 
	mqtt.buff[mqtt.Fixedheader_len++] = mqtt.MessageID%256;								//报文标识符低位 
	mqtt.MessageID++;
	if(mqtt.MessageID == 0)
	{
		mqtt.MessageID = 1;
	} 
	
	/************************有效负载*************************/ 
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(topic)/256;
	mqtt.buff[mqtt.Fixedheader_len++] = strlen(topic)%256;	
	memcpy(&mqtt.buff[mqtt.Fixedheader_len], topic, strlen(topic));
	
	mqtt.Fixedheader_len = 1 + statue;													//固定报头最终长度（包括剩余长度）
	
	mqtt.length =  mqtt.Fixedheader_len + mqtt.Variableheader_len + mqtt.Load_len;		//报文总长度 
}

/************************SUBACK函数*************************/ 
char MQTT_UNSUBACK(u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 4) && (rxdata[0] == 0xB0))
	{
	}else
	{
		return -1;
	}
	return 1;
} 

