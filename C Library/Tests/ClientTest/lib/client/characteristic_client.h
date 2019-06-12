/*
 * characteristic_client.h
 *
 *  Created on: Aug 14, 2018
 *      Author: Onur Efe
 */

#ifndef __CHARACTERISTIC_CLIENT_H
#define __CHARACTERISTIC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "queue_generic.h"
#include "generic.h"

#define CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY QUEUE_CAPACITY_256
#define CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_REQUESTS 5
#define CHARACTERISTIC_CLIENT_TIMEOUT_IN_MS 500

/* Exported types ------------------------------------------------------------*/
enum
{
  	CHARACTERISTIC_CLIENT_STATE_UNINIT = 0x00,
  	CHARACTERISTIC_CLIENT_STATE_READY,
	CHARACTERISTIC_CLIENT_STATE_OPERATING
};
typedef uint8_t CharacteristicClient_State_t;

// Delegates.
typedef void (*CharacteristicClient_ReadResponseReceivedDelegate_t)(uint8_t sourceAddr,
	uint8_t charId);
typedef void (*CharacteristicClient_OperationFailedDelegate_t)(uint8_t sourceAddr);
typedef void (*CharacteristicClient_NoResponseDelegate_t)(uint8_t sourceAddr);
typedef void (*CharacteristicClient_IdleStateDelegate_t)(void);

// Setup parameters.
typedef struct
{
 	CharacteristicClient_ReadResponseReceivedDelegate_t     readResponseReceivedDelegate;
 	CharacteristicClient_OperationFailedDelegate_t          operationFailedDelegate;
 	CharacteristicClient_NoResponseDelegate_t          		noResponseDelegate;
	CharacteristicClient_IdleStateDelegate_t				idleStateDelegate;
} CharacteristicClient_Delegates_t;

/* Exported functions --------------------------------------------------------*/
// Module control functions.
extern void CharacteristicClient_Setup(uint8_t homeAddress, 
	CharacteristicClient_Delegates_t *delegates);
extern Bool_t CharacteristicClient_Start(void);
extern void CharacteristicClient_Execute(void);
extern void CharacteristicClient_Stop(void);
extern void CharacteristicClient_ErrorHandler(void);
extern void CharacteristicClient_ClearPending(void);

// Functions to obtain state.
extern CharacteristicClient_State_t CharacteristicClient_GetState(void);

// Requests.
extern void CharacteristicClient_SendReadRequest(uint8_t destinationAddress, 
												 uint8_t charId, uint8_t *data,
                                                 uint16_t maxLength);

extern void CharacteristicClient_SendWriteRequest(uint8_t destinationAddress,
												  uint8_t charId, uint8_t *data,
                                                  uint16_t dataLength);

#ifdef __cplusplus
}
#endif

#endif /* CHARACTERISTIC_CLIENT_H */
