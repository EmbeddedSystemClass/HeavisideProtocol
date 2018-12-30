/*
 * characteristic_client.h
 *
 *  Created on: Aug 14, 2018
 *      Author: Onur Efe
 */

#ifndef CHARACTERISTIC_CLIENT_H
#define CHARACTERISTIC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_time.h"
#include "queue_generic.h"
#include "generic.h"

#define CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY QUEUE_CAPACITY_16K
#define CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_TIMEOUTS 5
#define CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_CHECK_TIMEOUTS 2
#define CHARACTERISTIC_CLIENT_TIMEOUT_IN_MS 500
#define CHARACTERISTIC_CLIENT_POLL_PERIOD_IN_MS 100000
#define CHARACTERISTIC_CLIENT_CHANGE_CHANNEL_AWAIT_IN_MS 6000

/* Exported types ------------------------------------------------------------*/
enum
{
  CHARACTERISTIC_CLIENT_STATE_UNINIT = 0x00,
  CHARACTERISTIC_CLIENT_STATE_READY,
	CHARACTERISTIC_CLIENT_STATE_OPERATING,
	CHARACTERISTIC_CLIENT_STATE_ERROR
};
typedef uint8_t CharacteristicClient_State_t;

typedef struct
{
	uint8_t deviceName[32];
	uint16_t versionNumber;
	uint8_t uuid[16];
} CharacteristicClient_DeviceId_t;

// Delegates.
typedef void (*CharacteristicClient_CheckResultDelegate_t)(uint8_t channel, uint8_t *data, Bool_t deviceFound);
typedef void (*CharacteristicClient_ConnectionStateChangedDelegate_t)(Bool_t isConnected);
typedef void (*CharacteristicClient_ReadResponseReceivedDelegate_t)(uint8_t charId);
typedef void (*CharacteristicClient_OperationFailedDelegate_t)(void);
typedef void (*CharacteristicClient_TimeoutOccurredDelegate_t)(void);
typedef void (*CharacteristicClient_IdleStateDelegate_t)(void);
typedef void (*CharacteristicClient_ErrorOccurredDelegate_t)(Bool_t wasConnected);

// Setup parameters.
typedef struct
{
	CharacteristicClient_CheckResultDelegate_t							checkResultDelegate;
	CharacteristicClient_ConnectionStateChangedDelegate_t   connectionStateChangedDelegate;
  CharacteristicClient_ReadResponseReceivedDelegate_t     readResponseReceivedDelegate;
  CharacteristicClient_OperationFailedDelegate_t          operationFailedDelegate;
  CharacteristicClient_TimeoutOccurredDelegate_t          timeoutOccurredDelegate;
	CharacteristicClient_ErrorOccurredDelegate_t						errorOccurredDelegate;
	CharacteristicClient_IdleStateDelegate_t								idleStateDelegate;
} CharacteristicClient_SetupParams_t;

/* Exported functions --------------------------------------------------------*/
// Module control functions.
extern void CharacteristicClient_Setup(CharacteristicClient_SetupParams_t *pSetupParams);
extern Bool_t CharacteristicClient_Start(uint8_t channelId);
extern void CharacteristicClient_Execute(void);
extern void CharacteristicClient_Stop(void);
extern void CharacteristicClient_ErrorHandler(void);
extern void CharacteristicClient_ClearPending(void);

// Functions to obtain state.
extern Bool_t CharacteristicClient_IsConnected(void);
extern CharacteristicClient_State_t CharacteristicClient_GetState(void);

// Requests.
extern void CharacteristicClient_ChangeChannel(uint8_t channelId);
extern void CharacteristicClient_SendConnectionRequest(void);
extern void CharacteristicClient_SendDisconnectionRequest(void);
extern void CharacteristicClient_SendReadRequest(uint8_t charId, uint8_t *data,
                                                 uint16_t maxLength);

extern void CharacteristicClient_SendWriteRequest(uint8_t charId, uint8_t *data,
                                                  uint16_t dataLength);
extern void CharacteristicClient_SendCheckRequest(CharacteristicClient_DeviceId_t *deviceId);
extern uint8_t CharacteristicClient_MapAvailableChannels(void);

#ifdef __cplusplus
}
#endif
#endif /* CHARACTERISTIC_CLIENT_H */
