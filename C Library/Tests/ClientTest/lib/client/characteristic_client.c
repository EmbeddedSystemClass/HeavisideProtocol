#include "characteristic_client.h"
#include "characteristic_protocol.h"
#include "sys_time.h"

/* Private constants ---------------------------------------------------------*/
#define PROCESS_QUEUE_BUFFER_SIZE (1 << CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY)

/* Private typedefs ----------------------------------------------------------*/
enum
{
	PROCESS_CODE_READ_REQ = 0,
    PROCESS_CODE_WRITE_REQ
};
typedef uint8_t ProcessCode_t;

typedef struct
{
	uint8_t		destinationAddress;
	uint8_t		code;
    uint8_t		charId;
	uint16_t 	dataLength;
    uint8_t		*data;
} Process_t;

/* Private function declarations ---------------------------------------------*/
static void process(Process_t *process);
static void pduReceivedEventHandler(uint8_t sourceAddress, 
									CharacteristicProtocol_PduType_t pduType,
                                    OperationResult_t operationResult,
                                    uint16_t unparsedPduSize);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static CharacteristicClient_State_t                           	State = CHARACTERISTIC_CLIENT_STATE_UNINIT;
static Bool_t                                                	WaitingResponse;

static Process_t											 	Cache;
static uint32_t												 	SuccessiveRequestCount;
static uint32_t												 	LastRequestTimeStamp;

// Delegates.
static CharacteristicClient_ReadResponseReceivedDelegate_t		ReadResponseReceivedDelegate;
static CharacteristicClient_OperationFailedDelegate_t			OperationFailedDelegate;
static CharacteristicClient_NoResponseDelegate_t				NoResponseDelegate;
static CharacteristicClient_IdleStateDelegate_t					IdleStateDelegate;

// Containers.
static uint8_t ProcessQueueContainer[PROCESS_QUEUE_BUFFER_SIZE];
static QueueGeneric_Buffer_t ProcessQueue;

/* Public function implementations. ------------------------------------------*/
void CharacteristicClient_Setup(uint8_t homeAddress, CharacteristicClient_Delegates_t *delegates)
{
	CharacteristicProtocol_Setup(homeAddress, pduReceivedEventHandler);

	// Set delegates.
	ReadResponseReceivedDelegate = delegates->readResponseReceivedDelegate;
	OperationFailedDelegate = delegates->operationFailedDelegate;
	NoResponseDelegate = delegates->noResponseDelegate;
	IdleStateDelegate = delegates->idleStateDelegate;
	
	// Init process queue.
	QueueGeneric_InitBuffer(&ProcessQueue, ProcessQueueContainer,
                            sizeof(Process_t), CHARACTERISTIC_CLIENT_PROCESS_QUEUE_CAPACITY);

	State = CHARACTERISTIC_CLIENT_STATE_READY;
}

Bool_t CharacteristicClient_Start(void)
{
	CharacteristicProtocol_Start();

	// Clear process queue.
	QueueGeneric_ClearBuffer(&ProcessQueue);

	// Set state variables.
    WaitingResponse = FALSE;
	
	// Set state to operating.
	State = CHARACTERISTIC_CLIENT_STATE_OPERATING;

	return TRUE;
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

	if (WaitingResponse)
	{
		Bool_t no_response = FALSE;

		// Check for timeout.
	    if ((sys_time - LastRequestTimeStamp) > CHARACTERISTIC_CLIENT_TIMEOUT_IN_MS)
		{
			if (++SuccessiveRequestCount > CHARACTERISTIC_CLIENT_MAX_SUCCESSIVE_REQUESTS)
	        {
	            no_response = TRUE;
	        }

	        // Reprocess the request if successive request limit has not been exceeded.
			if (!no_response)
			{
				process(&Cache);
			}
			else
			{
	        	NoResponseDelegate? NoResponseDelegate(Cache.destinationAddress):(void)0;

				WaitingResponse = FALSE;
			}
		}
	}
	else
	{
		// If any request pending; process it.
		if (QueueGeneric_GetElementCount(&ProcessQueue))
		{
            QueueGeneric_Dequeue(&ProcessQueue, &Cache);
			process(&Cache);
			SuccessiveRequestCount = 0;
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
	// Stop characteristic protocol.
	CharacteristicProtocol_Stop();

	// Set state to ready.
	State = CHARACTERISTIC_CLIENT_STATE_READY;
}

CharacteristicClient_State_t CharacteristicClient_GetState(void)
{
	return State;
}

void CharacteristicClient_SendReadRequest(uint8_t destinationAddress, 
	uint8_t charId, uint8_t *data, uint16_t maxLength)
{
	Process_t process;

	process.destinationAddress = destinationAddress;
	process.code = PROCESS_CODE_READ_REQ;
	process.charId = charId;
	process.data = data;
	process.dataLength = maxLength;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

void CharacteristicClient_SendWriteRequest(uint8_t destinationAddress, 
	uint8_t charId, uint8_t *data, uint16_t dataLength)
{
	Process_t process;

	process.destinationAddress = destinationAddress;
    process.code = PROCESS_CODE_WRITE_REQ;
	process.charId = charId;
	process.data = data;
	process.dataLength = dataLength;
	QueueGeneric_Enqueue(&ProcessQueue, (uint8_t *)&process);
}

/* Private function implementations ------------------------------------------*/
static void process(Process_t *process)
{
	switch (process->code)
    {
	case PROCESS_CODE_READ_REQ:
	{
		CharacteristicProtocol_Send(process->destinationAddress, 
									CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ,
									process->charId, 0, 0);
		
	}
	break;

	case PROCESS_CODE_WRITE_REQ:
	{
		CharacteristicProtocol_Send(process->destinationAddress, 
									CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ,
									process->charId, 
									process->data, process->dataLength);
	}
	break;
	}

	LastRequestTimeStamp = SysTime_GetTimeInMs();
	WaitingResponse = TRUE;
}

static void pduReceivedEventHandler(uint8_t sourceAddress,
									CharacteristicProtocol_PduType_t pduType,
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
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP:
	{
		if (Cache.code != PROCESS_CODE_READ_REQ)
		{
			return;
		}

		// If success, call read response received delegate.
		if (operationResult == OPERATION_RESULT_SUCCESS)
		{
			CharacteristicProtocol_ParsePduData(Cache.data, Cache.dataLength, unparsedPduSize);

			ReadResponseReceivedDelegate? \
						ReadResponseReceivedDelegate(sourceAddress, Cache.charId):(void)0;
		}
		else
		{
			OperationFailedDelegate? OperationFailedDelegate(sourceAddress):(void)0;
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP:
	{
		if (Cache.code != PROCESS_CODE_WRITE_REQ)
		{
            return;
		}

		if (operationResult == OPERATION_RESULT_FAILURE)
		{
            OperationFailedDelegate? OperationFailedDelegate(sourceAddress) : (void)0;
		}
	}
	break;

	default:
	break;
	}

	WaitingResponse = FALSE;
}
