#include "mqtt_type.h"
#include "app_ctx.h"
#include <stdio.h>
#include <string.h> 




inline void app_ctx_init(app_evt_queue_t* ctx)
{
	if(ctx) {
		memset(ctx, 0, sizeof(*ctx));
	}
}

inline int app_evt_push(app_evt_queue_t* ctx, app_evt_t e)
{
	if(!ctx) {
		return -1; // Invalid event queue context
	}
	uint8_t next_head = (ctx->head + 1) % APP_EVT_Q_SIZE;
	if(next_head == ctx->tail) {
		return 0; // 队列已满，无法推入新事件，这个head回卷了
	}
	ctx->queue[ctx->head] = e;
	ctx->head = next_head;
	return 1; // Event pushed successfully
}

inline int app_evt_pop(app_evt_queue_t* ctx, app_evt_t* e)
{
	if(!ctx || !e) {
		return -1; // Invalid event queue context or output parameter
	}
	if(ctx->head == ctx->tail) {
		return 0; // Event queue is empty
	}
	*e = ctx->queue[ctx->tail];
	ctx->tail = (ctx->tail + 1) % APP_EVT_Q_SIZE;
	return 1; // Event popped successfully
}

void my_on_publish(void* user_ctx, const mqtt_publish_view_t* msg)
{
	if(!msg) {
		return; // 无效的消息视图
	}
	if(user_ctx) {
		app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
		app_evt_t e = {0};
		e.type = APP_EVT_PUBLISH_RX;
		app_evt_push(ctx, e);
	}
	// 这里可以添加其他处理接收到的 PUBLISH 消息的逻辑，例如打印消息内容等
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
	//需要注意的是这个回调是在return之前，所以就算是错的也要考虑
	if(!user_ctx || !v) {
		return; // 无效的 CONNACK 视图
	}
	app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
	app_evt_t e = {0};
	if (v->return_code == 0) {
		e.type = APP_EVT_CONNACK_OK;
	} else {
		e.type = APP_EVT_CONNACK_FAIL;
	}
	app_evt_push(ctx, e);
}

void my_on_suback(void* user_ctx, const mqtt_suback_view_t* v)
{
	if(!user_ctx || !v) {
		return; // 无效的用户上下文或 SUBACK 视图
	}
	if(v->return_codes == NULL || v->return_codes_len == 0) {
		return; // SUBACK 中没有返回码，无法处理
	}
	app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
	app_evt_t e = {0};
	if(v->return_codes[0] == 0x00 || v->return_codes[0] == 0x01 || v->return_codes[0] == 0x02) {
		e.type = APP_EVT_SUBACK_OK;
	} else if(v->return_codes[0] == 0x80) {
		e.type = APP_EVT_SUBACK_FAIL;
	}else {
		e.type = APP_EVT_SUBACK_FAIL; // 其他返回码也视为订阅失败
	}
	e.pid = v->packet_id; // 可以将 Packet ID 传递给业务层，以便进行更精细的事件处理
	app_evt_push(ctx, e);
}

void my_on_unsuback(void* user_ctx, const mqtt_unsuback_view_t* v)
{
	if(!user_ctx || !v) {
		return; // 无效的用户上下文或 UNSUBACK 视图
	}
	app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
	app_evt_t e = {0};
	// UNSUBACK 只有一个返回码，就是成功与否，所以只要能解析出包 ID 就认为是成功的
	if(v->packet_id != 0) {
		e.type = APP_EVT_UNSUBACK_OK;
	} else {
		e.type = APP_EVT_UNSUBACK_FAIL;
	}
	e.pid = v->packet_id; // 将 Packet ID 传递给业务层，以便进行更精细的事件处理
	app_evt_push(ctx, e);
}

void my_on_pingresp(void* user_ctx)
{
	if(!user_ctx) {
		return; // 无效的用户上下文
	}
	app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
	app_evt_t e = {0};
	e.type = APP_EVT_PINGRESP;
	app_evt_push(ctx, e);

	// 标记已收到 PINGRESP
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
	
	app_evt_queue_t* ctx = (app_evt_queue_t*)user_ctx;
	app_evt_t e = {0};
	e.type = APP_EVT_PUBACK_OK;
	e.pid = v->packet_id; // 将 Packet ID 传递给业务层，以便进行更精细的事件处理
	app_evt_push(ctx, e);
}


