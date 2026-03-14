#ifndef __MQTT_PACK_H
#define __MQTT_PACK_H
#include <stdint.h>

typedef struct MQTT_TCB MQTT_TCB;
typedef struct mqtt_publish_params_t mqtt_publish_params_t;

int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive);
int mqtt_pack_subscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, const char * topic, char qos);
int mqtt_pack_unsubscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, char * topic);
int mqtt_pack_publish_two(MQTT_TCB* m,uint8_t* out,uint16_t out_size, mqtt_publish_params_t *params);


#endif // !__MQTT_PACK_H
