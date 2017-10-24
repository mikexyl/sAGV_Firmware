/**
 * @file     rsi_ffd_app.c
 * @version  0.1
 * @date     2016-Jan-07
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
 * Definitions of all MAC Frame Descriptors
 *
 * @section Description
 * This file contains definition of all frame descriptors used in MAC.
 * These definition are used to construct frames. 
 *
 *
 */

/**
 * Includes
 */

//! include file to refer driver apis
#include "rsi_driver.h"

//! Memory length for driver
#define ZIGB_GLOBAL_BUFF_LEN                10000

//! Event IDs
#define ASSOC_RESP_EVENT                    0x1
#define DATA_REQ_EVENT                      0x2
#define ORPHAN_RESPONSE_EVENT               0x3

// Application parameters
/************** Start Req Params *****************/
#define PAN_ID_L                            0x34
#define PAN_ID_H                            0x12
#define CHANNEL                             0xF
#define BEACON_ORDER                        15
#define SUPERFRAME_ORDER                    10
/***********************************************/

/************** Data Req Params *****************/
#define SHORT_ADDR_MODE                     0x2
#define EXTND_ADDR_MODE                     0x3
#define MSDU_HANDLE                         0x5
#define MSDU_LENGTH                         20
#define ACK_TX_OPTIONS                      0x1
#define MAC_PAYLOAD                         {'R','E','D','P','I','N','E',' ','S','I','G','N','A','L','S',' ','M','A','C',' '}
/***********************************************/

//! Assoc Response params
#define CHILD_SHORT_ADDR_L                  0xCD
#define CHILD_SHORT_ADDR_H                  0xAB

//! Orphan Response params
#define ORPHAN_DEV_SHORT_ADDR_L             0x56
#define ORPHAN_DEV_SHORT_ADDR_H             0x78
#define FFD_SHORT_ADDR                      {0x00,0x00}

//! MAC DEFINES
#define MAC_OPER_MODE                       0x23
#define MAC_MAX_PKT_LEN                     156
#define MAC_TRUE                            1
#define MAC_FALSE                           0
#define MAC_SUCCESS                         0
#define MAC_FAILURE                         1

//! Memory to initialize driver
uint8_t zigb_global_buf[ZIGB_GLOBAL_BUFF_LEN];

//! Event BitMap
uint32_t Event_Bitmap=0;

//! Start Request
MLME_Start_Request_t  StartRequest;

//! Assoc Request
MLME_Associate_Response_t Assoc_resp;

//! Data Request
MCPS_Data_Request_t Data_Request;

//! Comm-Status Indication
MLME_Comm_Status_Indication_t MLME_Comm_Status_Indication_bkp;

uint8_t RFD_Extendedaddr[8];

/*
 * Event APIs
 */
void rsi_zigb_event_init(void)
{
	Event_Bitmap = 0;
}

void rsi_zigb_SetEvent(uint32_t EventId)
{
	Event_Bitmap |= EventId;
}

void rsi_zigb_ClearEvent(uint32_t EventId)
{
	Event_Bitmap &= ~(EventId);
}

uint32_t rsi_zigb_GetEvents()
{
	return Event_Bitmap;
}

/*===========================================================================
 *
 * @fn          void main_loop(void)
 *
 * @brief       Manages events. 
 * @param[in]   none
 * @param[out]  none 
 * @return      Status
 * @section description
 * This API is used to send commands based on events scheduled.
 *
 * ===========================================================================*/
void main_loop(void)
{
  uint8_t   i=0;
  uint32_t event_map = 0;
  while(1)
  {
    event_map = rsi_zigb_GetEvents(); 
    ////////////////////////
    //! Application code ///
    ////////////////////////
    switch(event_map)
    {
      case ASSOC_RESP_EVENT:
        {
          Assoc_resp.msg_type = g_MLME_ASSOCIATE_RESPONSE_c;
          memcpy(Assoc_resp.a_device_addr,RFD_Extendedaddr,g_EXTENDED_ADDRESS_LENGTH_c);
          Assoc_resp.a_assoc_short_addr[0]= CHILD_SHORT_ADDR_L;
          Assoc_resp.a_assoc_short_addr[1]= CHILD_SHORT_ADDR_H;
          Assoc_resp.status = MAC_SUCCESS;
          Assoc_resp.security_level = MAC_FALSE;
          rsi_zigb_mac_assoc_response(&Assoc_resp); 
          rsi_zigb_ClearEvent(ASSOC_RESP_EVENT);
        }
        break;

      case DATA_REQ_EVENT:
        {
          uint8_t mac_payload[MSDU_LENGTH] = MAC_PAYLOAD;
          Data_Request.msg_type = g_MCPS_DATA_REQUEST_c;
          Data_Request.src_addr_mode = MLME_Comm_Status_Indication_bkp.src_addr_mode; 
          Data_Request.dst_addr_mode = MLME_Comm_Status_Indication_bkp.dst_addr_mode; 
          Data_Request.a_dst_pan_id[0]=MLME_Comm_Status_Indication_bkp.a_pan_id[0];
          Data_Request.a_dst_pan_id[1]=MLME_Comm_Status_Indication_bkp.a_pan_id[1];
          for(i=0;i<g_EXTENDED_ADDRESS_LENGTH_c;i++)
          {
            Data_Request.dst_addr.a_extend_addr[i] = MLME_Comm_Status_Indication_bkp.dst_addr.a_extend_addr[i];
          }
          Data_Request.msdu_handle = MSDU_HANDLE;
          Data_Request.tx_options = ACK_TX_OPTIONS;
          Data_Request.msdu_length = MSDU_LENGTH;
          for(i=0;i< Data_Request.msdu_length;i++)
          {
            Data_Request.a_msdu[i] = mac_payload[i];
          }

          rsi_zigb_mac_data_req(0, 0, &Data_Request);
          rsi_zigb_ClearEvent(DATA_REQ_EVENT);
        }
        break;
      case ORPHAN_RESPONSE_EVENT:
        {
          MLME_Orphan_Response_t Orphan_response;
          Orphan_response.msg_type = g_MLME_ORPHAN_RESPONSE_c;

          for(i=0;i<g_EXTENDED_ADDRESS_LENGTH_c;i++)
          {
            Orphan_response.a_orphan_addr[i]= MLME_Comm_Status_Indication_bkp.dst_addr.a_extend_addr[i];
          }
          Orphan_response.a_short_addr[0]=ORPHAN_DEV_SHORT_ADDR_L;
          Orphan_response.a_short_addr[1]=ORPHAN_DEV_SHORT_ADDR_H;
          Orphan_response.assoc_member = MAC_TRUE;
          rsi_zigb_mac_orphan_response(&Orphan_response);
          rsi_zigb_ClearEvent(ORPHAN_RESPONSE_EVENT);
        }
        break;
    }
    rsi_non_os_event_loop();
  }

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
  StartRequest.msg_type = g_MLME_START_REQUEST_c;
  StartRequest.a_pan_id[0] = PAN_ID_L;
  StartRequest.a_pan_id[1] = PAN_ID_H;
  StartRequest.logical_channel = CHANNEL;
  StartRequest.beacon_order = BEACON_ORDER;
  StartRequest.superframe_order = SUPERFRAME_ORDER;
  StartRequest.pan_coord = MAC_TRUE;
  StartRequest.coord_realignmnt = MAC_FALSE;

  rsi_zigb_register_mac_callbacks(
      rsi_zigb_mac_data_confirm,            
      rsi_zigb_mac_data_indication,         
      rsi_zigb_mac_assoc_confirm,           
      rsi_zigb_mac_assoc_indication,        
      rsi_zigb_mac_disassoc_confirm,        
      rsi_zigb_mac_disassoc_indication,     
      rsi_zigb_mac_beacon_notify_indication,
      rsi_zigb_mac_orphan_indication,       
      rsi_zigb_mac_rx_enable_confirm,       
      rsi_zigb_mac_scan_confirm,            
      rsi_zigb_mac_comm_status_confirm,
      rsi_zigb_mac_start_confirm,           
      rsi_zigb_mac_poll_confirm            
      );
}


/*===========================================================================
 *
 * @fn          int32_t rsi_mac_app(void)
 *
 * @brief       MAC APP. 
 * @param[in]   none
 * @param[out]  none 
 * @return      Status
 * @section description
 * This app does driver,module & mac intializations and then it starts the FFD.
 *
 * ===========================================================================*/
int32_t rsi_mac_app()
{
  int32_t status = RSI_SUCCESS;
  uint8_t short_address_set[2] = FFD_SHORT_ADDR;
  uint8_t permit_join = MAC_TRUE;

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
  status = rsi_wireless_init(0, MAC_OPER_MODE);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
  
  //! Application initialization
  rsi_zb_app_init();

  //! zigb MAC init 
  rsi_zigb_init_mac();

  //! zigb MAC reset 
  rsi_zigb_reset_mac();

  //! Set Short Address
  rsi_zigb_mac_mlmepib_set(g_MAC_SHORT_ADDRESS_c,g_INVALID_INDEX_c,short_address_set);
  
  //! Start Request
  rsi_zigb_mac_start_req(&StartRequest);

  //! Enable Permit Join
  rsi_zigb_mac_mlmepib_set(g_MAC_ASSOCIATION_PERMIT_c,g_INVALID_INDEX_c,&permit_join);

	//! Application main loop
  main_loop();
}

/*===========================================================================
 *
 * @fn          int main(void)
 *
 * @brief       main. 
 * @param[in]   none
 * @param[out]  none 
 * @return      Status
 * @section description
 * This is the app main.
 *
 * ===========================================================================*/
int main()
{
  int32_t status;

  //! Call to MAC application
  status = rsi_mac_app();

  return status;

}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_data_confirm(MCPS_Data_Confirm_t* pMCPS_Data_Confirm)
 * @brief       Data Confirm Callback
 * @param[in]   MCPS_Data_Confirm_t* pMCPS_Data_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api reports the results of a request to transfer
 * a data SPDU (MSDU) from a local SSCS entity to a single peer SSCS entity.
 */
int16_t rsi_zigb_mac_data_confirm(MCPS_Data_Confirm_t* pMCPS_Data_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC DATA CONFIRM:\n status:%d",pMCPS_Data_Confirm->status);
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_data_indication(MCPS_Data_Indication_t* pMCPS_Data_Indication)
 * @brief       Data Indication Callback
 * @param[in]   MCPS_Data_Indication_t* pMCPS_Data_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api  indicates the transfer of a data SPDU
 * (i.e., MSDU) from the MAC sublayer to the local SSCS entity.
 */
int16_t rsi_zigb_mac_data_indication(MCPS_Data_Indication_t* pMCPS_Data_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC DATA INDICATION");
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_assoc_confirm(MLME_Associate_Confirm_t* pMLME_Associate_Confirm)
 * @brief       Association Confirm
 * @param[in]   MLME_Associate_Confirm_t* pMLME_Associate_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api is used to inform the next higher
 * layer of the initiating device whether its request to associate was successful
 * or unsuccessful.
 */
int16_t rsi_zigb_mac_assoc_confirm(MLME_Associate_Confirm_t* pMLME_Associate_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC ASSOC CONFIRM");
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_assoc_indication(MLME_Associate_Indication_t* pMLME_Associate_Indication)
 * @brief       Association Indication
 * @param[in]   MLME_Associate_Indication_t* pMLME_Associate_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api is used to indicate the reception of
 * an association request command.
 */
int16_t rsi_zigb_mac_assoc_indication(MLME_Associate_Indication_t* pMLME_Associate_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC ASSOC INDICATION");
 
  memcpy(RFD_Extendedaddr,pMLME_Associate_Indication->a_device_addr,g_EXTENDED_ADDRESS_LENGTH_c);
 
  rsi_zigb_SetEvent(ASSOC_RESP_EVENT);
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_disassoc_confirm(MLME_Disassociate_Confirm_t* pMLME_Disassociate_Confirm)
 * @brief       Disassociation Confirm
 * @param[in]   MLME_Disassociate_Confirm_t* pMLME_Disassociate_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api reports the results of an MLME-DISASSOCIATE.request primitive.
 */
int16_t rsi_zigb_mac_disassoc_confirm(MLME_Disassociate_Confirm_t* pMLME_Disassociate_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC DISASSOC CONFIRM");
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_disassoc_indication(MLME_Disassociate_Indication_t* pMLME_Disassociate_Indication)
 * @brief       Disassociation Indication
 * @param[in]   MLME_Disassociate_Indication_t* pMLME_Disassociate_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api is used to indicate the
 * reception of a disassociation notification command.
 */
int16_t rsi_zigb_mac_disassoc_indication(MLME_Disassociate_Indication_t* pMLME_Disassociate_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC DISASSOC INDICATION");
  RSI_DPRINT(RSI_PL1,"\n Dissoac Reason:%x",pMLME_Disassociate_Indication->disassociate_reason);
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_beacon_notify_indication(MLME_Beacon_Notify_Indication_t* pMLME_Beacon_Notify_Indication)
 * @brief       Beacon Notify Indication
 * @param[in]   MLME_Beacon_Notify_Indication_t* pMLME_Beacon_Notify_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api  is used to send parameters contained within a beacon frame
 * received by the MAC sublayer to the next higher layer.
 * The primitive also sends a measure of the LQI and the time the
 * beacon frame was received.
 */
int16_t rsi_zigb_mac_beacon_notify_indication(MLME_Beacon_Notify_Indication_t* pMLME_Beacon_Notify_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC BEACON NOTIFY");
  return 0;//App Developer Code 
}


/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_orphan_indication(MLME_Orphan_Indication_t* pMLME_Orphan_Indication)
 * @brief       Orphan Indication
 * @param[in]   MLME_Orphan_Indication_t* pMLME_Orphan_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api allows the next higher layer of a
 * coordinator to respond to the MLME-ORPHAN.indication primitive.
 */
int16_t rsi_zigb_mac_orphan_indication(MLME_Orphan_Indication_t* pMLME_Orphan_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC ORPHAN INDICATION");
  rsi_zigb_SetEvent(ORPHAN_RESPONSE_EVENT);
  return 0;//App Developer Code 
}


/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_rx_enable_confirm(MLME_RX_Enable_Confirm_t* pMLME_RX_Enable_Confirm)
 * @brief       MAC RX ENABLE Confirm
 * @param[in]   MLME_RX_Enable_Confirm_t* pMLME_RX_Enable_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api  reports the results of the attempt to
 * enable the receiver.
 */
int16_t rsi_zigb_mac_rx_enable_confirm(MLME_RX_Enable_Confirm_t* pMLME_RX_Enable_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC RX EN CONFIRM");
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_scan_confirm(MLME_Scan_Confirm_t* pMLME_Scan_Confirm)
 * @brief       MAC Scan Confirm
 * @param[in]   MLME_Scan_Confirm_t* pMLME_Scan_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api reports the result of the channel scan
 * request.
 */
int16_t rsi_zigb_mac_scan_confirm(MLME_Scan_Confirm_t* pMLME_Scan_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC SCAN CONFIRM");
  return 0;//App Developer Code 
}

/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_comm_status_confirm(MLME_Comm_Status_Indication_t* pMLME_Comm_Status_Indication)
 * @brief       MAC Comm-Status Confirm
 * @param[in]   MLME_Comm_Status_Indication_t* pMLME_Comm_Status_Indication
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api allows the MLME to indicate a
 * communications status.
 */
int16_t rsi_zigb_mac_comm_status_confirm(MLME_Comm_Status_Indication_t* pMLME_Comm_Status_Indication)
{
  RSI_DPRINT(RSI_PL1,"\n MAC COMM STATUS INDICATION");
  
  memcpy((uint8_t*)&MLME_Comm_Status_Indication_bkp,pMLME_Comm_Status_Indication,sizeof(MLME_Comm_Status_Indication_t));
 
  return 0;//App Developer Code 
}


/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_start_confirm(MLME_Start_Confirm_t* pMLME_Start_Confirm)
 * @brief       MAC Start Confirm
 * @param[in]   MLME_Start_Confirm_t* pMLME_Start_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api reports the results of the attempt to start
 * using a new superframe configuration.
 */
int16_t rsi_zigb_mac_start_confirm(MLME_Start_Confirm_t* pMLME_Start_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC START CONFIRM");
  return 0;//App Developer Code 
}


/*==============================================*/
/**
 * @fn          int16_t rsi_zigb_mac_poll_confirm(MLME_Poll_Confirm_t* pMLME_Poll_Confirm)
 * @brief       MAC Poll Confirm
 * @param[in]   MLME_Poll_Confirm_t* pMLME_Poll_Confirm
 * @param[out]  none
 * @return      errCode
 *               0  = SUCCESS
 *
 * @section description
 * This api reports reports the results of a request to poll the
 * coordinator for data.
 */
int16_t rsi_zigb_mac_poll_confirm(MLME_Poll_Confirm_t* pMLME_Poll_Confirm)
{
  RSI_DPRINT(RSI_PL1,"\n MAC POLL CONFIRM");
  return 0;//App Developer Code 
}

