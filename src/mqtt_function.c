#include "mqtt_function.h"
#include "mqtt_type.h"


void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx)
{
	m->on_connack = cb;
	m->user_ctx = user_ctx;
}


void MQTT_SetOnMessage(MQTT_TCB* m, mqtt_on_message_cb cb, void* user_ctx)
{
	if((cb == NULL) || (m == NULL)){
		return; // Invalid callback
	}
	m->on_message = cb;
	m->user_ctx = user_ctx;
}

void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx)
{
	if((cb == NULL) || (m == NULL)) {
		return; // Invalid callback
	}
	m->on_send = cb;
	m->user_ctx = user_ctx;
}
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx)
{
	if(!cb || !m) {
		return; // Invalid callback
	}
	m->on_suback = cb;
	m->user_ctx = user_ctx;
}

void mqtt_emit_send(MQTT_TCB* m)
{
    if (m->on_send && m->length.Totallength > 0) {
        m->on_send(m->user_ctx, m->buff, (uint16_t)m->length.Totallength);
    }
}

void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view)
{
    if (m->on_message) {
        m->on_message(m->user_ctx, view);
    }
}