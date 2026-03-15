#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H
#include <stdint.h>
#include "mqtt_type.h"
int mqtt_handle_publish(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len);
int mqtt_handle_connack(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len);
int mqtt_handle_suback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len);
int mqtt_handle_pingresp(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len);
int mqtt_handle_puback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len);

#endif // !MQTT_HANDLER_H
