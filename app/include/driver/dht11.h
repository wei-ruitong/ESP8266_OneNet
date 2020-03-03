/*
 * dht11.h
 *
 *  Created on: 2019Äê9ÔÂ4ÈÕ
 *      Author: xsj321
 */

#ifndef DHT11_APP_H_
#define DHT11_APP_H_
#include "os_type.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#define uchar unsigned char

typedef struct
{
	uchar Humi_H;
	uchar Humi_L;
	uchar Temp_H;
	uchar Temp_L;
	int Temp;
	int Humi;
	uchar Check_Sum;
}DHT11_Data;


ICACHE_FLASH_ATTR uchar DHT11_Start(int PIN_NAME,int PIN);
ICACHE_FLASH_ATTR uchar DHT11_Read_Bit(int PIN);
ICACHE_FLASH_ATTR uchar DHT11_Read_Byte(int PIN);
ICACHE_FLASH_ATTR uchar DHT11_Read_Data(DHT11_Data *Data,int PIN_NAME,int PIN);


#endif /* APP_INCLUDE_DRIVER_DHT11_H_ */
