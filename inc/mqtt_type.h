#ifndef __MQTT_TYPE_H
#define __MQTT_TYPE_H

#include <stdint.h>

#define PARA_SIZE 64
#define BUFF_SIZE 256
#define TOPIC_SIZE 256
#define DATA_SIZE 256

#ifndef MQTT_RXBUF_SIZE
#define MQTT_RXBUF_SIZE 1024
#endif

typedef enum {
    MQTT_CONN_DISCONNECTED = 0,
    MQTT_CONN_CONNECTED = 1,
} mqtt_conn_state_t;

typedef enum {
    MQTT_SUBSCRIBED_NONE = 0,
    MQTT_SUBSCRIBED_ONE = 1,
} mqtt_subscribed_state_t;

typedef enum {
    MQTT_RX_UNHANDLED = 0,

    MQTT_RX_CONNACK = 1,
    MQTT_RX_SUBACK  = 2,
    MQTT_RX_PINGRESP= 3,

    MQTT_RX_PUBACK  = 4,

    MQTT_RX_PUBLISH_QOS0 = 10,
    MQTT_RX_PUBLISH_QOS1 = 11,          // 收到 QoS1 PUBLISH（已解析）
    MQTT_RX_PUBLISH_QOS1_ACKED = 12,    // QoS1 且已生成并触发 on_send 发送 PUBACK
	

    MQTT_RX_PUBLISH_QOS2_UNSUPPORTED = 19,
} mqtt_rx_event_t;

typedef enum {
    MQTT_ERR_ARG        	= -1,// 参数错误
    MQTT_ERR_INCOMPLETE 	= -2,// 包不完整
    MQTT_ERR_MALFORMED  	= -3,// 包格式错误
    MQTT_ERR_UNSUPPORTED	= -4,// 不支持的功能，比如 QoS 2
	MQTT_ERR_PID_MISMATCH 	= -5, // SUBACK 的报文标识符与最近一次 SUBSCRIBE 不匹配
} mqtt_err_t;

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

typedef struct{
    const char* topic;
    const void* payload;
    uint16_t payload_len;

    uint8_t qos;     // 0/1
    uint8_t retain;  // 0/1
    uint8_t dup;     // 0/1
}mqtt_publish_params_t;

typedef struct {
    volatile mqtt_conn_state_t connected;
    volatile mqtt_subscribed_state_t subscribed;
} app_ctx_t;

typedef struct {
    const uint8_t* topic;
    uint16_t topic_len;

    const uint8_t* payload;
    uint32_t payload_len;

    uint8_t qos;
    uint8_t dup;
    uint8_t retain;

    uint16_t packet_id; // qos>0 才有效，否则 0
    uint32_t packet_len;
} mqtt_publish_view_t;

typedef struct {
    uint8_t session_present;
    uint8_t return_code;   // 0=accepted
} mqtt_connack_view_t;

typedef struct {
    uint16_t packet_id;

    const uint8_t* return_codes;
    uint8_t return_codes_len;    // 至少 1

    uint32_t packet_len;         // 这帧总长（方便调试）
} mqtt_suback_view_t;

typedef void (*mqtt_on_message_cb)(void* user_ctx, const mqtt_publish_view_t* msg);
typedef void (*mqtt_on_send_cb)(void* user_ctx, const uint8_t* data, uint16_t len);
typedef void (*mqtt_on_connack_cb)(void* user_ctx, const mqtt_connack_view_t* v);
typedef void (*mqtt_on_suback_cb)(void* user_ctx, const mqtt_suback_view_t* v);

typedef struct{
	uint8_t  rx_buf[MQTT_RXBUF_SIZE];		//接收缓冲区		这是用在input函数里面处理服务器发来信息的接受缓冲
	uint16_t rx_buf_len;					//接收缓冲区长度	这是用在input函数里面处理服务器发来信息的接受缓冲

	uint32_t MessageID;						//报文标识符 
	MQTT_length_t length;				    //长度结构体
	uint8_t buff[BUFF_SIZE];			//数据缓冲区 
	MQTT_config_t	param;					//参数结构体
	char topic[TOPIC_SIZE];					//接收到服务器推送的订阅的主题缓冲区 
	uint8_t data[DATA_SIZE];						//接收到服务推送的数据
	mqtt_on_message_cb on_message;			//消息回调函数指针
	mqtt_on_send_cb on_send;					//发送回调函数指针
	mqtt_on_connack_cb on_connack;			//连接确认回调函数指针
	mqtt_on_suback_cb on_suback;			//订阅确认回调函数指针
	void* user_ctx;						//用户上下文指针，在回调函数中传递给用户使用
	uint16_t last_event_code;					//上次接收事件的事件代码，主要用于调试，被使用在input函数里面
	uint16_t last_subscribe_pid;
    uint8_t connack_rc;          // 最近一次 CONNACK return code//用于调试
    uint8_t session_present;	// 最近一次 CONNACK session present 标志//用于调试
}MQTT_TCB;





#endif

