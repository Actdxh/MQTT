#ifndef __APP_DEMO_H
#define __APP_DEMO_H

#include <stdint.h>
#include "MQTT.h"


void my_on_connack(void* user_ctx, const mqtt_connack_view_t* v);
void my_on_message(void* user_ctx, const mqtt_publish_view_t* msg);
void my_on_send(void* user_ctx, const uint8_t* data, uint16_t len);
void my_on_suback(void* user_ctx, const mqtt_suback_view_t* v);


const char* MQTT_RxEventStr(int code);


#endif // __APP_DEMO_H