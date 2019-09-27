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

// Variables to store delegate pointers.
static ObjshareProtocol_PduReceivedDelegate_t PduReceivedDelegate;
static ObjshareProtocol_SwitchDirectionDelegate_t SwitchDirectionDelegate;

/* Exported functions --------------------------------------------------------*/
#ifdef OBJSHARE_PROTOCOL_HOST
extern void ObjshareProtocol_Setup(ObjshareProtocol_PduReceivedDelegate_t pduReceivedEventHandler,
								   ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler)
#else
extern void ObjshareProtocol_Setup(ObjshareProtocol_PduReceivedDelegate_t pduReceivedEventHandler,
								   ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler)

#endif
{
	// Packet manager setup.
	PacketManager_Setup(&packetManagerEventHandler);

	// Store setup data(delegates).
	PduReceivedDelegate = pduReceivedEventHandler;
	SwitchDirectionDelegate = switchDirectionEventHandler;

	// Set state.
	State = OBJSHARE_PROTOCOL_STATE_READY;
}

void ObjshareProtocol_Start(void)
{
	// Start packet manager.
	PacketManager_Start();

	SwitchDirectionDelegate ? SwitchDirectionDelegate(OBJSHARE_PROTOCOL_DIRECTION_RX) : (void)0;

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

#ifdef OBJSHARE_PROTOCOL_HOST
void ObjshareProtocol_Send(uint8_t slot, ObjshareProtocol_PduType_t pduType,
						   uint8_t objId, uint8_t *data, uint16_t dataLength)
#else
void ObjshareProtocol_Send(ObjshareProtocol_PduType_t pduType, OperationResult_t operationResult,
						   uint8_t *data, uint16_t dataLength)

#endif
{
	if (State != OBJSHARE_PROTOCOL_STATE_OPERATING)
	{
		return;
	}

	PacketManager_PduField_t pdu_fields[4];
	uint8_t idx = 0;

	SwitchDirectionDelegate ? SwitchDirectionDelegate(OBJSHARE_PROTOCOL_DIRECTION_TX)
							: (void)0;

	// Add type field.
	pdu_fields[idx].data = (uint8_t *)&pduType;
	pdu_fields[idx++].length = sizeof(pduType);

	// Add pdu specific fields.
	switch (pduType)
	{
#ifdef OBJSHARE_PROTOCOL_HOST
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
#else
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

	// Packet transmission is completed and switch to receiving state.
	if (event == PACKET_MANAGER_TRANSMISSION_COMPLETED_EVENT)
	{
		SwitchDirectionDelegate ? SwitchDirectionDelegate(OBJSHARE_PROTOCOL_DIRECTION_RX)
								: (void)0;

		return;
	}

	ObjshareProtocol_PduType_t pdu_type;
	uint16_t unparsed_pdu_size;

#ifdef OBJSHARE_PROTOCOL_HOST
	OperationResult_t operation_result = OPERATION_RESULT_SUCCESS;
#else
	uint8_t obj_id = 0;
#endif

	// Parse pdu type.
	unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&pdu_type,
												 sizeof(pdu_type), unparsedPduSize);

	switch (pdu_type)
	{
#ifdef OBJSHARE_PROTOCOL_HOST
	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_RESP:
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_RESP:
	{
		unparsed_pdu_size = PacketManager_ParseField((uint8_t *)&operation_result,
													 sizeof(operation_result), unparsedPduSize);
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_POLL_RESP:
	break;
#else
	case OBJSHARE_PROTOCOL_PDUTYPE_READ_REQ:
	case OBJSHARE_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		unparsed_pdu_size = PacketManager_ParseField(&obj_id, sizeof(obj_id), unparsedPduSize);
	}
	break;

	case OBJSHARE_PROTOCOL_PDUTYPE_POLL_REQ:
	break;
#endif

	default:
		break;
	}

#ifdef OBJSHARE_PROTOCOL_HOST
	PduReceivedDelegate(pdu_type, operation_result, unparsed_pdu_size);
#else
	PduReceivedDelegate(pdu_type, obj_id, unparsed_pdu_size);
#endif
}