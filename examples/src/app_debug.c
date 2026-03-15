#include "app_demo.h"


const char* MQTT_RxEventStr(int code)
{
	switch(code) {
		case MQTT_RX_CONNACK:
			return "Received CONNACK";
		case MQTT_RX_PUBLISH_QOS0:
			return "Received PUBLISH QoS 0";
		case MQTT_RX_PUBLISH_QOS1:
			return "Received PUBLISH QoS 1 but not yet sent PUBACK";
		case MQTT_RX_PUBLISH_QOS1_ACKED:
			return "Received PUBLISH QoS 1 and sent PUBACK";
		case MQTT_RX_PUBLISH_QOS2_UNSUPPORTED:
			return "Received PUBLISH QoS 2 (unsupported)";
		case MQTT_RX_SUBACK:
			return "Received SUBACK";
		case MQTT_RX_PINGRESP:
			return "Received PINGRESP";
		case MQTT_RX_PUBACK:
			return "Received PUBACK";
		default:
			return "Unknown event code";
	}
}