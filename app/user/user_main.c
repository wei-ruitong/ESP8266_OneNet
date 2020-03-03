/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
# include <stdio.h>
# include <stdlib.h>
#include "../../app/include/driver/uart.h"
#include "../../app/include/modules/config.h"
#include "../../app/include/modules/wifi.h"
#include "../../app/include/mqtt/debug.h"
#include "../../app/include/mqtt/mqtt.h"
#include "user_interface.h"
#include "mem.h"
#include "driver/dht11.h"
#include "sntp.h"

#define SPI_FLASH_SIZE_MAP 2

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif
os_timer_t OS_Timer_1;
MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;
const char* p;
char temp_humi[4];
void mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;

    INFO("MQTT: Published\r\n");
}
void ICACHE_FLASH_ATTR OS_Timer_1_cb(void)
{
	DHT11_Data Data;
	if(DHT11_Read_Data(&Data,PERIPHS_IO_MUX_GPIO2_U,2) == 0)
	{
		os_printf("\n read ok \n");
		os_printf("\n Humi: %d\t Temp: %d\n",Data.Humi,Data.Temp);
		//在这里完成温度数据的发布，10s上传一次
		//strcpy(tm_hu,Data.Humi);
		//itoa(Data.Humi,tm_hu,10);
		//sprintf(tm_hu,"%d",Data.Humi);
		//p=(char*)&(Data.Humi);
		temp_humi[0]=Data.Temp/10+'0';
		temp_humi[1]=Data.Temp%10+'0';
		temp_humi[2]=Data.Humi/10+'0';
		temp_humi[3]=Data.Humi%10+'0';
		MQTT_Publish(&mqttClient, "temp_humi",temp_humi,4, 0, 0);
	}
	else
		os_printf("\n\n\t\t read failed \n\n");
}

void ICACHE_FLASH_ATTR OS_Timer_1_Init_JX(u32 time_ms, u8 time_repetitive)
{

	os_timer_disarm(&OS_Timer_1);	// �رն�ʱ��
	os_timer_setfn(&OS_Timer_1,(os_timer_func_t *)OS_Timer_1_cb, NULL);	// ���ö�ʱ��
	os_timer_arm(&OS_Timer_1, time_ms, time_repetitive);  // ʹ�ܶ�ʱ��
}
void sntpfn()
{
    u32_t ts = 0;
    ts = sntp_get_current_timestamp();
    os_printf("current time : %s\n", sntp_get_real_time(ts));
    if (ts == 0) {
        //os_printf("did not get a valid time from sntp server\n");
    } else {
            os_timer_disarm(&sntp_timer);
            MQTT_Connect(&mqttClient);
    }
}

void wifiConnectCb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        sntp_setservername(0, "pool.ntp.org");        // set sntp server after got ip address
        sntp_init();
        os_timer_disarm(&sntp_timer);
        os_timer_setfn(&sntp_timer, (os_timer_func_t *)sntpfn, NULL);
        os_timer_arm(&sntp_timer, 1000, 1);//1s
    } else {
          MQTT_Disconnect(&mqttClient);
    }
}

void mqttConnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    INFO("MQTT: Connected123\r\n");
    MQTT_Subscribe(client, "/mqtt/topic/0", 0);
//    MQTT_Subscribe(client, "/mqtt/topic/1", 1);
//    MQTT_Subscribe(client, "/mqtt/topic/2", 2);

  // MQTT_Publish(client, "hellow", "hello1", 6, 1, 0);
//    MQTT_Publish(client, "/mqtt/topic/2", "hello2", 6, 2, 0);

}

void mqttDisconnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;

    INFO("MQTT: Disconnected111\r\n");
}



void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1),
            *dataBuf = (char*)os_zalloc(data_len+1);

    MQTT_Client* client = (MQTT_Client*)args;

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    INFO("Topic_len = %d, Data_len = %d\r\n", topic_len, data_len);//串口打印主题长度、有效载荷长度
    if( os_strcmp(topicBuf,"/mqtt/topic/0") == 0 )
       {
       	if( os_strcmp(dataBuf,"1") == 0 )	// 有效载荷是"LED_ON"
       	{
       	    //这里开灯的操作
       	 INFO("LED--ON");
       	GPIO_OUTPUT_SET(GPIO_ID_PIN(0),1);
       	MQTT_Publish(&mqttClient, "LED_STATUS","1",1, 0, 0);
       	}

       	else if( os_strcmp(dataBuf,"0") == 0 )	// 有效载荷 == "LED_OFF"
       	{
       	    //这里灭灯的操作
       	 INFO("LED--OFF");
     	GPIO_OUTPUT_SET(GPIO_ID_PIN(0),0);
     	MQTT_Publish(&mqttClient, "LED_STATUS","0",1, 0, 0);
       	}

       }
    os_free(topicBuf);
    os_free(dataBuf);
}

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);
    CFG_Load();
    WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
    MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
    MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
    OS_Timer_1_Init_JX(10000,1);
    MQTT_OnConnected(&mqttClient, mqttConnectedCb);
    MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
    MQTT_OnPublished(&mqttClient, mqttPublishedCb);
    MQTT_OnData(&mqttClient, mqttDataCb);
    INFO("\r\nSystem started ...\r\n");
}
