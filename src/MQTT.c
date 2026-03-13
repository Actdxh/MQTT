#include "MQTT.h"
#include "string.h"
#include "stdio.h"
#include "mqtt_utils.h"
#include <stdint.h>

#define MQTT_DEBUG


int MQTT_Init(MQTT_TCB *m, const MQTT_config_t *config)
{
	if(m == NULL || config == NULL) {
		return -1; // Invalid input
	}
	if(m->param.WillQoS > 2) {
		return -1; // Invalid Will QoS
	}
	memset(m, 0, sizeof(*m));
	m->param = *config;
	m->length.cid_len = strlen(m->param.ClientID);
	m->length.user_len = strlen(m->param.UserName);
	m->length.pwd_len = strlen(m->param.Passward);
	m->length.willtopic_len = strlen(m->param.WillTopic);
	m->length.willdata_len = strlen(m->param.WillData);
	//m->length.topic_len = strlen(m->topic);
	m->MessageID = 1;

	return 0;
}

int mqtt_pack_connect(MQTT_TCB*m, uint8_t* out, uint16_t out_size, uint16_t keepalive)
{
	uint16_t p = 0;
	uint16_t cid_len = strlen(m->param.ClientID);
	uint16_t user_len = strlen(m->param.UserName);
	uint16_t pwd_len = strlen(m->param.Passward);
	uint16_t willtopic_len = strlen(m->param.WillTopic);
	uint16_t willdata_len = strlen(m->param.WillData);
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

int MQTT_CONNECT(MQTT_TCB *m, u32 keepalive)														//keepalive是保持时间 
{
	return mqtt_pack_connect(m, m->buff, BUFF_SIZE, keepalive);
}


char MQTT_CONNACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x20))
	{
		
	}else
	{
		return -1;
	}
	return rxdata[3];
} 

/************************DISCONNECT函数*************************/ 
void MQTT_DISCONNECT(MQTT_TCB *m)
{
	m->buff[0] = 0xE0;
	m->buff[1] = 0x00;
	m->length.Totallength = 2;
}

/************************SUBSCRIBE函数*************************/ 

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

int MQTT_SUBSCRIBE(MQTT_TCB *m, char* topic, char QS)												//一次制订阅一个主题 
{
	mqtt_pack_subscribe(m, m->buff, BUFF_SIZE, topic, QS);
}

/************************SUBACK函数*************************/ 
char MQTT_SUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 5) && (rxdata[0] == 0x90))
	{
	}else
	{
		return -1;
	}
	return rxdata[4];																	//服务等级 
} 

/************************UNSUBSCRIBE函数*************************/ 

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

int MQTT_UNSUBSCRIBE(MQTT_TCB *m, char* topic)												//一次制订阅一个主题
{
	return mqtt_pack_unsubscribe(m, m->buff, BUFF_SIZE, topic);
}

/************************SUBACK函数*************************/ 
char MQTT_UNSUBACK(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 4) && (rxdata[0] == 0xB0))
	{
	}else
	{
		return -1;
	}
	return 1;
} 

/************************PINGREQ函数*************************/ 
void MQTT_PINGREQ(MQTT_TCB *m)
{
	m->buff[0] = 0xC0;
	m->buff[1] = 0x00;
	m->length.Totallength = 2;
}

/************************PINGRESP函数*************************/ 
char MQTT_PINGRESP(MQTT_TCB *m, u8* rxdata, u32 rxdata_len)
{
	if((rxdata_len == 2) && (rxdata[0] == 0xD0))
	{
	}else
	{
		return -1;
	}
	return 1;
} 

/************************PUBLISH0函数*************************/ 
void MQTT_PUBLISH0(MQTT_TCB *m, char retain, char* topic, u8 *data, u32 data_len)
{
	/************************固定报头*************************/ 
	m->length.Fixedheader_len = 1;
	uint16_t topic_len = (uint16_t)strlen(topic);
	int rem_len_bytes = 0;
	uint16_t p = m->length.Fixedheader_len;
	m->length.Variableheader_len = 2 + topic_len;
	m->length.Load_len = data_len;		
	m->length.Remaining_len = m->length.Variableheader_len + m->length.Load_len;	
			
	m->buff[0] = 0x30 | (retain << 0); 
	do{
		if(m->length.Remaining_len/128 == 0)													//不需要进位 
		{
			m->buff[p] = m->length.Remaining_len; 
		}else
		{
			m->buff[p] = (m->length.Remaining_len % 128) | 0x80; 
		}
		rem_len_bytes ++;
		p++;
		m->length.Remaining_len = m->length.Remaining_len/128; 
	}while(m->length.Remaining_len);
	
	/************************可变报头*************************/ 
	m->buff[p] = topic_len/256;
	m->buff[p+1] = topic_len%256;	
	memcpy(&m->buff[p+2], topic, topic_len);
	p = p + 2 + topic_len;

	/************************有效负载*************************/ 
	
	memcpy(&m->buff[p], data, data_len);
	
	
	m->length.Fixedheader_len = 1 + rem_len_bytes;													//固定报头最终长度（包括剩余长度）
	
	m->length.Totallength =  m->length.Fixedheader_len + m->length.Variableheader_len + m->length.Load_len;		//报文总长度 
}

/************************PUBLISH函数*************************/ 

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


int MQTT_PUBLISH(MQTT_TCB *m, char dup, char QoS, char retain, char* topic, void *data, u32 data_len)
{
	return mqtt_pack_publish(m, m->buff, BUFF_SIZE, topic, data, data_len, QoS, retain, dup);
}


/************************处理服务器推送的PUBLISH相关函数*************************/ 
char MQTT_ProcessPUBLISH(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u8 *qs, u32* messageid)
{
	char i;
	u32 topic_len;
	u32 data_len;
	
	if((rxdata[0] & 0xF0) == 0x30)
	{
		for(i = 1; i <5; i++)															//确定剩余长度，与上第八位，剩余长度最多四个字节所以是5 
		{
			if((rxdata[i] & 0x80) == 0)
			{
				break;
			} 
		}
		switch(rxdata[0] & 0x06)
		{
			case 0x00:
						*qs = 0;
						*messageid = 0;													//qs等级是0就没有报文标识符 
						topic_len = rxdata[1 + i] * 256 + rxdata[1 + i + 1];
						data_len = rxdata_len - 1 - i - 2 - topic_len;
						memset(m->topic, 0, TOPIC_SIZE);
						memset(m->data, 0, DATA_SIZE);
						memcpy(m->topic, &rxdata[1 + i], topic_len + 2);				//这里是直接把接收的主题按照mqtt的格式复制，然后加2的原因是要把最前面的长度算上也就是长度+主题
						m->data[0] = data_len/256;
						m->data[1] = data_len%256;
						memcpy(&m->data[2], &rxdata[1 + i + 2 + topic_len], data_len);
						break;
			case 0x02:
						*qs = 1;
						topic_len = rxdata[1 + i] * 256 + rxdata[1 + i + 1];
						*messageid = rxdata[1 + i + 2 + topic_len] * 256 + rxdata[1 + i + 2 + topic_len + 1];
						data_len = rxdata_len - 1 - i - 2 - topic_len - 2;
						memset(m->topic, 0, TOPIC_SIZE);
						memset(m->data, 0, DATA_SIZE);
						memcpy(m->topic, &rxdata[1 + i], topic_len + 2);				//这里是直接把接收的主题按照mqtt的格式复制，然后加2的原因是要把最前面的长度算上也就是长度+主题
						m->data[0] = data_len/256;
						m->data[1] = data_len%256;
						memcpy(&m->data[2], &rxdata[1 + i + 2 + topic_len + 2], data_len);
						break;
			case 0x04:
						*qs = 2;
						topic_len = rxdata[1 + i] * 256 + rxdata[1 + i + 1];
						*messageid = rxdata[1 + i + 2 + topic_len] * 256 + rxdata[1 + i + 2 + topic_len + 1];
						data_len = rxdata_len - 1 - i - 2 - topic_len - 2;
						memset(m->topic, 0, TOPIC_SIZE);
						memset(m->data, 0, DATA_SIZE);
						memcpy(m->topic, &rxdata[1 + i], topic_len + 2);				//这里是直接把接收的主题按照mqtt的格式复制，然后加2的原因是要把最前面的长度算上也就是长度+主题
						m->data[0] = data_len/256;
						m->data[1] = data_len%256;
						memcpy(&m->data[2], &rxdata[1 + i + 2 + topic_len + 2], data_len);
						break;
						
		}
	}else
	{
		return -1;
	}
	return i;
} 

/************************PUBAC函数*************************/ 
void MQTT_PUBACK(MQTT_TCB *m, u32 messageid)
{
	m->buff[0] = 0x40;
	m->buff[1] = 0x02;
	m->buff[2] = messageid/256;
	m->buff[3] = messageid%256;
	
	m->length.Totallength = 4;
}

/************************处理服务器推送的PUBLISHACK相关函数*************************/ 
char MQTT_ProcessPublish(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x40))
	{
		*messageid = rxdata[2]*256 + rxdata[3];
	}else
	{
		return -1;
	}
	return 1;
} 

/************************PUBREC函数*************************/ 
void MQTT_PUBREC(MQTT_TCB *m, u32 messageid)
{
	m->buff[0] = 0x50;
	m->buff[1] = 0x02;
	m->buff[2] = messageid/256;
	m->buff[3] = messageid%256;
	
	m->length.Totallength = 4;
}

/************************处理服务器收到qs2之后返回的PUBREC相关函数*************************/ 
char MQTT_ProcessPUBREC(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x50))
	{
		*messageid = rxdata[2]*256 + rxdata[3];
	}else
	{
		return -1;
	}
	return 1;
} 

/************************PUBREL函数*************************/ 
void MQTT_PUBREL(MQTT_TCB *m, u32 messageid)
{
	m->buff[0] = 0x62;
	m->buff[1] = 0x02;
	m->buff[2] = messageid/256;
	m->buff[3] = messageid%256;
	
	m->length.Totallength = 4;
}

/************************处理服务器收到qs2之后返回的PUBREC相关函数*************************/ 
char MQTT_ProcessPUBREL(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x62))
	{
		*messageid = rxdata[2]*256 + rxdata[3];
	}else
	{
		return -1;
	}
	return 1;
} 

/************************PUBCOMP函数*************************/ 
void MQTT_PUBCOMP(MQTT_TCB *m, u32 messageid)
{
	m->buff[0] = 0x70;
	m->buff[1] = 0x02;
	m->buff[2] = messageid/256;
	m->buff[3] = messageid%256;
	
	m->length.Totallength = 4;
}

/************************处理服务器收到qs2之后返回的PUBCOMP相关函数*************************/ 
char MQTT_ProcessPUBCOMP(MQTT_TCB *m, u8* rxdata, u32 rxdata_len, u32* messageid)
{
	if((rxdata_len == 4) && (rxdata[0] == 0x70))
	{
		*messageid = rxdata[2]*256 + rxdata[3];
	}else
	{
		return -1;
	}
	return 1;
} 


