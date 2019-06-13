/**
  * @author     Onur Efe
  */

#ifndef __PACKET_MANAGER_H
#define __PACKET_MANAGER_H

/* Include files -------------------------------------------------------------*/
#include "generic.h"

#ifdef __cplusplus
extern "C"
{
#endif
	/* Exported definitions -----------------------------------------------------*/
#define PACKET_MANAGER_MAX_PACKET_SIZE 64

	/* Exported types ------------------------------------------------------------*/
	enum
	{
		PACKET_MANAGER_STATE_UNINIT = 0,
		PACKET_MANAGER_STATE_READY,
		PACKET_MANAGER_STATE_OPERATING,
		PACKET_MANAGER_STATE_ERROR,
	};
	typedef uint8_t PacketManager_State_t;

	enum
	{
		PACKET_MANAGER_PDU_RECEIVED_EVENT = 0,
		PACKET_MANAGER_ERROR_OCCURRED_EVENT
	};
	typedef uint8_t PacketManager_Event_t;

	typedef struct
	{
		uint8_t *data;
		uint16_t length;
	} PacketManager_PduField_t;

	typedef void (*PacketManager_EventOccurredDelegate_t)(PacketManager_Event_t event, uint16_t rawSize);

	/* Exported functions --------------------------------------------------------*/
	/***
  * @Brief      Setup function for UART controller module.
  *
  * @Params     eventHandler-> Upper layer event handler function.
  */
	extern void PacketManager_Setup(PacketManager_EventOccurredDelegate_t eventHandler);

	/***
  * @Brief      Start packet manager.
  */
	extern Bool_t PacketManager_Start(void);

	/***
  * @Brief      Module executer function.
  */
	extern void PacketManager_Execute(void);

	/***
  * @Brief      Sets stop event.
  */
	extern void PacketManager_Stop(void);

	/***
  * @Brief      Encodes and frames PDU then pushes to the TX buffer.
  *
  * @Params     pduFields-> Pointer to the PDU fields.
  *             pduFieldCount-> Number of PDU fields.
  */
	extern void PacketManager_Send(PacketManager_PduField_t *pduFields, uint8_t pduFieldCount);

	/***
  * @Brief      Decodes pdu field.
  *
	* @Params     data-> Pointer to the PDU field data.
	*							length-> Length of the PDU field data.
	*							unparsedPduSize-> Remaining portion of the pdu that is unparsed.
  * 
	* @Return			Unparsed pdu size updated.
  */
	extern uint16_t PacketManager_ParseField(uint8_t *data, uint16_t length, uint16_t unparsedPduSize);

	/***
  * @Brief      Handles module errors. Recovers not corrupted messages and clears the
  *             hardware errors.
  */
	extern void PacketManager_ErrorHandler(void);

#ifdef __cplusplus
}
#endif

#endif
