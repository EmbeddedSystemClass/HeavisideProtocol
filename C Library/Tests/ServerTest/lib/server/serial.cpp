#include <Arduino.h>
#include "serial.h"
#include "queue.h"

Serial_EventOccurredDelegate_t EventOccurredDelegate;

void Serial_Setup(Serial_EventOccurredDelegate_t eventHandler)
{
    EventOccurredDelegate = eventHandler;
}

Bool_t Serial_Start(void)
{
    Serial2.begin(115200);

    return TRUE;
}

void Serial_Execute(void)
{
    if (Serial2.available())
    {
        EventOccurredDelegate ? EventOccurredDelegate(SERIAL_EVENT_DATA_READY) : (void)0;
    }
}

void Serial_Stop(void)
{
}

uint16_t Serial_GetAvailableSpace(void)
{
    return Serial2.availableForWrite();
}

void Serial_ReadBuffer(uint8_t *data, uint16_t maxLength, uint16_t *length)
{
    uint16_t read_len;
    uint16_t available_byte_count;
    available_byte_count = Serial2.available();

    read_len = (available_byte_count > maxLength) ? maxLength : available_byte_count;
    Serial2.readBytes(data, read_len);
    *length = read_len;
}

void Serial_Send(uint8_t *data, uint16_t dataLength)
{
    Serial2.write(data, dataLength);
}