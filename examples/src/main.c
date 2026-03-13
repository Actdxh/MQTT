#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "MQTT.h"
#include "cc.h"
#include <string.h>
#include "test.h"
#include "mqtt_utils.h"
#include <stdint.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int i;
int res;
u8 databuff[64];
u8 outbuff[64];
u8 QoS;
u32 messageid;

MQTT_config_t configA = {
	.ClientID = "USER001",
	.UserName = "USER001",
	.Passward = "USER001",
	.WillEnable = 1,
	.WillTopic = "WILL001",
	.WillData = "WILL001",
	.WillRetain = 0,
	.WillQoS = 0,
	.CleanSession = 1
};

MQTT_config_t configB = {
	.ClientID = "USER002",
	.UserName = "USER002",
	.Passward = "USER002",
	.WillEnable = 0,
	.WillTopic = "WILL002",
	.WillData = "WILL002",
	.WillRetain = 0,
	.WillQoS = 0,
	.CleanSession = 1
};


MQTT_TCB MqttA;
MQTT_TCB MqttB;

int main(int argc, char *argv[]) {

	MQTT_Init(&MqttA, &configA);
	MQTT_Init(&MqttB, &configB);

	printf("MQTT Client A:\r\n");
	connect_test(&MqttA);
	printf("\r\n");

	// connect_test(&MqttB);
	// printf("\r\n");

	printf("MQTT Subscriptions:\r\n");
	subscribe_test(&MqttA);
	printf("\r\n");

	printf("MQTT Publications:\r\n");
	publish_test(&MqttA);
	printf("\r\n");

	printf("MQTT Unsubscriptions:\r\n");
	unsubscribe_test(&MqttA);
	printf("\r\n");
	
	
	printf("\r\n");
	return 0;
}






