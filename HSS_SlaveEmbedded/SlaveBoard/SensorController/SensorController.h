#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

//#include "stm32g0xx_hal.h"
#include "main.h"

// Sensör esik degerleri ve timeoutlar
#define MQ7_THRESHOLD           				3500 
#define RAIN_HIGH_THRESHOLD        			2000


typedef struct
{
	uint16_t 					gasSensorAnalogValue;
	uint16_t 					rainSensorAnalogValue;

	uint8_t 	reedSwitchState;
	uint8_t 	pirSensorState;
	uint8_t 	panicButtonState;
	uint8_t 	rainSensorDigitalState;
	uint8_t		mq7DigitalState;
	uint8_t 	buzzerState;

	ADC_HandleTypeDef* hadc1;
	TIM_HandleTypeDef* dhtTimer;
	
	float temp;
	float humid;
	
} stSensorController_t;



stSensorController_t* sensorController_init(ADC_HandleTypeDef* hadc1, TIM_HandleTypeDef* dhtTimer);
void sensorController_eventLoop(stSensorController_t* me);





#endif 
