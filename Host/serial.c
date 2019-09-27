#include "stm32f4xx_hal.h"
#include "serial.h"

#define RING_BUFFER_MASK (SERIAL_RING_BUFFER_SIZE - 1)
#define HALF_BUFFER_SIZE (SERIAL_RING_BUFFER_SIZE >> 1)
#define MID_ELEMENT (HALF_BUFFER_SIZE - 1)

/* Private variables -------------------------------------------------------*/
// Receive buffer.
static uint8_t Buffer[SERIAL_RING_BUFFER_SIZE];
static volatile uint16_t BufferReadIdx;

// Flags.
static volatile Bool_t TxIdle;
static volatile Bool_t TxCompleted;
static volatile Bool_t RestartReceive;

// Delegates.
static Serial_EventOccurredDelegate_t EventOccurredDelegate;

/* Exported variables ------------------------------------------------------*/
extern UART_HandleTypeDef SERIAL_UART_HANDLE;

void Serial_Setup(Serial_EventOccurredDelegate_t eventHandler) {
	EventOccurredDelegate = eventHandler;
}

Bool_t Serial_Start(void) {
	// Fill the buffer with flag character.
	for (uint16_t i = 0; i < SERIAL_RING_BUFFER_SIZE; i++) {
		Buffer[i] = SERIAL_VOID_CHARACTER;
	}

	// Initialize flags.
	TxIdle = TRUE;
	TxCompleted = FALSE;
	RestartReceive = FALSE;

	// Initialize buffer.
	BufferReadIdx = 0;

	// Start receiving in circular manner.
	if (HAL_UART_Receive_DMA(&SERIAL_UART_HANDLE, Buffer, sizeof(Buffer)) != HAL_OK) {
		return FALSE;
	}

	return TRUE;
}

void Serial_Execute(void) {
	uint16_t size = 0;
	uint16_t __pos;
	__pos = BufferReadIdx;

	// DMA had written until the void character.
	while (Buffer[__pos] != SERIAL_VOID_CHARACTER) {
		__pos++;
		__pos &= RING_BUFFER_MASK;

		if (++size >= SERIAL_RING_BUFFER_SIZE) {
			break;
		}
	}

	// If there are any element in the buffer, call the delegate function.
	if (size) {
		EventOccurredDelegate ?
				EventOccurredDelegate(SERIAL_EVENT_DATA_READY,
						&Buffer[BufferReadIdx], size) :
				(void) 0;

		// Clear transferred items.
		for (uint16_t i = 0; i < size; i++) {
			Buffer[BufferReadIdx++] = SERIAL_VOID_CHARACTER;
			BufferReadIdx &= RING_BUFFER_MASK;
		}
	}

	if (TxIdle) {
		EventOccurredDelegate ?
				EventOccurredDelegate(SERIAL_EVENT_TX_IDLE, 0, 0xFFFF) :
				(void) 0;
	}

	if (TxCompleted)
	{
		EventOccurredDelegate ? EventOccurredDelegate(SERIAL_EVENT_TX_COMPLETED, 0, 0xFFFF) : (void)0;
		TxCompleted = FALSE;
	}

	// If some error occurred; restart receive.
	if (RestartReceive) {
		RestartReceive = FALSE;

		// Start receiving in circular manner.
		if (HAL_UART_Receive_DMA(&SERIAL_UART_HANDLE, Buffer, sizeof(Buffer)) != HAL_OK) {
			while (1)
				;
		}
	}
}

void Serial_Stop(void) {
	if (HAL_UART_DMAStop(&SERIAL_UART_HANDLE) != HAL_OK) {
		while (1)
			;
	}
}

void Serial_Send(uint8_t *data, uint16_t dataLength) {
	TxIdle = FALSE;

	if (HAL_UART_Transmit_DMA(&SERIAL_UART_HANDLE, data, dataLength) != HAL_OK) {
		while (1)
			;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &SERIAL_UART_HANDLE) {
		TxCompleted = TRUE;
		TxIdle = TRUE;
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (huart == &SERIAL_UART_HANDLE) {
		RestartReceive = TRUE;
	}
}