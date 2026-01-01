#include "rtcController.h"

static void DS1307_Init(I2C_HandleTypeDef *hi2c);
static void DS1307_SetClockHalt(uint8_t halt);
static uint8_t DS1307_GetClockHalt(void);
static void DS1307_SetRegByte(uint8_t regAddr, uint8_t val);
static uint8_t DS1307_GetRegByte(uint8_t regAddr);
static uint8_t DS1307_GetDate(void);
static uint8_t DS1307_GetMonth(void);
static uint16_t DS1307_GetYear(void);
static uint8_t DS1307_GetHour(void);
static uint8_t DS1307_GetMinute(void);
static uint8_t DS1307_GetSecond(void);
static void DS1307_SetDate(uint8_t date);
static void DS1307_SetMonth(uint8_t month);
static void DS1307_SetYear(uint16_t year);
static void DS1307_SetHour(uint8_t hour_24mode);
static void DS1307_SetMinute(uint8_t minute);
static void DS1307_SetSecond(uint8_t second);
static void DS1307_SetTimeZone(int8_t hr, uint8_t min);
static uint8_t DS1307_DecodeBCD(uint8_t bin);
static uint8_t DS1307_EncodeBCD(uint8_t dec);

static stRtcController_t* rtcController_getInstance();
static void rtcController_rtcGet( stRtcController_t* me );

I2C_HandleTypeDef* _ds1307_ui2c;

stRtcController_t* rtcController_init( I2C_HandleTypeDef* hi2c1)
{
	stRtcController_t* me = rtcController_getInstance();

	me->rtcI2c	= hi2c1;
	
	DS1307_Init( me->rtcI2c );
	me->date 					= DS1307_GetDate();
	me->hour 					= DS1307_GetHour();
	me->minute 				= DS1307_GetMinute();
	me->month					= DS1307_GetMonth();
	me->second				= DS1307_GetSecond();
	me->year					= DS1307_GetYear();

	return me;
}

void rtcController_eventloop( stRtcController_t* me )
{
	rtcController_rtcGet( me );
}

static void rtcController_rtcGet( stRtcController_t* me )
{
	me->date 	= DS1307_GetDate();
	me->month = DS1307_GetMonth();
	me->year 	= DS1307_GetYear();
	if( me->year < 2024 )	me->year = 2025;
	if( me->year > 3000 ) me->year = 2025;
	me->hour = DS1307_GetHour();
	me->minute = DS1307_GetMinute();
	me->second = DS1307_GetSecond();
}

void rtcController_rtcSet( stRtcController_t* me )
{

	DS1307_SetTimeZone( +3, 00 );
	DS1307_SetDate( me->date );
	DS1307_SetMonth( me->month );
	if( ( me->year <= 2022 ) && ( me->year >= 2060 ) )	me->year = 2023;
	DS1307_SetYear( me->year );
	DS1307_SetHour( me->hour );
	DS1307_SetMinute( me->minute );
	DS1307_SetSecond( me->second );
}

static void DS1307_Init(I2C_HandleTypeDef *hi2c) {
	_ds1307_ui2c = hi2c;
	DS1307_SetClockHalt(0);
}


static void DS1307_SetClockHalt(uint8_t halt) {
	uint8_t ch = (halt ? 1 << 7 : 0);
	DS1307_SetRegByte(DS1307_REG_SECOND, ch | (DS1307_GetRegByte(DS1307_REG_SECOND) & 0x7f));
}


static uint8_t DS1307_GetClockHalt(void) {
	return (DS1307_GetRegByte(DS1307_REG_SECOND) & 0x80) >> 7;
}


static void DS1307_SetRegByte(uint8_t regAddr, uint8_t val) {
	uint8_t bytes[2] = { regAddr, val };
	HAL_I2C_Master_Transmit(_ds1307_ui2c, DS1307_I2C_ADDR << 1, bytes, 2, DS1307_TIMEOUT);
}


static uint8_t DS1307_GetRegByte(uint8_t regAddr) {
	uint8_t val;
	HAL_I2C_Master_Transmit(_ds1307_ui2c, DS1307_I2C_ADDR << 1, &regAddr, 1, DS1307_TIMEOUT);
	HAL_I2C_Master_Receive(_ds1307_ui2c, DS1307_I2C_ADDR << 1, &val, 1, DS1307_TIMEOUT);
	return val;
}


static uint8_t DS1307_GetDate(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_DATE));
}


static uint8_t DS1307_GetMonth(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_MONTH));
}


static uint16_t DS1307_GetYear(void) {
	uint16_t cen = DS1307_GetRegByte(DS1307_REG_CENT) * 100;
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_YEAR)) + cen;
}


static uint8_t DS1307_GetHour(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_HOUR) & 0x3f);
}


static uint8_t DS1307_GetMinute(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_MINUTE));
}


static uint8_t DS1307_GetSecond(void) {
	return DS1307_DecodeBCD(DS1307_GetRegByte(DS1307_REG_SECOND) & 0x7f);
}

static void DS1307_SetDate(uint8_t date) {
	DS1307_SetRegByte(DS1307_REG_DATE, DS1307_EncodeBCD(date));
}


static void DS1307_SetMonth(uint8_t month) {
	DS1307_SetRegByte(DS1307_REG_MONTH, DS1307_EncodeBCD(month));
}


static void DS1307_SetYear(uint16_t year) {
	DS1307_SetRegByte(DS1307_REG_CENT, year / 100);
	DS1307_SetRegByte(DS1307_REG_YEAR, DS1307_EncodeBCD(year % 100));
}


static void DS1307_SetHour(uint8_t hour_24mode) {
	DS1307_SetRegByte(DS1307_REG_HOUR, DS1307_EncodeBCD(hour_24mode & 0x3f));
}


static void DS1307_SetMinute(uint8_t minute) {
	DS1307_SetRegByte(DS1307_REG_MINUTE, DS1307_EncodeBCD(minute));
}


static void DS1307_SetSecond(uint8_t second) {
	uint8_t ch = DS1307_GetClockHalt();
	DS1307_SetRegByte(DS1307_REG_SECOND, DS1307_EncodeBCD(second | ch));
}


static void DS1307_SetTimeZone(int8_t hr, uint8_t min) {
	DS1307_SetRegByte(DS1307_REG_UTC_HR, hr);
	DS1307_SetRegByte(DS1307_REG_UTC_MIN, min);
}


static uint8_t DS1307_DecodeBCD(uint8_t bin) {
	return (((bin & 0xf0) >> 4) * 10) + (bin & 0x0f);
}


static uint8_t DS1307_EncodeBCD(uint8_t dec) {
	return (dec % 10 + ((dec / 10) << 4));
}

static stRtcController_t* rtcController_getInstance()
{
	static stRtcController_t rtcController;
	static stRtcController_t* me = NULL;

	if( NULL == me )
	{
		return &rtcController;
	}

	return me;
}
