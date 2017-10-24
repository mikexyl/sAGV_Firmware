/******************************************************************************
 * OnOffSwitchDevice_Application_Framework.h
 ******************************************************************************
 * Program Description:
 * This file contains the OnOff Switch device application framework
 ******************************************************************************/
#ifndef _ON_OFF_SWITCH_APPLICATION_FRAMEWORK_H_
#define _ON_OFF_SWITCH_APPLICATION_FRAMEWORK_H_

#include "rsi_driver.h"
/*-----------------------------------------------------------------------------
 * Includes
 *----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Global Constants
 *----------------------------------------------------------------------------*/
#define g_ONOFF_SWITCH_c                                         0x0000
#define g_ONOFF_SWITCH_ENDPOINT_c                                0x01
#define g_BASIC_CLUSTER_c                                        0x0000
#define g_IDENTIFY_CLUSTER_c                                     0x0003
#define g_ON_OFF_CLUSTER_c                                       0x0006
#define g_ON_OFF_SWITCH_CONFIGURATION_CLUSTER_c                  0x0007
#define DEFAULT_RADIUS                                           0x1e
#define g_NULL_c                                                 0 
#define g_BROADCAST_ADDRESS_c                                    0xFFFF
#define g_SCAN_DURATION_c                                        0x08


extern Simple_Descriptor_t OnOff_Switch_Simple_Desc;
/*-----------------------------------------------------------------------------
 * Function Prototypes
 *----------------------------------------------------------------------------*/

typedef enum OnOff_Cluster
{
   g_Off_Command_c =0x00
   ,g_On_Command_c
   ,g_Toggle_Command_c

}OnOff_Cluster;

typedef enum FrameControl_Direction
{
  g_Client_To_Server_c = 0x00,
  g_Server_To_Client_c = 0x08

}Frame_Control_direction;

enum APSDE_Confirmation_Status {
    g_Aps_Success_c = 0x00,
    g_Aps_Illegal_Request_c = 0xa3,
    g_Aps_Invalid_Parameter_c = 0xa6,
    g_Aps_No_Ack_c = 0xa7,
    g_Aps_No_Bound_Device_c = 0xa8,
    g_Aps_No_Short_Address_c = 0xa9,
    g_Aps_Not_Supported_c = 0xaa,
    g_Aps_Security_Fail_c = 0xad,
    g_Aps_Transaction_Overflow_c = 0xb1,
    g_Aps_No_Buffer_For_Retries_c = 0xb2
};

typedef enum Bool_Tag {
  /*! -False is defined as 0x00 */
  g_FALSE_c,

  /*! -True is defined as 0x01 */
  g_TRUE_c
} Bool_t;

typedef enum event_ids{
	EVENT_START_NETWORK,
	EVENT_JOIN_NETWORK,
	EVENT_WAIT_STATE,
	EVENT_NETWORK_READY_STATE,
	EVENT_ERROR_STATE
}event_ids_t;


typedef enum data_events{
	g_DATA_NO_EVENT_c,
	g_TRIGGER_EVENT_c,
	g_MATCH_DESC_REQ_c,
	g_SEND_ON_CMD_c,
	g_SEND_TOGGLE_CMD_c,
	g_READ_ONOFF_ATTRIBUTE_c,
	g_SEND_OFF_CMD_c,
	g_WAIT_FOR_MATCH_RESP_c,
	g_WAIT_TO_COMPLETE_BCAST_c,
	g_WAIT_FOR_ON_CMD_CONF_c,
	g_WAIT_FOR_OFF_CMD_CONF_c,
	g_WAIT_FOR_READ_ATTRIB_RESP_c,
  g_INITATE_SLEEP_STATE_c,
  g_WAIT_FOR_TOGGLE_CMD_CONF_c,
}data_events_t;

typedef struct{
	event_ids_t state;
	data_events_t data_event;
}state_machine_t;

typedef struct lightDeviceInfo_tag{

	uint16_t shortaddress;
	uint8_t endpoint;
} lightDeviceInfo_t;
#endif                          /* _ON_OFF_SWITCH_APPLICATION_FRAMEWORK_H_ */
/*-----------------------------------------------------------------------------
 * End Of File
 *----------------------------------------------------------------------------*/
