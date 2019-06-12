#include <Arduino.h>
#include "characteristic_server.h"

#define HOME_ADDRESS 0x01

static void eventHandler(CharacteristicServer_Event_t event, uint8_t charId);

uint8_t char_u8;
uint16_t char_u16;
uint32_t char_u32;
float char_float;

void setup()
{
  CharacteristicServer_Setup(HOME_ADDRESS, eventHandler);

  CharacteristicServer_Register(0, &char_u8, sizeof(char_u8),
                                (CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ | CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE));
  CharacteristicServer_Register(1, &char_u16, sizeof(char_u16),
                                (CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ | CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE));
  CharacteristicServer_Register(2, &char_u32, sizeof(char_u32),
                                (CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ | CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE));
  CharacteristicServer_Register(3, &char_float, sizeof(char_float),
                                (CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ | CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE));

  CharacteristicServer_Start();
}

void loop()
{
  CharacteristicServer_Execute();
}

void eventHandler(CharacteristicServer_Event_t event, uint8_t charId)
{
}