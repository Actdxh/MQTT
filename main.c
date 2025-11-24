#include <stdio.h>
#include <stdlib.h>
#include "MQTT.h"
#include "cc.h"
#include <string.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int i;
int res;
u8 databuff[64];
u8 outbuff[64];

int Str_to_Hex(s8* indata, u8* outdata)
{
	int num = 0;
	s8 *str;
	s8 *endstr;
	
	str = indata;
	while(*str != '\0')
	{
		outdata[num] = strtol(str, &endstr, 16);
		num++;
		str = endstr;
	}
	
	return num;
} 

12
int main(int argc, char *argv[]) {
	MQTT_Init();
	printf("%s\r\n",mqtt.ClientID);
	printf("%s\r\n",mqtt.UserName);
	printf("%s\r\n",mqtt.Passward);
	MQTT_CONNECTWILL( 1, 2, 1, 100);
	gets(databuff);
	res = Str_to_Hex(databuff, outbuff);
	
	printf("%d\r\n",MQTT_CONNACK(outbuff, res));
//	for(i = 0; i < res; i++)
//	{
//		printf("%02x ",outbuff[i]);
//	}
//	printf("\r\n");
	return 0;
}
