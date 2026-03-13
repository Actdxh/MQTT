#ifndef __TEST_H
#define __TEST_H
#include "main.h"
#include "MQTT.h"

void data_test(MQTT_TCB *m);
void connect_test(MQTT_TCB *m);
void connectack_test(MQTT_TCB *m);
void disconnect_test(MQTT_TCB *m);
void subscribe_test(MQTT_TCB *m);
void suback_test(MQTT_TCB *m);
void unsubscribe_test(MQTT_TCB *m);
void unsuback_test(MQTT_TCB *m);
void ping(MQTT_TCB *m);
void pingresp_test(MQTT_TCB *m);
void publish0_test(MQTT_TCB *m);
void publish_test(MQTT_TCB *m);
void processpublish_test(MQTT_TCB *m);
void publishack_test(MQTT_TCB *m);
void processpublishack(MQTT_TCB *m);
void pubrec_test(MQTT_TCB *m);
void processpubrec(MQTT_TCB *m);
void pubrel_test(MQTT_TCB *m);
void pubcomp_test(MQTT_TCB *m);
void processpubcomp(MQTT_TCB *m);



#endif

