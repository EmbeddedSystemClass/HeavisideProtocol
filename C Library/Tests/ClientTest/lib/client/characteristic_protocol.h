/***
  * @author     Onur Efe
  */
#ifndef __CHARACTERISTIC_PROTOCOL_H
#define __CHARACTERISTIC_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Include files -------------------------------------------------------------*/
#include "generic.h"

	/* Exported constants --------------------------------------------------------*/
	//#define CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	#define CHARACTERISTIC_PROTOCOL_CLIENT_SIDE

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		CHARACTERISTIC_PROTOCOL_STATE_UNINIT = 0,
		CHARACTERISTIC_PROTOCOL_STATE_READY,
		CHARACTERISTIC_PROTOCOL_STATE_OPERATING
	};
	typedef uint8_t CharacteristicProtocol_State_t;

	enum
	{
		CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ,
		CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP,
		CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ,
		CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP,
	};
	typedef uint8_t CharacteristicProtocol_PduType_t;

#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	typedef void (*CharacteristicProtocol_PduReceivedDelegate_t)(
		uint8_t sourceAddress, CharacteristicProtocol_PduType_t pduType, 
		uint8_t charId, uint16_t unparsedPduSize);
#else
typedef void (*CharacteristicProtocol_PduReceivedDelegate_t)(
	uint8_t sourceAddress, CharacteristicProtocol_PduType_t pduType,
	OperationResult_t operationResult, uint16_t unparsedPduSize);
#endif

	/* Exported functions --------------------------------------------------------*/
	extern void CharacteristicProtocol_Setup(uint8_t homeAddress,
		CharacteristicProtocol_PduReceivedDelegate_t pduReceivedEventHandler);
	extern uint8_t CharacteristicProtocol_MapAvailableChannels(void);
	extern void CharacteristicProtocol_Start(void);
	extern void CharacteristicProtocol_Execute(void);
	extern CharacteristicProtocol_State_t CharacteristicProtocol_GetState();
	extern void CharacteristicProtocol_Stop(void);
	extern void CharacteristicProtocol_ParsePduData(uint8_t *data, uint16_t length, uint16_t unparsedPduSize);

#if defined(CHARACTERISTIC_PROTOCOL_SERVER_SIDE)
	extern void CharacteristicProtocol_Send(uint8_t destinationAddress,
											CharacteristicProtocol_PduType_t pduType, 
											OperationResult_t operationResult,
											uint8_t *data, uint16_t dataLength);
#else
extern void CharacteristicProtocol_Send(uint8_t destinationAddress,
										CharacteristicProtocol_PduType_t pduType, 
										uint8_t charId, uint8_t *data, 
										uint16_t dataLength);
#endif

#ifdef __cplusplus
}
#endif

#endif
