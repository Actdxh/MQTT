#ifndef __MQTT_PACK_H
#define __MQTT_PACK_H
#include <stdint.h>
#include "mqtt_type.h"




int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive);
int mqtt_pack_subscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, const char * topic, char qos);
int mqtt_pack_unsubscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, char * topic);
int mqtt_pack_publish(MQTT_TCB* m,uint8_t* out,uint16_t out_size,const char* topic,const void* payload,uint16_t payload_len,uint8_t qos,uint8_t retain,uint8_t dup);
int mqtt_pack_publish_two(MQTT_TCB* m,uint8_t* out,uint16_t out_size, mqtt_publish_params_t *params);
void mqtt_pack_puback(MQTT_TCB* m, uint16_t messageid);
void mqtt_pack_pingreq(MQTT_TCB* m);

#endif // !__MQTT_PACK_H
