#include <stdio.h>
#include <string.h>
#include "mqtt_type.h"
#include "mqtt_pack.h"
#include "mqtt_utils.h"
#include "mqtt_parse.h"


int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive)
{
	uint16_t p = 0;
	uint16_t cid_len = m->param.ClientID ? strlen(m->param.ClientID) : 0;
	uint16_t user_len = m->param.UserName ? strlen(m->param.UserName) : 0;
	uint16_t pwd_len = m->param.Passward ? strlen(m->param.Passward) : 0;
	uint16_t willtopic_len = m->param.WillTopic ? strlen(m->param.WillTopic) : 0;
	uint16_t willdata_len = m->param.WillData ? strlen(m->param.WillData) : 0;
	uint16_t Fixedheader_len = 0;
	uint16_t Remaining_len = 0;
	uint16_t Variableheader_len = 0;
	uint16_t Load_len = 0;
	uint16_t Totallength = 0;
	
	/************************固定报头*************************/
	int res = 0;
	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
	Fixedheader_len = 1;
	Variableheader_len = 10;														//协议名长度2字节+协议名4字节+协议级别1字节+连接标志1字节+保持时间2字节=10字节
	Load_len = 2 + cid_len +											//前面的2指接下来的数据长度，目前不包含遗嘱
			   2 + user_len +
			   2 + pwd_len;
	if(m->param.WillEnable == 1) {
		Load_len += 2 + willtopic_len + 2 + willdata_len;		//如果遗嘱使能了则加上遗嘱主题和遗嘱数据的长度
	}
	Remaining_len = Variableheader_len + Load_len;			//剩余长度等于可变报头长度加上负载长度

	out[p++] = 0x10; 																			//CONNECT报文类型，标志位为0
	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
		return -1; // Invalid remaining length
	}
	p = p + rem_len_bytes;

	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
	if(out_size < Totallength) {
		return -1; // Output buffer is too small
	}

	{
		/************************可变报头*************************/ 
		/* 
		* 7.   User Name Flag	1
		* 6.   Password Flag	1
		* 5.   Will Retain		0
		* 4.   Will QoS			0
		* 3.   Will QoS			0
		* 2.   Will Flag		0
		* 1.   Clean Session	1
		* 0.   Reserved			0
		*/
		out[p++] = 0x00;																			//长度高八位 
		out[p++] = 0x04;																		//长度低八位 
		out[p++] = 0x4D;																		//"M" 
		out[p++] = 0x51;																		//"Q" 
		out[p++] = 0x54;																		//"T" 
		out[p++] = 0x54;																		//"T" 
		out[p++] = 0x04;																		//协议级别
		uint8_t connect_flags = 0;																//连接标志位
		if(user_len > 0) connect_flags |= 1<<7;										//用户名标志位
		if(pwd_len  > 0) connect_flags |= 1<<6;										//密码标志位
		if(m->param.WillEnable && m->param.WillRetain) connect_flags |= 1<<5;					//遗嘱保留标志位
		if(m->param.WillEnable) connect_flags |= (m->param.WillQoS & 0x03) << 3;				//遗嘱服务等级标志位
		if(m->param.WillEnable  ) connect_flags |= 1<<2;										//遗嘱标志位
		if(m->param.CleanSession) connect_flags |= 1<<1;										//连接会话清除标志位

		out[p++] = connect_flags;																//连接标志位
	}
	out[p++] = keepalive/256;									//保持时间高位 
	out[p++] = keepalive%256;								//保持时间高低位 
	/************************有效负载*************************/
	res = mqtt_write_str(out, out_size, &p, m->param.ClientID);
	if(res < 0) {
		return -1; // Error writing client ID
	}
	if(m->param.WillEnable == 1) {

		 res = mqtt_write_str(out, out_size, &p, m->param.WillTopic);
		 if(res < 0) {
			 return -1; // Error writing will topic
		 }

		 res = mqtt_write_str(out, out_size, &p, m->param.WillData);
		 if(res < 0) {
			 return -1; // Error writing will data
		 }
	}
	res = mqtt_write_str(out, out_size, &p, m->param.UserName);
	if(res < 0) {
		return -1; // Error writing user name
	}
	
	res = mqtt_write_str(out, out_size, &p, m->param.Passward);
	if(res < 0) {
		return -1; // Error writing password
	}
	#ifdef MQTT_DEBUG
	printf("%d\t", Fixedheader_len);
	printf("%d\t", Variableheader_len);
	printf("%d\t", Load_len);
	printf("%d\t", Remaining_len);
	printf("%d\t", Totallength);
	printf("%d\r\n", p);
	#endif
	if(p != Totallength) {
		return -1; // Packed length does not match expected total length
	}

	m->length.Fixedheader_len = Fixedheader_len;
	m->length.Variableheader_len = Variableheader_len;
	m->length.Load_len = Load_len;
	m->length.Remaining_len = Remaining_len;
	m->length.Totallength = Totallength;

	return m->length.Totallength; // Return the total length of the packed MQTT CONNECT message
}


int mqtt_pack_subscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, const char * topic, char qos)
{	
	if(topic == NULL) {
		return -1; // Invalid topic
	}
	if(qos < 0 || qos > 2) {
		return -1; // Invalid QoS
	}

	
	uint16_t p = 0;
	uint16_t Fixedheader_len = 0;
	uint16_t Remaining_len = 0;
	uint16_t Variableheader_len = 0;
	uint16_t Load_len = 0;
	uint16_t Totallength = 0;
	uint16_t topic_len = (uint16_t)strlen(topic);
	/************************固定报头*************************/
	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
	Fixedheader_len = 0;
	Variableheader_len = 2;
	Load_len = 2 + topic_len + 1;												//2:主题长度，1服务等级 
	Remaining_len = Variableheader_len + Load_len;
				
	out[p++] = 0x82; 
	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
		return -1; // Invalid remaining length
	}
	p = p + rem_len_bytes;

	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
	if(out_size < Totallength) {
		return -1; // Output buffer is too small
	}
	/************************可变报头*************************/ 
	
	out[p++] = m->MessageID/256;								//报文标识符高位 
	out[p++] = m->MessageID%256;								//报文标识符低位 
	m->last_subscribe_pid = m->MessageID;
	m->MessageID++;
	if(m->MessageID == 0)
	{
		m->MessageID = 1;
	} 

	/************************有效负载*************************/ 
	int res = mqtt_write_str(out, out_size, &p, topic);
	if(res < 0) {
		return -1; // Error writing topic
	}
	out[p++] = qos;
	
	if(p != Totallength) {
		return -1;
	}

	m->length.Fixedheader_len = Fixedheader_len;
	m->length.Variableheader_len = Variableheader_len;
	m->length.Load_len = Load_len;
	m->length.Remaining_len = Remaining_len;
	m->length.Totallength = Totallength;
	
	return Totallength;
}


int mqtt_pack_unsubscribe(MQTT_TCB *m, uint8_t* out, uint16_t out_size, char * topic)
{
	uint16_t p = 0;
	uint16_t Fixedheader_len = 0;
	uint16_t Remaining_len = 0;
	uint16_t Variableheader_len = 0;
	uint16_t Load_len = 0;
	uint16_t Totallength = 0;
	/************************固定报头*************************/ 
	Fixedheader_len = 1;
	uint16_t topic_len = (uint16_t)strlen(topic);
	int rem_len_bytes = 0;		//这是在固定报头里面的剩余长度的字节数
	Variableheader_len = 2;
	Load_len = 2 + topic_len;													//2:主题长度 
	Remaining_len = Variableheader_len + Load_len;
				
	out[p++] = 0xA2; 
	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
		return -1; // Invalid remaining length
	}
	p = p + rem_len_bytes;

	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
	if(out_size < Totallength) {
		return -1; // Output buffer is too small
	}
	
	/************************可变报头*************************/ 
	
	out[p++] = m->MessageID/256;								//报文标识符高位 
	out[p++] = m->MessageID%256;								//报文标识符低位 
	m->MessageID++;
	if(m->MessageID == 0)
	{
		m->MessageID = 1;
	}

	/************************有效负载*************************/ 
	int res = mqtt_write_str(out, out_size, &p, topic);
	if(res < 0) {
		return -1; // Error writing topic
	}
	if(p != Totallength) {
		return -1;
	}

	m->length.Fixedheader_len = Fixedheader_len;
	m->length.Variableheader_len = Variableheader_len;
	m->length.Load_len = Load_len;
	m->length.Remaining_len = Remaining_len;
	m->length.Totallength = Totallength;

	return Totallength;
}


int mqtt_pack_publish(
    MQTT_TCB* m,
    uint8_t* out,
    uint16_t out_size,
    const char* topic,
    const void* payload,
    uint16_t payload_len,
    uint8_t qos,       // 0/1（先不做2）
    uint8_t retain,    // 0/1
    uint8_t dup        // 0/1（qos=0 时 dup 也允许但一般没意义）
)
{

	if(topic == NULL || payload == NULL) {
		return -1; // Invalid input
	}
	if(qos >= 2) {
		return -1; // Invalid QoS
	}
	if(dup > 1) {
		return -1; // Invalid DUP flag
	}
	if(retain > 1) {
		return -1; // Invalid retain flag
	}
	
	uint16_t p = 0;
	uint16_t Fixedheader_len = 0;
	uint16_t Remaining_len = 0;
	uint16_t Variableheader_len = 0;
	uint16_t Load_len = 0;
	uint16_t Totallength = 0;
	/************************固定报头*************************/ 
	uint16_t topic_len = (uint16_t)strlen(topic);
	int rem_len_bytes = 0;;

	Fixedheader_len = 1;
	Variableheader_len = 2 + topic_len + ((qos > 0) ? 2 : 0);		//主题长度+主题内容+（如果服务等级大于0则加上报文标识符长度）
	Load_len = payload_len;
	Remaining_len = Variableheader_len + Load_len;
	out[p++] = 0x30 | (dup << 3) | (qos << 1)| (retain << 0);
	rem_len_bytes = mqtt_write_rem_len(out + p, Remaining_len);
	if((rem_len_bytes == 0) || (rem_len_bytes > 4)) {
		return -1; // Invalid remaining length
	}
	p = p + rem_len_bytes;
	
	Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	Totallength =  Fixedheader_len + Variableheader_len + Load_len;		//报文总长度 
	if(out_size < Totallength) {
		return -1; //这里的长度检测还有一个不足就是没有考虑要发送超级大数组导致超过uint16_t然后回卷导致判断是够的，当然也被实际的数组长度限制，这里暂时不做修改
	}
	/************************可变报头*************************/ 

	int res = mqtt_write_str(out, out_size, &p, topic);
	if(res < 0) {
		return -1; // Error writing topic
	}
	if(qos > 0) {
		out[p++] = m->MessageID/256;								//报文标识符高位 
		out[p++] = m->MessageID%256;								//报文标识符低位 
		m->MessageID++;
		if(m->MessageID == 0)
		{
			m->MessageID = 1;
		}
	} 
	/************************有效负载*************************/ 
	memcpy(out+p, payload, payload_len);
	p += payload_len;

	if(p != Totallength) {
		return -1;
	}

	m->length.Fixedheader_len = Fixedheader_len;
	m->length.Variableheader_len = Variableheader_len;
	m->length.Load_len = Load_len;
	m->length.Remaining_len = Remaining_len;
	m->length.Totallength = Totallength;

	return Totallength;
}

//提供一个更方便的接口，直接传入参数结构体
int mqtt_pack_publish_two(MQTT_TCB* m,uint8_t* out,uint16_t out_size, mqtt_publish_params_t *params)
{
	if(params == NULL) {
		return -1; // Invalid input
	}
	return mqtt_pack_publish(m, out, out_size, params->topic, params->payload, params->payload_len, params->qos, params->retain, params->dup);
}

void mqtt_pack_puback(MQTT_TCB* m, uint16_t messageid)
{
	m->buff[0] = 0x40;
	m->buff[1] = 0x02;
	m->buff[2] = messageid/256;
	m->buff[3] = messageid%256;
	
	m->length.Totallength = 4;
}