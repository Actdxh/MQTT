#ifndef __MQTT_PARSE_H
#define __MQTT_PARSE_H

#include <stdint.h>
#include "mqtt_type.h"


int mqtt_parse_publish_view(const uint8_t* rx, uint32_t rx_len, mqtt_publish_view_t* view);
int mqtt_parse_connack_view(const uint8_t* rx, uint32_t rx_len, mqtt_connack_view_t* view);
int mqtt_parse_suback_view(const uint8_t* rx, uint32_t rx_len, mqtt_suback_view_t* view);
int mqtt_parse_unsuback_view(const uint8_t* rx, uint32_t rx_len, mqtt_unsuback_view_t* view);
int mqtt_parse_puback_view(const uint8_t* rx, uint32_t rx_len, mqtt_puback_view_t* view);
int mqtt_parse_pingresp(const uint8_t* rx, uint32_t rx_len);


#endif // !__MQTT_PARSE_H
