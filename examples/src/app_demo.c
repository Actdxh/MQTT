#include "app_demo.h"
#include "mqtt_core.h"

#define MQTT_DEMO_DEBUG


int res;


void app_demo(void)
{
    MQTT_TCB MqttA;
    //这是一个完整的示例，从初始化到连接服务器，再到订阅主题，最后发布消息，最后断开连接
    app_demo_init(&MqttA);
    app_demo_connect(&MqttA);
    app_demo_subscribe(&MqttA);
    app_demo_publish(&MqttA);
    app_demo_disconnect(&MqttA);
}

void app_demo_init(MQTT_TCB* m)
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

void app_demo_connect(MQTT_TCB* m)
{
    //这是一个连接服务器的示例，展示了如何使用 MQTT_pack_connect 函数来构建连接报文，并通过回调函数处理服务器的响应

    printf("MQTT Client A Connect:\r\n");
	
	printf("\r\n");

}

void app_demo_subscribe(MQTT_TCB* m)
{
    //这是一个订阅主题的示例，展示了如何使用 MQTT_pack_subscribe 函数来构建订阅报文，并通过回调函数处理服务器的响应
}

void app_demo_publish(MQTT_TCB* m)
{
    //这是一个发布消息的示例，展示了如何使用 mqtt_pack_publish_two 函数来构建发布报文，并通过回调函数处理服务器的响应
}

void app_demo_disconnect(MQTT_TCB* m)
{
    //这是一个断开连接的示例，展示了如何使用 MQTT_DISCONNECT 函数来构建断开连接报文，并通过回调函数处理服务器的响应
}