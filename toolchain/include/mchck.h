#ifndef __MCHCK_H
#define __MCHCK_H

#include <sys/types.h>
#include <sys/cdefs.h>
#include <stdint.h>

#include <mchck-cdefs.h>

#ifdef TARGET_HOST

#include <host/host.h>

#else

#ifndef __MCHCK_INTERNAL_H
#error Build system error: mchck_internal.h not included by compiler
#endif

#ifdef __cplusplus
 extern "C" {
#if 0                           /* to make emacs indent properly */
 }
#endif
#endif

/* From CMSIS: */

typedef enum IRQn
{
/******  Cortex-M# Processor Exceptions Numbers ***************************************************/

  NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt                        */
  MemoryManagement_IRQn         = -12,      /*!<  4 Memory Management Interrupt                   */
  BusFault_IRQn                 = -11,      /*!<  5 Bus Fault Interrupt                           */
  UsageFault_IRQn               = -10,      /*!<  6 Usage Fault Interrupt                         */
  SVCall_IRQn                   = -5,       /*!< 11 SV Call Interrupt                             */
  DebugMonitor_IRQn             = -4,       /*!< 12 Debug Monitor Interrupt                       */
  PendSV_IRQn                   = -2,       /*!< 14 Pend SV Interrupt                             */
  SysTick_IRQn                  = -1,       /*!< 15 System Tick Interrupt                         */

/******  Device Specific Interrupt Numbers ********************************************************/
  DMA0_IRQn,
  DMA1_IRQn,
  DMA2_IRQn,
  DMA3_IRQn,
  DMA4_IRQn,
  DMA5_IRQn,
  DMA6_IRQn,
  DMA7_IRQn,
  DMA8_IRQn,
  DMA9_IRQn,
  DMA10_IRQn,
  DMA11_IRQn,
  DMA12_IRQn,
  DMA13_IRQn,
  DMA14_IRQn,
  DMA15_IRQn,
  DMA_Error_IRQn,
  MCM_IRQn,
  FTFL_IRQn,
  Read_Collision_IRQn,
  LVD_LVW_IRQn,
  LLW_IRQn,
  Watchdog_IRQn,
  Reserved39_IRQn,
  I2C0_IRQn,
  I2C1_IRQn,
  SPI0_IRQn,
  SPI1_IRQn,
  SPI2_IRQn,
  CAN0_ORed_Message_buffer_IRQn,
  CAN0_Bus_Off_IRQn,
  CAN0_Error_IRQn,
  CAN0_Tx_Warning_IRQn,
  CAN0_Rx_Warning_IRQn,
  CAN0_Wake_Up_IRQn,
  Reserved51_IRQn,
  Reserved52_IRQn,
  CAN1_ORed_Message_buffer_IRQn,
  CAN1_Bus_Off_IRQn,
  CAN1_Error_IRQn,
  CAN1_Tx_Warning_IRQn,
  CAN1_Rx_Warning_IRQn,
  CAN1_Wake_Up_IRQn,
  Reserved59_IRQn,
  Reserved60_IRQn,
  UART0_RX_TX_IRQn,
  UART0_ERR_IRQn,
  UART1_RX_TX_IRQn,
  UART1_ERR_IRQn,
  UART2_RX_TX_IRQn,
  UART2_ERR_IRQn,
  UART3_RX_TX_IRQn,
  UART3_ERR_IRQn,
  UART4_RX_TX_IRQn,
  UART4_ERR_IRQn,
  UART5_RX_TX_IRQn,
  UART5_ERR_IRQn,
  ADC0_IRQn,
  ADC1_IRQn,
  CMP0_IRQn,
  CMP1_IRQn,
  CMP2_IRQn,
  FTM0_IRQn,
  FTM1_IRQn,
  FTM2_IRQn,
  CMT_IRQn,
  RTC_IRQn,
  Reserved83_IRQn,
  PIT0_IRQn,
  PIT1_IRQn,
  PIT2_IRQn,
  PIT3_IRQn,
  PDB0_IRQn,
  USB0_IRQn,
  USBDCD_IRQn,
  Reserved91_IRQn,
  Reserved92_IRQn,
  Reserved93_IRQn,
  Reserved94_IRQn,
  I2S0_IRQn,
  SDHC_IRQn,
  DAC0_IRQn,
  DAC1_IRQn,
  TSI0_IRQn,
  MCG_IRQn,
  LPTimer_IRQn,
  Reserved102_IRQn,
  PORTA_IRQn,
  PORTB_IRQn,
  PORTC_IRQn,
  PORTD_IRQn,
  PORTE_IRQn,
  Reserved108_IRQn,
  Reserved109_IRQn,
  Reserved110_IRQn,
  Reserved111_IRQn,
  Reserved112_IRQn,
  Reserved113_IRQn,
  Reserved114_IRQn,
  Reserved115_IRQn,
  Reserved116_IRQn,
  Reserved117_IRQn,
  Reserved118_IRQn,
  Reserved119_IRQn
} IRQn_Type;

#include <MK20DZ10.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _app_rom;

#include <intnums.h>

#include <kinetis/ftfl.h>
#include <kinetis/usbotg.h>
#include <kinetis/sim.h>
#include <kinetis/mcg.h>
#include <kinetis/rcm.h>
#include <kinetis/port.h>
#include <kinetis/gpio.h>

#include <arm/scb.h>
#include <arm/nvic.h>

#include <mchck/mchck.h>

#ifdef __cplusplus
}
#endif

#endif
#endif
