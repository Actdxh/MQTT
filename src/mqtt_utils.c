#include "mqtt_utils.h"
#include "string.h"
#include "stdio.h"

int Str_to_Hex(char* indata, uint8_t* outdata)
{
	int num = 0;
	char *str;
	char *endstr;
	
	str = indata;
	while(*str != '\0')
	{
		outdata[num] = strtol(str, (char**)&endstr, 16);
		num++;
		str = endstr;
	}
	
	return num;
}

uint8_t mqtt_write_rem_len(uint8_t* out, uint32_t rem_len)
{
	int rem_len_bytes = 0;
	do{
		if(rem_len/128 == 0)													//不需要进位 
		{
			out[rem_len_bytes] = rem_len; 
		}else
		{
			out[rem_len_bytes] = (rem_len % 128) | 0x80; 
		}
		rem_len_bytes ++;
		rem_len = rem_len/128; 
	}while(rem_len);
	return rem_len_bytes;
}

int mqtt_write_str(uint8_t* out, uint16_t out_size, uint16_t *p, const char* s)
{
	uint16_t s_len = strlen(s);
	if((out_size - *p) < 2 + s_len) {
		return -1; // Output buffer is too small
	}
	out[*p] = s_len/256;
	out[*p+1] = s_len%256;
	memcpy(&out[*p+2], s, s_len);
	*p = *p + 2 + s_len;

	return 0;
}

int mqtt_read_rem_len(const uint8_t* in,uint32_t in_len,uint32_t* rem_len,uint8_t* rem_len_bytes)
{
	if((in == NULL) || (rem_len == NULL) || (rem_len_bytes == NULL) || (in_len == 0))
	{
		return -1; // 参数无效
	}
	uint32_t multiplier = 1;
	*rem_len = 0;
	*rem_len_bytes = 0;
	int i; 
	for( i = 0; i < 4; i++) {
		if(i >= in_len) {
			return -2; // 输入数据不足
		}
		(*rem_len_bytes) ++;
		*rem_len += (in[i] & 0x7F) * multiplier;
		multiplier *= 128;
		if((in[i] & 0x80) == 0) break;
		if((i == 3) && (in[i] & 0x80)) {
			return -3; // 格式非法，剩余长度超过4字节
		}
	}
	if(*rem_len_bytes > 4) {
		return -3; // 格式非法
	}
	return 0;
}
//为了测试mqtt_read_rem_len函数，可以把下面这段粘贴到main函数里
/*

	printf("TEST\r\n");
	uint32_t rem;
	uint8_t n;

	uint8_t a[] = {0x7F};
	res = mqtt_read_rem_len(a, sizeof(a), &rem, &n);   // 应该成功 rem=127 n=1
	printf("res = %d, rem = %u, n = %u\r\n", res, rem, n);

	uint8_t b[] = {0xC1, 0x02};
	res = mqtt_read_rem_len(b, sizeof(b), &rem, &n);   // 应该成功 rem=321 n=2
	printf("res = %d, rem = %u, n = %u\r\n", res, rem, n);

	uint8_t c[] = {0x80};
	res = mqtt_read_rem_len(c, sizeof(c), &rem, &n);   // 应该成功 rem=128 n=2
	printf("res = %d, rem = %u, n = %u\r\n", res, rem, n);

*/
