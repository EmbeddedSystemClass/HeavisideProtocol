#include "objshare_protocol.h"
#include "objshare_server.h"

/* Private typedefs ----------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
static void pduReceivedEventHandler(uint8_t sourceAddress, ObjshareProtocol_PduType_t pduType,
									uint8_t objId, uint16_t unparsedPduSize);

static ObjshareServer_Object_t *getChar(uint8_t objId);
static uint8_t getCharIdx(uint8_t objId);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static ObjshareServer_State_t State = OBJSHARE_SERVER_STATE_UNINIT;

// Object table related data.
static ObjshareServer_Object_t
	ObjectTable[OBJSHARE_SERVER_MAX_NUMBER_OF_CHARS];

static uint8_t NumOfChars;
static ObjshareServer_EventOccurredDelegate_t EventOccurredDelegate;

/* Public function implementations. ------------------------------------------*/
void ObjshareServer_Setup(uint8_t homeAddress,
						  ObjshareServer_EventOccurredDelegate_t eventHandler)
{
	ObjshareProtocol_Setup(homeAddress, &pduReceivedEventHandler);

	// Set delegates.
	EventOccurredDelegate = eventHandler;

	// Set parameters.
	NumOfChars = 0;

	// Set state to ready.
	State = OBJSHARE_SERVER_STATE_READY;
}

void ObjshareServer_Register(uint8_t objId, void *obj, uint16_t objSize, uint8_t properties)
{
	ObjectTable[NumOfChars].objId = objId;
	ObjectTable[NumOfChars].data = (uint8_t *)obj;
	ObjectTable[NumOfChars].length = objSize;
	ObjectTable[NumOfChars++].properties = properties;
}

void ObjshareServer_DeRegister(uint8_t objId)
{
	uint8_t char_idx = getCharIdx(objId);

	if (char_idx == 0xFF)
	{
		return;
	}

	// Write last object's pointer to deleted object's slot.
	if (NumOfChars > 0)
	{
		ObjectTable[char_idx] = ObjectTable[--NumOfChars];
	}
}

void ObjshareServer_Start(void)
{
	// Start object protocol.
	ObjshareProtocol_Start();

	// Set state to operating.
	State = OBJSHARE_SERVER_STATE_OPERATING;
}

void ObjshareServer_Execute(void)
{
	if (State != OBJSHARE_SERVER_STATE_OPERATING)
	{
		return;
	}

	// Call submodule's executer.
	ObjshareProtocol_Execute();
}

void ObjshareServer_Stop(void)
{
	// Stop object protocol.
	ObjshareProtocol_Stop();

	// Set state to ready.
	State = OBJSHARE_SERVER_STATE_READY;
}

ObjshareServer_Object_t *ObjshareServer_ParseObject(uint8_t objId)
{
	return &ObjectTable[getCharIdx(objId)];
}

/* Private function implementations ------------------------------------------*/
static void pduReceivedEventHandler(uint8_t sourceAddress,
									ObjshareProtocol_PduType_t pduType,
									uint8_t objId, uint16_t unparsedPduSize)
{
	// If not connected and operating send response. Discard the PDU otherwise.
	if (State != OBJSHARE_SERVER_STATE_OPERATING)
	{
		return;
	}

	switch (pduType)
	{
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ:
	{
		ObjshareServer_Object_t *object = getChar(objId);

		// If the object is readable; read and send it.
		if (object && (object->properties & OBJSHARE_SERVER_CHAR_PROPERTY_READ))
		{
			EventOccurredDelegate ? EventOccurredDelegate(OBJSHARE_SERVER_READ_CHAR_EVENT,
														  objId)
								  : (void)0;

			ObjshareProtocol_Send(sourceAddress, OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP,
								  OPERATION_RESULT_SUCCESS,
								  object->data, object->length);
		}
		else
		{
			ObjshareProtocol_Send(sourceAddress, OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP,
								  OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		ObjshareServer_Object_t *object = getChar(objId);

		// If the object is writable; write it.
		if (object && (object->properties & OBJSHARE_SERVER_CHAR_PROPERTY_WRITE))
		{
			// Parse pdu data.
			ObjshareProtocol_ParsePduData(object->data, object->length,
										  unparsedPduSize);

			EventOccurredDelegate ? EventOccurredDelegate(OBJSHARE_SERVER_WRITE_CHAR_EVENT,
														  objId)
								  : (void)0;

			// Send succeeded response.
			ObjshareProtocol_Send(sourceAddress, OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP,
								  OPERATION_RESULT_SUCCESS, 0, 0);
		}
		else
		{
			// Send failure response.
			ObjshareProtocol_Send(sourceAddress, OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP,
								  OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	default:
		break;
	}
}

static ObjshareServer_Object_t *getChar(uint8_t objId)
{
	for (uint8_t i = 0; i < NumOfChars; i++)
	{
		if (ObjectTable[i].objId == objId)
		{
			return &ObjectTable[i];
		}
	}
	return 0;
}

static uint8_t getCharIdx(uint8_t objId)
{
	for (uint8_t i = 0; i < NumOfChars; i++)
	{
		if (ObjectTable[i].objId == objId)
		{
			return i;
		}
	}
	return 0xFF;
}