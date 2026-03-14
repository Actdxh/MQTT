// #include "MQTT.h"
// #include "string.h"
// #include "stdio.h"
// #include "mqtt_utils.h"
// #include <stdint.h>
// #include "mqtt_parse.h"
// //#define MQTT_DEBUG

// static void mqtt_emit_send(MQTT_TCB* m);
// static void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view);



// int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config)
// {
// 	if(m == NULL || config == NULL) {
// 		return -1; // Invalid input
// 	}

// 	if(config->WillQoS > 2) {
// 		return -1; // Invalid Will QoS
// 	}

// 	if(config->ClientID[0] == '\0') {
// 		return -1; // ClientID, UserName, and Passward cannot be empty
// 	}
// 	if(config->WillEnable) {
// 		if(config->WillTopic[0] == '\0' || config->WillData[0] == '\0') {
// 			return -1; // Will topic and data cannot be empty when Will is enabled
// 		}
// 	}
// 	memset(m, 0, sizeof(*m));
// 	m->param = *config;

// 	m->length.cid_len = m->param.ClientID ? strlen(m->param.ClientID) : 0;
// 	m->length.user_len = m->param.UserName ? strlen(m->param.UserName) : 0;
// 	m->length.pwd_len = m->param.Passward ? strlen(m->param.Passward) : 0;
// 	if(m->param.WillEnable) {
// 		m->length.willtopic_len = m->param.WillTopic ? strlen(m->param.WillTopic) : 0;
// 		m->length.willdata_len = m->param.WillData ? strlen(m->param.WillData) : 0;
// 	} else {
// 		m->length.willtopic_len = 0;
// 		m->length.willdata_len = 0;
// 	}
// 	//m->length.topic_len = strlen(m->topic);
// 	m->MessageID = 1;
// 	m->rx_buf_len = 0;

// 	return 0;
// }

// int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive)
// {
// 	uint16_t p = 0;
// 	uint16_t cid_len = m->param.ClientID ? strlen(m->param.ClientID) : 0;
// 	uint16_t user_len = m->param.UserName ? strlen(m->param.UserName) : 0;
// 	uint16_t pwd_len = m->param.Passward ? strlen(m->param.Passward) : 0;
// 	uint16_t willtopic_len = m->param.WillTopic ? strlen(m->param.WillTopic) : 0;
// 	uint16_t willdata_len = m->param.WillData ? strlen(m->param.WillData) : 0;
// 	uint16_t Fixedheader_len = 0;
// 	uint16_t Remaining_len = 0;
// 	uint16_t Variableheader_len = 0;
// 	uint16_t Load_len = 0;
// 	uint16_t Totallength = 0;
	
// 	/************************固定报头*************************/
// 	int res = 0;
// 	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
// 	Fixedheader_len = 1;
// 	Variableheader_len = 10;														//协议名长度2字节+协议名4字节+协议级别1字节+连接标志1字节+保持时间2字节=10字节
// 	Load_len = 2 + cid_len +											//前面的2指接下来的数据长度，目前不包含遗嘱
// 			   2 + user_len +
// 			   2 + pwd_len;
// 	if(m->param.WillEnable == 1) {
// 		Load_len += 2 + willtopic_len + 2 + willdata_len;		//如果遗嘱使能了则加上遗嘱主题和遗嘱数据的长度
// 	}
// 	Remaining_len = Variableheader_len + Load_len;			//剩余长度等于可变报头长度加上负载长度

// 	out[p++] = 0x10; 																			//CONNECT报文类型，标志位为0
// 	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
// 	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
// 		return -1; // Invalid remaining length
// 	}
// 	p = p + rem_len_bytes;

// 	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
// 	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
// 	if(out_size < Totallength) {
// 		return -1; // Output buffer is too small
// 	}

// 	{
// 		/************************可变报头*************************/ 
// 		/* 
// 		* 7.   User Name Flag	1
// 		* 6.   Password Flag	1
// 		* 5.   Will Retain		0
// 		* 4.   Will QoS			0
// 		* 3.   Will QoS			0
// 		* 2.   Will Flag		0
// 		* 1.   Clean Session	1
// 		* 0.   Reserved			0
// 		*/
// 		out[p++] = 0x00;																			//长度高八位 
// 		out[p++] = 0x04;																		//长度低八位 
// 		out[p++] = 0x4D;																		//"M" 
// 		out[p++] = 0x51;																		//"Q" 
// 		out[p++] = 0x54;																		//"T" 
// 		out[p++] = 0x54;																		//"T" 
// 		out[p++] = 0x04;																		//协议级别
// 		uint8_t connect_flags = 0;																//连接标志位
// 		if(user_len > 0) connect_flags |= 1<<7;										//用户名标志位
// 		if(pwd_len  > 0) connect_flags |= 1<<6;										//密码标志位
// 		if(m->param.WillEnable && m->param.WillRetain) connect_flags |= 1<<5;					//遗嘱保留标志位
// 		if(m->param.WillEnable) connect_flags |= (m->param.WillQoS & 0x03) << 3;				//遗嘱服务等级标志位
// 		if(m->param.WillEnable  ) connect_flags |= 1<<2;										//遗嘱标志位
// 		if(m->param.CleanSession) connect_flags |= 1<<1;										//连接会话清除标志位

// 		out[p++] = connect_flags;																//连接标志位
// 	}
// 	out[p++] = keepalive/256;									//保持时间高位 
// 	out[p++] = keepalive%256;								//保持时间高低位 
// 	/************************有效负载*************************/
// 	res = mqtt_write_str(out, out_size, &p, m->param.ClientID);
// 	if(res < 0) {
// 		return -1; // Error writing client ID
// 	}
// 	if(m->param.WillEnable == 1) {

// 		 res = mqtt_write_str(out, out_size, &p, m->param.WillTopic);
// 		 if(res < 0) {
// 			 return -1; // Error writing will topic
// 		 }

// 		 res = mqtt_write_str(out, out_size, &p, m->param.WillData);
// 		 if(res < 0) {
// 			 return -1; // Error writing will data
// 		 }
// 	}
// 	res = mqtt_write_str(out, out_size, &p, m->param.UserName);
// 	if(res < 0) {
// 		return -1; // Error writing user name
// 	}
	
// 	res = mqtt_write_str(out, out_size, &p, m->param.Passward);
// 	if(res < 0) {
// 		return -1; // Error writing password
// 	}
// 	#ifdef MQTT_DEBUG
// 	printf("%d\t", Fixedheader_len);
// 	printf("%d\t", Variableheader_len);
// 	printf("%d\t", Load_len);
// 	printf("%d\t", Remaining_len);
// 	printf("%d\t", Totallength);
// 	printf("%d\r\n", p);
// 	#endif
// 	if(p != Totallength) {
// 		return -1; // Packed length does not match expected total length
// 	}

// 	m->length.Fixedheader_len = Fixedheader_len;
// 	m->length.Variableheader_len = Variableheader_len;
// 	m->length.Load_len = Load_len;
// 	m->length.Remaining_len = Remaining_len;
// 	m->length.Totallength = Totallength;

// 	return m->length.Totallength; // Return the total length of the packed MQTT CONNECT message
// }

// int MQTT_CONNECT(MQTT_TCB *m, u32 keepalive)														//keepalive是保持时间 
// {
// 	return mqtt_pack_connect(m, m->buff, BUFF_SIZE, keepalive);
// }


// char MQTT_CONNACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
// {
// 	if((rxdata_len == 4) && (rxdata[0] == 0x20))
// 	{
		
// 	}else
// 	{
// 		return -1;
// 	}
// 	return rxdata[3];
// } 

// /************************DISCONNECT函数*************************/ 
// void MQTT_DISCONNECT(MQTT_TCB *m)
// {
// 	m->buff[0] = 0xE0;
// 	m->buff[1] = 0x00;
// 	m->length.Totallength = 2;
// }

// /************************SUBSCRIBE函数*************************/ 

// int mqtt_pack_subscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, const char * topic, char qos)
// {	
// 	if(topic == NULL) {
// 		return -1; // Invalid topic
// 	}
// 	if(qos < 0 || qos > 2) {
// 		return -1; // Invalid QoS
// 	}

	
// 	uint16_t p = 0;
// 	uint16_t Fixedheader_len = 0;
// 	uint16_t Remaining_len = 0;
// 	uint16_t Variableheader_len = 0;
// 	uint16_t Load_len = 0;
// 	uint16_t Totallength = 0;
// 	uint16_t topic_len = (uint16_t)strlen(topic);
// 	/************************固定报头*************************/
// 	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
// 	Fixedheader_len = 0;
// 	Variableheader_len = 2;
// 	Load_len = 2 + topic_len + 1;												//2:主题长度，1服务等级 
// 	Remaining_len = Variableheader_len + Load_len;
				
// 	out[p++] = 0x82; 
// 	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
// 	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
// 		return -1; // Invalid remaining length
// 	}
// 	p = p + rem_len_bytes;

// 	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
// 	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
// 	if(out_size < Totallength) {
// 		return -1; // Output buffer is too small
// 	}
// 	/************************可变报头*************************/ 
	
// 	out[p++] = m->MessageID/256;								//报文标识符高位 
// 	out[p++] = m->MessageID%256;								//报文标识符低位 
// 	m->last_subscribe_pid = m->MessageID;
// 	m->MessageID++;
// 	if(m->MessageID == 0)
// 	{
// 		m->MessageID = 1;
// 	} 

// 	/************************有效负载*************************/ 
// 	int res = mqtt_write_str(out, out_size, &p, topic);
// 	if(res < 0) {
// 		return -1; // Error writing topic
// 	}
// 	out[p++] = qos;
	
// 	if(p != Totallength) {
// 		return -1;
// 	}

// 	m->length.Fixedheader_len = Fixedheader_len;
// 	m->length.Variableheader_len = Variableheader_len;
// 	m->length.Load_len = Load_len;
// 	m->length.Remaining_len = Remaining_len;
// 	m->length.Totallength = Totallength;
	
// 	return Totallength;
// }

// int MQTT_SUBSCRIBE(MQTT_TCB *m, char* topic, char QS)												//一次制订阅一个主题 
// {
// 	return mqtt_pack_subscribe(m, m->buff, BUFF_SIZE, topic, QS);
// }

// /************************SUBACK函数*************************/ 
// char MQTT_SUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
// {
// 	if((rxdata_len == 5) && (rxdata[0] == 0x90))
// 	{
// 	}else
// 	{
// 		return -1;
// 	}
// 	return rxdata[4];																	//服务等级 
// } 

// /************************UNSUBSCRIBE函数*************************/ 

// int mqtt_pack_unsubscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, char * topic)
// {
// 	uint16_t p = 0;
// 	uint16_t Fixedheader_len = 0;
// 	uint16_t Remaining_len = 0;
// 	uint16_t Variableheader_len = 0;
// 	uint16_t Load_len = 0;
// 	uint16_t Totallength = 0;
// 	/************************固定报头*************************/ 
// 	Fixedheader_len = 1;
// 	uint16_t topic_len = (uint16_t)strlen(topic);
// 	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
// 	Variableheader_len = 2;
// 	Load_len = 2 + topic_len;													//2:主题长度 
// 	Remaining_len = Variableheader_len + Load_len;
				
// 	out[p++] = 0xA2; 
// 	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
// 	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
// 		return -1; // Invalid remaining length
// 	}
// 	p = p + rem_len_bytes;

// 	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
// 	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
// 	if(out_size < Totallength) {
// 		return -1; // Output buffer is too small
// 	}
	
// 	/************************可变报头*************************/ 
	
// 	out[p++] = m->MessageID/256;								//报文标识符高位 
// 	out[p++] = m->MessageID%256;								//报文标识符低位 
// 	m->MessageID++;
// 	if(m->MessageID == 0)
// 	{
// 		m->MessageID = 1;
// 	}

// 	/************************有效负载*************************/ 
// 	int res = mqtt_write_str(out, out_size, &p, topic);
// 	if(res < 0) {
// 		return -1; // Error writing topic
// 	}
// 	if(p != Totallength) {
// 		return -1;
// 	}

// 	m->length.Fixedheader_len = Fixedheader_len;
// 	m->length.Variableheader_len = Variableheader_len;
// 	m->length.Load_len = Load_len;
// 	m->length.Remaining_len = Remaining_len;
// 	m->length.Totallength = Totallength;

// 	return Totallength;
// }

// int MQTT_UNSUBSCRIBE(MQTT_TCB *m, char* topic)												//一次制订阅一个主题
// {
// 	return mqtt_pack_unsubscribe(m, m->buff, BUFF_SIZE, topic);
// }

// /************************SUBACK函数*************************/ 
// char MQTT_UNSUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
// {
// 	if((rxdata_len == 4) && (rxdata[0] == 0xB0))
// 	{
// 	}else
// 	{
// 		return -1;
// 	}
// 	return 1;
// } 

// /************************PINGREQ函数*************************/ 
// void MQTT_PINGREQ(MQTT_TCB *m)
// {
// 	m->buff[0] = 0xC0;
// 	m->buff[1] = 0x00;
// 	m->length.Totallength = 2;
// }

// /************************PINGRESP函数*************************/ 
// char MQTT_PINGRESP(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
// {
// 	if((rxdata_len == 2) && (rxdata[0] == 0xD0))
// 	{
// 	}else
// 	{
// 		return -1;
// 	}
// 	return 1;
// } 

// /************************PUBLISH0函数*************************/ 
// void MQTT_PUBLISH0(MQTT_TCB *m, char retain, char* topic, u8 *data, u32 data_len)
// {
// 	/************************固定报头*************************/ 
// 	m->length.Fixedheader_len = 1;
// 	uint16_t topic_len = (uint16_t)strlen(topic);
// 	int rem_len_bytes = 0;
// 	uint16_t p = m->length.Fixedheader_len;
// 	m->length.Variableheader_len = 2 + topic_len;
// 	m->length.Load_len = data_len;		
// 	m->length.Remaining_len = m->length.Variableheader_len + m->length.Load_len;	
			
// 	m->buff[0] = 0x30 | (retain << 0); 
// 	do{
// 		if(m->length.Remaining_len/128 == 0)													//不需要进位 
// 		{
// 			m->buff[p] = m->length.Remaining_len; 
// 		}else
// 		{
// 			m->buff[p] = (m->length.Remaining_len % 128) | 0x80; 
// 		}
// 		rem_len_bytes ++;
// 		p++;
// 		m->length.Remaining_len = m->length.Remaining_len/128; 
// 	}while(m->length.Remaining_len);
	
// 	/************************可变报头*************************/ 
// 	m->buff[p] = topic_len/256;
// 	m->buff[p+1] = topic_len%256;	
// 	memcpy(&m->buff[p+2], topic, topic_len);
// 	p = p + 2 + topic_len;

// 	/************************有效负载*************************/ 
	
// 	memcpy(&m->buff[p], data, data_len);
	
	
// 	m->length.Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	
// 	m->length.Totallength =  m->length.Fixedheader_len + m->length.Variableheader_len + m->length.Load_len;		//报文总长度 
// }

// /************************PUBLISH函数*************************/ 

// int mqtt_pack_publish(
//     MQTT_TCB* m,
//     uint8_t* out,
//     uint16_t out_size,
//     const char* topic,
//     const void* payload,
//     uint16_t payload_len,
//     uint8_t qos,       // 0/1（先不做2）
//     uint8_t retain,    // 0/1
//     uint8_t dup        // 0/1（qos=0 时 dup 也允许但一般没意义）
// )
// {

// 	if(topic == NULL || payload == NULL) {
// 		return -1; // Invalid input
// 	}
// 	if(qos >= 2) {
// 		return -1; // Invalid QoS
// 	}
// 	if(dup > 1) {
// 		return -1; // Invalid DUP flag
// 	}
// 	if(retain > 1) {
// 		return -1; // Invalid retain flag
// 	}
	
// 	uint16_t p = 0;
// 	uint16_t Fixedheader_len = 0;
// 	uint16_t Remaining_len = 0;
// 	uint16_t Variableheader_len = 0;
// 	uint16_t Load_len = 0;
// 	uint16_t Totallength = 0;
// 	/************************固定报头*************************/ 
// 	uint16_t topic_len = (uint16_t)strlen(topic);
// 	int rem_len_bytes = 0;;

// 	Fixedheader_len = 1;
// 	Variableheader_len = 2 + topic_len + ((qos > 0) ? 2 : 0);		//主题长度+主题内容+（如果服务等级大于0则加上报文标识符长度）
// 	Load_len = payload_len;
// 	Remaining_len = Variableheader_len + Load_len;
// 	out[p++] = 0x30 | (dup << 3) | (qos << 1)| (retain << 0);
// 	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
// 	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
// 		return -1; // Invalid remaining length
// 	}
// 	p = p + rem_len_bytes;
	
// 	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
// 	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
// 	if(out_size < Totallength) {
// 		return -1; //这里的长度检测还有一个不足就是没有考虑要发送超级大数组导致超过uint16_t然后回卷导致判断是够的，当然也被实际的数组长度限制，这里暂时不做修改
// 	}
// 	/************************可变报头*************************/ 

// 	int res = mqtt_write_str(out, out_size, &p, topic);
// 	if(res < 0) {
// 		return -1; // Error writing topic
// 	}
// 	if(qos > 0) {
// 		out[p++] = m->MessageID/256;								//报文标识符高位 
// 		out[p++] = m->MessageID%256;								//报文标识符低位 
// 		m->MessageID++;
// 		if(m->MessageID == 0)
// 		{
// 			m->MessageID = 1;
// 		}
// 	} 
// 	/************************有效负载*************************/ 
// 	memcpy(out+p, payload, payload_len);
// 	p += payload_len;

// 	if(p != Totallength) {
// 		return -1;
// 	}

// 	m->length.Fixedheader_len = Fixedheader_len;
// 	m->length.Variableheader_len = Variableheader_len;
// 	m->length.Load_len = Load_len;
// 	m->length.Remaining_len = Remaining_len;
// 	m->length.Totallength = Totallength;

// 	return Totallength;
// }

// //提供一个更方便的接口，直接传入参数结构体
// int mqtt_pack_publish_two(MQTT_TCB* m,uint8_t* out,uint16_t out_size, mqtt_publish_params_t *params)
// {
// 	if(params == NULL) {
// 		return -1; // Invalid input
// 	}
// 	return mqtt_pack_publish(m, out, out_size, params->topic, params->payload, params->payload_len, params->qos, params->retain, params->dup);
// }


// int MQTT_PUBLISH(MQTT_TCB *m, char dup, char QoS, char retain, char* topic, void *data, u32 data_len)
// {
// 	return mqtt_pack_publish(m, m->buff, BUFF_SIZE, topic, data, data_len, QoS, retain, dup);
// }


// /************************处理服务器推送的PUBLISH相关函数*************************/ 


// void my_on_connack(void* user_ctx, const mqtt_connack_view_t* v)
// {
// 	//空，后续补充
// 	//需要注意的是这个回调是在return之前，所以就算是错的也要考虑
// }
// void MQTT_SetOnConnack(MQTT_TCB* m, mqtt_on_connack_cb cb, void* user_ctx)
// {
// 	m->on_connack = cb;
// 	m->user_ctx = user_ctx;
// }
// void my_on_message(void* user_ctx, const mqtt_publish_view_t* msg)
// {
// 	uint32_t i;
// 	printf("Received message:\n");
// 	printf("Topic Length: %d\n", msg->topic_len);
// 	printf("Topic: %.*s\n", msg->topic_len, msg->topic);
// 	printf("Payload Length: %d\n", msg->payload_len);
	
// 	printf("Payload: ");
// 	const uint8_t* pl = (const uint8_t*)msg->payload;
// 	for(i = 0; i < msg->payload_len; i++) {
// 		if(pl[i] >= 32 && pl[i] <= 126) {
// 			printf("%c", pl[i]);
// 		} else {
// 			printf("."); // 非法/不可见字符显示为点
// 		}
// 	}
// 	printf("\n");
	
// 	printf("QoS: %d\n", msg->qos);
// 	printf("Retain: %d\n", msg->retain);
// 	printf("DUP: %d\n", msg->dup);
// 	printf("Packet ID: %d\n", msg->packet_id);
// };

// void my_on_send(void* user_ctx, const uint8_t* data, uint16_t len)
// {
// 	uint16_t i; 
//     (void)user_ctx;

//     printf("TX %u bytes: ", (unsigned)len);
//     for (i = 0; i < len; i++) {
//         printf("%02X ", data[i]);
//     }
//     printf("\r\n");
// };

// void MQTT_SetOnMessage(MQTT_TCB* m, mqtt_on_message_cb cb, void* user_ctx)
// {
// 	if((cb == NULL) || (m == NULL)){
// 		return; // Invalid callback
// 	}
// 	m->on_message = cb;
// 	m->user_ctx = user_ctx;
// }

// void MQTT_SetOnSend(MQTT_TCB* m, mqtt_on_send_cb cb, void* user_ctx)
// {
// 	if((cb == NULL) || (m == NULL)) {
// 		return; // Invalid callback
// 	}
// 	m->on_send = cb;
// 	m->user_ctx = user_ctx;
// }

// int MQTT_OnRx(MQTT_TCB* m, const uint8_t* rx_data, uint32_t rx_len)
// {
// 	if (!m || !rx_data || rx_len < 2) return -1;
// 	uint8_t type = rx_data[0] & 0xF0;
// 	switch(type) {
// 		case 0x20: {
// 			return mqtt_handle_connack(m, rx_data, rx_len); // CONNACK
// 		}
// 		case 0x30: { 
// 			return mqtt_handle_publish(m, rx_data, rx_len); // PUBLISH
// 		}
// 		case 0x40:{
// 			return mqtt_handle_puback(m, rx_data, rx_len); // PUBACK
// 		}
// 		case 0x90: {
// 			return mqtt_handle_suback(m, rx_data, rx_len); // SUBACK
// 		}
// 		case 0xD0: {
// 			return mqtt_handle_pingresp(m, rx_data, rx_len); // PINGRESP
// 		}
// 		// 这里可以添加对其他类型报文的处理，比如CONNACK、SUBACK等
// 		default:
// 			return 0; // Unhandled packet type
// 	}
// }

// int MQTT_InputBytes(MQTT_TCB* m, const uint8_t* data, uint32_t len)
// {
// 	if(m == NULL || data == NULL || len == 0) {
// 	return -1; // Invalid input
// 	}
// 	if(len > sizeof(m->rx_buf)) {// 如果一次接收的数据长度超过缓冲区大小，直接丢弃并返回错误
// 	return -1; 
// 	}

// 	int res = 0;
// 	int frames = 0;
// 	uint8_t rem_len_bytes;
// 	uint32_t rem_len;
// 	uint32_t frame_len;


// 	if(len > sizeof(m->rx_buf) - m->rx_buf_len) {// 检查剩余缓冲区大小是否足够不够就清空缓冲区
// 		m->rx_buf_len = 0;
// 	}
// 	memcpy(m->rx_buf+m->rx_buf_len, data, len);
// 	m->rx_buf_len += len;
// 	while(m->rx_buf_len >= 2)
// 	{
// 		res = mqtt_read_rem_len(m->rx_buf + 1, m->rx_buf_len - 1, &rem_len, &rem_len_bytes);
// 		if(res == -2)
// 		{
// 			break; // 接收的包不完整，继续等待
// 		}else if(res < 0)
// 		{
// 			memmove(m->rx_buf, m->rx_buf + 1, m->rx_buf_len - 1); // 移除第一个字节，继续尝试解析下一个包
// 			m->rx_buf_len -= 1;
// 			continue;
// 		}
// 		frame_len = 1 + rem_len_bytes + rem_len;
// 		if(frame_len > m->rx_buf_len) {
// 			break; // 接收的包不完整，继续等待
// 		}
// 		// 处理完整的 MQTT 包
// 		res = MQTT_OnRx(m, m->rx_buf, frame_len);
// 		m->last_event_code = res;
// 		#ifdef MQTT_DEBUG
// 		printf("OnRx: %s (%d)\n", MQTT_RxEventStr(res), res);
// 		#endif
// 		// 移除已处理的包
// 		if(frame_len == m->rx_buf_len) {
// 			m->rx_buf_len = 0; // 刚好处理完所有数据，直接清空缓冲区
// 			frames++;
// 		} else if(frame_len < m->rx_buf_len) {
// 			memmove(m->rx_buf, m->rx_buf + frame_len, m->rx_buf_len - frame_len);//这个函数是从buf里面往后面移一帧的数据放到前面来第三个参数要减的原因就是移动减去帧长的长度
// 			m->rx_buf_len -= frame_len;
// 			frames++;
// 		}
// 	}
// 	return frames; // 返回处理的帧数
// }


// void MQTT_SetOnSuback(MQTT_TCB* m, mqtt_on_suback_cb cb, void* user_ctx)
// {
// 	if(!cb || !m) {
// 		return; // Invalid callback
// 	}
// 	m->on_suback = cb;
// 	m->user_ctx = user_ctx;
// }

// static void mqtt_emit_send(MQTT_TCB* m)
// {
//     if (m->on_send && m->length.Totallength > 0) {
//         m->on_send(m->user_ctx, m->buff, (uint16_t)m->length.Totallength);
//     }
// }

// static void mqtt_emit_message(MQTT_TCB* m, const mqtt_publish_view_t* view)
// {
//     if (m->on_message) {
//         m->on_message(m->user_ctx, view);
//     }
// }

// static int mqtt_handle_publish(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
// {
// 	mqtt_publish_view_t view;
// 			int res = mqtt_parse_publish_view(rx, rx_len, &view);
// 			if(res == 0) {
// 				if(view.qos == 0) {
// 					mqtt_emit_message(m, &view);
// 					return MQTT_RX_PUBLISH_QOS0;//处理了qos0的包
// 				} else if(view.qos == 1) {
// 					// QoS 1 先发送 PUBACK 再调用回调函数
// 					MQTT_PUBACK(m, view.packet_id);
// 					mqtt_emit_send(m);
// 					mqtt_emit_message(m, &view);
// 					if(m->on_send) {
// 						return MQTT_RX_PUBLISH_QOS1_ACKED;//处理了qos1的包并且已经发送了ack
// 					}
// 					return MQTT_RX_PUBLISH_QOS1;//处理了qos1的包
// 				}
// 				if(view.qos == 2) {
// 					// QoS 2 先发送 PUBREC 等待 PUBREL 再调用回调函数，这里暂时不实现完整的 QoS 2 流程
// 					return MQTT_RX_PUBLISH_QOS2_UNSUPPORTED; //处理了qos2的包，但还没有完成整个流程,当前就是当是2服务等级的时候返回错误
// 				}
// 			} else {
// 				return res; // 解析失败
// 			}
// }

// static int mqtt_handle_connack(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
// {
// 	if(m == NULL) {
// 		return MQTT_ERR_ARG; // Invalid MQTT control block
// 	}
// 	mqtt_connack_view_t view;
// 	int res = mqtt_parse_connack_view(rx, rx_len, &view);
	
// 	m->connack_rc = view.return_code;
// 	m->session_present = view.session_present;
// 	if (view.return_code == 0) {
// 		m->conn_state = MQTT_CONN_CONNECTED;
// 	} else {
// 		m->conn_state = MQTT_CONN_DISCONNECTED;
// 	}
// 	if(m->on_connack) {
// 		m->on_connack(m->user_ctx, &view);//需要注意的是这个回调是在return之前，所以就算是错的也要考虑
// 	}
// 	if(res < 0) {
// 		return res; // 解析失败
// 	}
// 	//这里可以考虑加回调
// 	return MQTT_RX_CONNACK; // 处理了 CONNACK 包
// }
// static int mqtt_handle_suback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
// {
// 	if(m == NULL) {
// 		return MQTT_ERR_ARG; // Invalid MQTT control block
// 	}
// 	mqtt_suback_view_t view;
// 	int res = mqtt_parse_suback_view(rx, rx_len, &view);
// 	if(res < 0) {
// 		return res; // 解析失败
// 	}
// 	if(view.message_id != m->last_subscribe_pid) {
// 		return MQTT_ERR_PID_MISMATCH; // SUBACK 的消息 ID 不匹配
// 	}
// 	if(m->on_suback) {
// 		m->on_suback(m->user_ctx, &view);
// 	}
// 	return MQTT_RX_SUBACK; // 处理了 SUBACK 包
// }
// static int mqtt_handle_pingresp(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
// {

// }
// static int mqtt_handle_puback(MQTT_TCB* m, const uint8_t* rx, uint32_t rx_len)
// {

// }
