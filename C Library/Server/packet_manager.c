/**
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "serial.h"
#include "crc.h"
#include "utils.h"
#include "packet_manager.h"

/* Private definitions -------------------------------------------------------*/
#define RECEIVE_BUFFER_SIZE (1 << PACKET_MANAGER_RECEIVE_QUEUE_CAPACITY)
#define TRANSMIT_BUFFER_SIZE (1 << PACKET_MANAGER_TRANSMIT_QUEUE_CAPACITY)

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
static void dequeueAndDecode(Queue_Buffer_t *buff, uint8_t *data, uint16_t *dataLength, uint16_t rawLength);
static void encodeAndEnqueue(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength);
static void serialEventHandler(Serial_Event_t event);

/* Private variables ---------------------------------------------------------*/
static PacketManager_EventOccurredDelegate_t EventOccurredDelegate;

static uint8_t State = PACKET_MANAGER_STATE_UNINIT;

static uint8_t ReceiveBufferContainer[RECEIVE_BUFFER_SIZE];
static uint8_t TransmitBufferContainer[TRANSMIT_BUFFER_SIZE];

static uint8_t Temp[RECEIVE_BUFFER_SIZE];
static uint16_t TempStartIndex;

static Queue_Buffer_t ReceiveBuffer;
static Queue_Buffer_t TransmitBuffer;

static Bool_t ErrorOccurredFlag;
static Bool_t DataReceivedFlag;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Setup function for UART controller module.
  *
  * @Params     eventHandler-> Upper layer event handler function.
  */
void PacketManager_Setup(PacketManager_EventOccurredDelegate_t eventHandler)
{
	assert_param(State == PACKET_MANAGER_STATE_UNINIT);

	// Initialize buffers.
	Queue_InitBuffer(&ReceiveBuffer, ReceiveBufferContainer,
					 PACKET_MANAGER_RECEIVE_QUEUE_CAPACITY);
	Queue_InitBuffer(&TransmitBuffer, TransmitBufferContainer,
					 PACKET_MANAGER_TRANSMIT_QUEUE_CAPACITY);

	EventOccurredDelegate = eventHandler;

	// Register delegates.
	Serial_Setup(&serialEventHandler);

	State = PACKET_MANAGER_STATE_READY;
}

uint8_t PacketManager_MapAvailableChannels(void)
{
	return ((uint8_t)Serial_MapAvailableChannels());
}

void PacketManager_ChangeChannel(uint8_t channelId)
{
    // Clear buffers.
	Queue_ClearBuffer(&ReceiveBuffer);
	Queue_ClearBuffer(&TransmitBuffer);

    TempStartIndex = 0;

	// Clear flags.
	ErrorOccurredFlag = FALSE;
	DataReceivedFlag = FALSE;

	Serial_ChangeChannel(channelId);
}

/***
  * @Brief      Sets start event.
  */
Bool_t PacketManager_Start(uint8_t channelId)
{
	assert_param(State == PACKET_MANAGER_STATE_READY);

	// Start serial.
	if (!Serial_Start(channelId))
	{
		return FALSE;
	}

	// Clear buffers.
	Queue_ClearBuffer(&ReceiveBuffer);
	Queue_ClearBuffer(&TransmitBuffer);

	// Clear flags.
	ErrorOccurredFlag = FALSE;
	DataReceivedFlag = FALSE;

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

	// If error occurred, set state to error and call delegate.
	if (ErrorOccurredFlag)
	{
        ErrorOccurredFlag = FALSE;
		State = PACKET_MANAGER_STATE_ERROR;
		Serial_Stop();

		EventOccurredDelegate ? EventOccurredDelegate(PACKET_MANAGER_ERROR_OCCURRED_EVENT, 0) : (void)0;
	}
	else if (DataReceivedFlag)
	{
		DataReceivedFlag = FALSE;
		uint16_t length;
		uint8_t buff[16];

		// Read buffer.
		Serial_ReadBuffer(buff, sizeof(buff), &length);

		// Enqueue elements to queue.
		Queue_EnqueueArr(&ReceiveBuffer, buff, length);
	}
	// Check if delimiter received.
	else
	{
		// Receive buffer related processes.
		{
			uint16_t start_index = Queue_Search(&ReceiveBuffer, START_CHARACTER);

			// Check start element.
			if (start_index != 0xFFFF)
			{
                // Remove till start index.
				if (start_index != 0)
				{
					Queue_Remove(&ReceiveBuffer, start_index);
				}

                // Search for terminate character.
				uint16_t terminate_index = Queue_Search(&ReceiveBuffer, TERMINATE_CHARACTER);

				// May seem like long sequence of operations but they are only activated when packet has been received.
				if (terminate_index != 0xFFFF)
				{
					uint16_t checksummed_pdu_size;

					// Dequeue start character.
					Queue_Dequeue(&ReceiveBuffer);

                    dequeueAndDecode(&ReceiveBuffer, Temp, &checksummed_pdu_size, terminate_index - 1);

					// Validate pdu.
					if (!CRC_Calculate16(0xFFFF, Temp, checksummed_pdu_size))
                    {
                        TempStartIndex = 0;

						// Call event occurred delegate.
						EventOccurredDelegate ? EventOccurredDelegate(PACKET_MANAGER_PDU_RECEIVED_EVENT,
																	  checksummed_pdu_size - sizeof(uint16_t))
											  : (void)0;
					}

					// Dequeue terminate character.
					Queue_Dequeue(&ReceiveBuffer);
				}
				else
				{
					Queue_Remove(&ReceiveBuffer, terminate_index + 1);
				}
			}
		}
	}

	// Transmit buffer related processes.
	{
		// While the serial buffer is available and there are data to send,
		// append them to serial port.
        uint8_t buff[32];
		uint16_t available_space = Serial_GetAvailableSpace();
		uint16_t element_count = Queue_GetElementCount(&TransmitBuffer);
		uint16_t bytes_to_write = (available_space < element_count) ? available_space : element_count;
		bytes_to_write = (sizeof(buff) < bytes_to_write) ? sizeof(buff) : bytes_to_write;

		if (bytes_to_write)
		{
            Queue_DequeueArr(&TransmitBuffer, buff, bytes_to_write);
            Serial_Send(buff, bytes_to_write);
		}
	}
}

void PacketManager_Stop(void)
{
	assert_param(State == PACKET_MANAGER_STATE_OPERATING);

	Serial_Stop();

	State = PACKET_MANAGER_STATE_READY;
}

void PacketManager_Send(PacketManager_PduField_t *pduFields, uint8_t pduFieldCount)
{
	assert_param((State == PACKET_MANAGER_STATE_OPERATING) &&
				 !pPduField && !pduFieldCount);

	if (ErrorOccurredFlag)
	{
		return;
	}

	uint16_t crc_code = 0xFFFF;
	Queue_Enqueue(&TransmitBuffer, START_CHARACTER);

	// Encode and enqueue pdu fields..
	for (uint8_t i = 0; i < pduFieldCount; i++)
	{
		encodeAndEnqueue(&TransmitBuffer, pduFields[i].data, pduFields[i].length);
		crc_code = CRC_Calculate16(crc_code, pduFields[i].data, pduFields[i].length);
	}

	// Encode and enqueue crc code.
    encodeAndEnqueue(&TransmitBuffer, &((uint8_t *)&crc_code)[1], sizeof(uint8_t));
    encodeAndEnqueue(&TransmitBuffer, &((uint8_t *)&crc_code)[0], sizeof(uint8_t));

	Queue_Enqueue(&TransmitBuffer, TERMINATE_CHARACTER);
}

uint16_t PacketManager_ParseField(uint8_t *data, uint16_t length, uint16_t unparsedPduSize)
{
	assert_param((State == PACKET_MANAGER_STATE_OPERATING) && !pPduField);

	uint16_t parse_length = (unparsedPduSize < length) ? unparsedPduSize : length;

	Utils_MemoryCopy(&Temp[TempStartIndex], data, parse_length);

	TempStartIndex += parse_length;

	return (unparsedPduSize - parse_length);
}

/***
  * @Brief      Handles module errors. Recovers corruption free messages and clears the
  *             hardware errors.
  */
void PacketManager_ErrorHandler(void)
{
	assert_param(State == PACKET_MANAGER_STATE_ERROR);

	// Set state to ready.
	State = PACKET_MANAGER_STATE_READY;
}

/* Private functions ---------------------------------------------------------*/
static void serialEventHandler(Serial_Event_t event)
{
	switch (event)
	{
	case SERIAL_EVENT_DATA_READY:
	{
		DataReceivedFlag = TRUE;
	}
	break;

	case SERIAL_EVENT_ERROR_OCCURRED:
	{
		ErrorOccurredFlag = TRUE;
	}
	break;
	}
}

static void dequeueAndDecode(Queue_Buffer_t *buff, uint8_t *data, uint16_t *dataLength, uint16_t rawLength)
{
	uint8_t element;
	Bool_t escape_mode = FALSE;

	(*dataLength) = 0;

	for (uint16_t i = 0; i < rawLength; i++)
	{
		element = Queue_Dequeue(buff);

		if (escape_mode)
		{
			// Decode character.
			switch (element)
			{
			case ESCAPE_CHARACTER_CODE:
				element = ESCAPE_CHARACTER;
				break;

			case START_CHARACTER_CODE:
				element = START_CHARACTER;
				break;

			default:
			case TERMINATE_CHARACTER_CODE:
				element = TERMINATE_CHARACTER;
				break;
			}

			escape_mode = FALSE;
		}
		else if (element == ESCAPE_CHARACTER)
		{
			escape_mode = TRUE;
			continue;
		}

        data[(*dataLength)++] = element;
	}
}

static void encodeAndEnqueue(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength)
{
	uint8_t element;

    for (uint16_t i = 0; i < dataLength; i++)
	{
		element = data[i];

		switch (element)
		{
		case START_CHARACTER:
			Queue_Enqueue(buff, ESCAPE_CHARACTER);
			Queue_Enqueue(buff, START_CHARACTER_CODE);
			break;

		case TERMINATE_CHARACTER:
			Queue_Enqueue(buff, ESCAPE_CHARACTER);
			Queue_Enqueue(buff, TERMINATE_CHARACTER_CODE);
			break;

		case ESCAPE_CHARACTER:
            Queue_Enqueue(buff, ESCAPE_CHARACTER);
			Queue_Enqueue(buff, ESCAPE_CHARACTER_CODE);
			break;

		default:
			Queue_Enqueue(buff, element);
			break;
		}
	}
}
