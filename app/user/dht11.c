/*
 * dht11.c
 *
 *  Created on: 2019年9月4日
 *      Author: xsj321
 */
#include "driver/dht11.h"

/**
 * @brief 毫秒延时函数
 * @param time 参数1@see 延时时间
 */
ICACHE_FLASH_ATTR void
dht_delay_ms(int time)
{
	for(;time > 0 ;time--)
	{
		os_delay_us(1000);
	}
}

/**
 * @brief DHT11启动信号发出
 * @param PIN_NAME 参数1@see 引脚地址 如：PERIPHS_IO_MUX_GPIO5_U
 * @param PIN 参数2@see gpio引脚号
 * @return uchar 返回值: 返回读取是否成功
 *  -<em>0</em> 读取成功
 *  -<em>1</em> 读取失败
 */
ICACHE_FLASH_ATTR uchar
DHT11_Start(int PIN_NAME,int PIN)
{
	uchar retry = 0;
	PIN_FUNC_SELECT(PIN_NAME,0);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(PIN),1);
	dht_delay_ms(1);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(PIN),0);
	dht_delay_ms(25);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(PIN),1);
	os_delay_us(5);

	PIN_FUNC_SELECT(PIN_NAME,0);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(PIN));

	while(GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && (retry<50))
	{
		retry++;
		os_delay_us(1);
	}
	if(retry>=50)
	{
		os_printf("\n\nStart timeout\n\n");
		return 1;
	}
	else
		retry = 0;


	while((!GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && (retry<170)))
	{
		retry++;
		os_delay_us(1);
	}

	if(retry>=120)
	{
		os_printf("\n\nStart timeout\n\n");
		return 1;
	}
	else
		retry = 0;
}

/**
 * @brief DHT11返回信号Bit位读取
 * @param PIN 参数1@see gpio引脚号
 * @return uchar 返回值: 读取的bit
 *  -<em>0</em> 读取到低电平该bit为"0"
 *  -<em>1</em> 读取到高电平该bit为"1"
 */
ICACHE_FLASH_ATTR uchar DHT11_Read_Bit(int PIN)
{
	uchar retry = 0;
	while(GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && (retry<120))
	{
		retry++;
		os_delay_us(1);
	}
	retry = 0;
	while(!GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && (retry<120))
	{
		retry++;
		os_delay_us(1);
	}
	os_delay_us(45);

	if(GPIO_INPUT_GET(GPIO_ID_PIN(PIN)))
	{
		return 1;
	}
	else
		return 0;

}

/**
 * @brief DHT11返回信号,将读取到的Bit位组合成Byte
 * @param PIN 参数1@see gpio引脚号
 * @return uchar 返回值 @see 得到的 "1"Byte的数据
 */
ICACHE_FLASH_ATTR uchar DHT11_Read_Byte(int PIN)
{
	uchar i,dat;
	dat = 0;
	for(i=0;i<8;i++)
	{
		dat<<=1;
		dat|=DHT11_Read_Bit(PIN);
	}
	return dat;
}

/**
 * @brief 读取DHT11的数据，改变传入指针指向的Data结构体中的Humi与Temp
 * @param Data 参数1@see 储存数据的结构体
 * @param PIN_NAME 参数2@see 引脚地址 如：PERIPHS_IO_MUX_GPIO5_U
 * @param PIN 参数3@see gpio引脚号
 * @return uchar 返回值: 返回读取是否成功
 *  -<em>0</em> 成功
 *  -<em>1</em> 失败
 */
ICACHE_FLASH_ATTR uchar DHT11_Read_Data(DHT11_Data *Data,int PIN_NAME,int PIN)
{
	uchar retry = 0;
	DHT11_Start(PIN_NAME,PIN);
	Data->Humi_H = DHT11_Read_Byte(PIN);
	Data->Humi_L = DHT11_Read_Byte(PIN);
	Data->Temp_H = DHT11_Read_Byte(PIN);
	Data->Temp_L = DHT11_Read_Byte(PIN);
	Data->Check_Sum = DHT11_Read_Byte(PIN);


	while(GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && retry<120)
	{
		retry++;
		os_delay_us(1);
	}
	retry = 0;
	while(!GPIO_INPUT_GET(GPIO_ID_PIN(PIN)) && retry<120)
	{
		retry++;
		os_delay_us(1);
	}

	if(retry>=120)
	{
		os_printf("flinsh fail\n\n");
		return 1;
	}
	else
		retry = 0;

	GPIO_OUTPUT_SET(PIN,1);
	if(Data->Check_Sum == Data->Humi_H + Data->Humi_L + Data->Temp_H + Data->Temp_L)
	{
		os_printf("Check_Sum macth!\n");
		Data->Humi = (int)Data->Humi_H;
		Data->Temp = (int)Data->Temp_H;
		return 0;
	}
	else
		return 1;

}





