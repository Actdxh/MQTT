#ifndef __MQTT_H
#define __MQTT_H

#include "cc.h"

#define PARA_SIZE 64
#define BUFF_SIZE 256
#define TOPIC_SIZE 256
#define DATA_SIZE 256


typedef struct{
	u32 Fixedheader_len;					//固定报头长度(也含剩余长度） 单位都是字节也就是八位二进制 
	u32 Reamining_len;						//剩余长度 
	u32 Variableheader_len;					//可变报头长度
	u32 Load_len;							//负载长度 
	u32 MessageID;							//报文标识符 
	u32 length;								//报文总长度 
	
	unsigned char buff[BUFF_SIZE];			//数据缓冲区 
	
	char ClientID[PARA_SIZE];				//参数缓冲区 
	char UserName[PARA_SIZE];				//参数缓冲区 
	char Passward[PARA_SIZE];				//参数缓冲区 
	
	char WillTopic[PARA_SIZE];				//遗嘱主题缓冲区 
	char WillData[PARA_SIZE];				//遗嘱数据缓冲区 
	
	char topic[TOPIC_SIZE];					//接收到服务器推送的订阅的主题缓冲区 
	u8 data[DATA_SIZE];						//接收到服务推送的数据 
}MQTT_TCB;

extern MQTT_TCB mqtt;




void MQTT_Init(void);
void MQTT_CONNECT(u32 keepalive);
void MQTT_CONNECTWILL( u8 Will_Retain, u8 Will_QoS, u8 Clean_Session, u32 keepalive);
char MQTT_CONNACK(u8* rxdata, u32 rxdata_len);
void MQTT_DISCONNECT(void);
char MQTT_SUBACK(u8* rxdata, u32 rxdata_len); 
void MQTT_UNSUBSCRIBE(char* topic);
char MQTT_UNSUBACK(u8* rxdata, u32 rxdata_len);
void MQTT_PINGREQ(void);
char MQTT_PINGRESP(u8* rxdata, u32 rxdata_len);
void MQTT_PUBLISH0(char retain, char* topic, u8 *data, u32 data_len);
void MQTT_PUBLISH(char dup, char QoS, char retain, char* topic, u8 *data, u32 data_len);
char MQTT_ProcessPUBLISH(u8* rxdata, u32 rxdata_len, u8 *qs, u32* messageid); 
void MQTT_PUBACK(u32 messageid);
char MQTT_ProcessPublish(u8* rxdata, u32 rxdata_len, u32* messageid);
void MQTT_PUBREC(u32 messageid);
char MQTT_ProcessPUBREC(u8* rxdata, u32 rxdata_len, u32* messageid);
void MQTT_PUBREL(u32 messageid); 
char MQTT_ProcessPUBREL(u8* rxdata, u32 rxdata_len, u32* messageid);



#endif
