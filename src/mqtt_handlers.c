#include "mqtt_pack.h"
#include "mqtt_utils.h"
#include "mqtt_parse.h"
#include "mqtt_handlers.h"
#include "mqtt_core.h"
#include "mqtt_type.h" 




int mqtt_handle_publish(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，PUBLISH最短是2字节
	}
	mqtt_publish_view_t view;
	int res = mqtt_parse_publish_view(rx, rx_len, &view);
	if(res == 0) {
		if(view.qos == 0) {
			if(m->callbacks.on_publish) {
				m->callbacks.on_publish(m->callbacks.on_publish_ctx, &view);
			}
			return MQTT_RX_PUBLISH_QOS0;//处理了qos0的包
		} else if(view.qos == 1) {
			// QoS 1 先发送 PUBACK 再调用回调函数
			mqtt_pack_puback(m, view.packet_id);
			mqtt_emit_send(m);
			if(m->callbacks.on_publish) {
				m->callbacks.on_publish(m->callbacks.on_publish_ctx, &view);
			}
			if(m->callbacks.on_puback) {
				return MQTT_RX_PUBLISH_QOS1_ACKED;//处理了qos1的包并且已经发送了ack
			}
			return MQTT_RX_PUBLISH_QOS1;//处理了qos1的包
		}
		if(view.qos == 2) {
			// QoS 2 先发送 PUBREC 等待 PUBREL 再调用回调函数，这里暂时不实现完整的 QoS 2 流程
			return MQTT_RX_PUBLISH_QOS2_UNSUPPORTED; //处理了qos2的包，但还没有完成整个流程,当前就是当是2服务等级的时候返回错误
		}
	} else {
		return res; // 解析失败
	}
	return MQTT_RX_UNHANDLED; // 没有处理这个包
}

int mqtt_handle_connack(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，CONNACK最短是2字节
	}
	mqtt_connack_view_t view;
	int res = mqtt_parse_connack_view(rx, rx_len, &view);
	if(res < 0) {
		return res; // 解析失败
	}
	m->ses.connack_rc = view.return_code;
	m->ses.session_present = view.session_present;
	if(m->callbacks.on_connack) {
		m->callbacks.on_connack(m->callbacks.on_connack_ctx, &view);//需要注意的是这个回调是在return之前，所以就算是错的也要考虑
	}
	//这里可以考虑加回调
	return MQTT_RX_CONNACK; // 处理了 CONNACK 包
}
int mqtt_handle_suback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，SUBACK最短是2字节
	}
	mqtt_suback_view_t view;
	int res = mqtt_parse_suback_view(rx, rx_len, &view);
	if(res < 0) {
		return res; // 解析失败A
	}
	if(view.packet_id != m->ses.last_subscribe_pid) {
		return MQTT_ERR_PID_MISMATCH; // SUBACK 的消息 ID 不匹配
	}
	if(view.return_codes == NULL || view.return_codes_len == 0) {
		return MQTT_ERR_MALFORMED; // SUBACK 中没有返回码
	}
	if(m->callbacks.on_suback) {
		m->callbacks.on_suback(m->callbacks.on_suback_ctx, &view);
	}
	return MQTT_RX_SUBACK; // 处理了 SUBACK 包
}

int mqtt_handle_unsuback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，UNSUBACK最短是2字节
	}
	mqtt_unsuback_view_t view;
	int res = mqtt_parse_unsuback_view(rx, rx_len, &view);
	if(res < 0) {
		return res; // 解析失败
	}
	if(view.packet_id != m->ses.last_unsubscribe_pid) {
		return MQTT_ERR_PID_MISMATCH; // UNSUBACK 的消息 ID 不匹配
	}
	if(m->callbacks.on_unsuback) {
		m->callbacks.on_unsuback(m->callbacks.on_unsuback_ctx, &view);
	}
	return MQTT_RX_UNSUBACK; // 处理了 UNSUBACK 包
}


int mqtt_handle_pingresp(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，PINGRESP最短是2字节
	}
	int res = mqtt_parse_pingresp(rx, rx_len);
	if(res < 0) {
		return res; // 解析失败
	}
	if(m->callbacks.on_pingresp) {
		m->callbacks.on_pingresp(m->callbacks.on_pingresp_ctx);
	}
	return MQTT_RX_PINGRESP; // 处理了 PINGRESP 包
}
int mqtt_handle_puback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
{
	if(m == NULL || rx == NULL) {
		return MQTT_ERR_ARG; // Invalid MQTT control block
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，PUBACK最短是4字节
	}
	mqtt_puback_view_t view;
	int res = mqtt_parse_puback_view(rx, rx_len, &view);
	if(res < 0) {
		return res; // 解析失败
	}
	if(m->callbacks.on_puback) {
		m->callbacks.on_puback(m->callbacks.on_puback_ctx, &view);
	}
	return MQTT_RX_PUBACK; // 处理了 PUBACK 包
}

