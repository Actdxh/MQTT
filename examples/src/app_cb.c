#include "mqtt_type.h"
#include <stdio.h>


void my_on_connack(void* user_ctx, const mqtt_connack_view_t* v)
{
	//空，后续补充
	//需要注意的是这个回调是在return之前，所以就算是错的也要考虑

	if(v == NULL) {
		return; // 无效的 CONNACK 视图
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	if (v->return_code == 0) {
		ctx->connected = MQTT_CONN_CONNECTED;
	} else {
		ctx->connected = MQTT_CONN_DISCONNECTED;
	}
}

void my_on_message(void* user_ctx, const mqtt_publish_view_t* msg)
{
	uint32_t i;
	printf("Received message:\n");
	printf("Topic Length: %d\n", msg->topic_len);
	printf("Topic: %.*s\n", msg->topic_len, msg->topic);
	printf("Payload Length: %d\n", msg->payload_len);
	
	printf("Payload: ");
	const uint8_t* pl = (const uint8_t*)msg->payload;
	for(i = 0; i < msg->payload_len; i++) {
		if(pl[i] >= 32 && pl[i] <= 126) {
			printf("%c", pl[i]);
		} else {
			printf("."); // 非法/不可见字符显示为点
		}
	}
	printf("\n");
	printf("QoS: %d\n", msg->qos);
	printf("Retain: %d\n", msg->retain);
	printf("DUP: %d\n", msg->dup);
	printf("Packet ID: %d\n", msg->packet_id);
};

void my_on_send(void* user_ctx, const uint8_t* data, uint16_t len)
{
	uint16_t i; 
    (void)user_ctx;

    printf("TX %u bytes: ", (unsigned)len);
    for (i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
};

void my_on_suback(void* user_ctx, const mqtt_suback_view_t* v)
{
	if(v->return_codes == NULL || v->return_codes_len == 0) {
		return; // SUBACK 中没有返回码，无法处理
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	ctx->subscribed = MQTT_SUBSCRIBED_ONE; // 标记为已订阅
}
