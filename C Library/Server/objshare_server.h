#ifndef __OBJSHARE_SERVER_H
#define __OBJSHARE_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "generic.h"

/* Exported definitions ----------------------------------------------------*/
#define OBJSHARE_SERVER_MAX_NUMBER_OF_CHARS 32
#define OBJSHARE_SERVER_CHAR_PROPERTY_READ 0x01
#define OBJSHARE_SERVER_CHAR_PROPERTY_WRITE 0x02

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		OBJSHARE_SERVER_STATE_UNINIT = 0x00,
		OBJSHARE_SERVER_STATE_READY,
		OBJSHARE_SERVER_STATE_OPERATING
	};
	typedef uint8_t ObjshareServer_State_t;

	enum
	{
		OBJSHARE_SERVER_READ_CHAR_EVENT = 0x00,
		OBJSHARE_SERVER_WRITE_CHAR_EVENT
	};
	typedef uint8_t ObjshareServer_Event_t;

	// Delegates.
	typedef void (*ObjshareServer_EventOccurredDelegate_t)(ObjshareServer_Event_t event,
																 uint8_t charId);

	// Objshare struct.
	typedef struct
	{
		uint8_t objId;
		uint8_t *data;
		uint16_t length;
		uint8_t properties;
	} ObjshareServer_Object_t;

	/* Exported functions --------------------------------------------------------*/
	// Functions controlling module behaviour.
	extern void ObjshareServer_Setup(uint8_t homeAddress,
		ObjshareServer_EventOccurredDelegate_t eventHandler);
	extern void ObjshareServer_Register(uint8_t objId, void *obj, uint16_t objSize, 
		uint8_t properties);
	extern void ObjshareServer_DeRegister(uint8_t objId);
	extern void ObjshareServer_Start(void);
	extern void ObjshareServer_Execute(void);
	extern void ObjshareServer_Stop(void);

	// Functions to control object server database.
	extern ObjshareServer_Object_t *
		ObjshareServer_ParseObject(uint8_t objId);

#ifdef __cplusplus
}
#endif

#endif