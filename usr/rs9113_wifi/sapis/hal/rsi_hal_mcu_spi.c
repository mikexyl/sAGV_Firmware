/**
 * @file       rsi_hal_mcu_spi.c
 * @version    0.1
 * @date       18 sept 2015
 *
 * Copyright(C) Redpine Signals 2015
 * All rights reserved by Redpine Signals.
 *
 * @section License
 * This program should be used on your own responsibility.
 * Redpine Signals assumes no responsibility for any losses
 * incurred by customers or third parties arising from the use of this file.
 *
 * @brief: HAL SPI API
 *
 * @Description:
 * This file Contains all the API's related to HAL 
 *
 */


/**
 * Includes
 */
#include <stdbool.h>

#include "rsi_driver.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "common/tools.h"

#define     RSI_SPI_PORT        SPI2
#define     SPI_CS_PORT         GPIOD
#define     SPI_CS_PIN          GPIO_Pin_10

#define     SPI_ASSERT_CS()     (SPI_CS_PORT->BSRRH = SPI_CS_PIN)
#define     SPI_DEASSERT_CS()   (SPI_CS_PORT->BSRRL = SPI_CS_PIN)

extern bool  spi_debug;

static void memdump_printf(const void*mem, uint32_t nbytes)
{
    int i;
    uint8_t *buf = (uint8_t*)mem;

    for(i=0; i<nbytes; i++)
    {
        printf("%02x ", buf[i]);
        if((i & MEMDUMP_MAXCHARS_ONELINE) == MEMDUMP_MAXCHARS_ONELINE )
          printf("\r\n");
    }

    if((i&MEMDUMP_MAXCHARS_ONELINE) != MEMDUMP_MAXCHARS_ONELINE )
      printf("\r\n");
}
/**
 * Global Variables
 */

/*==================================================================*/
/**
 * @fn         int16_t rsi_spi_transfer(uint8_t *ptrBuf,uint16_t bufLen,uint8_t *valBuf,uint8_t mode)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transfered
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description  
 * This API is used to tranfer/receive data to the Wi-Fi module through the SPI interface.
 */
int16_t rsi_spi_transfer(uint8_t *tx_buff, uint8_t *rx_buff, uint16_t transfer_length,uint8_t mode)
{
    int i;
    uint8_t dummy;

    if(spi_debug)
    {
      if( (tx_buff) && (rx_buff) )
          printf("SPI WR-RD %d\r\n", transfer_length);
      else if(tx_buff)
          printf("SPI WR %d\r\n", transfer_length);
      else if(rx_buff)
          printf("SPI RD %d\r\n", transfer_length);
      else
          printf("SPI dummy WRRD %d\r\n", transfer_length);

      if(tx_buff)
      {
          printf("Write:");
          memdump_printf(tx_buff, transfer_length);
      }
    }

    
  //! Assert the chip select pin  
    SPI_ASSERT_CS();

  //! Transfer the data 
  //! Receive the data 
    for(i=0; i<transfer_length; i++)
    {
        //while(SPI_I2S_GetFlagStatus(RSI_SPI_PORT, SPI_I2S_FLAG_TXE) == RESET);

        /* 发送一个字节 */
        if(tx_buff)
            SPI_I2S_SendData(RSI_SPI_PORT, tx_buff[i]);
        else
            SPI_I2S_SendData(RSI_SPI_PORT, 0);

        /* 等待数据接收完毕 */
        while(SPI_I2S_GetFlagStatus(RSI_SPI_PORT, SPI_I2S_FLAG_RXNE) == RESET);

        /* 读取接收到的数据 */
        if(rx_buff)
            rx_buff[i] = SPI_I2S_ReceiveData(RSI_SPI_PORT);
        else
            dummy = SPI_I2S_ReceiveData(RSI_SPI_PORT);
    }

  //! Deassert the chip select pin  
    //SPI_DEASSERT_CS();
    if(spi_debug)
    {
      if(rx_buff)
      {
          printf("Read:");
          memdump_printf(rx_buff, transfer_length);
      }
    }

    return 0;
}

