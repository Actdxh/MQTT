#include "mqtt_utils.h"
#include "mqtt_parse.h"
#include "mqtt_type.h"
#include "string.h" 

int mqtt_parse_publish_view(const uint8_t* rx, uint32_t rx_len, mqtt_publish_view_t* view)
{
	if(rx == NULL || view == NULL || rx_len == 0 || rx_len < 2) {
		return MQTT_ERR_MALFORMED; // 参数错误
	}
	if((rx[0] & 0xF0) != 0x30) {
		return MQTT_ERR_MALFORMED; // Not a PUBLISH packet，参数错误
	}
	memset(view, 0, sizeof(*view));
	int res = 0;
	uint8_t rem_len_bytes;
	uint16_t topic_len;
	uint32_t packet_len;
	uint32_t rem_len;
	uint32_t idx;
	

	view->dup = (rx[0] & 0x08) >> 3;
	view->qos = (rx[0] & 0x06) >> 1;
	view->retain = (rx[0] & 0x01);
	if(view->qos > 2) {
		return MQTT_ERR_MALFORMED; // 非法的服务等级
	}
	res = mqtt_read_rem_len(rx + 1, rx_len - 1, &rem_len, &rem_len_bytes);
    if(res == -2) {
        return MQTT_ERR_INCOMPLETE; // 包不完整
    }else if(res < 0) {
		return MQTT_ERR_MALFORMED; // Error reading remaining length
	}
	packet_len = 1 + rem_len_bytes + rem_len; // 先根据固定报头解析总长度，然后根据总长度判断接收的包是否完整
	if(rx_len < packet_len) {
		return MQTT_ERR_INCOMPLETE; // 接受的包不完整
	}
	idx = 1 + rem_len_bytes;			// 这里的idx就是固定报头的长度，也就是可变报头的起始位置（数组下标）
	if(idx + 2 > packet_len) return MQTT_ERR_MALFORMED; // 包格式错误，缺少主题长度(就是接收到的包的总长度不可能小于固定)

	topic_len = (uint16_t)(rx[idx] << 8 | rx[idx + 1]);
	idx += 2;							// 这里的idx就是主题内容的起始位置（数组下标）
	if(idx + topic_len > packet_len) return MQTT_ERR_MALFORMED; // 包格式错误，缺少主题内容
	view->topic = rx + idx;				// 结构体里面的topic是指针
	view->topic_len = topic_len;		// 结构体里面的topic_len是主题内容的长度
	idx += topic_len;					// 这里的idx就是负载的起始位置或者是可变报头的报文标识符的起始位置，如果服务等级是0则没有报文标识符，负载直接从这里开始，如果服务等级大于0则这里是报文标识符，负载从这里加2开始
	if(view->qos > 0) {
		if(idx + 2 > packet_len) return MQTT_ERR_MALFORMED; // 包格式错误，缺少报文标识符
		view->packet_id = (uint16_t)(rx[idx] << 8 | rx[idx + 1]); // 服务等级大于0才有报文标识符
		idx += 2;						// 这里的idx就是负载的起始位置（数组下标）
	} else {
		view->packet_id = 0;			// 服务等级为0没有报文标识符，设置为0
	}
	view->payload = rx + idx;			// 现在计算有效负载
	view->payload_len = packet_len - idx;	// 有效负载长度 = 包的总长度 - 负载的起始位置
	view->packet_len = packet_len;		// 包的总长度

	return 0; // Success
}

int mqtt_parse_connack_view(const uint8_t* rx, uint32_t rx_len, mqtt_connack_view_t* view)
{
	if(rx == NULL || view == NULL) {
		return MQTT_ERR_ARG; // 参数错误
	}
    if(rx_len < 4) {
        return MQTT_ERR_INCOMPLETE; // 包不完整
    }
	if(rx[0] != 0x20) {
		return MQTT_ERR_MALFORMED; // Not a CONNACK packet，参数错误
	}
    if(rx[1] != 0x02) {
        return MQTT_ERR_MALFORMED; // CONNACK剩余长度必须是2
    }
    if((rx[2] & 0xFE) != 0) {
        return MQTT_ERR_MALFORMED; // 其他位必须为0，只有最低位是session present标志
    }
    if(rx[3] != 0x00 && rx[2] & 0x01) {
        return MQTT_ERR_MALFORMED; // 如果返回码不为0，session present必须为0
    }
	memset(view, 0, sizeof(*view));
	view->session_present = (uint8_t)(rx[2] & 0x01);
	view->return_code = rx[3];
	return 0;
}

int mqtt_parse_suback_view(const uint8_t* rx, uint32_t rx_len, mqtt_suback_view_t* view)
{
    if(rx == NULL || view == NULL) {
        return MQTT_ERR_ARG; // 参数错误
    }
    if(rx_len < 2) {
        return MQTT_ERR_INCOMPLETE; // 包不完整，SUBACK最短是5字节
    }
    if(rx[0] != 0x90) {
        return MQTT_ERR_MALFORMED; // Not a SUBACK packet，参数错误
    }
    uint8_t rem_len_bytes;
    uint32_t rem_len;
    int res = mqtt_read_rem_len(rx + 1, rx_len - 1, &rem_len, &rem_len_bytes);
    if(res == -2) {
        return MQTT_ERR_INCOMPLETE; // 包不完整
    } else if(res < 0) {
        return MQTT_ERR_MALFORMED; // Error reading remaining length
    }
    uint32_t packet_len = 1 + rem_len_bytes + rem_len; // 先根据固定报头解析总长度，然后根据总长度判断接收的包是否完整
    if(rx_len < packet_len) {
        return MQTT_ERR_INCOMPLETE; // 接受的包不完整
    }
    if(rem_len < 3) {
        return MQTT_ERR_MALFORMED; // SUBACK剩余长度必须至少是3，因为要包含报文标识符2+至少1个返回码
    }
    uint32_t idx = 1 + rem_len_bytes; // 可变报头的起始位置
    uint16_t pid = (uint16_t)(rx[idx] << 8 | rx[idx + 1]);// 报文标识符
    idx += 2; // 返回码的起始位置
    uint32_t rc_len32 = packet_len - idx; // 返回码的长度
    if(rc_len32 == 0 || rc_len32 > 255) {
        return MQTT_ERR_MALFORMED; // 返回码的长度必须至少是1，最多是255
    }
    memset(view, 0, sizeof(*view));
    view->packet_id = pid;
    view->return_codes = rx + idx;
    view->return_codes_len = (uint8_t)rc_len32;
    view->packet_len = packet_len;
    return 0; // Success    
}

int mqtt_parse_puback_view(const uint8_t* rx, uint32_t rx_len, mqtt_puback_view_t* view)
{
	if(rx == NULL || view == NULL) {
		return MQTT_ERR_ARG; // 参数错误
	}
	if(rx_len < 4) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，PUBACK必须是4字节
	}
	uint32_t rem_len = 0;
	uint8_t rem_len_bytes = 0;
	int res = mqtt_read_rem_len(rx + 1, rx_len - 1, &rem_len, &rem_len_bytes);
	if(res == -2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整
	} else if(res < 0) {
		return MQTT_ERR_MALFORMED; // Error reading remaining length
	}
	if(rx[0] != 0x40) {
		return MQTT_ERR_MALFORMED; // Not a PUBACK packet，参数错误
	}
	if(rem_len != 0x02 || rem_len_bytes != 1) {
		return MQTT_ERR_MALFORMED; // PUBACK剩余长度必须是2
	}
	memset(view, 0, sizeof(*view));
	uint8_t idx = 1 + rem_len_bytes; // 报文标识符的起始位置
	view->packet_id = (uint16_t)(rx[idx] << 8 | rx[idx + 1]);
	return 0;
}
int mqtt_parse_pingresp(const uint8_t* rx, uint32_t rx_len)
{
	if(rx == NULL) {
		return MQTT_ERR_ARG; // 参数错误
	}
	if(rx_len < 2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整，PINGRESP必须是2字节
	}
	uint32_t rem_len = 0;
	uint8_t rem_len_bytes = 0;
	int res = mqtt_read_rem_len(rx + 1, rx_len - 1, &rem_len, &rem_len_bytes);
	if(res == -2) {
		return MQTT_ERR_INCOMPLETE; // 包不完整
	} else if(res < 0) {
		return MQTT_ERR_MALFORMED; // Error reading remaining length
	}
	if(rx[0] != 0xD0 || rem_len != 0 || rem_len_bytes != 1) {
		return MQTT_ERR_MALFORMED; // PINGRESP包格式错误
	}
	return 0; // Success
}
