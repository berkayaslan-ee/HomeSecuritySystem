#ifndef RTC_CONTROLLER_H
#define RTC_CONTROLLER_H

#include "main.h"

#define DS1307_I2C_ADDR 		0x68
#define DS1307_REG_SECOND 	0x00
#define DS1307_REG_MINUTE 	0x01
#define DS1307_REG_HOUR  		0x02
#define DS1307_REG_DOW    	0x03
#define DS1307_REG_DATE   	0x04
#define DS1307_REG_MONTH  	0x05
#define DS1307_REG_YEAR   	0x06
#define DS1307_REG_CONTROL 	0x07
#define DS1307_REG_UTC_HR		0x08
#define DS1307_REG_UTC_MIN	0x09
#define DS1307_REG_CENT    	0x10
#define DS1307_REG_RAM   		0x11
#define DS1307_TIMEOUT			1000

typedef enum DS1307_Rate{
	DS1307_1HZ, DS1307_4096HZ, DS1307_8192HZ, DS1307_32768HZ
} DS1307_Rate;

typedef enum DS1307_SquareWaveEnable{
	DS1307_DISABLED, DS1307_ENABLED
} DS1307_SquareWaveEnable;

typedef struct
{
	uint8_t date;
	uint8_t month;
	uint16_t year;
	uint8_t dow;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	I2C_HandleTypeDef* rtcI2c;

	_Bool timerFlag;
	_Bool timerSettingFlag;

}stRtcController_t;

stRtcController_t* rtcController_init( I2C_HandleTypeDef* hi2c1);
void rtcController_eventloop( stRtcController_t* me );
void rtcController_rtcSet( stRtcController_t* me );


#endif
