/*
 * _client.h
 *
 *  Created on: Aug 14, 2018
 *      Author: Onur Efe
 */

#ifndef __OBJSHARE_CLIENT_H
#define __OBJSHARE_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "queue_generic.h"
#include "generic.h"

#define OBJSHARE_CLIENT_PROCESS_QUEUE_CAPACITY QUEUE_CAPACITY_128
#define OBJSHARE_CLIENT_MAX_SUCCESSIVE_REQUESTS 5
#define OBJSHARE_CLIENT_TIMEOUT_IN_MS 500

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		OBJSHARE_CLIENT_STATE_UNINIT = 0x00,
		OBJSHARE_CLIENT_STATE_READY,
		OBJSHARE_CLIENT_STATE_OPERATING
	};
	typedef uint8_t ObjshareClient_State_t;

	// Delegates.
	typedef void (*ObjshareClient_ReadResponseReceivedDelegate_t)(uint8_t sourceAddr,
																  uint8_t charId);
	typedef void (*ObjshareClient_OperationFailedDelegate_t)(uint8_t sourceAddr);
	typedef void (*ObjshareClient_NoResponseDelegate_t)(uint8_t sourceAddr);
	typedef void (*ObjshareClient_IdleStateDelegate_t)(void);

	// Setup parameters.
	typedef struct
	{
		ObjshareClient_ReadResponseReceivedDelegate_t readResponseReceivedDelegate;
		ObjshareClient_OperationFailedDelegate_t operationFailedDelegate;
		ObjshareClient_NoResponseDelegate_t noResponseDelegate;
		ObjshareClient_IdleStateDelegate_t idleStateDelegate;
	} ObjshareClient_Delegates_t;

	/* Exported functions --------------------------------------------------------*/
	// Module control functions.
	extern void ObjshareClient_Setup(uint8_t homeAddress,
									 ObjshareClient_Delegates_t *delegates);
	extern Bool_t ObjshareClient_Start(void);
	extern void ObjshareClient_Execute(void);
	extern void ObjshareClient_Stop(void);
	extern void ObjshareClient_ErrorHandler(void);
	extern void ObjshareClient_ClearPending(void);

	// Functions to obtain state.
	extern ObjshareClient_State_t ObjshareClient_GetState(void);

	// Requests.
	extern void ObjshareClient_SendReadRequest(uint8_t destinationAddress,
											   uint8_t charId, uint8_t *data,
											   uint16_t maxLength);

	extern void ObjshareClient_SendWriteRequest(uint8_t destinationAddress,
												uint8_t charId, uint8_t *data,
												uint16_t dataLength);

#ifdef __cplusplus
}
#endif

#endif /* OBJSHARE_CLIENT_H */
