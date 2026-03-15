#include "app_demo.h"
#include "mqtt_core.h"
#include "mqtt_pack.h"
#include "mqtt_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define MQTT_DEMO_DEBUG

int i; 
int res;
uint8_t databuf[128];
uint16_t datalen;

void app_demo(void)
{
    MQTT_TCB MqttA;
    //这是一个完整的示例，从初始化到连接服务器，再到订阅主题，最后发布消息，最后断开连接
    app_demo_init(&MqttA);
    MQTT_SetOnMessage(&MqttA, my_on_message, NULL);
	MQTT_SetOnSend(&MqttA, my_on_send, NULL);
    app_demo_connect(&MqttA);
    app_demo_conack(&MqttA);
    app_demo_subscribe(&MqttA);
    app_demo_suback(&MqttA);
    app_demo_unsubscribe(&MqttA);
    app_demo_publish(&MqttA);
    app_demo_puback_myself(&MqttA); // 解析自己发送的 PUBLISH 报文，触发回调函数
    app_demo_puback(&MqttA);
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
    printf("MQTT Client A Connect:\r\n");
	res = mqtt_pack_connect(m, m->buff,BUFF_SIZE, 10000);
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
            printf("%02x ",m->buff[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
	printf("\r\n");

}

int app_demo_conack(MQTT_TCB* m)
{
	const char* connack_hex = "20 02 00 00";
	uint8_t b[8];
	int n = Str_to_Hex((char*)connack_hex, b);
	res = MQTT_InputBytes(m, (const uint8_t*)b, (uint16_t)n);
    #ifdef MQTT_DEMO_DEBUG
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->last_event_code), m->last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_subscribe(MQTT_TCB* m)
{
    //这是一个订阅主题的示例，展示了如何使用 MQTT_pack_subscribe 函数来构建订阅报文，并通过回调函数处理服务器的响应
    res = mqtt_pack_subscribe(m, m->buff, BUFF_SIZE, "TEST", 1);
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
            printf("%02x ",m->buff[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_suback(MQTT_TCB* m)
{
    //这是输入一串服务端发送来的suback报文来解析确认是suback报文然后再input里面调用了onrx,onrx再调用了回调函数，回调函数要自己去定义，这里只是打印对数据解析
    const char* test_suback_hex = "90 03 00 01 00";
    uint8_t hex_buff[64];
    int hex_len = Str_to_Hex((char*)test_suback_hex, hex_buff);
    
    res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)hex_len);
    #ifdef MQTT_DEMO_DEBUG
    printf("MQTT_InputBytes result=%d\n", res);
    printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->last_event_code), m->last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_unsubscribe(MQTT_TCB* m)
{
    //这是一个取消订阅的示例，展示了如何使用 MQTT_pack_unsubscribe 函数来构建取消订阅报文
    res = mqtt_pack_unsubscribe(m, m->buff, BUFF_SIZE, "TEST");
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
            printf("%02x ",m->buff[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_publish(MQTT_TCB* m)
{
    //这是一个发布消息的示例，展示了如何使用 mqtt_pack_publish_two 函数来构建发布报文，并通过回调函数处理服务器的响应
    mqtt_publish_params_t params = {
        .topic = "TEST",
        .payload = "Hello MQTT",
        .payload_len = strlen("Hello MQTT"),
        .qos = 1,
        .retain = 0,
        .dup = 0
    };
    res = mqtt_pack_publish_two(m, m->buff, BUFF_SIZE, &params);
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
            printf("%02x ",m->buff[i]);
        }
        printf("\r\n");
        #endif // DEBUG
    }
    return res;
}

int app_demo_puback_myself(MQTT_TCB* m)
{
    //这个要紧接着在publish后面，这还是对自己发送出去的publish报文的解析
    #ifdef MQTT_DEMO_DEBUG
    printf("Start parse myself publish message:\r\n");
    #endif // DEBUG
	res = MQTT_InputBytes(m, (const uint8_t*)m->buff, m->length.Totallength);
    #ifdef MQTT_DEMO_DEBUG
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->last_event_code), m->last_event_code);
    #endif // DEBUG
    return res;
}


int app_demo_puback(MQTT_TCB* m)
{
    //这是输入一串服务端发送来的publish报文来解析确认是pubulish报文然后再input里面调用了onrx,onrx再调用了回调函数，回调函数要自己去定义，这里只是打印对数据解析
    const char* test_publish_hex = "32 0E 00 04 54 45 53 54 00 01 35 32 31 31 32 33";
	uint8_t hex_buff[64];
	int hex_len = Str_to_Hex((char*)test_publish_hex, hex_buff);
	
	res = MQTT_InputBytes(m, (const uint8_t*)hex_buff, (uint16_t)3);                                           //这里故意把一帧完整的数据拆分成俩份展示分片输入的效果
    #ifdef MQTT_DEMO_DEBUG
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->last_event_code), m->last_event_code);
    #endif // DEBUG
	res = MQTT_InputBytes(m, (const uint8_t*)(hex_buff + 3), (uint16_t)(hex_len - 3));
    #ifdef MQTT_DEMO_DEBUG
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(m->last_event_code), m->last_event_code);
    #endif // DEBUG
    return res;
}

int app_demo_disconnect(MQTT_TCB* m)
{
    //这是一个断开连接的示例，展示了如何使用 MQTT_DISCONNECT 函数来构建断开连接报文，并通过回调函数处理服务器的响应
    return 0;
}
