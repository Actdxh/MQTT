#ifndef __MQTT_CORE_H
#define __MQTT_CORE_H
#include <stdint.h>
#include "mqtt_type.h"



/*--------------------核心功能函数-------------------------*/
/*--------------------对外api-------------------------*/
int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config);
int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len);
int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len);

/*--------------------回调函数的注册函数-------------------------*/
void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx);
void MQTT_SetOnPublish(MQTT_TCB* m, mqtt_on_publish_cb cb, void* user_ctx);
void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx);
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx);
void MQTT_SetOnUnsuback(MQTT_TCB* m, mqtt_on_unsuback_cb cb, void* user_ctx);
void MQTT_SetOnPingresp(MQTT_TCB* m, mqtt_on_pingresp_cb cb, void* user_ctx);
void MQTT_SetOnPuback(MQTT_TCB* m, mqtt_on_puback_cb cb, void* user_ctx);

/*--------------------封装的回调函数-------------------------*/
void mqtt_emit_send(MQTT_TCB* m);





#endif // !__MQTT_CORE_H
