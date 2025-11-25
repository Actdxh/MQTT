#include "main.h"
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
u8 QoS;
u32 messageid;


int main(int argc, char *argv[]) {
	MQTT_Init();	
	
	processpubcomp();



	printf("\r\n");
	return 0;
}



int Str_to_Hex(s8* indata, u8* outdata)
{
	int num = 0;
	s8 *str;
	s8 *endstr;
	
	str = indata;
	while(*str != '\0')
	{
		outdata[num] = strtol(str, (char**)&endstr, 16);
		num++;
		str = endstr;
	}
	
	return num;
}

