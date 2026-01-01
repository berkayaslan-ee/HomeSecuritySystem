#include "SlaveManager.h"

_Bool clockFlag = false;

static stSlaveManager_t* slaveManager_GetInstance();
static void SlaveManager_rs485DataSet( stSlaveManager_t* me );


stSlaveManager_t* SlaveManager_Init(ADC_HandleTypeDef* hadc1,	I2C_HandleTypeDef* hi2c2, UART_HandleTypeDef* huart2, TIM_HandleTypeDef* htim3, TIM_HandleTypeDef* htim1)
{
	stSlaveManager_t* me = slaveManager_GetInstance();
	
	me->SensorController 	= sensorController_init(hadc1, htim1);
	me->RS485Controller 	= RS485Controller_Init(huart2);
	me->RtcController 		= rtcController_init(hi2c2);
	
	HAL_TIM_Base_Start_IT(htim3);
	
	return me;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
   clockFlag = true;
}

void SlaveManager_EventLoop(stSlaveManager_t* me)
{
	sensorController_eventLoop( me->SensorController );
	/*
	if( me->SensorController->mq7DigitalState || me->SensorController->panicButtonState || me->SensorController->rainSensorDigitalState )
	{
		HAL_GPIO_WritePin( BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET );
	}
	else HAL_GPIO_WritePin( BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET );*/
	if( clockFlag )
	{
		rtcController_eventloop( me->RtcController );
		SlaveManager_rs485DataSet( me );
		me->RS485Controller->sendDataFlag = true;
		RS485Controller_EventLoop( me->RS485Controller );
		clockFlag = false;
	}
}

static void SlaveManager_rs485DataSet( stSlaveManager_t* me )
{
	me->RS485Controller->rs485Packet.rs485Packet.reedState 	= me->SensorController->reedSwitchState;
	me->RS485Controller->rs485Packet.rs485Packet.pirState		= me->SensorController->pirSensorState;
	me->RS485Controller->rs485Packet.rs485Packet.panicState	= me->SensorController->panicButtonState;
	me->RS485Controller->rs485Packet.rs485Packet.rainState	= me->SensorController->rainSensorDigitalState;
	me->RS485Controller->rs485Packet.rs485Packet.mq7State		= me->SensorController->mq7DigitalState;
	me->RS485Controller->rs485Packet.rs485Packet.temp				= me->SensorController->temp;
	me->RS485Controller->rs485Packet.rs485Packet.humid			= me->SensorController->humid;
	me->RS485Controller->rs485Packet.rs485Packet.day				= me->RtcController->date;
	me->RS485Controller->rs485Packet.rs485Packet.month			= me->RtcController->month;
	me->RS485Controller->rs485Packet.rs485Packet.year				= me->RtcController->year;
	me->RS485Controller->rs485Packet.rs485Packet.hour				= me->RtcController->hour;
	me->RS485Controller->rs485Packet.rs485Packet.minute			= me->RtcController->minute;
	me->RS485Controller->rs485Packet.rs485Packet.second			= me->RtcController->second;
}

static stSlaveManager_t* slaveManager_GetInstance()
{
	static stSlaveManager_t slaveManager;
	static stSlaveManager_t* me = NULL;
	
	if(NULL == me)
	{
		me = &slaveManager;
	}
	return me;
}
