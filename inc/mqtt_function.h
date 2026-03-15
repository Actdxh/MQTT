#ifndef __MQTT_FUNCTION_H
#define __MQTT_FUNCTION_H
#include <stdint.h>
#include "mqtt_type.h"


/*--------------------回调函数的注册函数-------------------------*/
void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx);
void MQTT_SetOnMessage(MQTT_TCB* m, mqtt_on_message_cb cb, void* user_ctx);
void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx);
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx);

/*--------------------封装的回调函数-------------------------*/
void mqtt_emit_send(MQTT_TCB* m);
void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view);

#endif // !__MQTT_FUNCTION_H