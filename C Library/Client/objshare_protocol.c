/***
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "packet_manager.h"
#include "objshare_protocol.h"

/* Private function prototypes -----------------------------------------------*/
static void packetManagerEventHandler(PacketManager_Event_t event, uint16_t unparsedPduSize);

/* Private variable declarations ---------------------------------------------*/
// State variables.
static ObjshareProtocol_State_t State = OBJSHARE_PROTOCOL_STATE_UNINIT;

// Variables to store some setup data.
static ObjshareProtocol_PduReceivedDelegate_t PduReceivedDelegate;
static uint8_t HomeAddress;

/* Exported functions --------------------------------------------------------*/
void ObjshareProtocol_Setup(uint8_t homeAddress,
							ObjshareProtocol_PduReceivedDelegate_t pduReceivedEventHandler)
{
	HomeAddress = homeAddress;

	// Packet manager setup.
	PacketManager_Setup(&packetManagerEventHandler);

	// Store setup data(delegates).
	PduReceivedDelegate = pduReceivedEventHandler;

	// Set state.
	State = OBJSHARE_PROTOCOL_STATE_READY;
}

void ObjshareProtocol_Start(void)
{
	// Start packet manager.
	PacketManager_Start();

	// Set state.
	State = OBJSHARE_PROTOCOL_STATE_OPERATING;
}

void ObjshareProtocol_Execute(void)
{
	if (State != OBJSHARE_PROTOCOL_STATE_OPERATING)
	{
		return;
	}

	// Execute sub-module.
	PacketManager_Execute();
}

void ObjshareProtocol_Stop(void)
{
	// Stop submodule.
	PacketManager_Stop();
}

#ifdef OBJSHARE_PROTOCOL_SERVER_SIDE
void ObjshareProtocol_Send(uint8_t destinationAddress, ObjshareProtocol_PduType_t pduType,
						   OperationResult_t operationResult,
						   uint8_t *data, uint16_t dataLength)
#else
void ObjshareProtocol_Send(uint8_t destinationAddress, ObjshareProtocol_PduType_t pduType,
						   uint8_t objId, uint8_t *data, uint16_t dataLength)
#endif
{
	if (State != OBJSHARE_PROTOCOL_STATE_OPERATING)
	{
		return;
	}

	PacketManager_PduField_t pdu_fields[5];
	uint8_t idx = 0;

	// Add source address.
	pdu_fields[idx].data = (uint8_t *)&HomeAddress;
	pdu_fields[idx++].length = sizeof(HomeAddress);

	// Add destination address.
	pdu_fields[idx].data = (uint8_t *)&destinationAddress;
	pdu_fields[idx++].length = sizeof(destinationAddress);

	// Add type field.
	pdu_fields[idx].data = (uint8_t *)&pduType;
	pdu_fields[idx++].length = sizeof(pduType);

	// Add pdu specific fields.
	switch (pduType)
	{
#ifdef OBJSHARE_PROTOCOL_SERVER_SIDE
	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP:
	{
		// Add operation result.
		pdu_fields[idx].data = (uint8_t *)&operationResult;
		pdu_fields[idx++].length = sizeof(operationResult);
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP:
	{
		// Add operation result.
		pdu_fields[idx].data = (uint8_t *)&operationResult;
		pdu_fields[idx++].length = sizeof(operationResult);

		// Add data.
		pdu_fields[idx].data = data;
		pdu_fields[idx++].length = dataLength;
	}
	break;
#else
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ:
	{
		// Add object id.
		pdu_fields[idx].data = &objId;
		pdu_fields[idx++].length = sizeof(objId);
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		// Add object id.
		pdu_fields[idx].data = &objId;
		pdu_fields[idx++].length = sizeof(objId);

		// Add data.
		pdu_fields[idx].data = data;
		pdu_fields[idx++].length = dataLength;
	}
	break;
#endif

	default:
		break;
	}

	PacketManager_Send(pdu_fields, idx);
}

void ObjshareProtocol_ParsePduData(uint8_t *data, uint16_t length, uint16_t unparsedPduSize)
{
	PacketManager_ParseField(data, length, unparsedPduSize);
}

ObjshareProtocol_State_t ObjshareProtocol_GetState(void)
{
	return State;
}

/* Private functions -------------------------------------------------------*/
void packetManagerEventHandler(PacketManager_Event_t event, uint16_t unparsedPduSize)
{
	if (event == PACKET_MANAGER_ERROR_OCCURRED_EVENT)
	{
		// Call packet manager error handler.
		PacketManager_ErrorHandler();
		return;
	}

	if (State != OBJSHARE_PROTOCOL_STATE_OPERATING)
	{
		return;
	}

	uint8_t source_address;
	uint8_t destination_address;
	ObjshareProtocol_PduType_t pdu_type;
	uint16_t unparsed_pdu_size;

#ifdef OBJSHARE_PROTOCOL_SERVER_SIDE
	uint8_t obj_id = 0;
#else
	OperationResult_t operation_result = OPERATION_RESULT_SUCCESS;
#endif

	// Parse source address.
	unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&source_address,
												 sizeof(source_address), unparsedPduSize);

	// Parse destination address.
	unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&destination_address,
												 sizeof(destination_address), unparsedPduSize);

	// Message isn't intended to this device; discard it!
	if (destination_address != HomeAddress)
	{
		return;
	}

	// Parse pdu type.
	unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&pdu_type,
												 sizeof(pdu_type), unparsedPduSize);

	switch (pdu_type)
	{
#ifdef OBJSHARE_PROTOCOL_SERVER_SIDE
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ:
	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		unparsed_pdu_size = PacketManager_ParseField(&obj_id, sizeof(obj_id), unparsedPduSize);
	}
	break;

#else
	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP:
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP:
	{
		unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&operation_result,
													 sizeof(operation_result), unparsedPduSize);
	}
	break;
#endif

	default:
		break;
	}

#ifdef OBJSHARE_PROTOCOL_SERVER_SIDE
	PduReceivedDelegate(source_address, pdu_type, obj_id, unparsed_pdu_size);
#else
	PduReceivedDelegate(source_address, pdu_type, operation_result, unparsed_pdu_size);
#endif
}