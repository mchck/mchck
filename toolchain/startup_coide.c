//*****************************************************************************
//
//! \file startup_coide.c
//! \brief NUC1xx Devices Startup code for CooCox CoIDE.
//!        This module performs:
//!           - Set the initial SP
//!           - Set the vector table entries with the exceptions ISR address
//!           - Initialize data and bss
//!           - Setup the microcontroller system. 			
//!           - Call the application's entry point.
//!           .
//! \version 1.0
//! \date 8/13/2011
//! \author CooCox
//! \copy
//!
//! Copyright (c)  2011, CooCox 
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//***************************************************************************** 
//
// Stack Configuration
//
//*****************************************************************************
//
// Stack size (in Words)
//
#define STACK_SIZE       0x00000100      
__attribute__ ((section(".co_stack")))
unsigned long pulStack[STACK_SIZE];      
 
#define WEAK __attribute__ ((weak))           

//*****************************************************************************
//
// Declaration of the default fault handlers
//
//*****************************************************************************
void WEAK  ResetHandler(void);
void WEAK  NMIIntHandler(void);
void WEAK  HardFaultIntHandler(void);
void WEAK  MemManageIntHandler(void);
void WEAK  BusFaultIntHandler(void);
void WEAK  UsageFaultIntHandler(void);
void WEAK  SVCIntHandler(void);
void WEAK  DebugMonIntHandler(void);
void WEAK  PendSVIntHandler(void);
void WEAK  SysTickIntHandler(void);
void WEAK  BODIntHandler(void);  
void WEAK  WDTIntHandler(void);  
void WEAK  EINT0IntHandler(void);
void WEAK  EINT1IntHandler(void);
void WEAK  GPABIntHandler(void); 
void WEAK  GPCDEIntHandler(void);
void WEAK  PWMAIntHandler(void); 
void WEAK  PWMBIntHandler(void); 
void WEAK  TIMER0IntHandler(void); 
void WEAK  TIMER1IntHandler(void); 
void WEAK  TIMER2IntHandler(void); 
void WEAK  TIMER3IntHandler(void); 
void WEAK  UART02IntHandler(void);
void WEAK  UART1IntHandler(void);
void WEAK  SPI0IntHandler(void); 
void WEAK  SPI1IntHandler(void); 
void WEAK  SPI2IntHandler(void); 
void WEAK  SPI3IntHandler(void); 
void WEAK  I2C0IntHandler(void); 
void WEAK  I2C1IntHandler(void); 
void WEAK  CAN0IntHandler(void); 
void WEAK  USBDIntHandler(void);  
void WEAK  PS2IntHandler(void);  
void WEAK  ACMPIntHandler(void); 
void WEAK  PDMAIntHandler(void);
void WEAK  I2SIntHandler(void);
void WEAK  PWRWUIntHandler(void);
void WEAK  ADCIntHandler(void);
void WEAK  RTCIntHandler(void);

//*****************************************************************************
//
// Symbols defined in linker script
//
//*****************************************************************************
//
// Start address for the initialization values of the .data section.
//  
extern unsigned long _sidata;    

//
// Start address for the .data section 
//
extern unsigned long _sdata;     

//
// End address for the .data section
//
extern unsigned long _edata;     

//
// Start address for the .bss section
//
extern unsigned long _sbss;      

//
// End address for the .bss section
//
extern unsigned long _ebss;      

//
// End address for ram 
//
extern void _eram;               

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
extern int main(void);            
void ResetHandler(void);   
static void DefaultIntHandler(void);  

//
// The minimal vector table for a Cortex M3.  Note that the proper constructs
// must be placed on this to ensure that it ends up at physical address
// 0x00000000.
//  
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void *)&pulStack[STACK_SIZE],          // The initial stack pointer     
    ResetHandler,                           // The reset handler                        
    NMIIntHandler,                          // The NMI handler                        
    HardFaultIntHandler,                    // The hard fault handler                  
    MemManageIntHandler,                    // The MPU fault handler                  
    BusFaultIntHandler,                     // The bus fault handler                   
    UsageFaultIntHandler,                   // The usage fault handler                  
    0,0,0,0,                                // Reserved                            
    SVCIntHandler,                          // SVCall handler                         
    DebugMonIntHandler,                     // Debug monitor handler                    
    0,                                      // Reserved                                  
    PendSVIntHandler,                       // The PendSV handler                      
    SysTickIntHandler,                      // The SysTick handler                    
    BODIntHandler,                          // Brownout low voltage detected
    WDTIntHandler,                          // Watch Dog Timer  
    EINT0IntHandler,                        // External signal interrupt from
                                            // PB.14 pin 
    EINT1IntHandler,                        // External signal interrupt from
                                            // PB.15 pin 
    GPABIntHandler,                         // External signal interrupt from 
                                            // PA[15:0] / PB[13:0]
    GPCDEIntHandler,                        // External interrupt from 
                                            // PC[15:0]/PD[15:0]/PE[15:0]
    PWMAIntHandler,                         // PWM0 or PWM2 
    PWMBIntHandler,                         // PWM1 or PWM3   
    TIMER0IntHandler,                       // Timer 0 
    TIMER1IntHandler,                       // Timer 1
    TIMER2IntHandler,                       // Timer 2
    TIMER3IntHandler,                       // Timer 3
    UART02IntHandler,                       // UART0
    UART1IntHandler,                        // UART1
    SPI0IntHandler,                         // SPI0 
    SPI1IntHandler,                         // SPI1 
    SPI2IntHandler,                         // SPI2
    SPI3IntHandler,                         // SPI3 
    I2C0IntHandler,                         // I2C0
    I2C1IntHandler,                         // I2C1
    CAN0IntHandler,                         // Reserved 
    DefaultIntHandler,                      // Reserved
    DefaultIntHandler,                      // Reserved 
    USBDIntHandler,                         // USB Device  
    PS2IntHandler,                          // PS2  
    ACMPIntHandler,                         // Analog Comparator 
    PDMAIntHandler,                         // PDMA
    I2SIntHandler,                          // I2S 
    PWRWUIntHandler,                        // Clock controller
    ADCIntHandler,                          // ADC
    DefaultIntHandler,                      // Reserved  
    RTCIntHandler,                          // RTC
};

//*****************************************************************************
//
//! \brief This is the code that gets called when the processor first
//! starts execution following a reset event. 
//!
//! \param None.
//!
//! Only the absolutely necessary set is performed, after which the 
//! application supplied main() routine is called. 
//!
//! \return None.
//
//*****************************************************************************
void Default_ResetHandler(void)
{
    //
    // Initialize data and bss
    //
    unsigned long *pulSrc, *pulDest;

    //
    // Copy the data segment initializers from flash to SRAM
    //
    pulSrc = &_sidata;

    for(pulDest = &_sdata; pulDest < &_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }

    //
    // Zero fill the bss segment.
    //
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }

    //
    // Call the application's entry point.
    //
    main();
}

//*****************************************************************************
//
// Provide weak aliases for each Exception handler to the DefaultIntHandler. 
// As they are weak aliases, any function with the same name will override 
// this definition.
//
//*****************************************************************************  
#pragma weak ResetHandler = Default_ResetHandler
#pragma weak NMIIntHandler = DefaultIntHandler
#pragma weak HardFaultIntHandler = DefaultIntHandler
#pragma weak MemManageIntHandler = DefaultIntHandler
#pragma weak BusFaultIntHandler = DefaultIntHandler
#pragma weak UsageFaultIntHandler = DefaultIntHandler
#pragma weak SVCIntHandler = DefaultIntHandler
#pragma weak DebugMonIntHandler = DefaultIntHandler
#pragma weak PendSVIntHandler = DefaultIntHandler
#pragma weak SysTickIntHandler = DefaultIntHandler
#pragma weak BODIntHandler = DefaultIntHandler  
#pragma weak WDTIntHandler = DefaultIntHandler  
#pragma weak EINT0IntHandler = DefaultIntHandler
#pragma weak EINT1IntHandler = DefaultIntHandler
#pragma weak GPABIntHandler = DefaultIntHandler 
#pragma weak GPCDEIntHandler = DefaultIntHandler
#pragma weak PWMAIntHandler = DefaultIntHandler 
#pragma weak PWMBIntHandler = DefaultIntHandler 
#pragma weak TIMER0IntHandler = DefaultIntHandler 
#pragma weak TIMER1IntHandler = DefaultIntHandler 
#pragma weak TIMER2IntHandler = DefaultIntHandler 
#pragma weak TIMER3IntHandler = DefaultIntHandler 
#pragma weak UART02IntHandler = DefaultIntHandler
#pragma weak UART1IntHandler = DefaultIntHandler
#pragma weak SPI0IntHandler = DefaultIntHandler 
#pragma weak SPI1IntHandler = DefaultIntHandler 
#pragma weak SPI2IntHandler = DefaultIntHandler 
#pragma weak SPI3IntHandler = DefaultIntHandler 
#pragma weak I2C0IntHandler = DefaultIntHandler 
#pragma weak I2C1IntHandler = DefaultIntHandler 
#pragma weak CAN0IntHandler = DefaultIntHandler 
#pragma weak USBDIntHandler = DefaultIntHandler  
#pragma weak PS2IntHandler = DefaultIntHandler  
#pragma weak ACMPIntHandler = DefaultIntHandler 
#pragma weak PDMAIntHandler = DefaultIntHandler
#pragma weak I2SIntHandler = DefaultIntHandler
#pragma weak PWRWUIntHandler = DefaultIntHandler
#pragma weak ADCIntHandler = DefaultIntHandler
#pragma weak RTCIntHandler = DefaultIntHandler  

//*****************************************************************************
//
//! \brief This is the code that gets called when the processor receives an 
//! unexpected interrupt.  
//!
//! \param None.
//!
//! This simply enters an infinite loop, preserving the system state for 
//! examination by a debugger.
//!
//! \return None.  
//*****************************************************************************  
static void DefaultIntHandler(void) 
{
    //
    // Go into an infinite loop.
    //
    while (1) 
    {
    }
}


