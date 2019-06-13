/**
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "serial.h"
#include "packet_manager.h"
#include "crc.h"

/* Private typedefs ----------------------------------------------------------*/
enum
{
	START_CHARACTER = 0x0D,
	TERMINATE_CHARACTER = 0x3A,
	ESCAPE_CHARACTER = 0x3B
};
typedef uint8_t SpecialCharacter_t;

enum
{
	START_CHARACTER_CODE = 0x00,
	TERMINATE_CHARACTER_CODE = 0x01,
	ESCAPE_CHARACTER_CODE = 0x02
};
typedef uint8_t SpecialCharacterEscapeCode_t;

/* Private function prototypes -----------------------------------------------*/
static uint8_t decode(uint8_t element);
static void encodeToBuffer(uint8_t *src, uint8_t *dest, uint16_t srcLength, uint16_t *destLength);
static void serialEventHandler(Serial_Event_t event, uint8_t *data, uint16_t length);

/* Private variables ---------------------------------------------------------*/
static PacketManager_EventOccurredDelegate_t EventOccurredDelegate;

static uint8_t State = PACKET_MANAGER_STATE_UNINIT;

static uint8_t Inbox[PACKET_MANAGER_MAX_PACKET_SIZE];
static uint8_t InboxDataLength;
static uint8_t InboxIdx;
static uint8_t InboxParseIdx;

static uint8_t Outbox[PACKET_MANAGER_MAX_PACKET_SIZE];
static uint8_t OutboxDataLength;
static uint8_t OutboxIdx;

static Bool_t PacketStartedFlag;
static Bool_t EscapeMode;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Setup function for UART controller module.
  *
  * @Params     eventHandler-> Upper layer event handler function.
  */
void PacketManager_Setup(PacketManager_EventOccurredDelegate_t eventHandler)
{
	EventOccurredDelegate = eventHandler;

	// Register delegates.
	Serial_Setup(&serialEventHandler);

	State = PACKET_MANAGER_STATE_READY;
}

/***
  * @Brief      Sets start event.
  */
Bool_t PacketManager_Start(void)
{
	// Start serial.
	if (!Serial_Start())
	{
		return FALSE;
	}

	// Clear buffers.
	InboxDataLength = 0;
	InboxIdx = 0;
	OutboxIdx = 0;
	OutboxDataLength = 0;
	
	// Clear flags.
	EscapeMode = FALSE;
	PacketStartedFlag = FALSE;

	State = PACKET_MANAGER_STATE_OPERATING;

	return TRUE;
}

/***
  * @Brief      Module executer function.
  */
void PacketManager_Execute(void)
{
	if (State != PACKET_MANAGER_STATE_OPERATING)
	{
		return;
	}

	// Call serial execute.
	Serial_Execute();
}

void PacketManager_Stop(void)
{
	Serial_Stop();

	State = PACKET_MANAGER_STATE_READY;
}

void PacketManager_Send(PacketManager_PduField_t *pduFields, uint8_t pduFieldCount)
{
	// Discard if not operating.
	if (State != PACKET_MANAGER_STATE_OPERATING)
	{
		return;
	}

	// Check if there is any data waiting.
	if ((OutboxDataLength != 0) && (OutboxDataLength != OutboxIdx))
	{
		return;
	}

	uint16_t crc_code = 0xFFFF;
	uint8_t outbox_idx = 0;
	uint16_t __dest_length;

	Outbox[outbox_idx++] = START_CHARACTER;

	// Encode and enqueue pdu fields..
	for (uint8_t i = 0; i < pduFieldCount; i++)
	{
		encodeToBuffer(pduFields[i].data, &Outbox[outbox_idx], pduFields[i].length, &__dest_length);
		outbox_idx += __dest_length;

		crc_code = CRC_Calculate16(crc_code, pduFields[i].data, pduFields[i].length);
	}

	// Encode and enqueue crc code.
	encodeToBuffer(&((uint8_t *)&crc_code)[1], &Outbox[outbox_idx], sizeof(uint8_t), &__dest_length);
	outbox_idx += __dest_length;

	encodeToBuffer(&((uint8_t *)&crc_code)[0], &Outbox[outbox_idx], sizeof(uint8_t), &__dest_length);
	outbox_idx += __dest_length;

	Outbox[outbox_idx++] = TERMINATE_CHARACTER;

	OutboxDataLength = outbox_idx;
	OutboxIdx = 0;
}

uint16_t PacketManager_ParseField(uint8_t *data, uint16_t length, uint16_t unparsedPduSize)
{
	uint16_t parse_length = (unparsedPduSize < length) ? unparsedPduSize : length;

	for (uint16_t i = 0; i < parse_length; i++)
	{
		data[i] = Inbox[InboxParseIdx++];
	}
	
	return (unparsedPduSize - parse_length);
}

void PacketManager_ErrorHandler(void)
{
	// Set state to ready.
	State = PACKET_MANAGER_STATE_READY;
}

/* Private functions ---------------------------------------------------------*/
static void serialEventHandler(Serial_Event_t event, uint8_t *data, uint16_t length)
{
	switch (event)
	{
	case SERIAL_EVENT_TX_IDLE:
	{
		uint16_t awaiting_element_count = OutboxDataLength - OutboxIdx;

		if (awaiting_element_count)
		{
			uint16_t bytes_to_write = (length < awaiting_element_count) ? length : awaiting_element_count;
			bytes_to_write =
				(PACKET_MANAGER_MAX_PACKET_SIZE < bytes_to_write) ? PACKET_MANAGER_MAX_PACKET_SIZE : bytes_to_write;

			Serial_Send(&Outbox[OutboxIdx], bytes_to_write);
			OutboxIdx += bytes_to_write;
		}
	}
	break;

	case SERIAL_EVENT_DATA_READY:
	{
		for (uint8_t i = 0; i < length; i++)
		{
			switch (data[i])
			{
			case START_CHARACTER:
			{
				InboxDataLength = 0;
				InboxIdx = 0;
				EscapeMode = FALSE;
				PacketStartedFlag = TRUE;
			}
			break;

			case TERMINATE_CHARACTER:
			{
				if (PacketStartedFlag)
				{
					// Validate pdu.
					if (!CRC_Calculate16(0xFFFF, Inbox, InboxIdx))
					{
						InboxParseIdx = 0;

						// Call event occurred delegate.
						EventOccurredDelegate ? EventOccurredDelegate(PACKET_MANAGER_PDU_RECEIVED_EVENT,
																		  InboxIdx - sizeof(uint16_t))
												  : (void)0;
					}

					PacketStartedFlag = FALSE;
				}
			}
			break;

			case ESCAPE_CHARACTER:
			{
				EscapeMode = TRUE;
			}
			break;

			default:
			{
				if (PacketStartedFlag)
				{
					// Discard this packet since it exceeded the packet size.
					if (InboxDataLength == PACKET_MANAGER_MAX_PACKET_SIZE)
					{
						PacketStartedFlag = FALSE;
					}

					if (EscapeMode)
					{
						Inbox[InboxIdx++] = decode(data[i]);
						EscapeMode = FALSE;
					}
					else
					{
						Inbox[InboxIdx++] = data[i];
					}
				}
			}
			break;
			}
		}
	}
	break;

	case SERIAL_EVENT_ERROR_OCCURRED:
	{
		State = PACKET_MANAGER_STATE_ERROR;
		Serial_Stop();

		EventOccurredDelegate ? EventOccurredDelegate(PACKET_MANAGER_ERROR_OCCURRED_EVENT,
													  0)
							  : (void)0;
	}
	break;
	}
}

static uint8_t decode(uint8_t element)
{
	uint8_t __element;

	// Decode character.
	switch (element)
	{
	case ESCAPE_CHARACTER_CODE:
		__element = ESCAPE_CHARACTER;
		break;

	case START_CHARACTER_CODE:
		__element = START_CHARACTER;
		break;

	default:
	case TERMINATE_CHARACTER_CODE:
		__element = TERMINATE_CHARACTER;
		break;
	}

	return __element;
}

static void encodeToBuffer(uint8_t *src, uint8_t *dest, uint16_t srcLength, uint16_t *destLength)
{
	uint8_t element;
	uint16_t __dest_length = 0;

	for (uint16_t i = 0; i < srcLength; i++)
	{
		element = src[i];

		switch (element)
		{
		case START_CHARACTER:
			dest[__dest_length++] = ESCAPE_CHARACTER;
			dest[__dest_length++] = START_CHARACTER_CODE;
			break;

		case TERMINATE_CHARACTER:
			dest[__dest_length++] = ESCAPE_CHARACTER;
			dest[__dest_length++] = TERMINATE_CHARACTER_CODE;
			break;

		case ESCAPE_CHARACTER:
			dest[__dest_length++] = ESCAPE_CHARACTER;
			dest[__dest_length++] = ESCAPE_CHARACTER_CODE;
			break;

		default:
			dest[__dest_length++] = element;
			break;
		}
	}

	*destLength = __dest_length;
}