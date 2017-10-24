/**
 * @file     rsi_zigb_app.c
 * @version  0.1
 * @date     2015-Aug-31
 *
 * Copyright(C) Redpine Signals 2012
 * All rights reserved by Redpine Signals.
 *
 * @section License
 * This program should be used on your own responsibility.
 * Redpine Signals assumes no responsibility for any losses
 * incurred by customers or third parties arising from the use of this file.
 *
 * @brief 
 * Definitions of all ZigBee Frame Descriptors
 *
 * @section Description
 * This file contains definition of all frame descriptors used in ZigBee.
 * These definition are used to construct frames. 
 *
 *
 */

/**
 * Includes
 */
#include "rsi_driver.h"
#include "rsi_zb_types.h"
#include "rsi_zb_api.h"
#include "rsi_zb_onoff.h"
#include "rsi_zb_interfaces.h"
#include "rsi_zb_config.h"
#ifdef LINUX_PLATFORM
#include <stdio.h>
#endif
//! Number of packet to send or receive
#define NUMBER_OF_PACKETS 50

//! Number
#define ZIGB_GLOBAL_BUFF_LEN   8000

#define RSI_DELAY                           0x07FFFFFF
//! Memory to initialize driver
uint8_t zigb_global_buf[ZIGB_GLOBAL_BUFF_LEN];

void rsi_zb_app_init();
void rsi_zigb_hold();
/*===========================================================================
 *
 * @fn          int32_t rsi_zigb_app(void)
 *
 * @brief       ZigBee Application to send toggle command to HA LIGHT. 
 * @param[in]   none
 * @param[out]  none 
 * @return      Status
 * @section description
 * This API is used to prepare the ZigBee Cluster command pkt
 *
 * ===========================================================================*/
int32_t rsi_zigb_app()
{
  int32_t status = RSI_SUCCESS;
  uint8_t device_type = 0;
  uint8_t   ieee_addr[8]={0x0};
  uint8_t   i=0;
  uint16_t* lightAddress;
  lightDeviceInfo_t lightDevInfo;
  rsi_zigb_app_info_t   *app_info = &rsi_zigb_app_info;
  //! Driver initialization
  status = rsi_driver_init(zigb_global_buf, ZIGB_GLOBAL_BUFF_LEN);
  if((status < 0) || (status > ZIGB_GLOBAL_BUFF_LEN))
  {
    RSI_DPRINT(RSI_PL1,"\n driver init failed");
    return status;
  }

  //! RS9113 intialisation 
  status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
  if(status != RSI_SUCCESS)
  {
    RSI_DPRINT(RSI_PL1,"\n device init failed");
    return status;
  }
 
  //! WC initialization
  RSI_DPRINT(RSI_PL1,"\n Sending Opermode\n");
  status = rsi_wireless_init(0, 3);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
  
  rsi_zb_app_init();
  RSI_DPRINT(RSI_PL1,"\n Sending init stack\n");
  //! zigb stack init 
  status = rsi_zigb_init_stack();


  RSI_DPRINT(RSI_PL1,"\n Sending reset stack\n");
  //! zigb stack init 
  status = rsi_zigb_reset_stack();

#if 0
  RSI_DPRINT(RSI_PL1,"\n Sending SAS update\n");
  //! zigb stack init 
  Startup_Attribute_Set_Default.a_extended_pan_id[1] = 0x23;
  status = rsi_zigb_update_sas(&Startup_Attribute_Set_Default); 

  RSI_DPRINT(RSI_PL1,"\n Sending ZDO update\n");
  //! zigb stack init 
  status = rsi_zigb_update_zdo_configuration(&g_Table_Default);

#endif
  RSI_DPRINT(RSI_PL1,"\n Sending ieee addr\n");
  //! zigb stack init 
  status = rsi_zigb_get_self_ieee_address(ieee_addr);
  
  status = rsi_zigb_get_device_type(&device_type);
  RSI_DPRINT(RSI_PL1,"\ndevice:0x%x", device_type);


  status = rsi_zigb_initiate_scan(g_MAC_ACTIVE_SCAN_TYPE_c,
      g_CHANNEL_MASK_c, g_SCAN_DURATION_c);
   rsi_zigb_hold();
   if(app_info->state != FSM_SCAN_COMPLETE)
   {
      RSI_DPRINT(RSI_PL1,"\n Could Not Find Network");
      return -1;
   }
  RSI_DPRINT(RSI_PL1,"\nscan_status:0x%x", status);
  
  status =rsi_zigb_join_network(device_type,
      rsi_zigb_app_info.nwkinfo.channel, 0x0f, rsi_zigb_app_info.nwkinfo.extendedPanId);

   if(app_info->state != FSM_JOIN_COMPLETE)
   {
      RSI_DPRINT(RSI_PL1,"\n Could Not Join the Network");
   
      return -1;
   }
  RSI_DPRINT(RSI_PL1,"\njoin_status:0x%x", status);

  status = rsi_zigb_set_simple_descriptor(ONOFF_SWITCH_END_POINT, 
      rsi_zigb_app_info.DeviceSimpleDesc);
  RSI_DPRINT(RSI_PL1,"\nset simple desc:0x%x", status);

 status = rsi_zigb_send_match_descriptors_request(0x0000, 
                  app_info->DeviceSimpleDesc->app_profile_id, 
                  (uint8_t*)app_info->DeviceSimpleDesc->p_incluster_list,
                  app_info->DeviceSimpleDesc->incluster_count, 
                  (uint8_t*) app_info->DeviceSimpleDesc->p_outcluster_list,
                  app_info->DeviceSimpleDesc->outcluster_count, 1, 0xFFFF);

  RSI_DPRINT(RSI_PL1,"\nMatch desc:0x%x", status);
   while(app_info->state != FSM_ZB_MATCH_DESC_RESP)
   {
      RSI_DPRINT(RSI_PL1,"\n Poll for response");
      rsi_zigb_end_device_poll_for_data();
      rsi_zigb_hold();
   }

  lightAddress =  (uint16_t *)&rsi_zigb_app_info.zb_resp_info.matchDescResp[2];
  lightDevInfo.shortaddress = *lightAddress;    
  lightDevInfo.endpoint = rsi_zigb_app_info.zb_resp_info.matchDescResp[5];

  for(i=0;i<NUMBER_OF_PACKETS;i++)
  {
    rsi_zigb_hold();
    
    RSI_DPRINT(RSI_PL1,"\nSend Data:0x%x", status);
    status = rsi_zigb_app_send_data( g_Client_To_Server_c, g_Cluster_Specific_Cmd_c, lightDevInfo.endpoint, lightDevInfo.shortaddress , g_TOGGLE_c, 0x0006, 0x00, 0x0);
  }

  if(status != RSI_SUCCESS)
  {
    return status;
  }
    return status;
}

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

  APSDERequest.ProfileId = HA_PROFILE_ID;
  APSDERequest.DestEndpoint = destEP;
  APSDERequest.ClusterId = cluster;
  APSDERequest.AsduLength = dataLength;
  APSDERequest.SrcEndpoint = ONOFF_SWITCH_END_POINT;
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

   /* Send the Toggle command to the light continuously */
#ifdef ZB_DEBUG        
        RSI_DPRINT(RSI_PL1,"\nSending Toggle command to Corrdinator Home Automation supported Light\n"); 
#endif        
  return status;
}




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
  rsi_zigb_app_info_t   *app_info = &rsi_zigb_app_info;
  if(!status)
  {
   app_info->state = FSM_SCAN_COMPLETE; 
  }
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\n ##Called AppScanCompleteHandler:");
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
 * @fn          void rsi_zigb_app_network_found_handler(ZigBeeNetworkDetails )
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
#ifdef ZB_DEBUG  
  RSI_DPRINT(RSI_PL1,"\n ##Called APPNETWORKFOUNDHANDLER");
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
  rsi_zigb_app_info_t *app_info = &rsi_zigb_app_info;
  if(!rsi_zigb_app_info.stack_status)
  {
    app_info->state = FSM_JOIN_COMPLETE; 
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
      
      app_info->state = FSM_ZB_MATCH_DESC_RESP; 
      }
#ifdef ZB_DEBUG 
  RSI_DPRINT(RSI_PL1,"\n Matxh Desc Response rcvd \n");
#endif  
    }
  }
  if( pDataIndication->cluster_id == 0x8003)//0x8003: power descriptor response
  { 
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.powerDescResp, 
                     pDataIndication->asdulength);
      app_info->status_var.powerDescRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8002)//0x8003: node descriptor response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.nodeDescResp, 
                     pDataIndication->asdulength);
      app_info->status_var.nodeDescRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8021)//0x8021: bind response
  {
    if(pDataIndication->a_asdu[1] == 0x84)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.bindResp, 
                     pDataIndication->asdulength);
      app_info->status_var.bindRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8022)//0x8022: unbind response
  {
    if(pDataIndication->a_asdu[1] == 0x84)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.unbindResp, 
                     pDataIndication->asdulength);
      app_info->status_var.unbindRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8005)//0x8005: active endpoint response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.actepResp, 
                     pDataIndication->asdulength);
      app_info->status_var.actepRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8001)//0x8001: ieee addr response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.ieeeaddrResp, 
                     pDataIndication->asdulength);
      app_info->status_var.ieeeaddrRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8004)//0x8004: simple desc response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.simpledespResp, 
                     pDataIndication->asdulength);
      app_info->status_var.simpledescRspStatus = 0x00;
    }
  }

  if( pDataIndication->cluster_id == 0x8000)//0x8000: network addr response
  {
    if(pDataIndication->a_asdu[1] == 0x00)
    {
      rsi_zigb_mcpy( pDataIndication->a_asdu,
                     app_info->zb_resp_info.networkaddrResp, 
                     pDataIndication->asdulength);
      app_info->status_var.networkaddrRspStatus = 0x00;
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
#ifdef ZB_DEBUG
uint32_t dataConfcnt = 0;
#endif
void rsi_zigb_app_handle_data_confirmation (APSDE_Data_Confirmation_t *pDataConfirmation)
{
  APSDE_Data_Confirmation_t *data_cnf = &(rsi_zigb_app_info.data_conf);      
  rsi_zigb_mcpy((uint8_t *)pDataConfirmation, 
      (uint8_t *)data_cnf, sizeof(APSDE_Data_Confirmation_t));

#ifdef ZB_DEBUG 
  dataConfcnt++;
  RSI_DPRINT(RSI_PL1,"\ndata confirmation status = %x , cnt = %d \n", data_cnf->status, dataConfcnt);
#endif  
  rsi_zigb_app_info.status_var.dataConfWait = 0;
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

int main()
{
  int32_t status;

  //! Call ZigBee Switch application
  status = rsi_zigb_app();

  return status;

}
/*===========================================================================
 *
 * @fn          void rsi_zb_app_init(void)
 *                                                  
 * @brief       application specific initializations 
 * @param[in]   none
 * @return      none
 * @section description
 * This API is used to do application specific initializations.
 *
 * ===========================================================================*/
void rsi_zb_app_init(void)
{
  rsi_zigb_app_info_t *app_info = &rsi_zigb_app_info;
#ifdef ZB_API_TEST  
  rsi_zigb_apitest_t  *api_test_info = &rsi_zigb_apitest;
#endif  

  app_info->state = FSM_CARD_NOT_READY;
  app_info->status_var.scanReqSent = 0;
  app_info->scan_done_cb.channel = 0xff; 
  app_info->scan_done_cb.scan_status = 0xff; 
 
  /*Initializing status variables */
  app_info->status_var.matchDescRspStatus = 0x0f;
  app_info->status_var.powerDescRspStatus = 0x0f;
  app_info->status_var.nodeDescRspStatus = 0x0f;
  app_info->status_var.bindRspStatus = 0x0f;
  app_info->status_var.unbindRspStatus = 0x0f;
  app_info->status_var.actepRspStatus = 0x0f;
  app_info->status_var.ieeeaddrRspStatus = 0x0f;
  app_info->status_var.simpledescRspStatus = 0x0f;
  app_info->status_var.networkaddrRspStatus = 0x0f;

  app_info->DeviceSimpleDesc = &SimpleDesc;
  
  app_info->channel_mask = g_CHANNEL_MASK_c;
  app_info->scan_duration = g_SCAN_DURATION_c;

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


void rsi_zigb_hold()
{
  int i=0,delay=0;
  //for(i=0; i<2; i++)
  {
    for(delay = 0; delay < RSI_DELAY; delay++)
    {
      rsi_wireless_driver_task();
    }
  }
}
