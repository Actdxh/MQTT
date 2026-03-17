#ifndef __MQTT_CORE_H
#define __MQTT_CORE_H
#include <stdint.h>
#include "mqtt_type.h"

#define MQTT_PING_RETRY_DEMO_BITMASK    			(1 << 0) // 用于标记 Mqtt_PingProcess 处理的事件，方便调试
#define MQTT_PUBACK_RETRY_DEMO_BITMASK 		(1 << 1) // 用于标记 Mqtt_puback_retry_process 处理的事件，方便调试

/*--------------------核心功能函数-------------------------*/
/*--------------------对外api-------------------------*/
int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config);
int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len);
int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len);
int mqtt_now_ms(MQTT_TCB* m);
int MQTT_Process(MQTT_TCB* m);
int Mqtt_puback_retry_process(MQTT_TCB* m);
int Mqtt_PingProcess(MQTT_TCB* m);
/*--------------------回调函数的注册函数-------------------------*/
void MQTT_SetAllOnCb_same(MQTT_TCB* m, const MQTT_Callbacks *cb);
void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx);
void MQTT_SetOnPublish(MQTT_TCB* m, mqtt_on_publish_cb cb, void* user_ctx);
void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx);
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx);
void MQTT_SetOnUnsuback(MQTT_TCB* m, mqtt_on_unsuback_cb cb, void* user_ctx);
void MQTT_SetOnPingresp(MQTT_TCB* m, mqtt_on_pingresp_cb cb, void* user_ctx);
void MQTT_SetOnPuback(MQTT_TCB* m, mqtt_on_puback_cb cb, void* user_ctx);

void MQTT_SetNowMs(MQTT_TCB* m, mqtt_now_ms_fn now_ms, void* user_ctx);
void Mqtt_SetKeepalive(MQTT_TCB* m, uint16_t keepalive_ms, uint16_t ping_timeout_ms);
/*--------------------封装的功能函数-------------------------*/
void mqtt_emit_send_buf(MQTT_TCB* m, const uint8_t* data, uint16_t len);
void mqtt_emit_send(MQTT_TCB* m);





#endif // !__MQTT_CORE_H
