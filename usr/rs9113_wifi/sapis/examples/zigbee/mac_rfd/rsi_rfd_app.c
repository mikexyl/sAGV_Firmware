/**
 * @file     rsi_rfd_app.c
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
#define ASSOC_REQ_EVENT                     0x1
#define DATA_REQ_EVENT                      0x2
#define POLL_REQ_EVENT                      0x3

// Application parameters

/************** SCAN Req Params *****************/
//! SCAN TYPE
#define ACTIVE_SCAN                         0x1

//! SCAN CHANNEL MASK
#define SCAN_CH_MASK_0                      0x00
#define SCAN_CH_MASK_1                      0x80
#define SCAN_CH_MASK_2                      0x00
#define SCAN_CH_MASK_3                      0x00

//! SCAN DURATION
#define SCAN_DURATION                       0x5
/***********************************************/

/************** Data Req Params *****************/
#define SHORT_ADDR_MODE                     0x2
#define EXTND_ADDR_MODE                     0x3
#define MSDU_HANDLE                         0x5
#define MSDU_LENGTH                         20
#define ACK_TX_OPTIONS                      0x1
#define MAC_PAYLOAD                         {'R','E','D','P','I','N','E',' ','S','I','G','N','A','L','S',' ','M','A','C',' '}
/***********************************************/

//! Assoc Req params
#define RFD_CAPABILITY_INFO                 0x04
#define FFD_CAPABILITY_INFO                 0x8E

//! MAC DEFINES
#define MAC_OPER_MODE                       0x23
#define MAC_MAX_PKT_LEN                     156
#define RSI_DELAY                           0x07FFFFFF
#define MAC_TRUE                            1
#define MAC_FALSE                           0
#define MAC_SUCCESS                         0
#define MAC_FAILURE                         1

//! Memory to initialize driver
uint8_t zigb_global_buf[ZIGB_GLOBAL_BUFF_LEN];

//! Event BitMap
uint32_t Event_Bitmap=0;

//! Scan Req 
MLME_Scan_Request_t   ScanRequest;

//! Assoc Req
MLME_Associate_Request_t Assoc_request;

//! Data Req
MCPS_Data_Request_t Data_Request;

//! Scan Confirm
MLME_Scan_Confirm_t MLME_Scan_Confirm;


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
      case ASSOC_REQ_EVENT:
        {
          Assoc_request.msg_type = g_MLME_ASSOCIATE_REQUEST_c;
          Assoc_request.logical_channel = MLME_Scan_Confirm.result_list.pan_desc_list[0].logical_channel;
          Assoc_request.coord_addr_mode = MLME_Scan_Confirm.result_list.pan_desc_list[0].coord_addr_mode;
          Assoc_request.a_coord_pan_id[0] = MLME_Scan_Confirm.result_list.pan_desc_list[0].a_coord_pan_id[0];
          Assoc_request.a_coord_pan_id[1] = MLME_Scan_Confirm.result_list.pan_desc_list[0].a_coord_pan_id[1];
          Assoc_request.coord_addr.a_short_addr[0] = MLME_Scan_Confirm.result_list.pan_desc_list[0].coord_addr.a_short_addr[0];
          Assoc_request.coord_addr.a_short_addr[1] = MLME_Scan_Confirm.result_list.pan_desc_list[0].coord_addr.a_short_addr[1];
          Assoc_request.capability_info = RFD_CAPABILITY_INFO;

          RSI_DPRINT(RSI_PL1,"\n Sending Assoc Req \n");
          rsi_zigb_mac_assoc_req(&Assoc_request);
          rsi_zigb_ClearEvent(ASSOC_REQ_EVENT);
        }
        break;

      case DATA_REQ_EVENT:
        {
          uint8_t mac_payload[MSDU_LENGTH] = MAC_PAYLOAD;
          Data_Request.msg_type = g_MCPS_DATA_REQUEST_c;
          Data_Request.src_addr_mode = SHORT_ADDR_MODE; 
          Data_Request.dst_addr_mode = Assoc_request.coord_addr_mode;
          Data_Request.a_dst_pan_id[0]=Assoc_request.a_coord_pan_id[0];
          Data_Request.a_dst_pan_id[1]=Assoc_request.a_coord_pan_id[1];
          Data_Request.dst_addr.a_short_addr[0] = Assoc_request.coord_addr.a_short_addr[0];
          Data_Request.dst_addr.a_short_addr[0] = Assoc_request.coord_addr.a_short_addr[1];
          Data_Request.msdu_handle = MSDU_HANDLE;
          Data_Request.tx_options = ACK_TX_OPTIONS;
          Data_Request.msdu_length = MSDU_LENGTH;

          RSI_DPRINT(RSI_PL1,"\n MSDU:");
          for(i=0;i<Data_Request.msdu_length;i++)
          {
            Data_Request.a_msdu[i] = mac_payload[i];
          }

          rsi_zigb_mac_data_req(0, 0, &Data_Request);
          rsi_zigb_ClearEvent(DATA_REQ_EVENT);
        }
        break;

       case POLL_REQ_EVENT:
        {
          MLME_Poll_Request_t Poll_req;
          Poll_req.msg_type = g_MLME_POLL_REQUEST_c;
          Poll_req.coord_addr_mode =  Assoc_request.coord_addr_mode;
          Poll_req.a_coord_pan_id[0]= Assoc_request.a_coord_pan_id[0];
          Poll_req.a_coord_pan_id[1]= Assoc_request.a_coord_pan_id[1];
          Poll_req.coord_addr.a_short_addr[0] = Assoc_request.coord_addr.a_short_addr[0];
          Poll_req.coord_addr.a_short_addr[1] = Assoc_request.coord_addr.a_short_addr[1];
          rsi_zigb_mac_poll_req(&Poll_req);
          rsi_zigb_ClearEvent(POLL_REQ_EVENT);
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

  ScanRequest.msg_type = g_MLME_SCAN_REQUEST_c;
  ScanRequest.scan_type = ACTIVE_SCAN;
  ScanRequest.a_scan_channels[0] = SCAN_CH_MASK_0;
  ScanRequest.a_scan_channels[1] = SCAN_CH_MASK_1;
  ScanRequest.a_scan_channels[2] = SCAN_CH_MASK_2;
  ScanRequest.a_scan_channels[3] = SCAN_CH_MASK_3;
  ScanRequest.scan_duration = SCAN_DURATION;

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
 * This is sample MAC APP.
 *
 * ===========================================================================*/
int32_t rsi_mac_app()
{
  int32_t status = RSI_SUCCESS;

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

  //! zigb MAC init 
  rsi_zigb_reset_mac();

  //! Scan Request
  rsi_zigb_mac_scan_req(&ScanRequest);
  
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

  //! Call MAC application
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
  uint32_t    delay;
  RSI_DPRINT(RSI_PL1,"\n MAC DATA CONFIRM:\n status:%d",pMCPS_Data_Confirm->status);
  
  //! wait in scheduler for some time 1.5sec
  for(delay = 0; delay < RSI_DELAY; delay++)
  {
    rsi_wireless_driver_task();
  }

  if(pMCPS_Data_Confirm->status == MAC_SUCCESS)
  {
    rsi_zigb_SetEvent(DATA_REQ_EVENT);
  }
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
  rsi_zigb_SetEvent(DATA_REQ_EVENT);
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
  memcpy((uint8_t*)&MLME_Scan_Confirm,pMLME_Scan_Confirm,sizeof(MLME_Scan_Confirm_t));
  rsi_zigb_SetEvent(ASSOC_REQ_EVENT);
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

