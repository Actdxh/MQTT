#ifndef __MQTT_TYPE_H
#define __MQTT_TYPE_H

#include <stdint.h>

#define PARA_SIZE 64
#define BUFF_SIZE 256
#define MQTT_TXBUF_SIZE 1024


#ifndef MQTT_RXBUF_SIZE
#define MQTT_RXBUF_SIZE 1024
#endif

#define MQTT_CONNECT_BUF_SIZE 256
#define MQTT_PUBLISH_BUF_SIZE 256
#define MQTT_SUBSCRIBE_BUF_SIZE 256
#define MQTT_UNSUBSCRIBE_BUF_SIZE 256
#define MQTT_PINGREQ_BUF_SIZE 256
#define MQTT_PUBACK_BUF_SIZE 256
#define MQTT_DISCONNECT_BUF_SIZE 256

#define MQTT_SUB_CACHE_TOPIC_SIZE 64//缓存用于reconnect后sub的话题

typedef enum {
    MQTT_CONN_DISCONNECTED = 0,
    MQTT_CONN_CONNECTED = 1,
} mqtt_conn_state_t;

// typedef enum {
//     MQTT_SUBSCRIBED_NONE = 0,
//     MQTT_SUBSCRIBED_ONE = 1,
// } mqtt_subscribed_state_t;

// typedef struct {
//     volatile mqtt_conn_state_t connected;
//     volatile mqtt_subscribed_state_t subscribed;
//     volatile uint8_t pingresp_seen;
//     volatile uint16_t puback_pid;//0是无效值
// } app_ctx_t;

typedef enum {
    MQTT_RX_UNHANDLED = 0,

    MQTT_RX_CONNACK = 1,
    MQTT_RX_SUBACK  = 2,
    MQTT_RX_UNSUBACK = 3,
    MQTT_RX_PINGRESP= 4,

    MQTT_RX_PUBACK  = 5,

    MQTT_RX_PUBLISH_QOS0 = 10,
    MQTT_RX_PUBLISH_QOS1 = 11,          // 收到 QoS1 PUBLISH（已解析）
    MQTT_RX_PUBLISH_QOS1_ACKED = 12,    // QoS1 且已生成并触发 on_send 发送 PUBACK
	

    MQTT_RX_PUBLISH_QOS2_UNSUPPORTED = 19,
} mqtt_rx_event_t;

typedef enum {
    MQTT_ERR_ARG        	    = -1,// 参数错误
    MQTT_ERR_INCOMPLETE 	    = -2,// 包不完整
    MQTT_ERR_MALFORMED  	    = -3,// 包格式错误
    MQTT_ERR_UNSUPPORTED	    = -4,// 不支持的功能，比如 QoS 2
	MQTT_ERR_PID_MISMATCH 	    = -5, // SUBACK 的报文标识符与最近一次 SUBSCRIBE 不匹配
    MQTT_ERR_NO_TIME            = -6, // 没有获取当前时间的函数，无法处理超时相关功能
    MQTT_ERR_TIMEOUT            = -7, // 超时
    MQTT_ERR_SEND_INCOMPLETE    = -8, // 发送数据不完整，可能需要重试
    MQTT_ERR_NEED_RECONNECT     = -9, // 需要重新连接，例如在保活机制中连续多次 PING 请求超时后
} mqtt_err_t;

typedef enum{
    MQTT_PROCESS_NONE = 0,
    MQTT_PROCESS_PING_SENT = 1,
    MQTT_PROCESS_PUBACK_RETRY = 10,
} mqtt_process_event_t;

typedef enum{
    MQTT_PENDING_NONE           = 0,
    MQTT_PENDING_CONNECT        = (1u << 0),
    MQTT_PENDING_SUBSCRIBE      = (1u << 1),
    MQTT_PENDING_PUBLISH_QOS0   = (1u << 2),
    MQTT_PENDING_PUBLISH_QOS1   = (1u << 3),
    MQTT_PENDING_PUBACK         = (1u << 4),
    MQTT_PENDING_UNSUBSCRIBE    = (1u << 5),
    MQTT_PENDING_DISCONNECT     = (1u << 6),
    MQTT_PENDING_PINGREQ        = (1u << 7),  
}mqtt_pending_bits_t;

typedef struct {
    uint8_t  valid;
    uint8_t  qos;
    char     topic[MQTT_SUB_CACHE_TOPIC_SIZE];
} mqtt_sub_cache_t;

typedef struct 
{
    uint16_t connect_buf_len;					//连接报文长度
    uint16_t publish_buf_len;					//发布报文长度
    uint16_t subscribe_buf_len;				//订阅报文长度
    uint16_t unsubscribe_buf_len;				//取消订阅报文长度
    uint16_t pingreq_buf_len;					//ping请求报文长度
    uint16_t puback_buf_len;					//puback报文长度
    uint16_t disconnect_buf_len;				//断开连接报文长度
}mqtt_pack_len_t;

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
    mqtt_pack_len_t pack_len;					//各种报文的长度，方便调试和重发时使用
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
    uint32_t puback_timeout_ms; // PUBACK 超时时间
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
    uint16_t packet_id;
} mqtt_puback_view_t;

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

typedef struct {
    uint16_t packet_id;
}mqtt_unsuback_view_t;

typedef void (*mqtt_on_publish_cb)(void* user_ctx, const mqtt_publish_view_t* msg);
typedef int (*mqtt_on_send_cb)(void* user_ctx, const uint8_t* data, uint16_t len);
typedef void (*mqtt_on_connack_cb)(void* user_ctx, const mqtt_connack_view_t* v);
typedef void (*mqtt_on_suback_cb)(void* user_ctx, const mqtt_suback_view_t* v);
typedef void (*mqtt_on_pingresp_cb)(void* user_ctx);
typedef void (*mqtt_on_puback_cb)(void* user_ctx, const mqtt_puback_view_t* v);
typedef void (*mqtt_on_unsuback_cb)(void* user_ctx, const mqtt_unsuback_view_t* v);
typedef uint32_t (*mqtt_now_ms_fn)(void* user_ctx);// 获取当前时间的函数指针，单位毫秒，返回值为当前时间的毫秒数，可以用于实现超时等功能

typedef struct {
    mqtt_on_publish_cb  on_publish;
    void*               on_publish_ctx;
    mqtt_on_send_cb     on_send;
    void*               on_send_ctx;
    mqtt_on_connack_cb  on_connack;
    void*               on_connack_ctx;
    mqtt_on_suback_cb   on_suback;
    void*               on_suback_ctx;
    mqtt_on_unsuback_cb on_unsuback;
    void*               on_unsuback_ctx;
    mqtt_on_puback_cb   on_puback;
    void*               on_puback_ctx;
    mqtt_on_pingresp_cb on_pingresp;
    void*               on_pingresp_ctx;
} MQTT_Callbacks;

typedef struct 
{
    mqtt_now_ms_fn now_ms;
    void* now_ms_ctx;
} MQTT_Platform;

typedef struct 
{
    uint8_t  rx_buf[MQTT_RXBUF_SIZE];		//接收缓冲区		这是用在input函数里面处理服务器发来信息的接受缓冲
	uint16_t rx_buf_len;					//接收缓冲区长度	这是用在input函数里面处理服务器发来信息的接受缓冲的长度

    uint8_t  connect_buf[MQTT_CONNECT_BUF_SIZE];	//连接报文缓冲区
    uint8_t  publish_buf[MQTT_PUBLISH_BUF_SIZE];			//发布报文缓冲区
    uint8_t  subscribe_buf[MQTT_SUBSCRIBE_BUF_SIZE];		//订阅报文缓冲区
    uint8_t  unsubscribe_buf[MQTT_UNSUBSCRIBE_BUF_SIZE];		//取消订阅报文缓冲区
    uint8_t  pingreq_buf[MQTT_PINGREQ_BUF_SIZE];			//ping请求报文缓冲区
    uint8_t  puback_buf[MQTT_PUBACK_BUF_SIZE];			//puback报文缓冲区
    uint8_t  disconnect_buf[MQTT_DISCONNECT_BUF_SIZE];		//断开连接报文缓冲区
    // uint8_t  tx_buf[MQTT_TXBUF_SIZE];		//发送缓冲区

}MQTT_Buffers;

typedef struct{
    //packet ids
    uint32_t tx_pending;
    
    uint16_t next_pid;
    uint16_t last_subscribe_pid;
    uint16_t last_publish_pid;
    uint16_t last_unsubscribe_pid;

    uint8_t connack_rc;          // 最近一次 CONNACK return code//用于调试
    uint8_t session_present;	// 最近一次 CONNACK session present 标志//用于调试

    // QoS1 单 inflight 重发
    uint8_t  puback_outstanding;      // 1=有QoS1 PUBLISH在等PUBACK
    uint16_t puback_pid;              // 正在等待的PID
    uint32_t puback_sent_ms;          // 上次发送该PUBLISH的时间
    uint16_t puback_frame_len;        // 上次打包的PUBLISH帧长度（tx_buf里）
    uint8_t  puback_retry_count;      // 重发次数

    mqtt_conn_state_t conn_state;			//连接状态
    mqtt_sub_cache_t sub_cache; // 订阅缓存，用于记录已订阅的主题和 QoS，方便重连后恢复订阅状态
    int last_event_code;					//上次接收事件的事件代码，主要用于调试，被使用在input函数里面
}MQTT_SESSION;

typedef struct {
    //keepalive runtime
    uint32_t keepalive_ms;
    uint32_t ping_timeout_ms;
    uint32_t last_tx_ms;
    uint32_t last_rx_ms;
    uint32_t last_pingreq_ms;

    uint8_t ping_outstanding; // 是否有未完成的 ping 请求

}MQTT_Keepalive;



typedef struct{
	MQTT_config_t	param;					//参数结构体
	MQTT_length_t   length;				    //长度结构体
    MQTT_Buffers    io;					    //缓冲区结构体
    MQTT_SESSION    ses;				    //会话状态结构体
    MQTT_Keepalive  ka;			            //保活机制结构体
    MQTT_Callbacks  callbacks;              //回调函数集合
    MQTT_Platform   platform;               //平台相关函数集合
}MQTT_TCB;





#endif

