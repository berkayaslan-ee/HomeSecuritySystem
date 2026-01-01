#include "RS485Controller.h"

static stRs485Controller_t* RS485Controller_GetInstance();

stRs485Controller_t* RS485Controller_Init(UART_HandleTypeDef* pUartHandle)
{
	stRs485Controller_t* me = RS485Controller_GetInstance();
	me->pUartHandle 				= pUartHandle;
	me->sendDataFlag				= false;
	HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);
	
	return me;
}

void RS485Controller_EventLoop( stRs485Controller_t* me )
{
  if( me->sendDataFlag )
	{
		// Change Room Name
		me->rs485Packet.rs485Packet.messageId = KitchenID;
		me->rs485Packet.rs485Packet.startByte = 0xA5;
		HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_SET);
		HAL_UART_Transmit( me->pUartHandle, me->rs485Packet.rs485PacketArray, sizeof( stRs485Packet_t ), 100 );
		HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);
		me->sendDataFlag = false;
	}
}

static stRs485Controller_t* RS485Controller_GetInstance()
{
	static stRs485Controller_t Rs485Instance;
    static stRs485Controller_t* me = NULL;

    if (me == NULL)
    {
        me = &Rs485Instance;
    }
    return me;
}


