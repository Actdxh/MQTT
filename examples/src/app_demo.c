#include "app_demo.h"
#include "app_ctx.h"
#include "mqtt_core.h"
#include "mqtt_pack.h"
#include "mqtt_utils.h"
#include "mqtt_parse.h"
#include "mqtt_handlers.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define MQTT_DEMO_DEBUG

typedef enum {
	ST_INIT = 0,
	ST_SEND_CONNECT,
	ST_SEND_SUBSCRIBE,
	ST_SEND_PING,
    ST_SEND_PUBLISH1,
	

    ST_FEED_CONNACK,
    ST_FEED_SUBACK,
    ST_FEED_PINGRESP,
    ST_FEED_PUBACK1,

    ST_WAIT_CONNACK,
    ST_WAIT_SUBACK,
    ST_WAIT_PINGRESP,
    ST_WAIT_PUBACK,

    ST_DONE,
	ST_ERROR,

} app_demo_state_t;


int i; 
int res;
uint8_t databuf[128];
uint16_t datalen;
static app_evt_queue_t g_demo_ctx; // 全局应用事件队列
static int connack_wait_guard = 0; // 用于防止死循环的简单计数器
static int suback_wait_guard = 0; // 用于防止死循环的简单计数器
static int pingresp_wait_guard = 0; // 用于防止死循环的简单计数器
static int puback_wait_guard = 0; // 用于防止死循环的简单计数器
void app_demo(void)
{
    MQTT_TCB MqttA;
    app_demo_state_t st = ST_INIT;
    int guard = 0; // 用于防止死循环的简单计数器
    while(st != ST_DONE && st != ST_ERROR) {
        if(++guard > 1000)
        {
            printf("app_demo: guard break!\r\n");
            st = ST_ERROR;
            break;
        }
        switch (st)
        {
            case ST_INIT:{
                if(app_demo_init(&MqttA) < 0)
                {
                    st = ST_ERROR;
                } else {
                    st = ST_SEND_CONNECT;
                }
                app_ctx_init(&g_demo_ctx); // 初始化应用事件队列
                MQTT_SetAllOnCb_same(&MqttA, &(MQTT_Callbacks){
                    .on_connack = my_on_connack,
                    .on_connack_ctx = &g_demo_ctx,
                    .on_publish = my_on_publish,
                    .on_publish_ctx = &g_demo_ctx,
                    .on_send = my_on_send,
                    .on_send_ctx = NULL, // 发送回调不需要用户上下文
                    .on_suback = my_on_suback,
                    .on_suback_ctx = &g_demo_ctx,
                    .on_unsuback = my_on_unsuback,
                    .on_unsuback_ctx = &g_demo_ctx,
                    .on_pingresp = my_on_pingresp,
                    .on_pingresp_ctx = &g_demo_ctx,
                    .on_puback = my_on_puback,
                    .on_puback_ctx = &g_demo_ctx,
                }); // 注册回调函数
            }
                break;
            case ST_SEND_CONNECT:{
                connack_wait_guard = 0;
                mqtt_pack_connect(&MqttA, MqttA.io.tx_buf, BUFF_SIZE, 10000);
                mqtt_emit_send(&MqttA);
                #ifdef MQTT_DEMO_DEBUG
                printf("MQTT CONNECT message sent\r\n");
                #endif // DEBUG
                st = ST_FEED_CONNACK;
            }
                break;
            case ST_FEED_CONNACK:{
                feed_data(&MqttA, "20 02 00 00"); // 模拟服务器返回 CONNACK 包，表示连接成功
                st = ST_WAIT_CONNACK; // 等待连接确认事件
            }
                break;
            case ST_WAIT_CONNACK:{
                connack_wait_guard ++;
                if(connack_wait_guard > 100) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for CONNACK timeout!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 超时未收到 CONNACK，进入错误状态
                    break;
                }
                app_evt_t e;
                int res = app_evt_pop(&g_demo_ctx, &e);
                if(res == 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for CONNACK...\r\n");
                    #endif // DEBUG
                    st = ST_WAIT_CONNACK; // 继续等待连接确认事件
                    break;
                }else if(res < 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Error while waiting for CONNACK!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 等待事件时发生错误，进入错误状态
                    break;
                }else if(e.type == APP_EVT_CONNACK_OK) {
                    st = ST_SEND_SUBSCRIBE; // 连接成功，进入下一步
                }else if(e.type == APP_EVT_CONNACK_FAIL) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Connection failed, rc = %u\r\n", (unsigned)MqttA.ses.connack_rc);
                    #endif // DEBUG
                    st = ST_ERROR; // 连接失败，进入错误状态
                }
            }
                break;
            case ST_SEND_SUBSCRIBE:{
                suback_wait_guard = 0;
                mqtt_pack_subscribe(&MqttA, MqttA.io.tx_buf, BUFF_SIZE, "TOPIC001", 0);
                mqtt_emit_send(&MqttA);
                st = ST_FEED_SUBACK;
            }
                break;
            case ST_FEED_SUBACK:{
                uint16_t pid = MqttA.ses.last_subscribe_pid; // 获取刚才发送的 SUBSCRIBE 报文的 Packet ID
                uint8_t pid_high = (pid >> 8) & 0xFF;
                uint8_t pid_low = pid & 0xFF;
                char suback_hex[32];
                snprintf(suback_hex, sizeof(suback_hex), "90 03 %02X %02X 00", pid_high, pid_low);
                feed_data(&MqttA, suback_hex); // 模拟服务器返回 SUBACK 包，表示订阅成功
                st = ST_WAIT_SUBACK; // 等待订阅确认事件
            }
                break;
            case ST_WAIT_SUBACK:{
                suback_wait_guard++;
                if(suback_wait_guard > 100) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for SUBACK timeout!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 超时未收到 SUBACK，进入错误状态
                    break;
                }
                app_evt_t e;
                int res = app_evt_pop(&g_demo_ctx, &e);
                if(res == 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for SUBACK...\r\n");
                    #endif // DEBUG
                    st = ST_WAIT_SUBACK; // 继续等待订阅确认事件
                    break;
                }else if(res < 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Error while waiting for SUBACK!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 等待事件时发生错误，进入错误状态
                    break;
                }else if(e.type == APP_EVT_SUBACK_OK) {
                    st = ST_SEND_PUBLISH1; // 订阅成功，进入下一步
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Subscription successful\r\n");
                    #endif // DEBUG
                } else if(e.type == APP_EVT_SUBACK_FAIL) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Subscription failed\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 订阅失败，进入错误状态
                }
            }
                break;
            case ST_SEND_PUBLISH1:{
                puback_wait_guard = 0; // 确保等待 PUBACK 的计数器被正确初始化
                mqtt_publish_params_t params = {
                    .topic = "TEST",
                    .payload = "Hello MQTT",
                    .payload_len = strlen("Hello MQTT"),
                    .qos = 1,
                    .retain = 0,
                    .dup = 0
                };
                mqtt_pack_publish_two(&MqttA, MqttA.io.tx_buf, BUFF_SIZE, &params);
                mqtt_emit_send(&MqttA);
                #ifdef MQTT_DEMO_DEBUG
                printf("MQTT_NEW_PID: %u\r\n", MqttA.ses.last_publish_pid);
                #endif // DEBUG
                st = ST_FEED_PUBACK1;
            }
                break;
            case ST_FEED_PUBACK1:{
                uint16_t pid = MqttA.ses.last_publish_pid; // 获取刚才发送的 PUBLISH 报文的 Packet ID
                uint8_t pid_high = (pid >> 8) & 0xFF;
                uint8_t pid_low = pid & 0xFF;
                char puback_hex[32];
                snprintf(puback_hex, sizeof(puback_hex), "40 02 %02X %02X", pid_high, pid_low);
                feed_data(&MqttA, puback_hex); // 模拟服务器返回 PUBACK 包，表示已收到 QoS 1 的 PUBLISH 包
                st = ST_WAIT_PUBACK; // 等待 PUBACK 事件
                #ifdef MQTT_DEMO_DEBUG
                printf("Waiting for PUBACK with PID: %u\r\n", pid);
                #endif // DEBUG
            }
                break;
            case ST_WAIT_PUBACK:{
                app_evt_t e;
                puback_wait_guard++;
                if(puback_wait_guard > 100) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for PUBACK timeout!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 超时未收到 PUBACK，进入错误状态
                    break;
                }
                int res = app_evt_pop(&g_demo_ctx, &e);
                if(res == 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for PUBACK...\r\n");
                    #endif // DEBUG
                    st = ST_WAIT_PUBACK; // 继续等待 PUBACK 事件
                    break;
                }else if(res < 0){
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Error while waiting for PUBACK!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 等待事件时发生错误，进入错误状态
                    break;
                } else if(e.type == APP_EVT_PUBACK_OK && e.pid == MqttA.ses.last_publish_pid) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("PUBACK received for PID: %u\r\n", e.pid);
                    #endif // DEBUG
                    st = ST_SEND_PING; // 收到 PUBACK，进入下一步
                }else{
					#ifdef MQTT_DEMO_DEBUG
                    if(puback_wait_guard % 10 == 0) { // 每10次打印一次
                        printf("PUBACK EXPERT PID: %u, RECEIVED: %u\r\n", MqttA.ses.last_publish_pid, e.pid);
                    }
                    #endif // DEBUG
                    st = ST_WAIT_PUBACK; // 继续等待 PUBACK 事件 
                }
            }
                break;
            case ST_SEND_PING:{
                pingresp_wait_guard = 0; // 确保等待 PINGRESP 的计数器被正确初始化
                mqtt_pack_pingreq(&MqttA);
                mqtt_emit_send(&MqttA);
                st = ST_FEED_PINGRESP;
            }
                break;
            case ST_FEED_PINGRESP:{//这个是模拟服务器返回 PINGRESP 包，表示服务器响应了 PING 请求
                feed_data(&MqttA, "D0 00"); // 模拟服务器返回 PINGRESP 包，表示服务器响应了 PING 请求
                st = ST_WAIT_PINGRESP; // 等待 PINGRESP 事件
            }
                break;
            case ST_WAIT_PINGRESP:{
                pingresp_wait_guard++;
                if(pingresp_wait_guard > 100) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for PINGRESP timeout!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 超时未收到 PINGRESP，进入错误状态
                }
                app_evt_t e;
                int res = app_evt_pop(&g_demo_ctx, &e);
                if(res == 0) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Waiting for PINGRESP...\r\n");
                    #endif // DEBUG
                    st = ST_WAIT_PINGRESP; // 继续等待 PINGRESP 事件
                    break;
                }else if (res < 0)
                {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("Error while waiting for PINGRESP!\r\n");
                    #endif // DEBUG
                    st = ST_ERROR; // 等待事件时发生错误，进入错误状态
                    break;
                } else if(e.type == APP_EVT_PINGRESP) {
                    #ifdef MQTT_DEMO_DEBUG
                    printf("PINGRESP received!\r\n");
                    #endif // DEBUG
                    st = ST_DONE; // 收到 PINGRESP，流程完成
                }else 
                {
                    #ifdef MQTT_DEMO_DEBUG
                    if(pingresp_wait_guard % 10 == 0) { // 每10次打印一次
                       printf("PINGRESP not received yet!\r\n");
                    }
                    #endif // DEBUG
                    st = ST_WAIT_PINGRESP; // 继续等待 PINGRESP 事件 
                }
            }
                break;
            default:
                st = ST_ERROR;
                break;
                 
        }
    }

    if(st == ST_DONE) {
        printf("app_demo status: DONE\r\n");
    } else {
        printf("app_demo status: ERROR\r\n");
    }
    
}

void app_demo_test(void)
{
    //这是一个完整的示例，从初始化到连接服务器，再到订阅主题，最后发布消息，最后断开连接
    MQTT_TCB MqttA;
    app_demo_init(&MqttA);
    app_demo_connect(&MqttA);
    app_demo_conack(&MqttA);
    app_demo_subscribe(&MqttA);
    app_demo_suback(&MqttA);
    app_demo_unsubscribe(&MqttA);
    app_demo_publish(&MqttA);
    app_demo_pack_puback_myself(&MqttA); // 解析自己发送的 PUBLISH 报文，触发回调函数
    app_demo_pack_puback(&MqttA);
    app_demo_parse_puback(&MqttA);
    app_demo_ping(&MqttA);
    app_demo_pingresp(&MqttA);
    app_demo_disconnect(&MqttA);
}


int app_demo_init(MQTT_TCB* m)
{
    //这是一个初始化示例，展示了如何使用 MQTT_Init 函数来初始化 MQTT_TCB 结构体，并设置连接参数
    MQTT_config_t configA = {
        .ClientID = "USER001",
        .UserName = "USER001",
        .Passward = "USER001",
        .WillEnable = 1,
        .WillTopic = "WILL001",
        .WillData = "WILL001",
        .WillRetain = 0,
        .WillQoS = 0,
        .CleanSession = 1
    };
    res = MQTT_Init(m, &configA);
    #ifdef MQTT_DEMO_DEBUG
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    if(res != 0) {
		printf("Failed to initialize MQTT Client A\r\n");
		return -1;
	}else {
        printf("MQTT Client A initialized successfully\r\n");
    }
    #endif // DEBUG


    // MQTT_TCB MqttB;
    // MQTT_config_t configB = {
    //     .ClientID = "USER002",
    //     .UserName = "USER002",
    //     .Passward = "USER002",
    //     .WillEnable = 0,
    //     .WillTopic = "WILL002",
    //     .WillData = "WILL002",
    //     .WillRetain = 0,
    //     .WillQoS = 0,
    //     .CleanSession = 1
    // };
    // MQTT_Init(&MqttB, &configB);
}

int app_demo_connect(MQTT_TCB* m)
{
    //这是一个连接服务器的示例，展示了如何使用 MQTT_pack_connect 函数来构建连接报文，并通过回调函数处理服务器的响应
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    printf("MQTT Client A Connect:\r\n");
    res = mqtt_pack_connect(m, m->io.tx_buf, BUFF_SIZE, 10000);
    if(res < 0) {
        #ifdef MQTT_DEMO_DEBUG
        printf("Failed to pack MQTT CONNECT message res: %d\r\n", res);
        #endif // DEBUG
        return -1;
    }else {
        #ifdef MQTT_DEMO_DEBUG
        printf("MQTT CONNECT message packed successfully, length: %d\r\n", res);
        for(i = 0; i < m->length.Totallength; i++)
        {
            printf("%02x ", m->io.tx_buf[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
	printf("\r\n");

}

int app_demo_conack(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
	const char* connack_hex = "20 02 00 00";
	uint8_t b[8];
	int n = Str_to_Hex((char*)connack_hex, b);
	res = MQTT_InputBytes(m, (const uint8_t*)b, (uint16_t)n);
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to process CONNACK message res: %d\r\n", res);
    }else {
        printf("CONNACK message processed successfully\r\n");
    }
	printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_subscribe(MQTT_TCB* m)
{
    //这是一个订阅主题的示例，展示了如何使用 MQTT_pack_subscribe 函数来构建订阅报文，并通过回调函数处理服务器的响应
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    res = mqtt_pack_subscribe(m, m->io.tx_buf, BUFF_SIZE, "TEST", 1);
    if(res < 0) {
        #ifdef MQTT_DEMO_DEBUG
        printf("Failed to pack MQTT SUBSCRIBE message res: %d\r\n", res);
        #endif // DEBUG
        return -1;
    }else {
        #ifdef MQTT_DEMO_DEBUG
        printf("MQTT SUBSCRIBE message packed successfully, length: %d\r\n", res);
        for(i = 0; i < m->length.Totallength; i++)
        {
            printf("%02x ", m->io.tx_buf[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_suback(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这是输入一串服务端发送来的suback报文来解析确认是suback报文然后再input里面调用了onrx,onrx再调用了回调函数，回调函数要自己去定义，这里只是打印对数据解析
    const char* test_suback_hex = "90 03 00 01 00";
    uint8_t hex_buff[64];
    int hex_len = Str_to_Hex((char*)test_suback_hex, hex_buff);
    
    res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)hex_len);
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to process SUBACK message res: %d\r\n", res);
    }else {
        printf("SUBACK message processed successfully\r\n");
    }
    printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_unsubscribe(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这是一个取消订阅的示例，展示了如何使用 MQTT_pack_unsubscribe 函数来构建取消订阅报文
    res = mqtt_pack_unsubscribe(m, m->io.tx_buf, BUFF_SIZE, "TEST");
    if(res < 0) {
        #ifdef MQTT_DEMO_DEBUG
        printf("Failed to pack MQTT UNSUBSCRIBE message res: %d\r\n", res);
        #endif // DEBUG
        return -1;
    }else {
        #ifdef MQTT_DEMO_DEBUG
        printf("MQTT UNSUBSCRIBE message packed successfully, length: %d\r\n", res);
        for(i = 0; i < m->length.Totallength; i++)
        {
            printf("%02x ", m->io.tx_buf[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_publish(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这是一个发布消息的示例，展示了如何使用 mqtt_pack_publish_two 函数来构建发布报文，并通过回调函数处理服务器的响应
    mqtt_publish_params_t params = {
                    .topic = "TEST",
                    .payload = "Hello MQTT",
                    .payload_len = strlen("Hello MQTT"),
                    .qos = 1,
                    .retain = 0,
                    .dup = 0
                };
    res = mqtt_pack_publish_two(m, m->io.tx_buf, BUFF_SIZE, &params);
    if(res < 0) {
        #ifdef MQTT_DEMO_DEBUG
        printf("Failed to pack MQTT PUBLISH message res: %d\r\n", res);
        #endif // DEBUG
        return -1;
    }else {
        #ifdef MQTT_DEMO_DEBUG
        printf("MQTT PUBLISH message packed successfully, length: %d\r\n", res);
        for(i = 0; i < m->length.Totallength; i++)
        {
            printf("%02x ", m->io.tx_buf[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_pack_puback_myself(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这个要紧接着在publish后面，这还是对自己发送出去的publish报文的解析
    #ifdef MQTT_DEMO_DEBUG
    printf("Start parse myself publish message:\r\n");
    #endif // DEBUG
    res = MQTT_InputBytes(m, m->io.tx_buf, m->length.Totallength);
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to parse myself PUBLISH message res: %d\r\n", res);
    }else {
        printf("Myself PUBLISH message parsed successfully\r\n");
    }
	printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}


int app_demo_pack_puback(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这是输入一串服务端发送来的publish报文来解析确认是pubulish报文然后再input里面调用了onrx,onrx再调用了回调函数，回调函数要自己去定义，这里只是打印对数据解析
    const char* test_publish_hex = "32 0E 00 04 54 45 53 54 00 01 35 32 31 31 32 33";
	uint8_t hex_buff[64];
	int hex_len = Str_to_Hex((char*)test_publish_hex, hex_buff);
	
	res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)3);                                           //这里故意把一帧完整的数据拆分成俩份展示分片输入的效果
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to parse PUBLISH message res: %d\r\n", res);
    }else {
        printf("PUBLISH message parsed successfully\r\n");
    }
	printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
	res = MQTT_InputBytes(m, (const uint8_t*)(hex_buff + 3), (uint16_t)(hex_len - 3));
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to parse PUBLISH message res: %d\r\n", res);
    }else {
        printf("PUBLISH message parsed successfully\r\n");
    }
	printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_disconnect(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    //这是一个断开连接的示例，展示了如何使用 MQTT_DISCONNECT 函数来构建断开连接报文，并通过回调函数处理服务器的响应
    return 0;
}

int app_demo_ping(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    mqtt_pack_pingreq(m);
    #ifdef MQTT_DEMO_DEBUG
    printf("MQTT PINGREQ message packed successfully!\r\n");
    for(i = 0; i < m->length.Totallength; i++)
    {
        printf("%02x ", m->io.tx_buf[i]);
    }
    printf("\r\n");
    #endif // DEBUG
    return 0;
}
int app_demo_pingresp(MQTT_TCB* m)
{   
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    const char* pingresp_hex = "D0 00";
    uint8_t hex_buff[8];
    int hex_len = Str_to_Hex((char*)pingresp_hex, hex_buff);
    
    res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)hex_len);
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0) {
        printf("Failed to parse MQTT PINGRESP message res: %d\r\n", res);
    }else {
        printf("MQTT PINGRESP message parsed successfully!\r\n");
    }
    printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_parse_puback(MQTT_TCB* m)
{
    printf("--------------------------------------------------------------\r\n");
    printf("--------------------------------------------------------------\r\n");
    mqtt_pack_puback(m, 1);
    res = MQTT_InputBytes(m, m->io.tx_buf, (uint16_t)m->length.Totallength);
    #ifdef MQTT_DEMO_DEBUG
    if(res < 0)
    {
        printf("Failed to parse MQTT PUBACK message res: %d\r\n", res);
    }
    else
    {
        printf("MQTT PUBACK message parsed successfully!\r\n");
    }
    printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}

/*---------------------HELPERS-------------------------*/
int feed_data(MQTT_TCB* m, const char* hex_str)
{
    uint8_t hex_buff[MQTT_RXBUF_SIZE];
    int hex_len = Str_to_Hex((char*)hex_str, hex_buff);
    int res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)hex_len);
    #ifdef MQTT_DEMO_DEBUG
    printf("data fed: %s (%d)\r\n", MQTT_RxEventStr(m->ses.last_event_code), m->ses.last_event_code);
    #endif // DEBUG
    return res;
}
