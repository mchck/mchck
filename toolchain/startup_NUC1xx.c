/**
 ******************************************************************************
 * @file      startup_nuc1xx.c
 * @author    Coocox
 * @version   V1.0
 * @date      20/07/2010
 * @brief     NUC1xx Devices Startup code.
 *            This module performs:
 *                - Set the initial SP
 *                - Set the vector table entries with the exceptions ISR address
 *                - Initialize data and bss
 *                - Setup the microcontroller system. 			
 *                - Call the application's entry point.
 *            After Reset the Cortex-M3 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 *******************************************************************************
 */
 
 
/*----------Stack Configuration-----------------------------------------------*/  
#define STACK_SIZE       0x00000100      /*!< Stack size (in Words)           */
__attribute__ ((section(".co_stack")))
unsigned long pulStack[STACK_SIZE];      


/*----------Macro definition--------------------------------------------------*/  
#define WEAK __attribute__ ((weak))           


/*----------Declaration of the default fault handlers-------------------------*/  
/* System exception vector handler */
void WEAK  Reset_Handler(void);
void WEAK  NMI_Handler(void);
void WEAK  HardFault_Handler(void);
void WEAK  MemManage_Handler(void);
void WEAK  BusFault_Handler(void);
void WEAK  UsageFault_Handler(void);
void WEAK  SVC_Handler(void);
void WEAK  DebugMon_Handler(void);
void WEAK  PendSV_Handler(void);
void WEAK  SysTick_Handler(void);
void WEAK  BOD_IRQHandler(void);  
void WEAK  WDT_IRQHandler(void);  
void WEAK  EINT0_IRQHandler(void);
void WEAK  EINT1_IRQHandler(void);
void WEAK  GPAB_IRQHandler(void); 
void WEAK  GPCDE_IRQHandler(void);
void WEAK  PWMA_IRQHandler(void); 
void WEAK  PWMB_IRQHandler(void); 
void WEAK  TMR0_IRQHandler(void); 
void WEAK  TMR1_IRQHandler(void); 
void WEAK  TMR2_IRQHandler(void); 
void WEAK  TMR3_IRQHandler(void); 
void WEAK  UART02_IRQHandler(void);
void WEAK  UART1_IRQHandler(void);
void WEAK  SPI0_IRQHandler(void); 
void WEAK  SPI1_IRQHandler(void); 
void WEAK  SPI2_IRQHandler(void); 
void WEAK  SPI3_IRQHandler(void); 
void WEAK  I2C0_IRQHandler(void); 
void WEAK  I2C1_IRQHandler(void); 
void WEAK  CAN0_IRQHandler(void); 
void WEAK  USBD_IRQHandler(void);  
void WEAK  PS2_IRQHandler(void);  
void WEAK  ACMP_IRQHandler(void); 
void WEAK  PDMA_IRQHandler(void);
void WEAK  I2S_IRQHandler(void);
void WEAK  PWRWU_IRQHandler(void);
void WEAK  ADC_IRQHandler(void);
void WEAK  RTC_IRQHandler(void);


/*----------Symbols defined in linker script----------------------------------*/  
extern unsigned long _sidata;    /*!< Start address for the initialization 
                                      values of the .data section.            */
extern unsigned long _sdata;     /*!< Start address for the .data section     */    
extern unsigned long _edata;     /*!< End address for the .data section       */    
extern unsigned long _sbss;      /*!< Start address for the .bss section      */
extern unsigned long _ebss;      /*!< End address for the .bss section        */      
extern void _eram;               /*!< End address for ram                     */


/*----------Function prototypes-----------------------------------------------*/  
extern int main(void);           /*!< The entry point for the application.    */
extern void SystemInit(void);    /*!< Setup the microcontroller system(CMSIS) */
void Default_Reset_Handler(void);   /*!< Default reset handler                */
static void Default_Handler(void);  /*!< Default exception handler            */


/**
  *@brief The minimal vector table for a Cortex M3.  Note that the proper constructs
  *       must be placed on this to ensure that it ends up at physical address
  *       0x00000000.  
  */
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
  /*----------Core Exceptions------------------------------------------------ */
  (void *)&pulStack[STACK_SIZE],       /*!< The initial stack pointer         */
  Reset_Handler,                       /*!< The reset handler                 */
  NMI_Handler,                         /*!< The NMI handler                   */ 
  HardFault_Handler,                   /*!< The hard fault handler            */
  MemManage_Handler,                   /*!< The MPU fault handler             */
  BusFault_Handler,                    /*!< The bus fault handler             */
  UsageFault_Handler,                  /*!< The usage fault handler           */ 
  0,0,0,0,                             /*!< Reserved                          */
  SVC_Handler,                         /*!< SVCall handler                    */
  DebugMon_Handler,                    /*!< Debug monitor handler             */
  0,                                   /*!< Reserved                          */
  PendSV_Handler,                      /*!< The PendSV handler                */
  SysTick_Handler,                     /*!< The SysTick handler               */ 
  
  /*----------External Exceptions---------------------------------------------*/
  BOD_IRQHandler,  
  WDT_IRQHandler,  
  EINT0_IRQHandler,
  EINT1_IRQHandler,
  GPAB_IRQHandler, 
  GPCDE_IRQHandler,
  PWMA_IRQHandler, 
  PWMB_IRQHandler, 
  TMR0_IRQHandler, 
  TMR1_IRQHandler, 
  TMR2_IRQHandler, 
  TMR3_IRQHandler, 
  UART02_IRQHandler,
  UART1_IRQHandler,
  SPI0_IRQHandler, 
  SPI1_IRQHandler, 
  SPI2_IRQHandler,
  SPI3_IRQHandler, 
  I2C0_IRQHandler, 
  I2C1_IRQHandler, 
  CAN0_IRQHandler, 
  Default_Handler,
  Default_Handler, 
  USBD_IRQHandler,  
  PS2_IRQHandler,  
  ACMP_IRQHandler, 
  PDMA_IRQHandler,
  I2S_IRQHandler, 
  PWRWU_IRQHandler,
  ADC_IRQHandler,
  Default_Handler,  
  RTC_IRQHandler,
};


/**
  * @brief  This is the code that gets called when the processor first
  *         starts execution following a reset event. Only the absolutely
  *         necessary set is performed, after which the application
  *         supplied main() routine is called. 
  * @param  None
  * @retval None
  */
void Default_Reset_Handler(void)
{
  /* Initialize data and bss */
  unsigned long *pulSrc, *pulDest;

  /* Copy the data segment initializers from flash to SRAM */
  pulSrc = &_sidata;

  for(pulDest = &_sdata; pulDest < &_edata; )
  {
    *(pulDest++) = *(pulSrc++);
  }
  
  /* Zero fill the bss segment. */
  for(pulDest = &_sbss; pulDest < &_ebss; )
  {
    *(pulDest++) = 0;
  }

  /* Setup the microcontroller system. */
  SystemInit();
	
  /* Call the application's entry point.*/
  main();
}


/**
  *@brief Provide weak aliases for each Exception handler to the Default_Handler. 
  *       As they are weak aliases, any function with the same name will override 
  *       this definition.
  */
  
#pragma weak Reset_Handler = Default_Reset_Handler
#pragma weak NMI_Handler = Default_Handler
#pragma weak HardFault_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler
#pragma weak BOD_IRQHandler = Default_Handler  
#pragma weak WDT_IRQHandler = Default_Handler  
#pragma weak EINT0_IRQHandler = Default_Handler
#pragma weak EINT1_IRQHandler = Default_Handler
#pragma weak GPAB_IRQHandler = Default_Handler 
#pragma weak GPCDE_IRQHandler = Default_Handler
#pragma weak PWMA_IRQHandler = Default_Handler 
#pragma weak PWMB_IRQHandler = Default_Handler 
#pragma weak TMR0_IRQHandler = Default_Handler 
#pragma weak TMR1_IRQHandler = Default_Handler 
#pragma weak TMR2_IRQHandler = Default_Handler 
#pragma weak TMR3_IRQHandler = Default_Handler 
#pragma weak UART02_IRQHandler = Default_Handler
#pragma weak UART1_IRQHandler = Default_Handler
#pragma weak SPI0_IRQHandler = Default_Handler 
#pragma weak SPI1_IRQHandler = Default_Handler 
#pragma weak SPI2_IRQHandler = Default_Handler 
#pragma weak SPI3_IRQHandler = Default_Handler 
#pragma weak I2C0_IRQHandler = Default_Handler 
#pragma weak I2C1_IRQHandler = Default_Handler 
#pragma weak CAN0_IRQHandler = Default_Handler 
#pragma weak USBD_IRQHandler = Default_Handler  
#pragma weak PS2_IRQHandler = Default_Handler  
#pragma weak ACMP_IRQHandler = Default_Handler 
#pragma weak PDMA_IRQHandler = Default_Handler
#pragma weak I2S_IRQHandler = Default_Handler
#pragma weak PWRWU_IRQHandler = Default_Handler
#pragma weak ADC_IRQHandler = Default_Handler
#pragma weak RTC_IRQHandler = Default_Handler  

/**
  * @brief  This is the code that gets called when the processor receives an 
  *         unexpected interrupt.  This simply enters an infinite loop, 
  *         preserving the system state for examination by a debugger.
  * @param  None
  * @retval None  
  */
static void Default_Handler(void) 
{
  /* Go into an infinite loop. */
  while (1) 
  {
  }
}

/*********************** (C) COPYRIGHT 2009 Coocox ************END OF FILE*****/
