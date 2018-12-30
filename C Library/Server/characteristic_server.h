#ifndef CHARACTERISTIC_SERVER_H
#define CHARACTERISTIC_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "generic.h"

/***
 * TODO: Add notification system. It can be integrated to queue system.
 */

/* Exported definitions ----------------------------------------------------*/
#define CHARACTERISTIC_SERVER_MAX_NUMBER_OF_CHARS 32
#define CHARACTERISTIC_SERVER_CONNECTION_DROPOUT_PERIOD_IN_MS 1500
#define CHARACTERISTIC_SERVER_CHAR_PROPERTY_READ 0x01
#define CHARACTERISTIC_SERVER_CHAR_PROPERTY_WRITE 0x02
#define CHARACTERISTIC_SERVER_CHAR_PROPERTY_NOTIFICATION 0x04

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		CHARACTERISTIC_SERVER_STATE_UNINIT = 0x00,
		CHARACTERISTIC_SERVER_STATE_READY,
		CHARACTERISTIC_SERVER_STATE_OPERATING
	};
	typedef uint8_t CharacteristicServer_State_t;

	enum
	{
		CHARACTERISTIC_SERVER_TIMEOUT_EVENT = 0,
		CHARACTERISTIC_SERVER_CONNECTION_EVENT,
		CHARACTERISTIC_SERVER_DISCONNECTION_EVENT,
		CHARACTERISTIC_SERVER_READ_CHAR_EVENT,
		CHARACTERISTIC_SERVER_WRITE_CHAR_EVENT
	};
	typedef uint8_t CharacteristicServer_Event_t;

	// Delegates.
	typedef void (*CharacteristicServer_EventOccurredDelegate_t)(CharacteristicServer_Event_t event,
																 uint8_t charId);

	// Characteristic struct.
	typedef struct
	{
		uint8_t charId;
		uint8_t *data;
		uint16_t length;
		uint8_t properties;
	} CharacteristicServer_Characteristic_t;

	/* Exported functions --------------------------------------------------------*/
	// Functions controlling module behaviour.
	extern void CharacteristicServer_Setup(CharacteristicServer_EventOccurredDelegate_t eventHandler,
										   uint8_t *deviceId, uint8_t deviceIdLength);
	extern void CharacteristicServer_Register(uint8_t charId, void *obj, uint16_t objSize, uint8_t properties);
	extern void CharacteristicServer_DeRegister(uint8_t charId);
	extern void CharacteristicServer_Start(uint8_t channelId);
	extern void CharacteristicServer_Execute(void);
	extern void CharacteristicServer_Stop(void);

	// Functions to control serial communication.
	extern uint8_t CharacteristicServer_MapAvailableChannels(void);

	// Functions to control characteristic database.
	extern CharacteristicServer_Characteristic_t *CharacteristicServer_ParseCharacteristic(uint8_t charId);

#ifdef __cplusplus
}
#endif

#endif