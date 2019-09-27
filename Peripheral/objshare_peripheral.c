#include "objshare_peripheral.h"

/* Private typedefs ----------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
static void pduReceivedEventHandler(ObjshareProtocol_PduType_t pduType,
									uint8_t objId, uint16_t unparsedPduSize);

static ObjsharePeripheral_Object_t *getObj(uint8_t objId);
static uint8_t getObjIdx(uint8_t objId);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static ObjsharePeripheral_State_t State = OBJSHARE_PERIPHERAL_STATE_UNINIT;

// Object table related data.
static ObjsharePeripheral_Object_t
	ObjectTable[OBJSHARE_PERIPHERAL_MAX_NUMBER_OF_CHARS];

static uint8_t NumOfObjects;

// Delegates.
static ObjsharePeripheral_EventOccurredDelegate_t EventOccurredDelegate;
static ObjsharePeripheral_IsAddressedDelegate_t IsAddressedDelegate;

/* Public function implementations. ------------------------------------------*/
void ObjsharePeripheral_Setup(ObjsharePeripheral_EventOccurredDelegate_t eventHandler,
							  ObjsharePeripheral_IsAddressedDelegate_t isAddressedEventHandler,
							  ObjshareProtocol_SwitchDirectionDelegate_t switchedEventHandler)
{
	ObjshareProtocol_Setup(&pduReceivedEventHandler, switchedEventHandler);

	// Set delegates.
	EventOccurredDelegate = eventHandler;
	IsAddressedDelegate = isAddressedEventHandler;

	// Set parameters.
	NumOfObjects = 0;

	// Set state to ready.
	State = OBJSHARE_PERIPHERAL_STATE_READY;
}

void ObjsharePeripheral_Register(uint8_t objId, void *obj, uint16_t objSize, uint8_t properties)
{
	ObjectTable[NumOfObjects].objId = objId;
	ObjectTable[NumOfObjects].data = (uint8_t *)obj;
	ObjectTable[NumOfObjects].length = objSize;
	ObjectTable[NumOfObjects++].properties = properties;
}

void ObjsharePeripheral_DeRegister(uint8_t objId)
{
	uint8_t obj_idx = getObjIdx(objId);

	if (obj_idx == 0xFF)
	{
		return;
	}

	// Write last object's pointer to deleted object's slot.
	if (NumOfObjects > 0)
	{
		ObjectTable[obj_idx] = ObjectTable[--NumOfObjects];
	}
}

void ObjsharePeripheral_Start(void)
{
	// Start object protocol.
	ObjshareProtocol_Start();

	// Set state to operating.
	State = OBJSHARE_PERIPHERAL_STATE_OPERATING;
}

void ObjsharePeripheral_Execute(void)
{
	if (State != OBJSHARE_PERIPHERAL_STATE_OPERATING)
	{
		return;
	}

	// Call submodule's executer.
	ObjshareProtocol_Execute();
}

void ObjsharePeripheral_Stop(void)
{
	// Stop object protocol.
	ObjshareProtocol_Stop();

	// Set state to ready.
	State = OBJSHARE_PERIPHERAL_STATE_READY;
}

ObjsharePeripheral_Object_t *ObjsharePeripheral_ParseObject(uint8_t objId)
{
	return &ObjectTable[getObjIdx(objId)];
}

/* Private function implementations ------------------------------------------*/
static void pduReceivedEventHandler(ObjshareProtocol_PduType_t pduType,
									uint8_t objId, uint16_t unparsedPduSize)
{
	// If not connected and operating send response. Discard the PDU otherwise.
	if (State != OBJSHARE_PERIPHERAL_STATE_OPERATING)
	{
		return;
	}

	Bool_t is_addressed;
	is_addressed = IsAddressedDelegate ? IsAddressedDelegate() : TRUE;

	if (!is_addressed)
	{
		return;
	}

	switch (pduType)
	{
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ:
	{
		ObjsharePeripheral_Object_t *object = getObj(objId);

		// If the object is readable; read and send it.
		if (object && (object->properties & OBJSHARE_PERIPHERAL_OBJ_PROPERTY_READ))
		{
			EventOccurredDelegate ? EventOccurredDelegate(OBJSHARE_PERIPHERAL_READ_CHAR_EVENT,
														  objId)
								  : (void)0;

			ObjshareProtocol_Send(OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP,
								  OPERATION_RESULT_SUCCESS,
								  object->data, object->length);
		}
		else
		{
			ObjshareProtocol_Send(OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP,
								  OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		ObjsharePeripheral_Object_t *object = getObj(objId);

		// If the object is writable; write it.
		if (object && (object->properties & OBJSHARE_PERIPHERAL_OBJ_PROPERTY_WRITE))
		{
			// Parse pdu data.
			ObjshareProtocol_ParsePduData(object->data, object->length,
										  unparsedPduSize);

			EventOccurredDelegate ? EventOccurredDelegate(OBJSHARE_PERIPHERAL_WRITE_CHAR_EVENT,
														  objId)
								  : (void)0;

			// Send succeeded response.
			ObjshareProtocol_Send(OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP,
								  OPERATION_RESULT_SUCCESS, 0, 0);
		}
		else
		{
			// Send failure response.
			ObjshareProtocol_Send(OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP,
								  OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_POLL_REQ:
	{
		// Say I'm here!
		ObjshareProtocol_Send(OBJSHARE_PROTOCOL_PDUTYPE_POLL_RESP,
							  0, 0, 0);
	}
	break;

	default:
		break;
	}
}

static ObjsharePeripheral_Object_t *getObj(uint8_t objId)
{
	for (uint8_t i = 0; i < NumOfObjects; i++)
	{
		if (ObjectTable[i].objId == objId)
		{
			return &ObjectTable[i];
		}
	}
	return 0;
}

static uint8_t getObjIdx(uint8_t objId)
{
	for (uint8_t i = 0; i < NumOfObjects; i++)
	{
		if (ObjectTable[i].objId == objId)
		{
			return i;
		}
	}
	return 0xFF;
}