/*******************************************************************************
 * OnOffSwitchDevice_Application_Framework.c
 ********************************************************************************
 * Program Description:
 *   This module provides all the functionality for OnOff Switch device event
 *   handling
 *******************************************************************************/

/*-----------------------------------------------------------------------------
 * Includes
 *----------------------------------------------------------------------------*/

#include "rsi_zb_config.h"


/*-----------------------------------------------------------------------------
 * Global Constants
 *----------------------------------------------------------------------------*/
#define m_BROADCAST_END_POINT_c						    0xFF


#ifndef g_APP_PROFILE_ID_c
/* By default profile would be HA profile 0x0104*/
#define g_APP_PROFILE_ID_c							0x0104	
#endif
/*-----------------------------------------------------------------------------
 * Public Memory declarations
 *----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
 * Private Memory declarations
 *----------------------------------------------------------------------------*/
const cluster_id_t a_In_Cluster_List_For_OnOff_Switch[] =
{
	g_BASIC_CLUSTER_c,
	g_IDENTIFY_CLUSTER_c,
	g_ON_OFF_SWITCH_CONFIGURATION_CLUSTER_c
};


const cluster_id_t a_Out_Cluster_List_For_OnOff_Switch[] =
{
	g_ON_OFF_CLUSTER_c,
};

/* simple descriptor for OnOff Switch device */
Simple_Descriptor_t OnOff_Switch_Simple_Desc =
{
	g_APP_PROFILE_ID_c,
	g_ONOFF_SWITCH_c,
	0x00,
	sizeof(a_In_Cluster_List_For_OnOff_Switch) /	sizeof(cluster_id_t),
	(cluster_id_t*)a_In_Cluster_List_For_OnOff_Switch,
	sizeof(a_Out_Cluster_List_For_OnOff_Switch) / sizeof(cluster_id_t),
	(cluster_id_t*)a_Out_Cluster_List_For_OnOff_Switch
};

/*---------------------------------------------------------------------------*/
Endpoint_Description_t  ga_Endpoint_Descriptors[1] =
{
	{ (ZigBeeSimpleDescriptor_t*)&OnOff_Switch_Simple_Desc,
		g_ONOFF_SWITCH_ENDPOINT_c },
};

const Endpoint_Description_t  g_Broadcast_Endpoint_Descriptors =
{
	0, m_BROADCAST_END_POINT_c
};

Endpoint_Description_t *gp_Endpoint_Descriptors = ga_Endpoint_Descriptors;
/*-----------------------------------------------------------------------------
 * Function Prototypes
 *----------------------------------------------------------------------------*/

/* None */

/*-----------------------------------------------------------------------------
 * Public Functions
 *----------------------------------------------------------------------------*/

/* None */

/*-----------------------------------------------------------------------------
 * Private Functions
 *----------------------------------------------------------------------------*/

/* None */

/*-----------------------------------------------------------------------------
 * Interrupt Service Routines
 *----------------------------------------------------------------------------*/

/* None */

/*-----------------------------------------------------------------------------
 * End Of File
 *----------------------------------------------------------------------------*/
