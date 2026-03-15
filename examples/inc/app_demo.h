#ifndef __APP_DEMO_H
#define __APP_DEMO_H

#include <stdint.h>
#include "mqtt_type.h"


/*--------------------回调函数-------------------------*/
void my_on_connack(void* user_ctx, const mqtt_connack_view_t* v);
void my_on_message(void* user_ctx, const mqtt_publish_view_t* msg);
void my_on_send(void* user_ctx, const uint8_t* data, uint16_t len);
void my_on_suback(void* user_ctx, const mqtt_suback_view_t* v);
void my_on_pingresp(void* user_ctx, uint8_t state);
void my_on_puback(void* user_ctx, const mqtt_puback_view_t* v);
/*--------------------调试打印功能函数-------------------------*/
const char* MQTT_RxEventStr(int code);

void app_demo(void);
int app_demo_init(MQTT_TCB* m);
int app_demo_connect(MQTT_TCB* m);
int app_demo_conack(MQTT_TCB* m);
int app_demo_subscribe(MQTT_TCB* m);
int app_demo_suback(MQTT_TCB* m);
int app_demo_unsubscribe(MQTT_TCB* m);
int app_demo_publish(MQTT_TCB* m);
int app_demo_pack_puback_myself(MQTT_TCB* m); // 解析自己发送的 PUBLISH 报文，触发回调函数
int app_demo_pack_puback(MQTT_TCB* m);
int app_demo_disconnect(MQTT_TCB* m);
int app_demo_ping(MQTT_TCB* m);
int app_demo_pingresp(MQTT_TCB* m);
int app_demo_parse_puback(MQTT_TCB* m);


#endif // __APP_DEMO_H