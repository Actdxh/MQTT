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
