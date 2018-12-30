#include "characteristic_protocol.h"
#include "characteristic_server.h"
#include "sys_time.h"

/* Private typedefs ----------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
static void pduReceivedEventHandler(CharacteristicProtocol_PduType_t pduType,
									uint8_t charId, uint16_t unparsedPduSize);

static CharacteristicServer_Characteristic_t *getChar(uint8_t charId);
static uint8_t getCharIdx(uint8_t charId);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static CharacteristicServer_State_t State = CHARACTERISTIC_SERVER_STATE_UNINIT;
static Bool_t IsConnected;
static uint32_t LastRequestTimestamp;

// Device Id. Used when device represent itself.
static uint8_t *DeviceId;
static uint8_t DeviceIdLength;

// Characteristic table related data.
static CharacteristicServer_Characteristic_t
	CharacteristicTable[CHARACTERISTIC_SERVER_MAX_NUMBER_OF_CHARS];

static uint8_t NumOfChars;

static CharacteristicServer_EventOccurredDelegate_t EventOccurredDelegate;

/* Public function implementations. ------------------------------------------*/
void CharacteristicServer_Setup(CharacteristicServer_EventOccurredDelegate_t eventHandler,
								uint8_t *deviceId, uint8_t deviceIdLength)
{
	assert_param(State == CHARACTERISTIC_SERVER_STATE_UNINIT);

	CharacteristicProtocol_Setup(&pduReceivedEventHandler);

	DeviceId = deviceId;
	DeviceIdLength = deviceIdLength;

	// Set delegates.
	EventOccurredDelegate = eventHandler;

	// Set parameters.
	NumOfChars = 0;

	// Set state to ready.
	State = CHARACTERISTIC_SERVER_STATE_READY;
}

void CharacteristicServer_Register(uint8_t charId, void *obj, uint16_t objSize, uint8_t properties)
{
	assert_param((State != CHARACTERISTIC_SERVER_STATE_UNINIT) &&
				 !pObj && !objSize && (NumOfChars < MAX_NUMBER_OF_CHARACTERISTICS));

	CharacteristicTable[NumOfChars].charId = charId;
	CharacteristicTable[NumOfChars].data = (uint8_t *)obj;
	CharacteristicTable[NumOfChars].length = objSize;
	CharacteristicTable[NumOfChars++].properties = properties;
}

void CharacteristicServer_DeRegister(uint8_t charId)
{
	assert_param(State != CHARACTERISTIC_SERVER_STATE_UNINIT);

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

void CharacteristicServer_Start(uint8_t channelId)
{
	assert_param(State == CHARACTERISTIC_SERVER_STATE_READY);

	// Start characteristic protocol.
	CharacteristicProtocol_Start(channelId);

	// Set not connected.
	IsConnected = FALSE;

	// Set state to operating.
	State = CHARACTERISTIC_SERVER_STATE_OPERATING;
}

void CharacteristicServer_Execute(void)
{
	assert_param(State == CHARACTERISTIC_SERVER_STATE_OPERATING);

	// Call submodule's executer.
	CharacteristicProtocol_Execute();

	// Check if connection dropout time has been passed.
	if (IsConnected && ((SysTime_GetTimeInMs() - LastRequestTimestamp) >
						CHARACTERISTIC_SERVER_CONNECTION_DROPOUT_PERIOD_IN_MS))
	{
		IsConnected = FALSE;

		// Call event occurred delegate.
		EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_DISCONNECTION_EVENT, 0) : (void)0;
	}
}

void CharacteristicServer_Stop(void)
{
	assert_param(State == CHARACTERISTIC_SERVER_STATE_OPERATING);

	// Stop characteristic protocol.
	CharacteristicProtocol_Stop();

	// Set state to ready.
	State = CHARACTERISTIC_SERVER_STATE_READY;
}

uint8_t CharacteristicServer_MapAvailableChannels(void)
{
	return CharacteristicProtocol_MapAvailableChannels();
}

CharacteristicServer_Characteristic_t *CharacteristicServer_ParseCharacteristic(uint8_t charId)
{
	assert_param(State != CHARACTERISTIC_SERVER_STATE_UNINIT);

	return &CharacteristicTable[getCharIdx(charId)];
}

/* Private function implementations ------------------------------------------*/
static void pduReceivedEventHandler(CharacteristicProtocol_PduType_t pduType,
									uint8_t charId, uint16_t unparsedPduSize)
{
	// If not connected and operating send response. Discard the PDU otherwise.
	if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
	{
		return;
	}

	// Update last request timestamp.
	LastRequestTimestamp = SysTime_GetTimeInMs();

	switch (pduType)
	{
	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_REQ:
	{
		if (!IsConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_CHECK_RESP, 0,
										DeviceId, DeviceIdLength);
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_REQ:
	{
		if (!IsConnected)
		{
			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_CONNECTION_EVENT, 0) : (void)0;

			IsConnected = TRUE;
		}

		CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_CONNECTION_RESP,
									OPERATION_RESULT_SUCCESS, 0, 0);
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_REQ:
	{
		if (IsConnected)
		{
			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_DISCONNECTION_EVENT, 0) : (void)0;

			IsConnected = FALSE;
		}

		CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_DISCONNECTION_RESP, 0, 0, 0);
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_REQ:
	{
		if (IsConnected)
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_POLL_RESP, 0, 0, 0);
		}
	}
	break;

	case CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_REQ:
	{
		CharacteristicServer_Characteristic_t *characteristic = getChar(charId);

		// If the characteristic is readable; read and send it.
		if (characteristic && (characteristic->properties & CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ))
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP,
										OPERATION_RESULT_SUCCESS,
										characteristic->data, characteristic->length);
			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_READ_CHAR_EVENT, charId) : (void)0;
		}
		else
		{
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_READ_RESP,
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
			CharacteristicProtocol_ParsePduData(characteristic->data, characteristic->length, unparsedPduSize);

			EventOccurredDelegate ? EventOccurredDelegate(CHARACTERISTIC_SERVER_WRITE_CHAR_EVENT, charId) : (void)0;
		
			// Send succeeded response.
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP,
										OPERATION_RESULT_SUCCESS, 0, 0);
		}
		else
		{
			// Send failure response.
			CharacteristicProtocol_Send(CHARACTERISTIC_PROTOCOL_PDUTYPE_WRITE_RESP,
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