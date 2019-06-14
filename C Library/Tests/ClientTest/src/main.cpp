#include <Arduino.h>
#include "objshare_client.h"

#define HOME_ADDRESS 0x00
#define SERVER_ADDRESS 0x01
#define TEST_COUNT 10

// #define __BASIC_READ_WRITE_TEST 1
#define __POLAR_BEAR_TEST 1

#ifdef __BASIC_READ_WRITE_TEST
static uint8_t char_read_u8;
static uint16_t char_read_u16;
static uint32_t char_read_u32;
static float char_read_float;

static uint8_t char_write_u8;
static uint16_t char_write_u16;
static uint32_t char_write_u32;
static float char_write_float;

static uint8_t *char_write[] = {(uint8_t *)&char_write_u8, (uint8_t *)&char_write_u16,
                                (uint8_t *)&char_write_u32, (uint8_t *)&char_write_float};
static uint8_t *char_read[] = {(uint8_t *)&char_read_u8, (uint8_t *)&char_read_u16,
                               (uint8_t *)&char_read_u32, (uint8_t *)&char_read_float};

static uint8_t char_ids[] = {0, 1, 2, 3};
static uint8_t char_sizes[] = {sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t),
                               sizeof(float)};
#endif

#ifdef __POLAR_BEAR_TEST
#define PLATE_TEMPERATURE_OBJ_ID 4
#define TEMPERATURE_READ_PERIOD 500

static float PlateTemperature;
static uint32_t LastTemperatureReadTime;
#endif

static void idleEventHandler(void);
static void readResponseReceived(uint8_t sourceAddr, uint8_t charId);
static void operationFailed(uint8_t sourceAddr);
static void noResponse(uint8_t sourceAddr);
static void test(const char *message, bool eq);

static bool isIdle;

void setup()
{
  delay(1000);

  ObjshareClient_Delegates_t delegates;

  delegates.noResponseDelegate = noResponse;
  delegates.operationFailedDelegate = operationFailed;
  delegates.readResponseReceivedDelegate = readResponseReceived;
  delegates.idleStateDelegate = idleEventHandler;

  ObjshareClient_Setup(HOME_ADDRESS, &delegates);

  Serial.begin(9600);

  ObjshareClient_Start();

  LastTemperatureReadTime = 0;
}

void loop()
{
  static int test_cnt = 0;

  ObjshareClient_Execute();

// Enqueue write requests.
#ifdef __BASIC_READ_WRITE_TEST
  if (isIdle)
  {
    if (test_cnt < TEST_COUNT)
    {
      char_read_u8 = 0;
      char_read_u16 = 0;
      char_read_u32 = 0;
      char_read_float = 0.0f;

      char_write_u8 = random(0, ((1 << 7)));
      char_write_u16 = random(0, ((1UL << 15)));
      char_write_u32 = random(0, ((1UL << 31)));
      char_write_float = (float)random();

      for (uint8_t i = 0; i < sizeof(char_ids) / sizeof(char_ids[0]); i++)
      {
        ObjshareClient_SendWriteRequest(SERVER_ADDRESS,
                                        i, char_write[i], char_sizes[i]);
      }

      // Enqueue read requests for written chars.
      for (uint8_t i = 0; i < sizeof(char_ids) / sizeof(char_ids[0]); i++)
      {
        ObjshareClient_SendReadRequest(SERVER_ADDRESS,
                                       char_ids[i], char_read[i],
                                       char_sizes[i]);
      }

      isIdle = false;
      test_cnt++;
    }
  }
#endif

#ifdef __POLAR_BEAR_TEST
  uint32_t __time;
  __time = millis();
  if (__time > LastTemperatureReadTime + TEMPERATURE_READ_PERIOD)
  {
    ObjshareClient_SendReadRequest(SERVER_ADDRESS, PLATE_TEMPERATURE_OBJ_ID, (uint8_t *)&PlateTemperature,
                                   sizeof(PlateTemperature));
    LastTemperatureReadTime = __time;
  }
#endif
}

void test(const char *message, bool eq)
{
#ifdef __BASIC_READ_WRITE_TEST
  Serial.print(message);
  Serial.println(eq ? "S" : "F");
#endif
}

void idleEventHandler(void)
{
#ifdef __BASIC_READ_WRITE_TEST
  if (!isIdle)
  {
    test("u8:", char_read_u8 == char_write_u8);
    test("u16:", char_read_u16 == char_write_u16);
    test("u32:", char_read_u32 == char_write_u32);
    test("float:", char_read_float == char_write_float);

    isIdle = true;
  }
#endif
}

void readResponseReceived(uint8_t sourceAddr, uint8_t charId)
{
#ifdef __POLAR_BEAR_TEST
  String __str;
  __str = "Plate temperature: ";
  __str += PlateTemperature;

  Serial.println(__str);
#endif
#ifdef __BASIC_READ_WRITE_TEST
  Serial.println("Read response received");
#endif
}

void operationFailed(uint8_t sourceAddr)
{
#ifdef __BASIC_READ_WRITE_TEST
  Serial.println("Operation failed");
#endif
}

void noResponse(uint8_t sourceAddr)
{
#ifdef __BASIC_READ_WRITE_TEST
  Serial.println("No response");
#endif
}