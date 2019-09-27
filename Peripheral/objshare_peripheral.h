#ifndef __OBJSHARE_PERIPHERAL_H
#define __OBJSHARE_PERIPHERAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "generic.h"
#include "objshare_protocol.h"

/* Exported definitions ----------------------------------------------------*/
#define OBJSHARE_PERIPHERAL_MAX_NUMBER_OF_CHARS 16
#define OBJSHARE_PERIPHERAL_OBJ_PROPERTY_READ 0x01
#define OBJSHARE_PERIPHERAL_OBJ_PROPERTY_WRITE 0x02

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		OBJSHARE_PERIPHERAL_STATE_UNINIT = 0x00,
		OBJSHARE_PERIPHERAL_STATE_READY,
		OBJSHARE_PERIPHERAL_STATE_OPERATING
	};
	typedef uint8_t ObjsharePeripheral_State_t;

	enum
	{
		OBJSHARE_PERIPHERAL_READ_CHAR_EVENT = 0x00,
		OBJSHARE_PERIPHERAL_WRITE_CHAR_EVENT
	};
	typedef uint8_t ObjsharePeripheral_Event_t;

	// Delegates.
	typedef void (*ObjsharePeripheral_EventOccurredDelegate_t)(ObjsharePeripheral_Event_t event,
															   uint8_t objId);
	typedef Bool_t (*ObjsharePeripheral_IsAddressedDelegate_t)(void);

	// Objshare struct.
	typedef struct
	{
		uint8_t objId;
		uint8_t *data;
		uint16_t length;
		uint8_t properties;
	} ObjsharePeripheral_Object_t;

	/* Exported functions --------------------------------------------------------*/
	// Functions controlling module behaviour.
	extern void ObjsharePeripheral_Setup(ObjsharePeripheral_EventOccurredDelegate_t eventHandler,
										 ObjsharePeripheral_IsAddressedDelegate_t isAddressedEventHandler,
										 ObjshareProtocol_SwitchDirectionDelegate_t switchDirectionEventHandler);

	extern void ObjsharePeripheral_Register(uint8_t objId, void *obj, uint16_t objSize,
											uint8_t properties);
	extern void ObjsharePeripheral_DeRegister(uint8_t objId);
	extern void ObjsharePeripheral_Start(void);
	extern void ObjsharePeripheral_Execute(void);
	extern void ObjsharePeripheral_Stop(void);

	// Functions to control object server database.
	extern ObjsharePeripheral_Object_t *ObjsharePeripheral_ParseObject(uint8_t objId);

#ifdef __cplusplus
}
#endif

#endif
