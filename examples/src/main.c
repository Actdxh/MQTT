#include <stdio.h>
#include "app_demo.h"
#include "main.h"
#include"mqtt_core.h"
#include "mqtt_pack.h"
#include "mqtt_parse.h"


/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int i;
int res;
uint8_t databuff[64];
uint8_t outbuff[64];
uint8_t QoS;
uint32_t messageid;

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


	// printf("MQTT Client A:\r\n");
	// connect_test(&MqttA);
	// printf("\r\n");

	// connect_test(&MqttB);
	// printf("\r\n");

	// printf("MQTT Subscriptions:\r\n");
	// subscribe_test(&MqttA);
	// printf("\r\n");

	MQTT_SetOnMessage(&MqttA, my_on_message, NULL);
	MQTT_SetOnSend(&MqttA, my_on_send, NULL);
	const char* test_publish_hex = "32 0E 00 04 54 45 53 54 00 01 35 32 31 31 32 33";
	uint8_t hex_buff[64];
	int hex_len = Str_to_Hex((char*)test_publish_hex, hex_buff);
	
	res = MQTT_InputBytes(&MqttA, hex_buff, 3);
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(MqttA.last_event_code), MqttA.last_event_code);
	res = MQTT_InputBytes(&MqttA, hex_buff + 3, hex_len - 3);
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(MqttA.last_event_code), MqttA.last_event_code);

	const char* connack_hex = "20 02 00 00";
	uint8_t b[8];
	int n = Str_to_Hex((char*)connack_hex, b);
	res = MQTT_InputBytes(&MqttA, b, n);
	printf("MQTT_InputBytes result=%d\n", res);
	printf("OnRx: %s (%d)\n", MQTT_RxEventStr(MqttA.last_event_code), MqttA.last_event_code);

/*
验证pubulish_pack和mqtt_parse_publish_view的正确性
	printf("MQTT Publications:\r\n");
	publish_test(&MqttA);
	printf("\r\n");
*/
/*
验证unsubscribe功能
	printf("MQTT Unsubscriptions:\r\n");
	unsubscribe_test(&MqttA);
	printf("\r\n");
*/
	
	// publish_pack_parse_test(&MqttA);
	



	printf("\r\n");
	return 0;
}






