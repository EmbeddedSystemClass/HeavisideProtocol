#ifndef __SERIAL_H
#define __SERIAL_H

#include "generic.h"
#include "stm32f3xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported definitions ----------------------------------------------------*/
#define SERIAL_VOID_CHARACTER 0xAA
#define SERIAL_UART_HANDLE huart1
#define SERIAL_RING_BUFFER_SIZE (1U << 8)

	/* Exported typedefs -------------------------------------------------------*/
	enum
	{
		SERIAL_STATE_UNINIT = 0,
		SERIAL_STATE_READY,
		SERIAL_STATE_OPERATING,
		SERIAL_STATE_ERROR
	};
	typedef uint8_t Serial_State_t;

	enum
	{
		SERIAL_EVENT_DATA_READY = 0,
		SERIAL_EVENT_TX_IDLE,
		SERIAL_EVENT_TX_COMPLETED, 
		SERIAL_EVENT_ERROR_OCCURRED
	};
	typedef uint8_t Serial_Event_t;

	typedef void (*Serial_EventOccurredDelegate_t)(Serial_Event_t event, uint8_t *data,
												   uint16_t length);

	/* Exported functions ------------------------------------------------------*/
	extern void Serial_Setup(Serial_EventOccurredDelegate_t eventHandler);
	extern Bool_t Serial_Start(void);
	extern void Serial_Execute(void);
	extern void Serial_Stop(void);
	extern void Serial_Send(uint8_t *data, uint16_t dataLength);

#ifdef __cplusplus
}
#endif

#endif
