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

	res = MQTT_Init(&MqttA, &configA);
	if(res != 0) {
		printf("Failed to initialize MQTT Client A\r\n");
		return -1;
	}
	// MQTT_Init(&MqttB, &configB);

	printf("MQTT Client A:\r\n");
	connect_test(&MqttA);
	printf("\r\n");

	// connect_test(&MqttB);
	// printf("\r\n");

	printf("MQTT Subscriptions:\r\n");
	subscribe_test(&MqttA);
	printf("\r\n");

	MQTT_SetOnMessage(&MqttA, my_on_message, NULL);
	MQTT_SetOnSend(&MqttA, my_on_send, NULL);
	const char* test_publish_hex = "32 0E 00 04 54 45 53 54 00 01 35 32 31 31 32 33";
	uint8_t hex_buff[64];
	int hex_len = Str_to_Hex((char*)test_publish_hex, hex_buff);
	
	res = MQTT_InputBytes(&MqttA, hex_buff, 3);
	printf("MQTT_InputBytes result: %d\r\n", res);
	res = MQTT_InputBytes(&MqttA, hex_buff + 3, hex_len - 3);
	printf("MQTT_InputBytes result: %d\r\n", res);

	// printf("MQTT Publications:\r\n");
	// publish_test(&MqttA);
	// printf("\r\n");

	// printf("MQTT Unsubscriptions:\r\n");
	// unsubscribe_test(&MqttA);
	// printf("\r\n");
	
	// publish_pack_parse_test(&MqttA);
	



	printf("\r\n");
	return 0;
}






