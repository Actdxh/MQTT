#include <stdio.h>
#include <string.h>
#include "mqtt_core.h"
#include "mqtt_handlers.h"
#include "mqtt_pack.h"
#include "mqtt_utils.h"
#include "mqtt_parse.h"




int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config)
{
	if(m == NULL || config == NULL) {
		return -1; // Invalid input
	}

	if(config->WillQoS > 2) {
		return -1; // Invalid Will QoS
	}

	if(config->ClientID[0] == '\0') {
		return -1; // ClientID, UserName, and Passward cannot be empty
	}
	if(config->WillEnable) {
		if(config->WillTopic[0] == '\0' || config->WillData[0] == '\0') {
			return -1; // Will topic and data cannot be empty when Will is enabled
		}
	}
	memset(m, 0, sizeof(*m));
	m->param = *config;

	m->length.cid_len = m->param.ClientID ? strlen(m->param.ClientID) : 0;
	m->length.user_len = m->param.UserName ? strlen(m->param.UserName) : 0;
	m->length.pwd_len = m->param.Passward ? strlen(m->param.Passward) : 0;
	if(m->param.WillEnable) {
		m->length.willtopic_len = m->param.WillTopic ? strlen(m->param.WillTopic) : 0;
		m->length.willdata_len = m->param.WillData ? strlen(m->param.WillData) : 0;
	} else {
		m->length.willtopic_len = 0;
		m->length.willdata_len = 0;
	}
	//m->length.topic_len = strlen(m->topic);
	m->MessageID = 1;
	m->rx_buf_len = 0;

	return 0;
}

int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len)
{
	if (!m || !rx_data || rx_len < 2) return -1;
	uint8_t type = rx_data[0] & 0xF0;
	switch(type) {
		case 0x20: {
			return mqtt_handle_connack(m, rx_data, rx_len); // CONNACK
		}
		case 0x30: { 
			return mqtt_handle_publish(m, rx_data, rx_len); // PUBLISH
		}
		case 0x40:{
			return mqtt_handle_puback(m, rx_data, rx_len); // PUBACK
		}
		case 0x90: {
			return mqtt_handle_suback(m, rx_data, rx_len); // SUBACK
		}
		case 0xD0: {
			return mqtt_handle_pingresp(m, rx_data, rx_len); // PINGRESP
		}
		// 这里可以添加对其他类型报文的处理，比如CONNACK、SUBACK等
		default:
			return 0; // Unhandled packet type
	}
}

int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len)
{
	if(m == NULL || data == NULL || len == 0) {
	return -1; // Invalid input
	}
	if(len > sizeof(m->rx_buf)) {// 如果一次接收的数据长度超过缓冲区大小，直接丢弃并返回错误
	return -1; 
	}

	int res = 0;
	int frames = 0;
	uint8_t rem_len_bytes;
	uint32_t rem_len;
	uint32_t frame_len;


	if(len > sizeof(m->rx_buf) - m->rx_buf_len) {// 检查剩余缓冲区大小是否足够不够就清空缓冲区
		m->rx_buf_len = 0;
	}
	memcpy(m->rx_buf+m->rx_buf_len, data, len);
	m->rx_buf_len += len;
	while(m->rx_buf_len >= 2)
	{
		res = mqtt_read_rem_len(m->rx_buf + 1, m->rx_buf_len - 1, &rem_len, &rem_len_bytes);
		if(res == -2)
		{
			break; // 接收的包不完整，继续等待
		}else if(res < 0)
		{
			memmove(m->rx_buf, m->rx_buf + 1, m->rx_buf_len - 1); // 移除第一个字节，继续尝试解析下一个包
			m->rx_buf_len -= 1;
			continue;
		}
		frame_len = 1 + rem_len_bytes + rem_len;
		if(frame_len > m->rx_buf_len) {
			break; // 接收的包不完整，继续等待
		}
		// 处理完整的 MQTT 包
		res = MQTT_OnRx(m, m->rx_buf, frame_len);
		m->last_event_code = res; // 记录上次事件代码，方便调试
		#ifdef MQTT_DEBUG
		printf("OnRx:return :%d\n", res);
		#endif
		// 移除已处理的包
		if(frame_len == m->rx_buf_len) {
			m->rx_buf_len = 0; // 刚好处理完所有数据，直接清空缓冲区
			frames++;
		} else if(frame_len < m->rx_buf_len) {
			memmove(m->rx_buf, m->rx_buf + frame_len, m->rx_buf_len - frame_len);//这个函数是从buf里面往后面移一帧的数据放到前面来第三个参数要减的原因就是移动减去帧长的长度
			m->rx_buf_len -= frame_len;
			frames++;
		}
	}
	return frames; // 返回处理的帧数
}


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

void MQTT_SetOnPingresp(MQTT_TCB* m, mqtt_on_pingresp_cb cb, void* user_ctx)
{
	if(!cb || !m) {
		return; // Invalid callback
	}
	m->on_pingresp = cb;
	m->user_ctx = user_ctx;
}

void MQTT_SetOnPuback(MQTT_TCB* m, mqtt_on_puback_cb cb, void* user_ctx)
{
	if(!cb || !m) {
		return; // Invalid callback
	}
	m->on_puback = cb;
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

