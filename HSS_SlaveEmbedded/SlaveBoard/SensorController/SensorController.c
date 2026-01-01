#include "SensorController.h"

static stSensorController_t* SensorController_getInstance();
static void sensorController_getState( stSensorController_t* me );

DHT_t am2301;

stSensorController_t* sensorController_init(ADC_HandleTypeDef* hadc1, TIM_HandleTypeDef* dhtTimer)
{
	stSensorController_t* me = SensorController_getInstance();
	
	me->dhtTimer								= dhtTimer;
	me->hadc1 								 	= hadc1;
	me->reedSwitchState        	= false;
	me->pirSensorState      	 	= false;
	me->panicButtonState       	= false;
	me->rainSensorDigitalState 	= false;
	me->mq7DigitalState					= false;
	me->buzzerState							= false;
	
	DHT_init( &am2301, DHT_Type_AM2301, me->dhtTimer, 72, DHT_GPIO_Port, DHT_Pin);
	
	return me;
}

void sensorController_eventLoop( stSensorController_t* me )
{
	sensorController_getState( me );
}

static void sensorController_getState( stSensorController_t* me )
{
	if( HAL_GPIO_ReadPin( REED_SWITCH_GPIO_Port, REED_SWITCH_Pin ) == GPIO_PIN_RESET )		me->reedSwitchState = true;
	else																																									me->reedSwitchState = false;
		
	if( HAL_GPIO_ReadPin( PIR_GPIO_Port, PIR_Pin ) == GPIO_PIN_SET )											me->pirSensorState	= true;
	else																																									me->pirSensorState	= false;
	
	if( HAL_GPIO_ReadPin( PANIK_BUTTON_GPIO_Port, PANIK_BUTTON_Pin ) == GPIO_PIN_RESET )	me->panicButtonState = true;
	else																																									me->panicButtonState = false;
	
	HAL_ADC_Start(me->hadc1);
	
	HAL_ADC_PollForConversion(me->hadc1, 100);
	me->gasSensorAnalogValue = HAL_ADC_GetValue(me->hadc1);
	
	HAL_ADC_PollForConversion(me->hadc1, 100);
	me->rainSensorAnalogValue = HAL_ADC_GetValue(me->hadc1);
	
	HAL_ADC_Stop(me->hadc1);
	
	if( me->gasSensorAnalogValue >= MQ7_THRESHOLD )																				me->mq7DigitalState = true;
	else																																									me->mq7DigitalState = false;
	
	if( me->rainSensorAnalogValue >= RAIN_HIGH_THRESHOLD )																me->rainSensorDigitalState = true;
	else																																									me->rainSensorDigitalState = false;
	
	if(	me->panicButtonState || me->mq7DigitalState || me->rainSensorDigitalState ) 			me->buzzerState = true;
	else																																									me->buzzerState = false;
	
	DHT_readData( &am2301, &me->temp, &me->humid     );
}

static stSensorController_t* SensorController_getInstance()
{
    static stSensorController_t sensorInstance;
    static stSensorController_t* me = NULL;

    if (me == NULL)
    {
        me = &sensorInstance;
    }
    return me;
}
