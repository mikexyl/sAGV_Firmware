/**
 * @file    rsi_ble_gatt_test.c
 * @version 0.1
 * @date    09 Sep 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application to test the GATT client API's
 *
 *  @section Description  This application has covered most of the GATT client API's
 */

/**
 * Include files
 * */

//! BLE include files to refer BLE APIs
#include<rsi_ble_apis.h>
#include<rsi_ble_config.h>
#include<rsi_common_apis.h>

#define LOG_PRINT  printf

//! Memory length for driver
#define BT_GLOBAL_BUFF_LEN   10000

//! Memory to initialize driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN];

#define BLE_ATT_REC_SIZE         500
#define NO_OF_VAL_ATT             5

#define SERVER                0
#define CLIENT                1

#define GATT_ROLE             SERVER

//! BLE attribute service types uuid values
#define  RSI_BLE_CHAR_SERV_UUID         0x2803
#define  RSI_BLE_CLIENT_CHAR_UUID       0x2902

//! BLE characteristic service uuid
#define  RSI_BLE_NEW_SERVICE_UUID       0xAABB
#define  RSI_BLE_ATTRIBUTE_1_UUID       0x1AA1
//! local device name
#define RSI_BLE_APP_GATT_TEST  "RSI_BLE_GATT_TEST"

//! immediate alert service uuid
#define  RSI_BLE_NEW_CLIENT_SERVICE_UUID       0x180F
//! Alert level characteristic
#define  RSI_BLE_CLIENT_ATTRIBUTE_1_UUID       0x2A19

#define  RSI_BLE_MAX_DATA_LEN   20

//! attribute properties
#define  RSI_BLE_ATT_PROPERTY_READ      0x02
#define  RSI_BLE_ATT_PROPERTY_WRITE     0x08
#define  RSI_BLE_ATT_PROPERTY_NOTIFY    0x10

//! application event list
#define  RSI_BLE_CONN_EVENT                     0x01
#define  RSI_BLE_DISCONN_EVENT                  0x02
#define  RSI_BLE_GATT_WRITE_EVENT               0x03
#define  RSI_BLE_READ_REQ_EVENT                 0x04
#define  RSI_BLE_MTU_EVENT                      0x05
#define  RSI_BLE_GATT_PROFILE_RESP_EVENT        0x06
#define  RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT  0x07

//! remote device address
#define  RSI_BLE_REMOTE_BD_ADDRESS   "00:23:A7:92:4E:45"

typedef struct rsi_ble_att_list_s{
  uuid_t  char_uuid;
  uint16_t  handle;
  uint16_t  len;         
  void     *value;
}rsi_ble_att_list_t;

typedef struct rsi_ble_s{
  uint8_t   DATA[BLE_ATT_REC_SIZE];
  uint16_t  DATA_ix;
  uint16_t  att_rec_list_count;
  rsi_ble_att_list_t  att_rec_list[NO_OF_VAL_ATT];
}rsi_ble_t;

rsi_ble_t att_list;

uint16_t   mtu_size;

#ifdef RSI_WITH_OS
//! BLE task stack size
#define RSI_BT_TASK_STACK_SIZE  1000

//! BT task priority
#define RSI_BT_TASK_PRIORITY   1

//! Number of packet to send or receive
#define NUMBER_OF_PACKETS 1000

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   2

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  3000

void rsi_wireless_driver_task(void);

#endif

//! global parameters list
static uint32_t ble_app_event_map;
static uint32_t ble_app_event_mask;
static rsi_ble_event_conn_status_t conn_event_to_app;
static rsi_ble_event_disconnect_t disconn_event_to_app;
static rsi_ble_event_write_t app_ble_write_event;
static uint16_t rsi_ble_att1_val_hndl;
static rsi_ble_read_req_t app_ble_read_event;
static rsi_ble_event_mtu_t app_ble_mtu_event;
static profile_descriptors_t  rsi_ble_service;
static rsi_ble_resp_char_services_t char_servs;

/*==============================================*/
/**
 * @fn         rsi_ble_fill_128bit_uuid  
 * @brief      this function is used to form 128bit uuid of apple device from 32 bit uuid.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static void rsi_ble_fill_128bit_uuid (uint32_t uuid_32bit, uuid_t *serv_uuid)
{
  uint8_t ix;
	serv_uuid->size = 16;
	rsi_uint32_to_4bytes(serv_uuid->val.val128.data1, uuid_32bit);
	rsi_uint16_to_2bytes(serv_uuid->val.val128.data2, 0x00);
	rsi_uint16_to_2bytes(serv_uuid->val.val128.data3, 0x1000);
	rsi_uint16_to_2bytes(&serv_uuid->val.val128.data4[0], 0x8000);
  for (ix =0 ; ix < 6;ix++) {
    serv_uuid->val.val128.data4[2] = 0x26; 
    serv_uuid->val.val128.data4[3] = 0x00; 
    serv_uuid->val.val128.data4[4] = 0x91; 
    serv_uuid->val.val128.data4[5] = 0x52; 
    serv_uuid->val.val128.data4[6] = 0x76; 
    serv_uuid->val.val128.data4[7] = 0xBB; 
  }

  return;
}
void print_data_pkt (unsigned char *input_buf, uint16_t buf_len)
{
  uint16_t  ix;

  LOG_PRINT ("buf_len: %d\r\n", buf_len);
  for (ix = 0; ix < buf_len; ix++)
  {
    if (ix % 16 == 0)
    {
      LOG_PRINT ("\r\n");
    }
    else if (ix % 8 == 0)
    {
      LOG_PRINT ("\t\t");
    }
    LOG_PRINT (" 0x%02X ", input_buf[ix]);
  }
  LOG_PRINT ("\r\n");
  
  return;
}


/*==============================================*/
/**
 * @fn         rsi_ble_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
static void rsi_ble_add_char_serv_att (void *serv_handler,
		uint16_t handle,
		uint8_t val_prop,
		uint16_t att_val_handle,
		uuid_t att_val_uuid)
{
	rsi_ble_req_add_att_t  new_att = {0};


	//! preparing the attribute service structure
	new_att.serv_handler = serv_handler;
	new_att.handle = handle;
	new_att.att_uuid.size = 2;
	new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
	new_att.property = RSI_BLE_ATT_PROPERTY_READ;


	//! preparing the characteristic attribute value
	new_att.data_len = 6;
	new_att.data[0] = val_prop;
	rsi_uint16_to_2bytes (&new_att.data[2], att_val_handle);
	rsi_uint16_to_2bytes (&new_att.data[4], att_val_uuid.val.val16);

	//! Add attribute to the service
	rsi_ble_add_attribute (&new_att);

	return;
}
/*==============================================*/
/**
 * @fn         rsi_gatt_add_attribute_to_list
 * @brief      This function is used to store characteristic service attribute.
 * @param[in]  p_val, pointer to homekit structure
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  data_len, characteristic value length
 * @param[in]  data, characteristic value pointer
 * @param[in]  uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used to store all attribute records  
 */
void rsi_gatt_add_attribute_to_list (rsi_ble_t *p_val,uint16_t handle,
                                     uint16_t data_len,uint8_t *data, uuid_t uuid) 
{
  if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE)
  {
    LOG_PRINT ("no data memory for att rec values");
    return;
  }

  p_val->att_rec_list[p_val->att_rec_list_count].char_uuid = uuid;
  p_val->att_rec_list[p_val->att_rec_list_count].handle = handle;
  p_val->att_rec_list[p_val->att_rec_list_count].len = data_len;
  memcpy (p_val->DATA + p_val->DATA_ix, data, data_len);
  p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
  p_val->att_rec_list_count++;
  p_val->DATA_ix += data_len;

  return;
}
/*==============================================*/
/**
 * @fn         rsi_ble_add_char_val_att
 * @brief      this function is used to add characteristic value attribute.
 * @param[in]  serv_handler, new service handler.
 * @param[in]  handle, characteristic value attribute handle.
 * @param[in]  att_type_uuid, attribute uuid value.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  data, characteristic value data pointer.
 * @param[in]  data_len, characteristic value length.
 * @return     none.
 * @section description
 * This function is used at application to create new service.
 */

static void rsi_ble_add_char_val_att (void *serv_handler,
		uint16_t handle,
		uuid_t   att_type_uuid,
		uint8_t  val_prop,
		uint8_t *data,
		uint8_t  data_len,
		uint8_t   auth_read)
{
	rsi_ble_req_add_att_t  new_att = {0};

	//! preparing the attributes
	new_att.serv_handler = serv_handler;
	new_att.handle = handle;
	new_att.reserved = auth_read;
	memcpy (&new_att.att_uuid, &att_type_uuid, sizeof (uuid_t));
	new_att.property = val_prop;

	//! preparing the attribute value
	new_att.data_len = RSI_MIN(sizeof (new_att.data), data_len);
  if(data != NULL)
	memcpy (new_att.data, data, new_att.data_len);

	//! add attribute to the service
	rsi_ble_add_attribute (&new_att);
  
  if ((auth_read == 1) || (data_len > 20)) {
     if(data != NULL)
    rsi_gatt_add_attribute_to_list (&att_list, handle, data_len, data, att_type_uuid);
  }

	//! check the attribute property with notification
	if (val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY)
	{
		//! if notification property supports then we need to add client characteristic service.

		//! preparing the client characteristic attribute & values
		memset (&new_att, 0, sizeof(rsi_ble_req_add_att_t));
		new_att.serv_handler = serv_handler;
		new_att.handle = handle + 1;
		new_att.att_uuid.size = 2;
		new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
		new_att.property = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
		new_att.data_len = 2;

		//! add attribute to the service
		rsi_ble_add_attribute (&new_att);
	}

	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_chat_add_new_serv
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */

static uint32_t rsi_ble_add_simple_chat_serv (void)
{
	uuid_t  new_uuid = {0};
	rsi_ble_resp_add_serv_t  new_serv_resp = {0};
  uint8_t data1[100] = {1,0};
 
	//! adding new service
	new_uuid.size = 2;
	new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;
	rsi_ble_add_service (new_uuid, 10, 100, &new_serv_resp);

	//! adding characteristic service attribute to the service
	new_uuid.size = 2;
	new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
	rsi_ble_add_char_serv_att (new_serv_resp.serv_handler,
			new_serv_resp.start_handle + 1,
			RSI_BLE_ATT_PROPERTY_WRITE  | RSI_BLE_ATT_PROPERTY_READ| RSI_BLE_ATT_PROPERTY_NOTIFY,
			new_serv_resp.start_handle + 2,
			new_uuid);

	//! adding characteristic value attribute to the service
	rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
	new_uuid.size = 2;
	new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
	rsi_ble_add_char_val_att (new_serv_resp.serv_handler,
			new_serv_resp.start_handle + 2,
			new_uuid,
			RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ| RSI_BLE_ATT_PROPERTY_NOTIFY,
			data1,
			sizeof (data1),1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_ble_app_init_events()
{
	ble_app_event_map = 0;
	ble_app_event_mask = 0xFFFFFFFF;
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_ble_app_set_event(uint32_t event_num)
{
	ble_app_event_map |= BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
	ble_app_event_map &= ~BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_ble_app_get_event(void)
{
	uint32_t  ix;

	for (ix = 0; ix < 32; ix++)
	{
		if (ble_app_event_map & (1 << ix))
		{
		   return ix;
		}
	}

	return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
	memcpy(&conn_event_to_app, resp_conn, sizeof(rsi_ble_event_conn_status_t));
	rsi_ble_app_set_event (RSI_BLE_CONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
	memcpy(&disconn_event_to_app, resp_disconnect, sizeof(rsi_ble_event_conn_status_t));
	rsi_ble_app_set_event (RSI_BLE_DISCONN_EVENT);
}
/*==============================================*/
/**
 * @fn         rsi_ble_profile
 * @brief      invoked when the specific service details are received for
 *             supported specific services.
 * @param[out] p_ble_resp_profile, specific profile details
 * @return     none
 * @section description
 * This is a callback function
 */
static void rsi_ble_profile (uint16_t resp_status, profile_descriptors_t *rsi_ble_resp_profile)
{
    rsi_ble_app_set_event (RSI_BLE_GATT_PROFILE_RESP_EVENT);
    return;
}
/*==============================================*/
/**
 * @fn         rsi_ble_char_services
 * @brief      invoked when response is received for characteristic services details
 * @param[out] p_ble_resp_char_services, list of characteristic services.
 * @return     none
 * @section description
 */
static void rsi_ble_char_services (uint16_t resp_status, rsi_ble_resp_char_services_t *rsi_ble_resp_char_services)
{
	rsi_ble_app_set_event (RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_gatt_write_event (uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
	memcpy (&app_ble_write_event, rsi_ble_write, sizeof (rsi_ble_event_write_t));
	rsi_ble_app_set_event (RSI_BLE_GATT_WRITE_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_read_req_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_read_req_event (uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req)
{

  memcpy (&app_ble_read_event, rsi_ble_read_req, sizeof (rsi_ble_read_req_t));
  rsi_ble_app_set_event (RSI_BLE_READ_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_mtu_event (rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy (&app_ble_mtu_event, rsi_ble_mtu, sizeof (rsi_ble_event_mtu_t));
  mtu_size = (uint16_t)app_ble_mtu_event.mtu_size;
	rsi_ble_app_set_event (RSI_BLE_MTU_EVENT);
}




/*==============================================*/
/**
 * @fn         rsi_ble_simple_gatt_test
 * @brief      this is the application of ble GATT client api's test cases.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used at application.
 */
void rsi_ble_simple_gatt_test (void)
{
	int32_t status = 0;
	uint8_t adv[31] = {2,1,6};
	int32_t  event_id;
	int8_t   data[20] = {0};
	int8_t   client_data[100] = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0};

  int8_t   rssi_data;
	uint8_t  remote_dev_addr[18] = {0};
	uuid_t   search_serv;
	rsi_ble_resp_local_att_value_t  local_att_val;
	uint8_t loop, rssi, ix;
	uuid_t  service_uuid;

    uint8_t   read_data[100] = {2};

	//! Driver initialization
	status = rsi_driver_init(bt_global_buf, BT_GLOBAL_BUFF_LEN);
  if((status < 0) || (status > BT_GLOBAL_BUFF_LEN))
	{
		return status;
	}
  
  //! RS9113 intialisation 
  status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
	if(status != RSI_SUCCESS)
	{
		return status;
	}

	//! WC initialization
	status = rsi_wireless_init(0, RSI_OPERMODE_WLAN_BLE);
	if(status != RSI_SUCCESS)
	{
		return status;
	}

#if(GATT_ROLE == SERVER)
  rsi_ble_add_simple_chat_serv();
#endif
	
  //! registering the GAP call back functions
	rsi_ble_gap_register_callbacks(
			NULL,
			rsi_ble_on_connect_event,
			rsi_ble_on_disconnect_event,
      NULL);

	//! registering the GATT call back functions
	rsi_ble_gatt_register_callbacks (NULL,
			rsi_ble_profile,
			rsi_ble_char_services,
			NULL,
			NULL,
			NULL,
			NULL,
			rsi_ble_on_gatt_write_event,
      NULL,
      NULL,
      rsi_ble_on_read_req_event,
   	  rsi_ble_on_mtu_event);

	//!  initializing the application events map
	rsi_ble_app_init_events ();

	//! Set local name
	rsi_bt_set_local_name(RSI_BLE_APP_GATT_TEST);

#if(GATT_ROLE == SERVER)
  //! prepare advertise data //local/device name
  adv[3] = strlen (RSI_BLE_APP_GATT_TEST) + 1;
  adv[4] = 9;
  strcpy (&adv[5], RSI_BLE_APP_GATT_TEST);

	//! set advertise data
	rsi_ble_set_advertise_data (adv, strlen (RSI_BLE_APP_GATT_TEST) + 5);

	//! set device in advertising mode.
	rsi_ble_start_advertising ();
#endif

#if(GATT_ROLE == CLIENT)
	//! initiating connection with remote device
	rsi_ble_connect ((int8_t)0, (int8_t *)RSI_BLE_REMOTE_BD_ADDRESS);
#endif
	
  //! waiting for events from controller.
	while (1) {

		//! Application main loop
		rsi_wireless_driver_task();

		//! checking for events list
		event_id = rsi_ble_app_get_event ();
		if (event_id == -1) {
			continue;
		}

		switch (event_id) {
      case RSI_BLE_CONN_EVENT:
				{
					//! event invokes when connection was completed

					//! clear the served event
          rsi_ble_app_clear_event (RSI_BLE_CONN_EVENT);
          rsi_6byte_dev_address_to_ascii ((int8_t *)remote_dev_addr, (uint8_t *)conn_event_to_app.dev_addr);

#if(GATT_ROLE == CLIENT)
          service_uuid.size = 2;
          service_uuid.val.val16 = RSI_BLE_NEW_CLIENT_SERVICE_UUID;
retry:          
          status = rsi_ble_get_profile_async (remote_dev_addr, service_uuid, &rsi_ble_service);
          if(status != 0)
             goto retry;
#endif
				}
				break;

			case RSI_BLE_DISCONN_EVENT:
				{
					//! event invokes when disconnection was completed

					//! clear the served event
					rsi_ble_app_clear_event (RSI_BLE_DISCONN_EVENT);
#if(GATT_ROLE == SERVER)
adv:
					//! set device in advertising mode.
					status = rsi_ble_start_advertising ();
					if (status != RSI_SUCCESS) {
						goto adv;
          }
#endif
#if(GATT_ROLE == CLIENT)
          //! initiating connection with remote device
          rsi_ble_connect ((int8_t)0, (int8_t *)RSI_BLE_REMOTE_BD_ADDRESS);
#endif
				}
				break;
			case RSI_BLE_GATT_WRITE_EVENT:
				{
					//! event invokes when write/notification events received

					//! clear the served event
					rsi_ble_app_clear_event (RSI_BLE_GATT_WRITE_EVENT);

					//! prepare the data to set as local attribute value.
					memset (data, 0, sizeof (data));
					memcpy(data, app_ble_write_event.att_value, app_ble_write_event.length);

					rsi_ble_get_local_att_value (rsi_ble_att1_val_hndl, &local_att_val);
					//! set the local attribute value.
					rsi_ble_set_local_att_value (rsi_ble_att1_val_hndl, RSI_BLE_MAX_DATA_LEN, data);
					rsi_ble_get_local_att_value (rsi_ble_att1_val_hndl, &local_att_val);
				}
				break;
      case RSI_BLE_READ_REQ_EVENT:
				{
					//! event invokes when write/notification events received

					//! clear the served event
					rsi_ble_app_clear_event (RSI_BLE_READ_REQ_EVENT);

          rsi_6byte_dev_address_to_ascii ((int8_t *)remote_dev_addr, (uint8_t *)conn_event_to_app.dev_addr);
					rsi_ble_gatt_read_response (remote_dev_addr,
							0,
							app_ble_read_event.handle,
							0,
							RSI_MIN(mtu_size - 2, sizeof(read_data)), 
							read_data);
				}
				break;
			
      case RSI_BLE_GATT_PROFILE_RESP_EVENT:
			{
				//! event invokes when get profile response received

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_GATT_PROFILE_RESP_EVENT);

#if(GATT_ROLE == CLIENT)
				//! get characteristics of the immediate alert servcie
				rsi_6byte_dev_address_to_ascii ((int8_t *)remote_dev_addr, (uint8_t *)conn_event_to_app.dev_addr);
				rsi_ble_get_char_services_async (remote_dev_addr, *(uint16_t *)rsi_ble_service.start_handle, *(uint16_t *)rsi_ble_service.end_handle, &char_servs);
#endif
			}
			break;
      
      case RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT:
      {
        //! event invokes when get characteristics of the service response received

        //! clear the served event
        rsi_ble_app_clear_event (RSI_BLE_GATT_CHAR_SERVICES_RESP_EVENT);

#if(GATT_ROLE == CLIENT)
        //! verifying the immediate alert characteristic
        for (ix = 0; ix < char_servs.num_of_services; ix++) {
          if (char_servs.char_services[ix].char_data.char_uuid.val.val16 == RSI_BLE_CLIENT_ATTRIBUTE_1_UUID) {
            rsi_ble_att1_val_hndl = char_servs.char_services[ix].char_data.char_handle;

              rsi_ble_set_att_cmd (remote_dev_addr, rsi_ble_att1_val_hndl, RSI_MIN(mtu_size - 3, 100), &client_data);
            //! set the event to calculate RSSI value
#ifndef RSI_SAMPLE_HAL
            last_time = GetTickCount ();
#endif
            break;
          }
        }
#endif
      }
      break;

      case RSI_BLE_MTU_EVENT:
        {
          //! event invokes when write/notification events received

          //! clear the served event
          rsi_ble_app_clear_event (RSI_BLE_MTU_EVENT);

        }
				break;
		}
	}

	return;
}

/*==============================================*/
/**
 * @fn         main_loop
 * @brief      Schedules the Driver task.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function schedules the Driver task.
 */
void main_loop (void)
{
	while (1) {
		rsi_wireless_driver_task ();
	}
}

/*==============================================*/
/**
 * @fn         main
 * @brief      main function
 * @param[in]  none.
 * @return     none.
 * @section description
 * This is the main function to call Application
 */
int main (void)
{
#ifndef RSI_SAMPLE_HAL
  //! Board Initialization
  Board_init();
#endif

  //! Call WLAN BLE application
  rsi_ble_simple_gatt_test();

  //! Application main loop
  main_loop();

  return 0;
}
