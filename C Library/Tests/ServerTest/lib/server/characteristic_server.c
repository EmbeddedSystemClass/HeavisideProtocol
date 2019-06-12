#include "characteristic_protocol.h"
#include "characteristic_server.h"

/* Private typedefs ----------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
static void pduReceivedEventHandler(uint8_t sourceAddress, CharacteristicProtocol_PduType_t pduType,
									uint8_t charId, uint16_t unparsedPduSize);

static CharacteristicServer_Characteristic_t *getChar(uint8_t charId);
static uint8_t getCharIdx(uint8_t charId);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static CharacteristicServer_State_t State = CHARACTERISTIC_SERVER_STATE_UNINIT;

// Characteristic table related data.
static CharacteristicServer_Characteristic_t
	CharacteristicTable[CHARACTERISTIC_SERVER_MAX_NUMBER_OF_CHARS];

static uint8_t NumOfChars;
static CharacteristicServer_EventOccurredDelegate_t EventOccurredDelegate;

/* Public function implementations. ------------------------------------------*/
void CharacteristicServer_Setup(uint8_t homeAddress,
								CharacteristicServer_EventOccurredDelegate_t eventHandler)
{
	CharacteristicProtocol_Setup(homeAddress, &pduReceivedEventHandler);

	// Set delegates.
	EventOccurredDelegate = eventHandler;

	// Set parameters.
	NumOfChars = 0;

	// Set state to ready.
	State = CHARACTERISTIC_SERVER_STATE_READY;
}

void CharacteristicServer_Register(uint8_t charId, void *obj, uint16_t objSize, uint8_t properties)
{
	CharacteristicTable[NumOfChars].charId = charId;
	CharacteristicTable[NumOfChars].data = (uint8_t *)obj;
	CharacteristicTable[NumOfChars].length = objSize;
	CharacteristicTable[NumOfChars++].properties = properties;
}

void CharacteristicServer_DeRegister(uint8_t charId)
{
	uint8_t char_idx = getCharIdx(charId);

	if (char_idx == 0xFF)
	{
		return;
	}

	// Write last characteristic's pointer to deleted characteristic's slot.
	if (NumOfChars > 0)
	{
		CharacteristicTable[char_idx] = CharacteristicTable[--NumOfChars];
	}
}

void CharacteristicServer_Start(void)
{
	// Start characteristic protocol.
	CharacteristicProtocol_Start();

	// Set state to operating.
	State = CHARACTERISTIC_SERVER_STATE_OPERATING;
}

void CharacteristicServer_Execute(void)
{
	if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
	{
		return;
	}

	// Call submodule's executer.
	CharacteristicProtocol_Execute();
}

void CharacteristicServer_Stop(void)
{
	// Stop characteristic protocol.
	CharacteristicProtocol_Stop();

	// Set state to ready.
	State = CHARACTERISTIC_SERVER_STATE_READY;
}

CharacteristicServer_Characteristic_t *CharacteristicServer_ParseCharacteristic(uint8_t charId)
{
	return &CharacteristicTable[getCharIdx(charId)];
}

/* Private function implementations ------------------------------------------*/
static void pduReceivedEventHandler(uint8_t sourceAddress,
									CharacteristicProtocol_PduType_t pduType,
									uint8_t charId, uint16_t unparsedPduSize)
{
	// If not connected and operating send response. Discard the PDU otherwise.
	if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
	{
		return;
	}

	switch (pduType)
	{
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ:
	{
		CharacteristicServer_Characteristic_t *characteristic = getChar(charId);

		// If the characteristic is readable; read and send it.
		if (characteristic && (characteristic->properties & CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ))
		{
			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_READ_CHAR_EVENT,
														  charId)
								  : (void)0;

			CharacteristicProtocol_Send(sourceAddress, CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP,
										OPERATION_RESULT_SUCCESS,
										characteristic->data, characteristic->length);
		}
		else
		{
			CharacteristicProtocol_Send(sourceAddress, CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP,
										OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_REQ:
	{
		CharacteristicServer_Characteristic_t *characteristic = getChar(charId);

		// If the characteristic is writable; write it.
		if (characteristic && (characteristic->properties & CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE))
		{
			// Parse pdu data.
			CharacteristicProtocol_ParsePduData(characteristic->data, characteristic->length,
												unparsedPduSize);

			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_WRITE_CHAR_EVENT,
														  charId)
								  : (void)0;

			// Send succeeded response.
			CharacteristicProtocol_Send(sourceAddress, CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP,
										OPERATION_RESULT_SUCCESS, 0, 0);
		}
		else
		{
			// Send failure response.
			CharacteristicProtocol_Send(sourceAddress, CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP,
										OPERATION_RESULT_FAILURE, 0, 0);
		}
	}
	break;

	default:
		break;
	}
}

// Characteristic id is basically the index of the characteristic at the characteristic table.
static CharacteristicServer_Characteristic_t *getChar(uint8_t charId)
{
	for (uint8_t i = 0; i < NumOfChars; i++)
	{
		if (CharacteristicTable[i].charId == charId)
		{
			return &CharacteristicTable[i];
		}
	}
	return 0;
}

static uint8_t getCharIdx(uint8_t charId)
{
	for (uint8_t i = 0; i < NumOfChars; i++)
	{
		if (CharacteristicTable[i].charId == charId)
		{
			return i;
		}
	}
	return 0xFF;
}