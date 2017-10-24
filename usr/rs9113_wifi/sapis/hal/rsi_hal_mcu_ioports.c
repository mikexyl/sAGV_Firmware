/**
 * @file       rsi_hal_mcu_ioports.c
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
 * @brief Functions to control IO pins of the microcontroller
 *  
 * @section Description
 * This file contains API to control different pins of the microcontroller 
 * which interface with the module and other components related to the module. 
 *
 */


/**
 * Includes
 */
#include "rsi_driver.h"
#include "stm32f4xx_gpio.h"

/**
 * Global Variales
 */
#define WIFI_RESET_N_PORT   GPIOG
#define WIFI_RESET_N_PIN    GPIO_Pin_12

#define WIFI_INTR_PORT      GPIOC
#define WIFI_INTR_PIN       GPIO_Pin_7

typedef struct
{
    GPIO_TypeDef *port;
    uint32_t    pin;
}gpio_t;

//
static gpio_t wifi_io[6] = 
{
    {WIFI_RESET_N_PORT, WIFI_RESET_N_PIN},    //reset
    {WIFI_INTR_PORT, WIFI_INTR_PIN},    //wifi interrupt pin
    {0,0}
};

static int get_gpio(uint8_t gpio_number, gpio_t *gpio)
{
    if( (gpio_number < RSI_HAL_RESET_PIN) ||
        (gpio_number > RSI_HAL_MODULE_POWER_CONTROL) )
        return -1;

    gpio->port = wifi_io[gpio_number-RSI_HAL_RESET_PIN].port;
    gpio->pin = wifi_io[gpio_number-RSI_HAL_RESET_PIN].pin;

    return 0;
}

/*===========================================================*/
/**
 * @fn            void rsi_hal_config_gpio(uint8_t gpio_number,uint8_t mode,uint8_t value)
 * @brief         Configures gpio pin in output mode,with a value
 * @param[in]     uint8_t gpio_number, gpio pin number to be configured
 * @param[in]     uint8_t mode , input/output mode of the gpio pin to configure
 *                0 - input mode
 *                1 - output mode
 * @param[in]     uint8_t value, default value to be driven if gpio is configured in output mode
 *                0 - low
 *                1 - high
 * @param[out]    none
 * @return        none
 * @description This API is used to configure host gpio pin in output mode. 
 */
void rsi_hal_config_gpio(uint8_t gpio_number,uint8_t mode,uint8_t value)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
    gpio_t  gpio;
    int ret;

  //! Initialise the gpio pins in input/output mode
    ret = get_gpio(gpio_number, &gpio);
    if( (ret == -1) || (gpio.port == 0) )
      return ;

    if(mode == RSI_HAL_GPIO_OUTPUT_MODE)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    }
    else
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    }

    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_Pin = gpio.pin;
	  GPIO_Init(gpio.port, &GPIO_InitStructure);

  //! Drive a default value on gpio if gpio is configured in output mode
    if(mode == RSI_HAL_GPIO_OUTPUT_MODE)
    {
        if(value == RSI_HAL_GPIO_HIGH)
            gpio.port->BSRRL = gpio.pin;
        else
            gpio.port->BSRRH = gpio.pin;
    }

   return;
}



/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio  value high
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none 
 * @description   This API is used to drives or makes the host gpio value high. 
 */
void rsi_hal_set_gpio(uint8_t gpio_number)
{
    gpio_t gpio;
    int ret;

  //! drives a high value on GPIO 
    ret = get_gpio(gpio_number, &gpio);
    if( (ret == -1) || (gpio.port == 0) )
      return ;

    gpio.port->BSRRL = gpio.pin;

    return;
}




/*===========================================================*/
/**
 * @fn          uint8_t rsi_hal_get_gpio(void)
 * @brief       get the gpio pin value
 * @param[in]   uint8_t gpio_number, gpio pin number
 * @param[out]  none  
 * @return      gpio pin value 
 * @description This API is used to configure get the gpio pin value. 
 */
uint8_t rsi_hal_get_gpio(uint8_t gpio_number)
{
  uint8_t gpio_value = 0;
  gpio_t gpio;
  int ret;

  //! Get the gpio value
    ret = get_gpio(gpio_number, &gpio);
    if( (ret != -1) && (gpio.port != 0) )
        gpio_value = GPIO_ReadInputDataBit(gpio.port, gpio.pin);

  return gpio_value;
}




/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio value to low
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none 
 * @description   This API is used to drives or makes the host gpio value low. 
 */
void rsi_hal_clear_gpio(uint8_t gpio_number)
{
   gpio_t gpio;
    int ret;

  //! drives a low value on GPIO 
    ret = get_gpio(gpio_number, &gpio);
    if( (ret == -1) || (gpio.port == 0) )
      return ;

    gpio.port->BSRRH = gpio.pin;

  return;
}

