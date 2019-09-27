/***
  * @author     Onur Efe
  */
#ifndef __OBJSHARE_PROTOCOL_H
#define __OBJSHARE_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Include files -------------------------------------------------------------*/
#include "generic.h"

/* Exported constants --------------------------------------------------------*/
//#define OBJSHARE_PROTOCOL_HOST
#define OBJSHARE_PROTOCOL_PERIPHERAL

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		OBJSHARE_PROTOCOL_STATE_UNINIT = 0,
		OBJSHARE_PROTOCOL_STATE_READY,
		OBJSHARE_PROTOCOL_STATE_OPERATING
	};
	typedef uint8_t ObjshareProtocol_State_t;

	enum
	{
		OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ,
		OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP,
		OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ,
		OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP,
		OBJSHARE_PROTOCOL_PDUTYPE_POLL_REQ,
		OBJSHARE_PROTOCOL_PDUTYPE_POLL_RESP
	};
	typedef uint8_t ObjshareProtocol_PduType_t;

	enum
	{
		OBJSHARE_PROTOCOL_DIRECTION_TX,
		OBJSHARE_PROTOCOL_DIRECTION_RX
	};
	typedef uint8_t ObjshareProtocol_Direction_t;

	typedef void (*ObjshareProtocol_SwitchDirectionDelegate_t)(ObjshareProtocol_Direction_t direction);

#ifdef OBJSHARE_PROTOCOL_HOST
	typedef void (*ObjshareProtocol_PduReceivedDelegate_t)(
		ObjshareProtocol_PduType_t pduType, OperationResult_t operationResult, uint16_t unparsedPduSize);
#else
typedef void (*ObjshareProtocol_PduReceivedDelegate_t)(
	ObjshareProtocol_PduType_t pduType,
	uint8_t objId, uint16_t unparsedPduSize);
#endif

/* Exported functions --------------------------------------------------------*/
#ifdef OBJSHARE_PROTOCOL_HOST
	extern void ObjshareProtocol_Setup(ObjshareProtocol_PduReceivedDelegate_t pduReceivedEventHandler,
									   ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler);
#else
extern void ObjshareProtocol_Setup(ObjshareProtocol_PduReceivedDelegate_t pduReceivedEventHandler,
								   ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler);
#endif
	extern void ObjshareProtocol_Start(void);
	extern void ObjshareProtocol_Execute(void);
	extern ObjshareProtocol_State_t ObjshareProtocol_GetState(void);
	extern void ObjshareProtocol_Stop(void);
	extern void ObjshareProtocol_ParsePduData(uint8_t *data, uint16_t length,
											  uint16_t unparsedPduSize);

#if defined(OBJSHARE_PROTOCOL_HOST)
	extern void ObjshareProtocol_Send(uint8_t slot,
									  ObjshareProtocol_PduType_t pduType,
									  uint8_t objId, uint8_t *data,
									  uint16_t dataLength);
#else
extern void ObjshareProtocol_Send(ObjshareProtocol_PduType_t pduType,
								  OperationResult_t operationResult,
								  uint8_t *data, uint16_t dataLength);

#endif

#ifdef __cplusplus
}
#endif

#endif
