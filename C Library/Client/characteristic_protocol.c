/***
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "packet_manager.h"
#include "characteristic_protocol.h"
#include "debug.h"

/* Private function prototypes -----------------------------------------------*/
static void packetManagerEventHandler(PacketManager_Event_t event, uint16_t unparsedPduSize);

/* Private variable declarations ---------------------------------------------*/
// State variables.
static CharacteristicProtocol_State_t State = CHARACTERISTIC_PROTOCOL_STATE_UNINIT;

// Variables to store some setup data.
static CharacteristicProtocol_PduReceivedDelegate_t PduReceivedDelegate;
static CharacteristicProtocol_ErrorOccurredDelegate_t ErrorOccurredDelegate;

/* Exported functions --------------------------------------------------------*/
void CharacteristicProtocol_Setup(CharacteristicProtocol_PduReceivedDelegate_t pduReceivedEventHandler,
																	CharacteristicProtocol_ErrorOccurredDelegate_t errorOccurredEventHandler)
{
	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_UNINIT);

	// Packet manager setup.
	PacketManager_Setup(&packetManagerEventHandler);

	// Store setup data(delegates).
	PduReceivedDelegate = pduReceivedEventHandler;
	ErrorOccurredDelegate = errorOccurredEventHandler;

	// Set state.
	State = CHARACTERISTIC_PROTOCOL_STATE_READY;
}

uint8_t CharacteristicProtocol_MapAvailableChannels(void)
{
	return PacketManager_MapAvailableChannels();
}

Bool_t CharacteristicProtocol_Start(uint8_t channelId)
{
	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_READY);

	// Start packet manager.
	if (!PacketManager_Start(channelId))
	{
		return FALSE;
	}

	// Set state.
	State = CHARACTERISTIC_PROTOCOL_STATE_OPERATING;

	return TRUE;
}

void CharacteristicProtocol_ChangeChannel(uint8_t channelId)
{
	PacketManager_ChangeChannel(channelId);
}

void CharacteristicProtocol_Execute(void)
{
	if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
	{
		return;
	}

	// Execute sub-module.
	PacketManager_Execute();
}

void CharacteristicProtocol_Stop(void)
{
	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_OPERATING);

	// Stop submodule.
	PacketManager_Stop();
}

void CharacteristicProtocol_ErrorHandler(void)
{
	State = CHARACTERISTIC_PROTOCOL_STATE_READY;

	PacketManager_ErrorHandler();
}

#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
void CharacteristicProtocol_Send(CharacteristicProtocol_PduType_t pduType,
								 OperationResult_t operationResult,
								 uint8_t *data, uint16_t dataLength)
#else
void CharacteristicProtocol_Send(CharacteristicProtocol_PduType_t pduType, uint8_t charId,
								 uint8_t *data, uint16_t dataLength)
#endif
{
	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_OPERATING);

	PacketManager_PduField_t pdu_fields[3];
	uint8_t idx = 0;

	// Add type field.
	pdu_fields[idx].data = (uint8_t *)&pduType;
	pdu_fields[idx++].length = sizeof(pduType);

	// Add pdu specific fields.
	switch (pduType)
	{
#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_RESP:
	{
		pdu_fields[idx].data = data;
		pdu_fields[idx++].length = dataLength;
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_RESP:
		break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_RESP:
	{
		pdu_fields[idx].data = (uint8_t *)&operationResult;
		pdu_fields[idx++].length = sizeof(operationResult);
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP:
	{
		pdu_fields[idx].data = (uint8_t *)&operationResult;
		pdu_fields[idx++].length = sizeof(operationResult);

		pdu_fields[idx].data = data;
		pdu_fields[idx++].length = dataLength;
	}
	break;
#else
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_REQ:
		break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ:
	{
		pdu_fields[idx].data = &charId;
		pdu_fields[idx++].length = sizeof(charId);
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		pdu_fields[idx].data = &charId;
		pdu_fields[idx++].length = sizeof(charId);

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

void CharacteristicProtocol_ParsePduData(uint8_t *data, uint16_t length, uint16_t unparsedPduSize)
{
	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_OPERATING);

	PacketManager_ParseField(data, length, unparsedPduSize);
}

CharacteristicProtocol_State_t CharacteristicProtocol_GetState(void)
{
	return State;
}

/* Private functions -------------------------------------------------------*/
void packetManagerEventHandler(PacketManager_Event_t event, uint16_t unparsedPduSize)
{
	if (event == PACKET_MANAGER_ERROR_OCCURRED_EVENT)
	{
		// Call upper layer error handler.
		ErrorOccurredDelegate ? ErrorOccurredDelegate() : (void)0;
		State = CHARACTERISTIC_PROTOCOL_STATE_ERROR;

		return;
	}

	assert_param(State == CHARACTERISTIC_PROTOCOL_STATE_OPERATING);

	CharacteristicProtocol_PduType_t pdu_type;
	uint16_t unparsed_pdu_size;

#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	uint8_t char_id = 0;
#else
	OperationResult_t operation_result = OPERATION_RESULT_SUCCESS;
#endif

	// Parse pdu type.
	unparsed_pdu_size = PacketManager_ParseField(&pdu_type, sizeof(pdu_type),
																							 unparsedPduSize);

	switch (pdu_type)
	{
#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_REQ:
		break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ:
		unparsed_pdu_size = PacketManager_ParseField(&char_id, sizeof(char_id), unparsedPduSize);
		break;
#else
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_RESP:
		break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP:
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP:
		unparsed_pdu_size = PacketManager_ParseField(&operation_result,
													 sizeof(operation_result), unparsed_pdu_size);
		break;
#endif

	default:
		break;
	}

#ifdef CHARACTERISTIC_PROTOCOL_SERVER_SIDE
	PduReceivedDelegate(pdu_type, char_id, unparsed_pdu_size);
#else
	PduReceivedDelegate(pdu_type, operation_result, unparsed_pdu_size);
#endif
}
