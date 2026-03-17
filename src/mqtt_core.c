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
	m->ses.next_pid = 1;
	m->io.rx_buf_len = 0;
	m->ses.tx_pending = 0;
	m->ses.puback_outstanding = 0;

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
		case 0xB0: {
			return mqtt_handle_unsuback(m, rx_data, rx_len); // UNSUBACK
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
	if(len > sizeof(m->io.rx_buf)) {// 如果一次接收的数据长度超过缓冲区大小，直接丢弃并返回错误
	return -1; 
	}

	int res = 0;
	int frames = 0;
	uint8_t rem_len_bytes;
	uint32_t rem_len;
	uint32_t frame_len;


	if(len > sizeof(m->io.rx_buf) - m->io.rx_buf_len) {// 检查剩余缓冲区大小是否足够不够就清空缓冲区
		m->io.rx_buf_len = 0;
	}
	memcpy(m->io.rx_buf + m->io.rx_buf_len, data, len);
	m->io.rx_buf_len += len;
	while(m->io.rx_buf_len >= 2)
	{
		res = mqtt_read_rem_len(m->io.rx_buf + 1, m->io.rx_buf_len - 1, &rem_len, &rem_len_bytes);
		if(res == -2)
		{
			break; // 接收的包不完整，继续等待
		}else if(res < 0)
		{
			memmove(m->io.rx_buf, m->io.rx_buf + 1, m->io.rx_buf_len - 1); // 移除第一个字节，继续尝试解析下一个包
			m->io.rx_buf_len -= 1;
			continue;
		}
		frame_len = 1 + rem_len_bytes + rem_len;
		if(frame_len > m->io.rx_buf_len) {
			break; // 接收的包不完整，继续等待
		}
		// 处理完整的 MQTT 包
		res = MQTT_OnRx(m, m->io.rx_buf, frame_len);
		m->ses.last_event_code = res; // 记录上次事件代码，方便调试
		#ifdef MQTT_DEBUG
		printf("OnRx:return :%d\n", res);
		#endif
		// 移除已处理的包
		if(frame_len == m->io.rx_buf_len) {
			m->io.rx_buf_len = 0; // 刚好处理完所有数据，直接清空缓冲区
			frames++;
		} else if(frame_len < m->io.rx_buf_len) {
			memmove(m->io.rx_buf, m->io.rx_buf + frame_len, m->io.rx_buf_len - frame_len);//这个函数是从buf里面往后面移一帧的数据放到前面来第三个参数要减的原因就是移动减去帧长的长度
			m->io.rx_buf_len -= frame_len;
			frames++;
		}
		if(res > 0) {
			m->ka.last_rx_ms = mqtt_now_ms(m); // 更新最后接收时间用于保活
		}
	}
	return frames; // 返回处理的帧数
}

int mqtt_now_ms(MQTT_TCB* m)
{
	if(m == NULL || m->platform.now_ms == NULL) {
		return -1; // Invalid MQTT control block or now_ms function not set
	}
	return m->platform.now_ms(m->platform.now_ms_ctx);
}

int MQTT_ReconnectReset(MQTT_TCB* m)
{
	if(m == NULL) {	
		return 	MQTT_ERR_ARG; // Invalid MQTT control block
	}
	m->ses.tx_pending = 0;
	m->ses.puback_outstanding = 0;
	m->ses.puback_pid = 0;
	m->ses.puback_sent_ms = 0;
	m->ses.puback_retry_count = 0;
	m->ses.puback_frame_len = 0;
	m->ka.last_tx_ms = 0;
	m->ka.last_rx_ms = 0;
	m->ka.last_pingreq_ms = 0;
	m->ka.ping_outstanding = 0;

	int now = mqtt_now_ms(m);
	if(now >= 0) {
		m->ka.last_tx_ms = now;
		m->ka.last_rx_ms = now;
	}
	m->io.rx_buf_len = 0; // 清空接收缓冲区，丢弃未处理的数据
	
	return 0; // Reconnect reset successful
}

int MQTT_Process(MQTT_TCB* m)
{
	if(m == NULL) {
        return MQTT_ERR_ARG;
    }

	int res = Mqtt_puback_retry_process(m);
    if(res < 0) {
        return res;
    }

    res = Mqtt_PingProcess(m);
    if(res < 0) {
        return res; // timeout/no_time/...
    }
	
    return 0;
}

int Mqtt_puback_retry_process(MQTT_TCB* m)
{
	if(m == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(m->ses.puback_outstanding == 1) {
		if(m->platform.now_ms == NULL) {
			return MQTT_ERR_NO_TIME; // No time function, cannot process PUBACK retry
		}
		uint32_t now = mqtt_now_ms(m);
		if((now - m->ses.puback_sent_ms >= m->param.puback_timeout_ms))
		{
			if(m->ses.puback_retry_count < 3) {
			m->io.publish_buf[0] |= 0x08; // 设置 DUP 标志重发 
			m->ses.tx_pending |= MQTT_PENDING_PUBLISH_QOS1;
			mqtt_emit_send(m); // 重发 PUBLISH 包
			m->ses.puback_sent_ms = now;
			m->ses.puback_retry_count++;
			return MQTT_PROCESS_PUBACK_RETRY; // 已重发 PUBACK
			} else if(m->ses.puback_retry_count >= 3) {
				m->ses.puback_outstanding = 0; // 超过重试次数后放弃，重置状态
				m->ses.puback_pid = 0;
				m->ses.puback_sent_ms = 0;
				m->ses.puback_retry_count = 0;
				m->ses.puback_frame_len = 0;
				return MQTT_ERR_NEED_RECONNECT; // PUBACK 重试超时
			}
		}
	}
	return MQTT_PROCESS_NONE;
}

int Mqtt_PingProcess(MQTT_TCB* m)
{
	if(m == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(m->ka.keepalive_ms == 0) {
		return MQTT_PROCESS_NONE;//不自动重连
	}
	if(m->platform.now_ms == NULL) {
		return MQTT_ERR_NO_TIME;// No time function, cannot process keepalive
	}
	uint32_t now = mqtt_now_ms(m);
	uint32_t last_activity_ms = m->ka.last_rx_ms > m->ka.last_tx_ms ? m->ka.last_rx_ms : m->ka.last_tx_ms;
	if(last_activity_ms == 0) {
		m->ka.last_rx_ms = now; // 如果从未有过活动，使用当前时间作为基准
		m->ka.last_tx_ms = now;
	}
	if((!m->ka.ping_outstanding) && (now - last_activity_ms >= m->ka.keepalive_ms)) {
		mqtt_pack_pingreq(m);
		mqtt_emit_send(m);
		m->ka.last_pingreq_ms = now;
		m->ka.ping_outstanding = 1;
		return MQTT_PROCESS_PING_SENT; // 已发送 Ping 请求
	}
	if((m->ka.ping_outstanding) && (now - m->ka.last_pingreq_ms >= m->ka.ping_timeout_ms)) {
		m->ka.ping_outstanding = 0; // 超时后重置 ping 状态，等待下一次发送
		return MQTT_ERR_NEED_RECONNECT; // Ping 请求超时
	}
	return	MQTT_PROCESS_NONE; 
}

void MQTT_SetAllOnCb_same(MQTT_TCB* m, const MQTT_Callbacks *cb)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	MQTT_SetOnConnack(m, cb->on_connack, cb->on_connack_ctx);
	MQTT_SetOnPublish(m, cb->on_publish, cb->on_publish_ctx);
	MQTT_SetOnSend(m, cb->on_send, cb->on_send_ctx);
	MQTT_SetOnSuback(m, cb->on_suback, cb->on_suback_ctx);
	MQTT_SetOnUnsuback(m, cb->on_unsuback, cb->on_unsuback_ctx);
	MQTT_SetOnPingresp(m, cb->on_pingresp, cb->on_pingresp_ctx);
	MQTT_SetOnPuback(m, cb->on_puback, cb->on_puback_ctx);
	
}


void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_connack = cb;
	m->callbacks.on_connack_ctx = user_ctx;
}


void MQTT_SetOnPublish(MQTT_TCB* m, mqtt_on_publish_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_publish = cb;
	m->callbacks.on_publish_ctx = user_ctx;
}

void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_send = cb;
	m->callbacks.on_send_ctx = user_ctx;
}
void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_suback = cb;
	m->callbacks.on_suback_ctx = user_ctx;
}

void MQTT_SetOnUnsuback(MQTT_TCB* m, mqtt_on_unsuback_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_unsuback = cb;
	m->callbacks.on_unsuback_ctx = user_ctx;
}

void MQTT_SetOnPingresp(MQTT_TCB* m, mqtt_on_pingresp_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_pingresp = cb;
	m->callbacks.on_pingresp_ctx = user_ctx;
}

void MQTT_SetOnPuback(MQTT_TCB* m, mqtt_on_puback_cb cb, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid callback
	}
	m->callbacks.on_puback = cb;
	m->callbacks.on_puback_ctx = user_ctx;
}

void MQTT_SetNowMs(MQTT_TCB* m, mqtt_now_ms_fn now_ms, void* user_ctx)
{
	if(m == NULL) {
		return; // Invalid MQTT control block
	}
	m->platform.now_ms = now_ms;
	m->platform.now_ms_ctx = user_ctx;
}

void Mqtt_SetKeepalive(MQTT_TCB* m, uint16_t keepalive_ms, uint16_t ping_timeout_ms)
{
	if(m == NULL) {
		return; // Invalid MQTT control block
	}
	m->ka.keepalive_ms = keepalive_ms;
	m->ka.ping_timeout_ms = ping_timeout_ms;
}

int mqtt_emit_send_buf(MQTT_TCB* m, const uint8_t* data, uint16_t len)
{
	if(!m ||!m->callbacks.on_send || !data || len == 0) {
		#ifdef MQTT_DEBUG
		printf("mqtt_emit_send_buf: Invalid input parameters\n");
		#endif
		return MQTT_ERR_ARG; // Invalid input
	}
	int rc = m->callbacks.on_send(m->callbacks.on_send_ctx, data, len);
	if(rc < 0) {
		#ifdef MQTT_DEBUG
		printf("mqtt_emit_send_buf: Send callback returned error code %d\n", rc);
		#endif
		return rc; // Send callback returned an error
	}
	if((uint16_t)rc != len) {
		return MQTT_ERR_SEND_INCOMPLETE; // Send callback did not send all data
	}
	uint32_t now = mqtt_now_ms(m);
	m->ka.last_tx_ms = now; // 更新最后发送时间用于保活

	if(m->ses.puback_outstanding && m->ses.puback_sent_ms > 0) {
		m->ses.puback_sent_ms = now; // 更新 PUBACK 发送时间用于重试
	}
	#ifdef MQTT_DEBUG
	if(rc < 0)
	{
		printf("mqtt_emit_send_buf: Send callback returned error code %d\n", rc);
	} else {
		printf("mqtt_emit_send_buf: Sent %d bytes successfully\n", rc);
	}
	#endif
	return rc; // Success
	
}

int mqtt_emit_send(MQTT_TCB* m)
{
	if(m == NULL) {
		// #ifdef MQTT_DEBUG
		// printf("mqtt_emit_send: Invalid MQTT control block\n");
		// #endif
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(m->ses.tx_pending & ((uint32_t) (0xfffffff &~MQTT_PENDING_PINGREQ))) {
		m->ses.tx_pending &= ~MQTT_PENDING_PINGREQ;
	}
	int rc;
	if(m->ses.tx_pending == 0) {

	}else if(m->ses.tx_pending & MQTT_PENDING_CONNECT) {
		rc = mqtt_emit_send_buf(m, m->io.connect_buf, m->length.pack_len.connect_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_CONNECT; // 发送后清除待发送标志
	}else if(m->ses.tx_pending & MQTT_PENDING_PUBACK) {
		rc = mqtt_emit_send_buf(m, m->io.puback_buf, m->length.pack_len.puback_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_PUBACK;
	}else if(m->ses.tx_pending & MQTT_PENDING_PUBLISH_QOS0) {
		rc = mqtt_emit_send_buf(m, m->io.publish_buf, m->length.pack_len.publish_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_PUBLISH_QOS0;
	}else if(m->ses.tx_pending & MQTT_PENDING_PUBLISH_QOS1) {
		rc = mqtt_emit_send_buf(m, m->io.publish_buf, m->length.pack_len.publish_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_PUBLISH_QOS1;
	}else if(m->ses.tx_pending & MQTT_PENDING_SUBSCRIBE) {
		rc = mqtt_emit_send_buf(m, m->io.subscribe_buf, m->length.pack_len.subscribe_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_SUBSCRIBE;
	}else if(m->ses.tx_pending & MQTT_PENDING_UNSUBSCRIBE) {
		rc = mqtt_emit_send_buf(m, m->io.unsubscribe_buf, m->length.pack_len.unsubscribe_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_UNSUBSCRIBE;
	}else if(m->ses.tx_pending & MQTT_PENDING_DISCONNECT) {
		rc = mqtt_emit_send_buf(m, m->io.disconnect_buf, m->length.pack_len.disconnect_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_DISCONNECT;
	}else if(m->ses.tx_pending & MQTT_PENDING_PINGREQ) {
		rc = mqtt_emit_send_buf(m, m->io.pingreq_buf, m->length.pack_len.pingreq_buf_len);
		if(rc < 0) {
			return rc; // Send failed, keep the pending flag for retry
		}
		m->ses.tx_pending &= ~MQTT_PENDING_PINGREQ;
	}
	return 0; // Success
}

