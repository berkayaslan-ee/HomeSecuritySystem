#ifndef SLAVE_MANAGER_H
#define SLAVE_MANAGER_H

#include "main.h"
#include "RS485Controller.h"
#include "SensorController.h"
#include "rtcController.h"

typedef struct 
{
	ADC_HandleTypeDef* hadc1;
	I2C_HandleTypeDef* hi2c2;
	UART_HandleTypeDef* huart2;
	TIM_HandleTypeDef* htim3;
	
	stSensorController_t* SensorController;
	stRs485Controller_t* RS485Controller;
	stRtcController_t* RtcController;
	
}stSlaveManager_t;


stSlaveManager_t* SlaveManager_Init(ADC_HandleTypeDef* hadc1,	I2C_HandleTypeDef* hi2c2, UART_HandleTypeDef* huart2, TIM_HandleTypeDef* htim3, TIM_HandleTypeDef* htim1);
void SlaveManager_EventLoop(stSlaveManager_t* me);
void SlaveManager_HandlePacket(stSlaveManager_t* me, const stRs485Packet_t* pPacket);


#endif
