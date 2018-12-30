#include "characteristic_client.h"
#include "characteristic_protocol.h"
#include "sys_time.h"
#include "debug.h"

/* Private constants ---------------------------------------------------------*/
#define PROCESS_QUEUE_BUFFER_SIZE (1 << CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY)

/* Private typedefs ----------------------------------------------------------*/
enum
{
	PROCESS_CODE_CHANGE_CHANNEL = 0,
	PROCESS_CODE_CHECK_REQ,		// Use when not connected.
	PROCESS_CODE_POLL_REQ,
	PROCESS_CODE_CONN_REQ,
	PROCESS_CODE_DISCONN_REQ,
	PROCESS_CODE_READ_REQ,
    PROCESS_CODE_WRITE_REQ
};
typedef uint8_t ProcessCode_t;

typedef struct
{
    uint8_t		  channelId;
	uint8_t       code;
    uint8_t       charId;
	uint16_t      dataLength;
    uint8_t       *data;
} Process_t;

/* Private function declarations ---------------------------------------------*/
static void errorOccurredEventHandler(void);
static void process(Process_t *pProcess, Bool_t isConnected);
static void pduReceivedEventHandler(CharacteristicProtocol_PduType_t pduType,
                                    OperationResult_t operationResult,
                                    uint16_t unparsedPduSize);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static CharacteristicClient_State_t                           State = CHARACTERISTIC_CLIENT_STATE_UNINIT;
static Bool_t                                                 IsConnected;
static Bool_t                                                 IsNopping;
static Bool_t                                                 WaitingResponse;

static Process_t											  Cache;
static uint32_t												  SuccessiveRequestTimeoutCount;
static uint32_t												  LastRequestTimeStamp;
static uint32_t											      LastScheduledPollTimeStamp;
static uint32_t                                               NopStartTimeStamp;

// Delegates.
static CharacteristicClient_ConnectionStateChangedDelegate_t  ConnectionStateChangedDelegate;
static CharacteristicClient_ReadResponseReceivedDelegate_t    ReadResponseReceivedDelegate;
static CharacteristicClient_TimeoutOccurredDelegate_t         TimeoutOccurredDelegate;
static CharacteristicClient_OperationFailedDelegate_t         OperationFailedDelegate;
static CharacteristicClient_CheckResultDelegate_t			  CheckResultDelegate;
static CharacteristicClient_IdleStateDelegate_t				  IdleStateDelegate;
static CharacteristicClient_ErrorOccurredDelegate_t		      ErrorOccurredDelegate;

// Containers.
static uint8_t ProcessQueueContainer[PROCESS_QUEUE_BUFFER_SIZE];
static QueueGeneric_Buffer_t ProcessQueue;

/* Public function implementations. ------------------------------------------*/
void CharacteristicClient_Setup(CharacteristicClient_SetupParams_t *pSetupParams)
{
	assert_param((State == CHARACTERISTIC_CLIENT_STATE_UNINIT) && pSetupParams);

	CharacteristicProtocol_Setup(pduReceivedEventHandler,
															 errorOccurredEventHandler);

	// Set delegates.
	CheckResultDelegate = pSetupParams->checkResultDelegate;
	ConnectionStateChangedDelegate = pSetupParams->connectionStateChangedDelegate;
	ReadResponseReceivedDelegate = pSetupParams->readResponseReceivedDelegate;
	TimeoutOccurredDelegate = pSetupParams->timeoutOccurredDelegate;
	OperationFailedDelegate = pSetupParams->operationFailedDelegate;
	IdleStateDelegate = pSetupParams->idleStateDelegate;
	ErrorOccurredDelegate = pSetupParams->errorOccurredDelegate;

	// Init process queue.
	QueueGeneric_InitBuffer(&ProcessQueue, ProcessQueueContainer,
                            sizeof(Process_t), CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY);

	State = CHARACTERISTIC_CLIENT_STATE_READY;
    SysTime_Start();
}

uint8_t CharacteristicClient_MapAvailableChannels(void)
{
	return CharacteristicProtocol_MapAvailableChannels();
}

Bool_t CharacteristicClient_Start(uint8_t channelId)
{
	assert_param(State == CHARACTERISTIC_CLIENT_STATE_READY);

	if (!CharacteristicProtocol_Start(channelId))
	{
		return FALSE;
	}

	// Clear process queue.
	QueueGeneric_ClearBuffer(&ProcessQueue);

	// Set state variables.
    WaitingResponse = FALSE;
	IsConnected = FALSE;
    IsNopping = FALSE;

	// Set state to operating.
	State = CHARACTERISTIC_CLIENT_STATE_OPERATING;

	return TRUE;
}

void CharacteristicClient_ChangeChannel(uint8_t channelId)
{
	Process_t process;

    // Enqueue channel change request.
	process.channelId = channelId;
	process.code = PROCESS_CODE_CHANGE_CHANNEL;
	QueueGeneric_Enqueue(&ProcessQueue, &process);
}

void CharacteristicClient_Execute(void)
{
	if (State != CHARACTERISTIC_CLIENT_STATE_OPERATING)
	{
		return;
	}

	// Call submodule's executer.
	CharacteristicProtocol_Execute();

	uint32_t sys_time = SysTime_GetTimeInMs();

	// Enqueue poll if not polled for poll period(and connected).
    if (IsConnected && ((sys_time - LastScheduledPollTimeStamp) > CHARACTERISTIC_CLIENT_POLL_PERIOD_IN_MS) && !WaitingResponse)
    {
        Cache.code = PROCESS_CODE_POLL_REQ;

        process(&Cache, IsConnected);
	}

    if ((IsNopping) && ((sys_time - NopStartTimeStamp) > CHARACTERISTIC_CLIENT_CHANGE_CHANNEL_AWAIT_IN_MS))
    {
        IsNopping = FALSE;
    }

    if (WaitingResponse)
	{
		Bool_t timeout_occurred = FALSE;

		// Check for timeout.
        if ((sys_time - LastRequestTimeStamp) > CHARACTERISTIC_CLIENT_TIMEOUT_IN_MS)
		{
            if (Cache.code == PROCESS_CODE_CHECK_REQ)
            {
                if (++SuccessiveRequestTimeoutCount > CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_CHECK_TIMEOUTS)
                {
                    timeout_occurred = TRUE;
                }
            }
            else if (++SuccessiveRequestTimeoutCount > CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_TIMEOUTS)
            {
                timeout_occurred = TRUE;
            }

            // Reprocess the request if timeout didn't occur.
			if (!timeout_occurred)
			{
				process(&Cache, IsConnected);
			}
			else
			{
                TimeoutOccurredDelegate? TimeoutOccurredDelegate():(void)0;

				if (IsConnected)
				{
					IsConnected = FALSE;
					ConnectionStateChangedDelegate? ConnectionStateChangedDelegate(IsConnected):(void)0;
				}

				WaitingResponse = FALSE;
			}
		}
	}
    else if (!IsNopping)
	{
		// If any request pending; process it.
		if (QueueGeneric_GetElementCount(&ProcessQueue))
		{
            QueueGeneric_Dequeue(&ProcessQueue, &Cache);
			process(&Cache, IsConnected);
			SuccessiveRequestTimeoutCount = 0;
		}
		else
		{
			IdleStateDelegate? IdleStateDelegate():(void)0;
		}
	}
}

void CharacteristicClient_ClearPending(void)
{
	QueueGeneric_ClearBuffer(&ProcessQueue);
}

void CharacteristicClient_Stop(void)
{
	assert_param(State == CHARACTERISTIC_CLIENT_STATE_OPERATING);

	// Stop characteristic protocol.
	CharacteristicProtocol_Stop();

	// Set state to ready.
	State = CHARACTERISTIC_CLIENT_STATE_READY;
}

CharacteristicClient_State_t CharacteristicClient_GetState(void)
{
	return State;
}

void CharacteristicClient_ErrorHandler(void)
{
	CharacteristicProtocol_ErrorHandler();

	State = CHARACTERISTIC_CLIENT_STATE_READY;
}

void CharacteristicClient_SendConnectionRequest(void)
{
	Process_t process;

	// Enqueue connection request command.
	process.code = PROCESS_CODE_CONN_REQ;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

void CharacteristicClient_SendDisconnectionRequest(void)
{
	Process_t process;

	process.code = PROCESS_CODE_DISCONN_REQ;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

void CharacteristicClient_SendReadRequest(uint8_t charId, uint8_t *data, uint16_t maxLength)
{
	Process_t process;

	process.code = PROCESS_CODE_READ_REQ;
	process.charId = charId;
	process.data = data;
	process.dataLength = maxLength;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

void CharacteristicClient_SendWriteRequest(uint8_t charId, uint8_t *data, uint16_t dataLength)
{
	Process_t process;

    process.code = PROCESS_CODE_WRITE_REQ;
	process.charId = charId;
	process.data = data;
	process.dataLength = dataLength;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

void CharacteristicClient_SendCheckRequest(CharacteristicClient_DeviceId_t *deviceId)
{
	Process_t process;

	// Enqueue check command.
	process.code = PROCESS_CODE_CHECK_REQ;
	process.data = (uint8_t *)deviceId;
	process.dataLength = sizeof(CharacteristicClient_DeviceId_t);

	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

Bool_t CharacteristicClient_IsConnected(void)
{
	return (IsConnected && \
					(State == CHARACTERISTIC_CLIENT_STATE_OPERATING));
}

/* Private function implementations ------------------------------------------*/
static void errorOccurredEventHandler(void)
{
	State = CHARACTERISTIC_CLIENT_STATE_ERROR;
	ErrorOccurredDelegate ? ErrorOccurredDelegate(IsConnected) : (void)0;
}

static void process(Process_t *pProcess, Bool_t isConnected)
{
	switch (pProcess->code)
    {
    case PROCESS_CODE_CHANGE_CHANNEL:
	{
        if (!isConnected)
		{
            IsNopping = TRUE;
            NopStartTimeStamp = SysTime_GetTimeInMs();
            CharacteristicProtocol_ChangeChannel(pProcess->channelId);
            WaitingResponse = FALSE;
		}
	}
	break;

	case PROCESS_CODE_CHECK_REQ:
	{
		if (!isConnected)
		{
            CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_REQ, 0, 0, 0);
			LastRequestTimeStamp = SysTime_GetTimeInMs();
			WaitingResponse = TRUE;
		}
	}
	break;

	case PROCESS_CODE_POLL_REQ:
	{
		if (isConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_REQ, 0, 0, 0);
            LastRequestTimeStamp = SysTime_GetTimeInMs();
            LastScheduledPollTimeStamp = LastRequestTimeStamp;
            WaitingResponse = TRUE;
		}
	}
	break;

	case PROCESS_CODE_CONN_REQ:
	{
		if (!isConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_REQ,
																	0, 0, 0);
			LastRequestTimeStamp = SysTime_GetTimeInMs();
			WaitingResponse = TRUE;
		}
	}
	break;

	case PROCESS_CODE_DISCONN_REQ:
	{
		if (isConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_REQ,
																	0, 0, 0);
			LastRequestTimeStamp = SysTime_GetTimeInMs();
			WaitingResponse = TRUE;
		}
	}
	break;

	case PROCESS_CODE_READ_REQ:
	{
		if (isConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ,
																	pProcess->charId, 0, 0);
			LastRequestTimeStamp = SysTime_GetTimeInMs();
			WaitingResponse = TRUE;
		}
	}
	break;

	case PROCESS_CODE_WRITE_REQ:
	{
		if (isConnected)
		{
            CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ,
																	pProcess->charId, pProcess->data,
																	pProcess->dataLength);

			LastRequestTimeStamp = SysTime_GetTimeInMs();
			WaitingResponse = TRUE;
		}
	}
	break;
	}
}

static void pduReceivedEventHandler(CharacteristicProtocol_PduType_t pduType,
																		OperationResult_t operationResult,
																		uint16_t unparsedPduSize)
{
	// If there isn't any ongoing transaction; discard.
	if ((State != CHARACTERISTIC_CLIENT_STATE_OPERATING) || !WaitingResponse)
	{
		return;
	}

	switch (pduType)
	{
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_RESP:
	{
		if ((Cache.code != PROCESS_CODE_CHECK_REQ) || IsConnected)
		{
			return;
		}

        // Parse pdu data(which is device id in this case)
		CharacteristicProtocol_ParsePduData(Cache.data, Cache.dataLength, unparsedPduSize);

		CheckResultDelegate ?
                    CheckResultDelegate(Cache.channelId, Cache.data, TRUE):(void)0;
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_RESP:
	{
		if ((Cache.code != PROCESS_CODE_POLL_REQ) || !IsConnected)
		{
			return;
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_RESP:
	{
		if ((Cache.code != PROCESS_CODE_CONN_REQ) || IsConnected)
		{
			return;
		}

		// Update connection state.
		if (operationResult == OPERATION_RESULT_SUCCESS)
		{
			IsConnected = TRUE;
			LastScheduledPollTimeStamp = SysTime_GetTimeInMs();
			ConnectionStateChangedDelegate? \
						ConnectionStateChangedDelegate(IsConnected) : (void)0;
		}
		else
		{
			OperationFailedDelegate? OperationFailedDelegate():(void)0;
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_RESP:
	{
		if ((Cache.code != PROCESS_CODE_DISCONN_REQ) || !IsConnected)
		{
			return;
		}

		IsConnected = FALSE;

		ConnectionStateChangedDelegate? \
					ConnectionStateChangedDelegate(IsConnected):(void)0;
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP:
	{
		if ((Cache.code != PROCESS_CODE_READ_REQ) || !IsConnected)
		{
			return;
		}

		// If success, call read response received delegate.
		if (operationResult == OPERATION_RESULT_SUCCESS)
		{
			CharacteristicProtocol_ParsePduData(Cache.data, Cache.dataLength, unparsedPduSize);

			ReadResponseReceivedDelegate? \
						ReadResponseReceivedDelegate(Cache.charId):(void)0;
		}
		else
		{
			OperationFailedDelegate? OperationFailedDelegate():(void)0;
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP:
	{
		if ((Cache.code != PROCESS_CODE_WRITE_REQ) || !IsConnected)
		{
            return;
		}

		if (operationResult == OPERATION_RESULT_FAILURE)
		{
            OperationFailedDelegate? OperationFailedDelegate():(void)0;
		}
	}
	break;

	default:
	break;
	}

	WaitingResponse = FALSE;
}
