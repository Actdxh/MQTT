#ifndef __MQTT_PARSE_H
#define __MQTT_PARSE_H

#include <stdint.h>

typedef struct {
    const uint8_t* topic;
    uint16_t topic_len;

    const uint8_t* payload;
    uint32_t payload_len;

    uint8_t qos;
    uint8_t dup;
    uint8_t retain;

    uint16_t packet_id; // qos>0 才有效，否则 0
    uint32_t packet_len;
} mqtt_publish_view_t;

typedef struct {
    uint8_t session_present;
    uint8_t return_code;   // 0=accepted
} mqtt_connack_view_t;

typedef struct {
    uint16_t packet_id;

    const uint8_t* return_codes;
    uint8_t return_codes_len;    // 至少 1

    uint32_t packet_len;         // 这帧总长（方便调试）
} mqtt_suback_view_t;

int mqtt_parse_publish_view(const uint8_t* rx, uint32_t rx_len, mqtt_publish_view_t* view);
int mqtt_parse_connack_view(const uint8_t* rx, uint32_t rx_len, mqtt_connack_view_t* view);
int mqtt_parse_suback_view(const uint8_t* rx, uint32_t rx_len, mqtt_suback_view_t* view);




#endif // !__MQTT_PARSE_H
