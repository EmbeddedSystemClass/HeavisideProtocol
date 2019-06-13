#include <Arduino.h>
#include "serial.h"

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
    uint8_t available_bytes;
    available_bytes = Serial2.available();
    
    if (available_bytes)
    {
        uint8_t read_length;
        uint8_t buff[16];

        read_length = sizeof(buff) > available_bytes ? available_bytes : sizeof(buff);
        Serial2.readBytes(buff, read_length);

        EventOccurredDelegate ? EventOccurredDelegate(SERIAL_EVENT_DATA_READY,
        buff, read_length) : (void)0;
    }

    uint8_t available_space;
    available_space = Serial2.availableForWrite();
    
    if (available_space)
    {
        EventOccurredDelegate ? EventOccurredDelegate(SERIAL_EVENT_TX_IDLE,
        0, available_space) : (void)0;
    } 
}

void Serial_Stop(void)
{
}

uint16_t Serial_GetAvailableSpace(void)
{
    return Serial2.availableForWrite();
}

void Serial_Send(uint8_t *data, uint16_t dataLength)
{
    Serial2.write(data, dataLength);
}