#ifndef __PERIPHERAL_H
#define __PERIPHERAL_H

#include "generic.h"

/* Exported constants ------------------------------------------------------*/
#define PRP_NUMBER_OF_SLOTS 4

#define PRP_BED_SLOT 0
#define PRP_EXTRUDER0_SLOT 1
#define PRP_EXTRUDER1_SLOT 2
#define PRP_EXTRUDER2_SLOT 3

#define PRP_NAME_OBJ_ID 0
#define PRP_PROPERTIES_OBJ_ID 1
#define PRP_TARGET_VALUE_OBJ_ID 2
#define PRP_CURRENT_VALUE_OBJ_ID 3
#define PRP_COMMAND_POINT_OBJ_ID 4
#define PRP_STATE_OBJ_ID 5
#define PRP_ERROR_CODE_OBJ_ID 6
#define PRP_PID_I_COEFF_OBJ_ID 7
#define PRP_PID_K_COEFF_OBJ_ID 8
#define PRP_PID_D_COEFF_OBJ_ID 9

#define PRP_MAX_NAME_LENGTH 32

/* Typedefs ----------------------------------------------------------------*/
typedef enum
{
    Prp_Type_Extruder = ((uint8_t)0),
    Prp_Type_Bed = ((uint8_t)1)
} Prp_Type_t;

typedef enum
{
    Prp_Control_OnOff = ((uint8_t)0),
    Prp_Control_Pneumatic = ((uint8_t)1),
    Prp_Control_StepDir = ((uint8_t)2),
    Prp_Control_NoControl = ((uint8_t)3)
} Prp_Control_t;

typedef enum
{
    Prp_Unit_Celsius = ((uint8_t)0),
    Prp_Unit_Lumen = ((uint8_t)1),
} Prp_Unit_t;

typedef struct
{
    float stepSize;
    float maxExtrusionSpeed;
    float operationRangeMin;
    float operationRangeMax;
    float timeout;
    float tolerance;
    Prp_Type_t type : 1;
    Prp_Control_t control : 2;
    Prp_Unit_t units : 1;
    Bool_t temperatureSensing : 1;
    Bool_t extrusion : 1;
    Bool_t heating : 1;
    Bool_t cooling : 1;
    Bool_t uvCuring : 1;
} Prp_Properties_t;

typedef enum
{
    Prp_Command_TurnOn = ((uint8_t)0),
    Prp_Command_TurnOff = ((uint8_t)1),
    Prp_Command_Engage = ((uint8_t)2),
    Prp_Command_Disengage = ((uint8_t)3),
    Prp_Command_PidAutotune = ((uint8_t)4),
    Prp_Command_Reset = ((uint8_t)5),
} Prp_Command_t;

typedef enum
{
    Prp_Status_Ready = ((uint8_t)0),
    Prp_Status_Active = ((uint8_t)1),
    Prp_Status_Engaged = ((uint8_t)2),
    Prp_Status_Error = ((uint8_t)3)
} Prp_Status_t;

typedef enum
{
    Prp_Error_OperationRangeExceeded = ((uint8_t)0),
    Prp_Error_TimeoutToReachDestinationValue = ((uint8_t)1),
    Prp_Error_SensorError = ((uint8_t)2),
    Prp_Error_HardwareError = ((uint8_t)3)
} Prp_Error_t;

#endif