#ifndef __MQTT_CORE_H
#define __MQTT_CORE_H
#include <stdint.h>
#include "MQTT.h"
void mqtt_emit_send(MQTT_TCB* m);
void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view);

#endif // !__MQTT_CORE_H
