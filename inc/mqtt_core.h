#ifndef __MQTT_CORE_H
#define __MQTT_CORE_H
#include <stdint.h>
#include "mqtt_type.h"



/*--------------------核心功能函数-------------------------*/
/*--------------------对外api-------------------------*/
int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config);
int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len);



/*--------------------内部api（不对外开放）-------------------------*/
int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len);


#endif // !__MQTT_CORE_H
