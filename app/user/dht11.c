/*
 * dht11.c
 *
 *  Created on: 2019��9��4��
 *      Author: xsj321
 */
#include "driver/dht11.h"

/**
 * @brief ������ʱ����
 * @param time ����1@see ��ʱʱ��
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
 * @brief DHT11�����źŷ���
 * @param PIN_NAME ����1@see ���ŵ�ַ �磺PERIPHS_IO_MUX_GPIO5_U
 * @param PIN ����2@see gpio���ź�
 * @return uchar ����ֵ: ���ض�ȡ�Ƿ�ɹ�
 *  -<em>0</em> ��ȡ�ɹ�
 *  -<em>1</em> ��ȡʧ��
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
 * @brief DHT11�����ź�Bitλ��ȡ
 * @param PIN ����1@see gpio���ź�
 * @return uchar ����ֵ: ��ȡ��bit
 *  -<em>0</em> ��ȡ���͵�ƽ��bitΪ"0"
 *  -<em>1</em> ��ȡ���ߵ�ƽ��bitΪ"1"
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
 * @brief DHT11�����ź�,����ȡ����Bitλ��ϳ�Byte
 * @param PIN ����1@see gpio���ź�
 * @return uchar ����ֵ @see �õ��� "1"Byte������
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
 * @brief ��ȡDHT11�����ݣ��ı䴫��ָ��ָ���Data�ṹ���е�Humi��Temp
 * @param Data ����1@see �������ݵĽṹ��
 * @param PIN_NAME ����2@see ���ŵ�ַ �磺PERIPHS_IO_MUX_GPIO5_U
 * @param PIN ����3@see gpio���ź�
 * @return uchar ����ֵ: ���ض�ȡ�Ƿ�ɹ�
 *  -<em>0</em> �ɹ�
 *  -<em>1</em> ʧ��
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





