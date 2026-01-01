#ifndef RS485_CONTROLLER_H
#define RS485_CONTROLLER_H

#include "main.h"

#define BedroomID 		0x01;
#define LivingRoomID 	0x02;
#define KitchenID 		0x03;

#pragma pack(1)

typedef struct 
{
	uint8_t startByte;
  uint8_t messageId;
  uint8_t reedState;
  uint8_t pirState;
  uint8_t panicState;
  uint8_t rainState;
  uint8_t mq7State;
	float temp;
	float humid;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
}stRs485Packet_t;

typedef union
{
	stRs485Packet_t rs485Packet;
	uint8_t rs485PacketArray[ sizeof( stRs485Packet_t ) ];
}uRs485Packet_t;


#pragma pack()

typedef struct 
{
	UART_HandleTypeDef* pUartHandle;
	uRs485Packet_t	rs485Packet;
	
	_Bool sendDataFlag;

}stRs485Controller_t;

stRs485Controller_t* RS485Controller_Init( UART_HandleTypeDef* pUartHandle );
void RS485Controller_EventLoop( stRs485Controller_t* me );



#endif