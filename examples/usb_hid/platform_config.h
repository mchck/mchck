/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : platform_config.h
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Evaluation board specific configuration file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)  &&  !defined (USE_STM3210C_EVAL)  &&  !defined (USE_STM32L152_EVAL)
 //#define USE_STM3210B_EVAL
 //#define USE_STM3210E_EVAL
 //#define USE_STM3210C_EVAL
 #define USE_STM32L152_EVAL
#endif


/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL
  #define USB_DISCONNECT                      GPIOD  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_9
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD
  #define ADC1_DR_Address                     ((uint32_t)0x4001244C)

#elif defined (USE_STM3210E_EVAL)
  #define USB_DISCONNECT                      GPIOB  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_14
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB
  #define ADC1_DR_Address                     ((uint32_t)0x4001244C)

#elif defined (USE_STM3210C_EVAL)
  #define USB_DISCONNECT                      0  
  #define USB_DISCONNECT_PIN                  0
  #define RCC_APB2Periph_GPIO_DISCONNECT      0
  #define ADC1_DR_Address                     ((uint32_t)0x4001244C)

#elif defined (USE_STM32L152_EVAL)
 /* 
   For STM32L15xx devices it is possible to use the internal USB pullup
   controlled by register SYSCFG_PMC (refer to RM0038 reference manual for
   more details).
   It is also possible to use external pullup (and disable the internal pullup)
   by setting the define USB_USE_EXTERNAL_PULLUP in file platform_config.h
   and configuring the right pin to be used for the external pull up configuration.
   To have more details on how to use an external pull up, please refer to 
   STM3210E-EVAL evaluation board manuals.
   */
 /* Uncomment the following define to use an external pull up instead of the 
    integrated STM32L15xx internal pull up. In this case make sure to set up
    correctly the external required hardware and the GPIO defines below.*/
/* #define USB_USE_EXTERNAL_PULLUP */

 #if !defined(USB_USE_EXTERNAL_PULLUP)
  #define STM32L15_USB_CONNECT                SYSCFG_USBPuCmd(ENABLE)
  #define STM32L15_USB_DISCONNECT             SYSCFG_USBPuCmd(DISABLE)
  #define RCC_AHBPeriph_GPIO_DISCONNECT       0

 #elif defined(USB_USE_EXTERNAL_PULLUP)
  /* PA0 is chosen just as illustrating example, you should modify the defines
    below according to your hardware configuration. */ 
  #define USB_DISCONNECT                      GPIOA
  #define USB_DISCONNECT_PIN                  GPIO_Pin_0
  #define RCC_AHBPeriph_GPIO_DISCONNECT       RCC_AHBPeriph_GPIOA
  #define STM32L15_USB_CONNECT                GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
  #define STM32L15_USB_DISCONNECT             GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
 #endif /* USB_USE_EXTERNAL_PULLUP */

  #define ADC1_DR_Address                     ((uint32_t)0x40012458)

#endif /* USE_STM3210B_EVAL */

#ifdef USE_STM32L152_EVAL
  #define RCC_AHBPeriph_GPIO_IOAIN            RCC_AHBPeriph_GPIOB
  #define GPIO_IOAIN                          GPIOB
  #define GPIO_IOAIN_PIN                      GPIO_Pin_12   /* PB.12 */
  #define ADC_AIN_CHANNEL                     ADC_Channel_18

#else
  #define RCC_APB2Periph_GPIO_IOAIN           RCC_APB2Periph_GPIOC
  #define GPIO_IOAIN                          GPIOC
  #define GPIO_IOAIN_PIN                      GPIO_Pin_4   /* PC.04 */
  #define ADC_AIN_CHANNEL                     ADC_Channel_14

#endif /* USE_STM32L152_EVAL */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

