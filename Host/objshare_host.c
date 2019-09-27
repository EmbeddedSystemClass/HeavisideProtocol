#include "objshare_host.h"
#include "objshare_protocol.h"
#include "sys_time.h"
#include "peripheral.h"
#include "sys_time.h"

/* Private constants ---------------------------------------------------------*/
#define MAX_PENDING_PROCESS_COUNT 32

#ifdef OBJSHARE_HOST_TEST
#define TEST_SLOT_COUNT 1
#define TARGET_VALUE_OBJ_ID 2
#endif

/* Private typedefs ----------------------------------------------------------*/
enum
{
	PROCESS_CODE_READ_REQ = 0,
	PROCESS_CODE_WRITE_REQ,
	PROCESS_CODE_POLL_REQ
};
typedef uint8_t ProcessCode_t;

typedef struct
{
	uint8_t slot;
	uint8_t code;
	uint8_t objId;
	uint16_t dataLength;
	uint8_t *data;
} Process_t;

/* Private function declarations ---------------------------------------------*/
static void process(Process_t *process);
static void pduReceivedEventHandler(ObjshareProtocol_PduType_t pduType,
									OperationResult_t operationResult,
									uint16_t unparsedPduSize);

#ifdef OBJSHARE_HOST_TEST
static void testReadResponseReceivedEventHandler(uint8_t slot, uint8_t objId);
static void testNoResponseEventHandler(uint8_t slot);
static void testOperationFailedEventHandler(uint8_t slot);
static void testPollResponseReceivedEventHandler(uint8_t);
#endif
/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static ObjshareHost_State_t State = OBJSHARE_HOST_STATE_UNINIT;
static Bool_t WaitingResponse;

static Process_t Cache;
static uint32_t SuccessiveRequestCount;
static uint32_t LastRequestTimestamp;

// Delegates.
static ObjshareHost_ReadResponseReceivedDelegate_t ReadResponseReceivedDelegate;
static ObjshareHost_OperationFailedDelegate_t OperationFailedDelegate;
static ObjshareHost_NoResponseDelegate_t NoResponseDelegate;
static ObjshareHost_PollResponseDelegate_t PollResponseReceivedDelegate;
static ObjshareHost_AddressSlotDelegate_t AddressSlotDelegate;

// Containers.
static Process_t ProcessQueueContainer[MAX_PENDING_PROCESS_COUNT];
static QueueGeneric_Buffer_t ProcessQueue;

#ifdef OBJSHARE_HOST_TEST
static float TargetValue[TEST_SLOT_COUNT];
static Bool_t PollResponse[TEST_SLOT_COUNT];
static Bool_t ReadResponse[TEST_SLOT_COUNT];
static Bool_t NoResponse[TEST_SLOT_COUNT];
static Bool_t OperationFailed[TEST_SLOT_COUNT];
#endif

/* Public function implementations. ------------------------------------------*/
#ifdef OBJSHARE_HOST_TEST
// There should be peripherals connected properly; which have test data as objects in their
//memory.
void ObjshareHost_Test(ObjshareHost_AddressSlotDelegate_t addressSlotEventHandler,
					   ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler)
{
	ObjshareHost_Delegates_t delegates;

	delegates.addressSlotDelegate = addressSlotEventHandler;
	delegates.switchDirectionDelegate = switchDirectionEventHandler;
	delegates.readResponseReceivedDelegate = testReadResponseReceivedEventHandler;
	delegates.noResponseDelegate = testNoResponseEventHandler;
	delegates.operationFailedDelegate = testOperationFailedEventHandler;
	delegates.pollResponseReceivedDelegate = testPollResponseReceivedEventHandler;

	ObjshareHost_Setup(&delegates);

	if (!ObjshareHost_Start())
	{
		while (TRUE)
			;
	}

	// Wait for a not short time.
	uint32_t sys_time;
	sys_time = SysTime_GetTimeInMs();

	while (SysTime_GetTimeInMs() - sys_time < 2000U)
	{
		ObjshareHost_Execute();
	}

	// Send poll requests.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		PollResponse[sl] = FALSE;
		ObjshareHost_SendPollRequest(sl);
	}

	sys_time = SysTime_GetTimeInMs();

	while (SysTime_GetTimeInMs() - sys_time < 200U)
	{
		ObjshareHost_Execute();
	}

	// Check if poll responses received.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		if (!PollResponse[sl])
		{
			while (TRUE)
				;
		}
	}

	// Send write requests.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		NoResponse[sl] = FALSE;
		ObjshareHost_SendWriteRequest(sl, TARGET_VALUE_OBJ_ID, (uint8_t *)&TargetValue, sizeof(float));
	}

	sys_time = SysTime_GetTimeInMs();

	while (SysTime_GetTimeInMs() - sys_time < 200U)
	{
		ObjshareHost_Execute();
	}

	// If no response received after write operations; there is problem.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		if (NoResponse[sl])
		{
			while (TRUE)
				;
		}
	}

	float read_objs[TEST_SLOT_COUNT];

	// Send read requests and check if the received data matches.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		ReadResponse[sl] = FALSE;
		ObjshareHost_SendReadRequest(sl, TARGET_VALUE_OBJ_ID, (uint8_t *)&read_objs[sl],
									 sizeof(read_objs[0]));
	}

	sys_time = SysTime_GetTimeInMs();

	while (SysTime_GetTimeInMs() - sys_time < 200U)
	{
		ObjshareHost_Execute();
	}

	// Read responses should be received.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		if (!ReadResponse[sl])
		{
			while (TRUE)
				;
		}
	}

	// Check if read and written data matches.
	for (uint8_t sl = 0; sl < TEST_SLOT_COUNT; sl++)
	{
		if (read_objs[sl] != TargetValue[sl])
		{
			while (TRUE)
				;
		}
	}

	// Tests completed successfully.
	while (TRUE)
		;
}

void testReadResponseReceivedEventHandler(uint8_t slot, uint8_t objId)
{
	ReadResponse[slot] = TRUE;
}

void testNoResponseEventHandler(uint8_t slot)
{
	NoResponse[slot] = TRUE;
}

void testOperationFailedEventHandler(uint8_t slot)
{
	OperationFailed[slot] = TRUE;
}

void testPollResponseReceivedEventHandler(uint8_t slot)
{
	PollResponse[slot] = TRUE;
}

#endif

void ObjshareHost_Setup(ObjshareHost_Delegates_t *delegates)
{
	ObjshareProtocol_Setup(pduReceivedEventHandler, delegates->switchDirectionDelegate);

	// Set delegates.
	ReadResponseReceivedDelegate = delegates->readResponseReceivedDelegate;
	OperationFailedDelegate = delegates->operationFailedDelegate;
	NoResponseDelegate = delegates->noResponseDelegate;
	PollResponseReceivedDelegate = delegates->pollResponseReceivedDelegate;
	AddressSlotDelegate = delegates->addressSlotDelegate;

	// Init process queue.
	QueueGeneric_InitBuffer(&ProcessQueue, ProcessQueueContainer,
							sizeof(Process_t), MAX_PENDING_PROCESS_COUNT);

	State = OBJSHARE_HOST_STATE_READY;
}

Bool_t ObjshareHost_Start(void)
{
	ObjshareProtocol_Start();

	// Clear process queue.
	QueueGeneric_ClearBuffer(&ProcessQueue);
	AddressSlotDelegate ? AddressSlotDelegate(0xFF) : (void)0;

	// Set state variables.
	WaitingResponse = FALSE;

	// Set state to operating.
	State = OBJSHARE_HOST_STATE_OPERATING;

	return TRUE;
}

void ObjshareHost_Execute(void)
{
	if (State != OBJSHARE_HOST_STATE_OPERATING)
	{
		return;
	}

	// Call submodule's executer.
	ObjshareProtocol_Execute();

	uint32_t sys_time = SysTime_GetTimeInMs();

	if (WaitingResponse)
	{
		Bool_t no_response = FALSE;

		// Check for timeout.
		if ((sys_time - LastRequestTimestamp) > OBJSHARE_HOST_TIMEOUT_IN_MS)
		{
			if (++SuccessiveRequestCount > OBJSHARE_HOST_MAX_SUCCESSIVE_REQUESTS)
			{
				no_response = TRUE;
			}

			// Reprocess the request if successive request limit has not been exceeded.
			if (no_response)
			{
				WaitingResponse = FALSE;
				NoResponseDelegate ? NoResponseDelegate(Cache.slot) : (void)0;
			}
			else
			{
				process(&Cache);
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
	}
}

void ObjshareHost_ClearPending(void)
{
	QueueGeneric_ClearBuffer(&ProcessQueue);
}

void ObjshareHost_Stop(void)
{
	// Stop objshare protocol.
	ObjshareProtocol_Stop();

	// Set state to ready.
	State = OBJSHARE_HOST_STATE_READY;
}

ObjshareHost_State_t ObjshareHost_GetState(void)
{
	return State;
}

void ObjshareHost_SendReadRequest(uint8_t slot,
								  uint8_t objId, uint8_t *data, uint16_t maxLength)
{
	Process_t process;

	process.slot = slot;
	process.code = PROCESS_CODE_READ_REQ;
	process.objId = objId;
	process.data = data;
	process.dataLength = maxLength;

	QueueGeneric_Enqueue(&ProcessQueue, &process);
}

void ObjshareHost_SendWriteRequest(uint8_t slot,
								   uint8_t objId, uint8_t *data, uint16_t dataLength)
{
	Process_t process;

	process.slot = slot;
	process.code = PROCESS_CODE_WRITE_REQ;
	process.objId = objId;
	process.data = data;
	process.dataLength = dataLength;

	QueueGeneric_Enqueue(&ProcessQueue, &process);
}

void ObjshareHost_SendPollRequest(uint8_t slot)
{
	Process_t process;

	process.slot = slot;
	process.code = PROCESS_CODE_POLL_REQ;

	QueueGeneric_Enqueue(&ProcessQueue, &process);
}

/* Private function implementations ------------------------------------------*/
static void process(Process_t *process)
{
	// Address related slot.
	AddressSlotDelegate(process->slot);

	switch (process->code)
	{
	case PROCESS_CODE_READ_REQ:
	{
		ObjshareProtocol_Send(process->slot,
							  OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ,
							  process->objId, 0, 0);
	}
	break;

	case PROCESS_CODE_WRITE_REQ:
	{
		ObjshareProtocol_Send(process->slot,
							  OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ,
							  process->objId,
							  process->data, process->dataLength);
	}
	break;

	case PROCESS_CODE_POLL_REQ:
	{
		ObjshareProtocol_Send(process->slot,
							  OBJSHARE_PROTOCOL_PDUTYPE_POLL_REQ,
							  0, 0, 0);
	}
	break;
	}

	LastRequestTimestamp = SysTime_GetTimeInMs();
	WaitingResponse = TRUE;
}

static void pduReceivedEventHandler(ObjshareProtocol_PduType_t pduType,
									OperationResult_t operationResult,
									uint16_t unparsedPduSize)
{
	if ((State != OBJSHARE_HOST_STATE_OPERATING) && !WaitingResponse)
	{
		return;
	}

	switch (pduType)
	{
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP:
	{
		if (Cache.code != PROCESS_CODE_READ_REQ)
		{
			return;
		}

		// If success, call read response received delegate.
		if (operationResult == OPERATION_RESULT_SUCCESS)
		{
			ObjshareProtocol_ParsePduData(Cache.data, Cache.dataLength, unparsedPduSize);

			ReadResponseReceivedDelegate ? ReadResponseReceivedDelegate(Cache.slot, Cache.objId) : (void)0;
		}
		else
		{
			OperationFailedDelegate ? OperationFailedDelegate(Cache.slot) : (void)0;
		}
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP:
	{
		if (Cache.code != PROCESS_CODE_WRITE_REQ)
		{
			return;
		}

		if (operationResult == OPERATION_RESULT_FAILURE)
		{
			OperationFailedDelegate ? OperationFailedDelegate(Cache.slot) : (void)0;
		}
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_POLL_RESP:
	{
		if (Cache.code != PROCESS_CODE_POLL_REQ)
		{
			return;
		}

		PollResponseReceivedDelegate ? PollResponseReceivedDelegate(Cache.slot) : (void)0;
	}
	break;

	default:
		break;
	}

	AddressSlotDelegate ? AddressSlotDelegate(0xFF) : (void)0;
	WaitingResponse = FALSE;
}