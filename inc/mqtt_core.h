#ifndef __MQTT_CORE_H
#define __MQTT_CORE_H
#include <stdint.h>
#include "MQTT.h"




int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config);
int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len);
int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len);
void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx);
void MQTT_SetOnMessage(MQTT_TCB* m, mqtt_on_message_cb cb, void* user_ctx);
void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx);
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx);


void mqtt_emit_send(MQTT_TCB* m);
void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view);

#endif // !__MQTT_CORE_H
