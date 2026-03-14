#ifndef __MQTT_H
#define __MQTT_H

#include "cc.h"
#include "mqtt_parse.h"


#define PARA_SIZE 64
#define BUFF_SIZE 256
#define TOPIC_SIZE 256
#define DATA_SIZE 256

#ifndef MQTT_RXBUF_SIZE
#define MQTT_RXBUF_SIZE 1024
#endif

typedef struct{
	uint16_t cid_len;
	uint16_t user_len;
	uint16_t pwd_len;
	uint16_t willtopic_len;
	uint16_t willdata_len;
//	uint16_t topic_len;							//接收到服务器推送的订阅的主题长度不是用来发送的，在函数内部使用的主题长度是用uint16_t topic_len = (uint16_t)strlen(topic);
	uint32_t Fixedheader_len;					//固定报头长度(也含剩余长度） 单位都是字节也就是八位二进制 
	uint32_t Remaining_len;						//剩余长度 
	uint32_t Variableheader_len;				//可变报头长度
	uint32_t Load_len;							//负载长度 
	uint32_t Totallength;						//报文总长度 
}MQTT_length_t;


typedef struct{	
	char ClientID[PARA_SIZE];				//参数缓冲区 
	char UserName[PARA_SIZE];				//参数缓冲区 
	char Passward[PARA_SIZE];				//参数缓冲区 
	char WillEnable;						//遗嘱使能标志，1使能，0不使能
	char WillTopic[PARA_SIZE];				//遗嘱主题缓冲区 
	char WillData[PARA_SIZE];				//遗嘱数据缓冲区
	char WillRetain;						//遗嘱保留标志，1保留，0不保留
	char WillQoS;							//遗嘱服务等级，0、1、2
	char CleanSession;						//连接会话清除标志，1清除，0不清除
}MQTT_config_t;

typedef void (*mqtt_on_message_cb)(void* user_ctx, const mqtt_publish_view_t* msg);
typedef void (*mqtt_on_send_cb)(void* user_ctx, const uint8_t* data, uint16_t len);

typedef struct{
	uint8_t  rx_buf[MQTT_RXBUF_SIZE];		//接收缓冲区			|这俩个是用在input函数里面处理服务器发来信息的接受缓冲
	uint16_t rx_buf_len;					//接收缓冲区长度		|

	uint32_t MessageID;						//报文标识符 
	MQTT_length_t length;				    //长度结构体
	uint8_t buff[BUFF_SIZE];			//数据缓冲区 
	MQTT_config_t	param;					//参数结构体
	char topic[TOPIC_SIZE];					//接收到服务器推送的订阅的主题缓冲区 
	uint8_t data[DATA_SIZE];						//接收到服务推送的数据
	mqtt_on_message_cb on_message;			//消息回调函数指针
	mqtt_on_send_cb on_send;					//发送回调函数指针
	void* user_ctx;						//用户上下文指针，在回调函数中传递给用户使用
}MQTT_TCB;

typedef struct {
    const char* topic;
    const void* payload;
    uint16_t payload_len;

    uint8_t qos;     // 0/1
    uint8_t retain;  // 0/1
    uint8_t dup;     // 0/1
}mqtt_publish_params_t;


int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config);
int MQTT_CONNECT(MQTT_TCB *m, u32 keepalive);
int MQTT_SUBSCRIBE(MQTT_TCB *m, char* topic, char QS);
char MQTT_CONNACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len);
void MQTT_DISCONNECT(MQTT_TCB *m);
char MQTT_SUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len); 
int  MQTT_UNSUBSCRIBE(MQTT_TCB *m, char* topic);
char MQTT_UNSUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len);
void MQTT_PINGREQ(MQTT_TCB *m);
char MQTT_PINGRESP(MQTT_TCB *m, u8* rxdata, u32 rxdata_len);
void MQTT_PUBLISH0(MQTT_TCB *m, char retain, char* topic, u8 *data, u32 data_len);
int MQTT_PUBLISH(MQTT_TCB *m, char dup, char QoS, char retain, char* topic, void *data, u32 data_len);
char MQTT_ProcessPUBLISH(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u8 *qs, u32* messageid); 
void MQTT_PUBACK(MQTT_TCB *m, u32 messageid);
char MQTT_ProcessPublish(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid);
void MQTT_PUBREC(MQTT_TCB *m, u32 messageid);
char MQTT_ProcessPUBREC(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid);
void MQTT_PUBREL(MQTT_TCB *m, u32 messageid); 
char MQTT_ProcessPUBREL(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid);
void MQTT_PUBCOMP(MQTT_TCB *m, u32 messageid);
char MQTT_ProcessPUBCOMP(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid);


int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive);
int mqtt_pack_subscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, const char * topic, char qos);
int mqtt_pack_unsubscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, char * topic);
int mqtt_pack_publish(
    MQTT_TCB* m,
    uint8_t* out,
    uint16_t out_size,
    const char* topic,
    const void* payload,
    uint16_t payload_len,
    uint8_t qos,       // 0/1（先不做2）
    uint8_t retain,    // 0/1
    uint8_t dup        // 0/1（qos=0 时 dup 也允许但一般没意义）
);
int mqtt_pack_publish_two(MQTT_TCB* m,uint8_t* out,uint16_t out_size, mqtt_publish_params_t *params);
int mqtt_parse_publish_view(const uint8_t* rx, uint32_t rx_len, mqtt_publish_view_t* view);
int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len);



void my_on_message(void* user_ctx, const mqtt_publish_view_t* msg);
void my_on_send(void* user_ctx, const uint8_t* data, uint16_t len);

void MQTT_SetOnMessage(MQTT_TCB* m, mqtt_on_message_cb cb, void* user_ctx);
void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx);

int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len);
#endif

