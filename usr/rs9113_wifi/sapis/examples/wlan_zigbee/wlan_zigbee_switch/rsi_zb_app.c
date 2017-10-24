/**
 * @file    rsi_app_main.c
 * @version 0.1
 * @date    15 Sep 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for TCP client socket
 *
 *  @section Description  This file contains example application for TCP client socket 
 *
 *
 */

/**
 * Include files
 * */

//! Driver Header file
#include "rsi_driver.h"

//! ZigBee include file to refer zigbee APIs
#include "rsi_zb_config.h"



#define TRIGGER_HA_SAMPLE_TOGGLE      3000000 
#define g_ON_OFF_SWITCH_TIMER_c       2000000
#define DATA_INDICATION_EVENT         0x01
#define RSI_ZIGBEE_SEND_CMD           0x01

/*******************************************************************************
 g_CHANNEL_MASK_c

 - This macro defines a 32-bit mask starting from the 11th bit from
 the LSB to  the 26th bit, consisting of 16 channels represented by
 each bit.

******************************************************************************/
#define g_MASK_FOR_11_CHANNEL_c  0x00000800
#define g_MASK_FOR_12_CHANNEL_c  0x00001000
#define g_MASK_FOR_13_CHANNEL_c  0x00002000
#define g_MASK_FOR_14_CHANNEL_c  0x00004000
#define g_MASK_FOR_15_CHANNEL_c  0x00008000
#define g_MASK_FOR_16_CHANNEL_c  0x00010000
#define g_MASK_FOR_17_CHANNEL_c  0x00020000
#define g_MASK_FOR_18_CHANNEL_c  0x00040000
#define g_MASK_FOR_19_CHANNEL_c  0x00080000
#define g_MASK_FOR_20_CHANNEL_c  0x00100000
#define g_MASK_FOR_21_CHANNEL_c  0x00200000
#define g_MASK_FOR_22_CHANNEL_c  0x00400000
#define g_MASK_FOR_23_CHANNEL_c  0x00800000
#define g_MASK_FOR_24_CHANNEL_c  0x01000000
#define g_MASK_FOR_25_CHANNEL_c  0x02000000
#define g_MASK_FOR_26_CHANNEL_c  0x04000000

#define g_CHANNEL_MASK_c g_MASK_FOR_24_CHANNEL_c 
/*-----------------------------------------------------------------------------
 * Global Constants
 *----------------------------------------------------------------------------*/
lightDeviceInfo_t LightDeviceInfo;
APSDE_Data_Indication_t DataIndication;
extern rsi_zigb_app_info_t rsi_zigb_app_info;
//! global parameters list
static uint32_t zigbee_app_event_map;
static uint32_t zigbee_app_event_mask;
static uint8_t rsi_zigbee_app_data_len;
static uint8_t rsi_zigbee_app_data[10];

extern void rsi_wlan_app_send_to_zigbee (uint16_t  msg_type, uint8_t *data, uint16_t data_len);
int32_t rsi_zigbee_app_send_to_wlan(uint8_t msg_type, uint8_t *buffer, uint32_t length);
/* Macro that holds the baud rate of UART */
/*-----------------------------------------------------------------------------
 * Public Memory declarations
 *******************************************************************************/
static void rsi_zigbee_app_set_event(uint32_t event_num);
static void rsi_zigbee_app_clear_event(uint32_t event_num);
static int32_t rsi_zigbee_app_get_event(void);
uint8_t rsi_zigb_app_send_data( uint8_t direction, uint8_t commandType, uint8_t destEP, uint16_t dest_short_address,
                uint8_t commandId, uint16_t cluster, uint8_t dataLength,uint8_t* payloadPointer );
/*******************************************************************************
 * Public Functions
*******************************************************************************/

/*==============================================*/
/**
 * @fn         rsi_wlan_app_send_to_zigbee
 * @brief      this function is used to send data to zigbee app.
 * @param[in]   msg_type, it indicates write/notification event id.
 * @param[in]  data, raw data pointer.
 * @param[in]  data_len, raw data length.
 * @return     none.
 * @section description
 * This is a callback function
 */
void rsi_wlan_app_send_to_zigbee (uint16_t  msg_type, uint8_t *data, uint16_t data_len)
{
  rsi_zigbee_app_data_len = RSI_MIN (sizeof (rsi_zigbee_app_data), data_len);
  memcpy (&rsi_zigbee_app_data, data, rsi_zigbee_app_data_len);
  rsi_zigbee_app_set_event (RSI_ZIGBEE_SEND_CMD);
}

/*==============================================*/
/**
 * @fn         rsi_zigbee_app_task
 * @brief      this function will execute when BLE events are raised.
 * @param[in]  none.
 * @return     none.
 * @section description
 */
void rsi_zigbee_app_task (void)
{
  int32_t  event_id;
  uint8_t  on[10] = "ON";
  uint8_t  off[10] = "OFF";
  uint8_t  toggle[10] = "TOGGLE";
  
  //! checking for events list
  event_id = rsi_zigbee_app_get_event ();
  if (event_id == -1) {
    return;
  }

  switch (event_id) {
    case RSI_ZIGBEE_SEND_CMD:
      {
       
        if(!strcmp((const char *)on,(const char *)rsi_zigbee_app_data))
        {
          RSI_DPRINT(RSI_PL1,"\n ON");
          rsi_zigb_app_send_data(g_Client_To_Server_c, g_Cluster_Specific_Cmd_c,LightDeviceInfo.endpoint,LightDeviceInfo.shortaddress, g_On_Command_c,g_ON_OFF_CLUSTER_c,0x00,g_NULL_c);
        }
        else if(!strcmp((const char *)rsi_zigbee_app_data,(const char *)off))
        {
          RSI_DPRINT(RSI_PL1,"\n OFF");
          rsi_zigb_app_send_data(g_Client_To_Server_c, g_Cluster_Specific_Cmd_c,LightDeviceInfo.endpoint,LightDeviceInfo.shortaddress, g_Off_Command_c,g_ON_OFF_CLUSTER_c,0x00,g_NULL_c);
        }
        else if(!strcmp((const char *)rsi_zigbee_app_data,(const char *)toggle))
        {
          RSI_DPRINT(RSI_PL1,"\n Toggle");
          rsi_zigb_app_send_data(g_Client_To_Server_c, g_Cluster_Specific_Cmd_c,LightDeviceInfo.endpoint,LightDeviceInfo.shortaddress, g_Toggle_Command_c,g_ON_OFF_CLUSTER_c,0x00,g_NULL_c);
        }
        else
        {
          //Nothing
        }
        memset(rsi_zigbee_app_data,0x0,10);
        rsi_zigbee_app_clear_event (RSI_ZIGBEE_SEND_CMD);
      }
      break;
  }

  return;
}


/*==============================================*/
/**
 * @fn         rsi_zigbee_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_zigbee_app_init_events()
{
  zigbee_app_event_map = 0;
  zigbee_app_event_mask = 0xFFFFFFFF;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_zigbee_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_zigbee_app_set_event(uint32_t event_num)
{
  zigbee_app_event_map |= BIT(event_num);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_zigbee_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_zigbee_app_clear_event(uint32_t event_num)
{
  zigbee_app_event_map &= ~BIT(event_num);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_zigbee_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_zigbee_app_get_event(void)
{
  uint32_t  ix;

  for (ix = 0; ix < 32; ix++)
  {
    if (zigbee_app_event_map & (1 << ix))
    {
      return ix;
    }
  }

  return (-1);
}

int rsi_zigbee_app_init ( void )
{
  int32_t status = 0;
  uint8_t device_type = 2;
  uint16_t nwk_addr_of_interest = 0x0000;
  uint16_t dest_addr = g_BROADCAST_ADDRESS_c;
  uint16_t* lightAddress;
  rsi_zigb_app_info_t   *app_info = &rsi_zigb_app_info;

  //! init events
  rsi_zigbee_app_init_events();
  
  //! register callbacks
  rsi_zigb_register_callbacks (
      rsi_zigb_app_scan_complete_handler,
      rsi_zigb_app_energy_scan_result_handler,             
      rsi_zigb_app_network_found_handler,
      rsi_zigb_app_stack_status_handler,
      rsi_zigb_app_incoming_many_to_one_route_req_handler,
      rsi_zigb_app_handle_data_indication,
      rsi_zigb_app_handle_data_confirmation,
      rsi_zigb_app_child_join_handler
	   ,rsi_zigb_nvm_backup_handler,
     NULL);
     


  RSI_DPRINT(RSI_PL1,"\n Sending init stack\n");
  //! zigb stack init 
  status = rsi_zigb_init_stack();


  RSI_DPRINT(RSI_PL1,"\n Sending reset stack\n");
  //! zigb stack reset 
  status = rsi_zigb_reset_stack();

  //! zigb init scan 
  status = rsi_zigb_initiate_scan(g_MAC_ACTIVE_SCAN_TYPE_c,
      g_CHANNEL_MASK_c, g_SCAN_DURATION_c);
  rsi_delay_ms(3000);
  if(app_info->state != FSM_SCAN_COMPLETE)
  {
    RSI_DPRINT(RSI_PL1,"\n Could Not Find Network");
    return -1;
  }
  RSI_DPRINT(RSI_PL1,"\nscan_status:0x%x", status);

  //! zigb join request 
  status =rsi_zigb_join_network(device_type,
      rsi_zigb_app_info.nwkinfo.channel, 0x0f, rsi_zigb_app_info.nwkinfo.extendedPanId);

  if(app_info->state != FSM_JOIN_COMPLETE)
  {
    RSI_DPRINT(RSI_PL1,"\n Could Not Join the Network");
       return -1;
  }
  RSI_DPRINT(RSI_PL1,"\njoin_status:0x%x", status);

  RSI_DPRINT(RSI_PL1,"\nrsi_zigb_set_simple_descriptor:");
  status = rsi_zigb_set_simple_descriptor(g_ONOFF_SWITCH_ENDPOINT_c, 
      &OnOff_Switch_Simple_Desc);

  //! send match descriptor request
  status = rsi_zigb_send_match_descriptors_request(nwk_addr_of_interest,
      OnOff_Switch_Simple_Desc.app_profile_id,
      (uint8_t *)OnOff_Switch_Simple_Desc.p_incluster_list,
      OnOff_Switch_Simple_Desc.incluster_count,
      (uint8_t *)OnOff_Switch_Simple_Desc.p_outcluster_list,
      OnOff_Switch_Simple_Desc.outcluster_count,
      g_TRUE_c,
      dest_addr);

   while(app_info->state != FSM_ZB_MATCH_DESC_RESP)
   {
      printf("\n Poll for response");
      rsi_zigb_end_device_poll_for_data();
      rsi_delay_ms(250);
   }

  //! Update light addr and EP on receiving match descriptor response 
  lightAddress =  (uint16_t *)&app_info->zb_resp_info.matchDescResp[2];
  LightDeviceInfo.shortaddress = *lightAddress;    
  LightDeviceInfo.endpoint = app_info->zb_resp_info.matchDescResp[5];
  RSI_DPRINT(RSI_PL1,"\n Light addr: 0x%x \n Light EP: 0x%x", LightDeviceInfo.shortaddress, LightDeviceInfo.endpoint);
  return 0;
}


//! Event callbacks
/*===========================================================================
 *
 * @fn          void rsi_zigb_app_scan_complete_handler (uint32_t channel, 
 *                                                       uint8_t status )
 * @brief       Scan complete handler 
 * @param[in]   Channel
 * @param[in]   Status of channel whether beacons are found
 * @return      none
 * @section description
 * This API is used to handle ZigBee scan complete state
 * It provides infromation of the channel whether beacons are found or not
 * Updating few app_info variables 
 *
 * ===========================================================================*/
  
void rsi_zigb_app_scan_complete_handler ( uint32_t channel, uint8_t status )
{
  rsi_zigb_app_info.scan_done_cb.channel = channel; 
  rsi_zigb_app_info.scan_done_cb.scan_status = status; 
  rsi_zigb_app_info.status_var.scanReqSent = 0;
  if(!status)
  {
   rsi_zigb_app_info.state = FSM_SCAN_COMPLETE; 
  }
  RSI_DPRINT(RSI_PL1,"\nAppScanCompleteHandler");
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\nScan Status = %x \n", status);
#endif  
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_energy_scan_result_handler( uint32_t channel,
 *                                                       uint8_t *pEnergyValue)
 * @brief       Energy Scan complete handler 
 * @param[in]   Channel
 * @param[in]   Energy Value (RSSI)
 * @return      none
 * @section description
 * This API is used to handle ZigBee Energy scan complete state
 * Here Energy in each channel is received, for the provided channels 
 * issued by user to scan
 *
 * ===========================================================================*/
void rsi_zigb_app_energy_scan_result_handler( uint32_t channel,uint8_t *pEnergyValue)
{
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_network_found_handler(ZigBeeNetworkDetails)
 * @brief       Network found indication handler 
 * @param[in]   NetworkInformation data 
 * @param[out]  none
 * @return      none
 * @section description
 * This API is used to handle network found indication frame  
 * Infromation about the found network is updated 
 *
 * ===========================================================================*/

void rsi_zigb_app_network_found_handler(ZigBeeNetworkDetails networkInformation)
{
  ZigBeeNetworkDetails *nwk_details = &(rsi_zigb_app_info.nwkinfo);
  /* Currently we are checking for any coordinator, if you know the specific 
   * extended panid, then check here for specific panid */
  rsi_zigb_mcpy((uint8_t *)&networkInformation, (uint8_t *)nwk_details, sizeof(ZigBeeNetworkDetails));
  RSI_DPRINT(RSI_PL1,"\nAppNetworkFoundHandler \n ");
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\nchannel: %d  ",networkInformation.channel);
  RSI_DPRINT(RSI_PL1,"\nshort panid: %d  ",networkInformation.shortPanId);
  
#endif  
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_stack_status_handler(ZigBeeNWKStatusInfo *statusInfo)
 * @brief       Stack status Indication
 * @param[in]   Network status Information 
 * @param[out]  none
 * @return      none
 * @section description
 * This API is used to handle network/stack status
 * Infromation about network status (If connection successful of failed) 
 *
 * ===========================================================================*/

void rsi_zigb_app_stack_status_handler(ZigBeeNWKStatusInfo *statusInfo)
{
  rsi_zigb_app_info.stack_status = *statusInfo;
  if(!rsi_zigb_app_info.stack_status)
  {
    rsi_zigb_app_info.state = FSM_JOIN_COMPLETE; 
  }
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\n Stack Status = %x \n", *statusInfo);
#endif  
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_incoming_many_to_one_route_req_handler(uint16_t SourceAddr,
 *                                                  uint8_t * pSrcIEEEAddr,uint8_t PathCost )
 * @brief       Many to one route request handler
 * @param[in]   Source short Addr
 * @param[in]   Source IEEE address
 * @param[in]   Path cost
 * @param[out]  none
 * @return      none
 * @section description
 * This API is used to handle Many to one route request
 * We have to decide which route to accept based on path cost 
 *
 * ===========================================================================*/

void rsi_zigb_app_incoming_many_to_one_route_req_handler( uint16_t SourceAddr, uint8_t * pSrcIEEEAddr,uint8_t PathCost )
{
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\n Called rsi_zigb_app_incoming_many_to_one_route_req_handler \n ");
  RSI_DPRINT(RSI_PL1,"\n SorceAddr: 0x%x",SourceAddr);
  RSI_DPRINT(RSI_PL1,"\n PathCost: %x",PathCost);
#endif  
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_handle_data_indication(
 *                                   APSDE_Data_Indication_t * pDataIndication )
 * @brief       Handle data indication frame
 * @param[in]   Data indication info struct 
 * @param[out]  none
 * @return      none
 * @section description
 * This API is used to handle received data indication frame
 *
 * ===========================================================================*/
void rsi_zigb_app_handle_data_indication(APSDE_Data_Indication_t *pDataIndication)
{
  rsi_zigb_app_info_t *app_info = &rsi_zigb_app_info;
  RSI_DPRINT(RSI_PL1,"\nData Indication");
  if( pDataIndication->cluster_id == 0x8006)//0x8006: Match decs response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      if(rsi_zigb_app_info.state != FSM_ZB_MATCH_DESC_RESP)
      {
        rsi_zigb_mcpy( pDataIndication->a_asdu, 
            app_info->zb_resp_info.matchDescResp,
            pDataIndication->asdulength);
        app_info->status_var.matchDescRspStatus = 0x00;
        rsi_zigb_app_info.state = FSM_ZB_MATCH_DESC_RESP;
      }
    }
  }
}
/*===========================================================================
 *
 * @fn          void rsi_zigb_app_handle_data_confirmation (
 *                                   APSDE_Data_Confirmation_t* pDataConfirmation )
 * @brief       Handle data confirmation frame
 * @param[in]   Buffer Index of actual data from the pointer
 * @param[in]   Data confirmation info struct 
 * @param[out]  none
 * @return      none
 * @section description
 * This API is used to handle received data confirmation frame for the 
 * data request sent
 *
 * ===========================================================================*/
uint32_t dataConfcnt;
void rsi_zigb_app_handle_data_confirmation (APSDE_Data_Confirmation_t *pDataConfirmation)
{
  uint8_t buffer[20]= {'R','E','C','V','D',' ','C','N','F','R','M','\n'};
  RSI_DPRINT(RSI_PL1,"\n Received confirmation");
  if((g_Aps_Success_c == pDataConfirmation->status) && (g_ONOFF_SWITCH_ENDPOINT_c == pDataConfirmation->src_endpoint))
  {
      rsi_zigbee_app_send_to_wlan(g_Aps_Success_c,buffer,20);
  }
}

/*===========================================================================
 *
 * @fn          void rsi_zigb_app_child_join_handler(uint16_t short_address,
 *                                                   BOOL joining)
 * @brief       Child join handler 
 * @param[in]   Short_addr of child
 * @param[in]   Status of child joining/leaving 
 * @return      none
 * @section description
 * This API is used to handle child join/leave status
 *
 * ===========================================================================*/

void rsi_zigb_app_child_join_handler(uint16_t short_address, char joining)
{
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\n Called rsi_zigb_app_child_join_handler \n ");
  RSI_DPRINT(RSI_PL1,"ShortAddr: 0x%x",short_address);
  RSI_DPRINT(RSI_PL1,"\n Joining: %x\n",joining);
#endif  
}


/*===========================================================================
 *
 * @fn          int main()
 *                                                  
 * @brief       main function to trigger the api test functionality. 
 * @param[in]   none
 * @return      status - 0 :success
 *                     - -1:failure
 * @section description
 * This API is used to extract channel from channel mask
 *
 * ===========================================================================*/


/*===========================================================================
 *
 * @fn          uint8_t rsi_zigb_zcl_create_command (uint8_t direction, uint8_t *p_asdu,
                                      void* p_ZCL_Cmd_Data, uint8_t ZCL_Cmd_Data_Length,
                                      uint8_t trans_seq_num)
 * @brief       Prepares the ZigBee Cluster command 
 * @param[in]   Direction
 * @param[in]   p_asdu - buffer pointer of data
 * @param[in]   p_ZCL_Cmd_Data - Cluster data
 * @param[in]   length of ZCL data
 * @param[in]   Seq num
 * @param[out]  none
 * @return      Final data length
 * @section description
 * This API is used to prepare the ZigBee Cluster command pkt
 *
 * ===========================================================================*/
  
uint8_t rsi_zigb_zcl_create_command (uint8_t direction, uint8_t *p_asdu,
                void* p_ZCL_Cmd_Data, uint8_t ZCL_Cmd_Data_Length,
                uint8_t trans_seq_num)
{
  App_ZCL_Request_t *p_received_data = ( App_ZCL_Request_t *)p_ZCL_Cmd_Data;
  uint8_t data_length = 0;
  BOOL manufacture_specific = p_received_data->manufacture_specific;
  BOOL disable_default_response = p_received_data->disable_default_response;
  uint8_t *p_ZCL_Header_Payload = p_asdu;

  if ( !( p_received_data->command_type & 0x01 )) {
          ((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->frame_control = g_Generic_Cmd_c ;
  } else {
          ((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->frame_control = g_Cluster_Specific_Cmd_c;
  }
  ((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->frame_control |= direction;

  data_length++;

  if ( disable_default_response ) {
          ((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->frame_control |= g_Disable_Default_Response_c;
  }
  if ( manufacture_specific ) {
          ((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->frame_control |= g_Manufacture_Specific_Bit_c ;
          rsi_zigb_mcpy((uint8_t*)p_received_data->a_manufacturer_code, (uint8_t*)((ZCL_Header_And_Payload_t*)p_ZCL_Header_Payload)->a_manufacture_code ,2 );
          data_length += sizeof(uint16_t);
          ZCL_Cmd_Data_Length -= 0x03;
  } else {
          ZCL_Cmd_Data_Length -= 0x05;
  }
  *( p_ZCL_Header_Payload + data_length ) = trans_seq_num;

  data_length++;

  *( p_ZCL_Header_Payload +  data_length ) = p_received_data->ZCL_command_received.command_identifier;

  ZCL_Cmd_Data_Length--;
  data_length++;

  rsi_zigb_mcpy((uint8_t*)&( p_received_data->ZCL_command_received.Foundation_Commands ) ,
                  p_ZCL_Header_Payload + data_length, ZCL_Cmd_Data_Length);

  data_length += ZCL_Cmd_Data_Length;

  return data_length;
}
/*===========================================================================
 *
 * @fn          uint8_t rsi_zigb_app_send_data( uint8_t direction, uint8_t commandType, 
 *                                              uint8_t destEP, uint16_t dest_short_address,
                                                uint8_t commandId, uint16_t cluster, 
                                                uint8_t dataLength,uint8_t* payloadPointer )
 * @brief       Prepares ZigBee data pkt
 * @param[in]   Direction
 * @param[in]   Command type
 * @param[in]   Destination End Point
 * @param[in]   Destination Short address
 * @param[in]   ZCL Command ID received
 * @param[in]   Cluster type
 * @param[in]   Data length
 * @param[in]   Payload pointer
 * @param[out]  none
 * @return      Status
 * @section description
 * This API is used to prepare the ZigBee Data pkt with cluster information
 *
 * ===========================================================================*/

uint8_t rsi_zigb_app_send_data( uint8_t direction, uint8_t commandType, uint8_t destEP, uint16_t dest_short_address,
                uint8_t commandId, uint16_t cluster, uint8_t dataLength,uint8_t* payloadPointer )
{
  uint8_t status;
  Address DestAddress;
  ZigBeeAPSDEDataRequest_t APSDERequest;
  App_ZCL_Request_t *pZcl_hdr;
  uint8_t *pAsdu;

  /*+1 is added for Command id*/
  uint8_t ZCLHeaderLength = ((sizeof(App_ZCL_Request_t) - sizeof(ZCL_Command_t)) + 1);

 // DestAddress.short_address = 0x00;
  DestAddress.short_address = dest_short_address;

  APSDERequest.ProfileId = OnOff_Switch_Simple_Desc.app_profile_id;
  APSDERequest.DestEndpoint = destEP;
  APSDERequest.ClusterId = cluster;
  APSDERequest.AsduLength = dataLength;
  APSDERequest.SrcEndpoint = g_ONOFF_SWITCH_ENDPOINT_c;
  APSDERequest.TxOptions = g_APS_Tx_Opt_Use_NWK_Key_c | g_APS_Tx_Opt_Ack_Req_c;
  APSDERequest.Radius = DEFAULT_RADIUS;

  pZcl_hdr = (App_ZCL_Request_t*)APSDERequest.aPayload;
  pZcl_hdr->command_type = commandType;
  pZcl_hdr->disable_default_response = g_Disable_Default_Response_c;
  pZcl_hdr->manufacture_specific = 0;
  pZcl_hdr->ZCL_command_received.command_identifier = commandId;
  pAsdu = APSDERequest.aPayload + ZCLHeaderLength;

  rsi_zigb_mcpy(payloadPointer,pAsdu, dataLength );

  APSDERequest.AsduLength =  rsi_zigb_zcl_create_command ( direction,
                  APSDERequest.aPayload,
                  (App_ZCL_Request_t*)&APSDERequest.aPayload,
                  dataLength + ZCLHeaderLength ,
                  0);

  status = rsi_zigb_send_unicast_data(ZigBee_Outgoing_Direct,
                  DestAddress  , &APSDERequest);

   /* Send the  command to the light  */
#ifdef ZB_DEBUG        
        RSI_DPRINT(RSI_PL1,"\nSending command to Corrdinator Home Automation supported Light\n"); 
#endif        
  return status;
}

void rsi_zigb_nvm_backup_handler(void *data, uint32_t offset)
{
  uint16_t SASIndex = 0;
  uint16_t length=0;
  rsi_zigb_app_info_t   *app_info = &rsi_zigb_app_info;
  SASIndex= app_info->nvm_storage.sas_index;
  printf("\n rsi_zigb_nvm_backup_handler");
  if(offset == g_NVM_SAS_TABLE_START_OFFSET_c)
  {
    length = 2;
    memcpy((uint8_t*)&app_info->nvm_storage.sas_index,data,length);
    SASIndex = app_info->nvm_storage.sas_index;
  }

  if(offset == (g_NVM_SAS_TABLE_START_OFFSET_c + 2))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.gp_ZDO_Configuration,data,sizeof(app_info->nvm_storage.gp_ZDO_Configuration));
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 1) * m_SIZE_CONFIGURATION_c)))
  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data,data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 2) * m_SIZE_CONFIGURATION_c)))
  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[1],data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 3) * m_SIZE_CONFIGURATION_c)))
  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[2],data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 4) * m_SIZE_CONFIGURATION_c)))

  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[3],data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 5) * m_SIZE_CONFIGURATION_c)))

  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[4],data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 6) * m_SIZE_CONFIGURATION_c)))

  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[5],data,m_SIZE_CONFIGURATION_c);
  }

  if(offset == ((g_NVM_SAS_TABLE_START_OFFSET_c) + ((SASIndex + 7) * m_SIZE_CONFIGURATION_c)))
  {
    memcpy((uint8_t*)app_info->nvm_storage.sas_data[6],data,m_SIZE_CONFIGURATION_c);
  }


  if(offset == g_NVM_TABLES_START_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.nvm_status,data,2);
  }

  if(offset == g_NEIGHBOR_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.Neighbor_Table,data,sizeof(app_info->nvm_storage.Neighbor_Table));
  }


//#if ( g_NWK_MESH_ROUTING_d == 1 )
#if (defined(ZIGBEE_ROUTER) || (ZIGBEE_COORDINATOR))
  if(offset == g_ROUTE_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.Route_Table,data,sizeof(app_info->nvm_storage.Route_Table));
  }
#endif 

  if(offset == g_SECURITY_MATERIAL_DESCRIPTOR_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.Security_Material_Descriptor,data,sizeof(app_info->nvm_storage.Security_Material_Descriptor));
  }


  if(offset == g_PERSISTENT_NIB_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.Persistent_NIB,data,sizeof(app_info->nvm_storage.Persistent_NIB));
  }


  if(offset == g_ZDO_INFORMATION_BASE_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.ZDO_Information_Base,data,sizeof(app_info->nvm_storage.ZDO_Information_Base));
  }


//#if ( g_NWK_MANY_TO_ONE_ROUTING_d == 1 )
//#if (g_NWK_ROUTE_RECORD_TABLE_d == 1)
  if(offset == g_ROUTE_RECORD_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.route_record_table,data,sizeof(app_info->nvm_storage.route_record_table));
  }
//#endif

  if(offset == g_LINK_KEY_COUNTER_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.LinkKeyFrameCounterTable,data,sizeof(app_info->nvm_storage.LinkKeyFrameCounterTable));
  }


  if(offset == g_LINK_KEY_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.LinkKeyDeviceKeyPairTable,data,sizeof(app_info->nvm_storage.LinkKeyDeviceKeyPairTable));
  }


  if(offset == g_BINDING_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.src_bind_enrty,data,sizeof(app_info->nvm_storage.src_bind_enrty));
  }


//#if ( g_APS_GROUP_DATA_RX_d == 1 )
//#ifdef ZIGBEE_END_DEVICE 
  if(offset == g_GROUP_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)&app_info->nvm_storage.Group_Table,data,sizeof(app_info->nvm_storage.Group_Table));
  }
//#endif       


  if(offset == g_ADDRESS_MAP_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.Address_Map_Table,data,sizeof(app_info->nvm_storage.Address_Map_Table));
  }


//#if ( g_APS_LAYER_SEC_PROCESSING_d == 1 )
//#if (( g_TRUST_CENTRE_d == 1 ) && ( g_USE_HASH_KEY == 0 ) )
//#if ( g_APP_LINK_KEY_TABLE_app_info->nvm_storage_d == RSI_ENABLE )
#ifdef ZIGBEE_COORDINATOR
  if(offset == g_APP_LINK_KEY_TABLE_OFFSET_c)
  {
    memcpy((uint8_t*)app_info->nvm_storage.LinkKeyTable,data,sizeof(LinkKeyTable_t));
  }


  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (1 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[1],data,sizeof(LinkKeyTable_t));
  }


  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (2 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[2],data,sizeof(LinkKeyTable_t));
  }


  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (3 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[3],data,sizeof(LinkKeyTable_t));
  }


  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (4 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[4],data,sizeof(LinkKeyTable_t));
  }


  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (5 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[5],data,sizeof(LinkKeyTable_t));
  }

  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (6 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[6],data,sizeof(LinkKeyTable_t));
  }

  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (7 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[7],data,sizeof(LinkKeyTable_t));
  }

  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (8 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[8],data,sizeof(LinkKeyTable_t));
  }

  if(offset == (g_APP_LINK_KEY_TABLE_OFFSET_c + (9 * sizeof(LinkKeyTable_t))))
  {
    memcpy((uint8_t*)&app_info->nvm_storage.LinkKeyTable[9],data,sizeof(LinkKeyTable_t));
  }
#endif      
  rsi_zigb_app_info.state = FSM_ZB_MATCH_DESC_RESP;

}
/*-----------------------------------------------------------------------------
 * Interrupt Service Routines
 *----------------------------------------------------------------------------*/

/* None */

/*-----------------------------------------------------------------------------
 * End Of File
 *----------------------------------------------------------------------------*/
