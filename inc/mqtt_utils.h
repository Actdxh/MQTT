#ifndef __MQTT_UTILS_H
#define __MQTT_UTILS_H
#include <stdint.h>

int Str_to_Hex(char* indata, uint8_t* outdata);
uint8_t mqtt_write_rem_len(uint8_t* out, uint32_t rem_len);
int mqtt_write_str(uint8_t* out, uint16_t out_size, uint16_t *p, const char* s);


#endif // !__MQTT_UTILS_H
