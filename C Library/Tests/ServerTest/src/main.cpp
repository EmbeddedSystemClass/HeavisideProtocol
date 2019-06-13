#include <Arduino.h>
#include "objshare_server.h"

#define HOME_ADDRESS 0x01

static void eventHandler(ObjshareServer_Event_t event, uint8_t charId);

uint8_t char_u8;
uint16_t char_u16;
uint32_t char_u32;
float char_float;

void setup()
{
  ObjshareServer_Setup(HOME_ADDRESS, eventHandler);

  ObjshareServer_Register(0, &char_u8, sizeof(char_u8),
                                (OBJSHARE_SERVER_CHAR_PROPERTY_READ | OBJSHARE_SERVER_CHAR_PROPERTY_WRITE));
  ObjshareServer_Register(1, &char_u16, sizeof(char_u16),
                                (OBJSHARE_SERVER_CHAR_PROPERTY_READ | OBJSHARE_SERVER_CHAR_PROPERTY_WRITE));
  ObjshareServer_Register(2, &char_u32, sizeof(char_u32),
                                (OBJSHARE_SERVER_CHAR_PROPERTY_READ | OBJSHARE_SERVER_CHAR_PROPERTY_WRITE));
  ObjshareServer_Register(3, &char_float, sizeof(char_float),
                                (OBJSHARE_SERVER_CHAR_PROPERTY_READ | OBJSHARE_SERVER_CHAR_PROPERTY_WRITE));

  ObjshareServer_Start();
}

void loop()
{
  ObjshareServer_Execute();
}

void eventHandler(ObjshareServer_Event_t event, uint8_t charId)
{
}