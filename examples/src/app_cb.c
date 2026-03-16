#include "mqtt_type.h"
#include <stdio.h>


void my_on_publish(void* user_ctx, const mqtt_publish_view_t* msg)
{
	if(!msg) {
		return; // 无效的消息视图
	}
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
	//不判断user_ctx因为这是作为一个可选字段，以后业务层的时候自定义其他作用
	if(!data || len == 0) {
		return; // 无效的用户上下文
	}
	uint16_t i; 
    (void)user_ctx;

    printf("TX %u bytes: ", (unsigned)len);
    for (i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
};

void my_on_connack(void* user_ctx, const mqtt_connack_view_t* v)
{
	//空，后续补充
	//需要注意的是这个回调是在return之前，所以就算是错的也要考虑
	if(!user_ctx || !v) {
		return; // 无效的 CONNACK 视图
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	if (v->return_code == 0) {
		ctx->connected = MQTT_CONN_CONNECTED;
	} else {
		ctx->connected = MQTT_CONN_DISCONNECTED;
	}
}

void my_on_suback(void* user_ctx, const mqtt_suback_view_t* v)
{
	if(!user_ctx || !v) {
		return; // 无效的用户上下文或 SUBACK 视图
	}
	if(v->return_codes == NULL || v->return_codes_len == 0) {
		return; // SUBACK 中没有返回码，无法处理
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	if(v->return_codes[0] == 0x00 || v->return_codes[0] == 0x01 || v->return_codes[0] == 0x02) {
		ctx->subscribed = MQTT_SUBSCRIBED_ONE; // 标记为已订阅
	} else if(v->return_codes[0] == 0x80) {
		ctx->subscribed = MQTT_SUBSCRIBED_NONE; // 订阅失败
	}else {
		ctx->subscribed = MQTT_SUBSCRIBED_NONE; // 其他返回码也视为订阅失败
	}
}

void my_on_unsuback(void* user_ctx, const mqtt_unsuback_view_t* v)
{

}

void my_on_pingresp(void* user_ctx)
{
	if(!user_ctx) {
		return; // 无效的用户上下文
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	ctx->pingresp_seen = 1; // 标记已收到 PINGRESP
	//当进入这个回调的时候就代表服务器收到了ping,可以摘这里写某个值等于当前的tick，例如stm32的get_tick
	//然后再业务层就可以开一个定时器或者看门狗来根据这个值判断是否需要重启或者重启连接
	//只有当业务层检测到timeout的时候才进行其他操作
	//eg:ctx->last_pingresp_tick = get_tick();

}

void my_on_puback(void* user_ctx, const mqtt_puback_view_t* v)
{
	if(!user_ctx || !v) {
		return; // 无效的用户上下文或 PUBACK 视图
	}
	app_ctx_t* ctx = (app_ctx_t*)user_ctx;
	ctx->puback_pid = v->packet_id; // 记录收到的 PUBACK 的
}
