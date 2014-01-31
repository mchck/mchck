#include <mchck.h>

#include "ftm-input.desc.h"

static struct cdc_ctx cdc;
static struct timeout_ctx t;
static uint8_t ch[4] = { 0, 1, 6, 7 };             // Channels CH0, CH1, CH6, CH7
static uint8_t pin[4] = { 1, 2, 6, 7 };            // For printf
static char pin_letter[4] = { 'C','C', 'D', 'D' }; // For printf
static uint32_t prev_counter[8];
static uint32_t width[8];

/*
 * Set up FTM0 registers, interrupts and pins for input capture mode
 */
void
ftm_input_init(void)
{
        /* Connect channels with PINs:
         * FTM0_CH6 (ALT4)   PTD6
         * FTM0_CH7 (ALT4)   PTD7
         * FTM0_CH0 (ALT4)   PTC1
         * FTM0_CH1 (ALT4)   PTC2
         */
        pin_mode(PIN_PTD6, PIN_MODE_MUX_ALT4); // FTM0_CH6
        pin_mode(PIN_PTD7, PIN_MODE_MUX_ALT4); // FTM0_CH7
        pin_mode(PIN_PTC1, PIN_MODE_MUX_ALT4); // FTM0_CH0
        pin_mode(PIN_PTC2, PIN_MODE_MUX_ALT4); // FTM0_CH1

        /*
         * Required for PWM input
         */
        for (int i = 0; i < sizeof(ch); i++) {
                FTM0.channel[ch[i]].csc.msa = 0;            // Required by datasheet input capture
                FTM0.channel[ch[i]].csc.msb = 0;            // Required by datasheet input capture
                FTM0.channel[ch[i]].csc.elsa = 1;           // Initialize to detect rising
                FTM0.channel[ch[i]].csc.elsb = 0;           // Initialize to ignore falling
                FTM0.channel[ch[i]].csc.chie = 1;           // Enable interrupt on edge for each channel used
                if (ch[i]%2 == 0) {
                        FTM0.combine[ch[i]/2].combine = 0;  // Required by datasheet input capture
                        FTM0.combine[ch[i]/2].decapen = 0;  // Required by datasheet input capture
                }
        }
        FTM0.sc.cpwms = 0;                                  // Required by datasheet input capture
        FTM0.cntin    = 0x0000;                             // Required by datasheet input capture
        FTM0.sc.clks = FTM_CLKS_SYSTEM;                     // Select clock for counter
        FTM0.sc.ps   = FTM_PS_DIV2;                         // Select prescale
        int_enable(IRQ_FTM0);                               // enable FTM0 interrupt
}

/* 
 * This interrupt is generated when edge is detected 
 */
void
FTM0_Handler(void) {
        for (int i = 0; i < sizeof(ch); i++) {
                if (FTM0.channel[ch[i]].csc.chf == 1) {                                                                 // This channel triggered interrupt
                        if (FTM0.channel[ch[i]].csc.elsa == 1) {                                                        // If edge is rising
                                prev_counter[ch[i]] = FTM0.channel[ch[i]].cv;                                           // Save previous counter
                                FTM0.channel[ch[i]].csc.elsb = 1;
                                FTM0.channel[ch[i]].csc.elsa = 0;
                        } else {                                                                                        // Elseif edge is falling
                                if (prev_counter[ch[i]] != 0) {                                                         // Check if previous counter exist
                                        if (prev_counter[ch[i]] > FTM0.channel[ch[i]].cv)
                                                width[ch[i]] = (FTM0.channel[ch[i]].cv + 0xFFFF) - prev_counter[ch[i]]; // Counter had overflow
                                        else
                                                width[ch[i]] = FTM0.channel[ch[i]].cv - prev_counter[ch[i]];
                                }
                                FTM0.channel[ch[i]].csc.elsb = 0;
                                FTM0.channel[ch[i]].csc.elsa = 1;
                        }
                        FTM0.channel[ch[i]].csc.chf = 0;                                                                // Set interrupt flag to zero
                }
        }
}

/*
 * Report over UART
 */
static void 
send_cv(void *data)
{
        for (int i = 0; i < sizeof(ch); i++)
                printf("PT%c%d: %lu\r\n", pin_letter[i], pin[i], width[ch[i]]);
        timeout_add(&t, 20, send_cv, NULL);
}

/*
 * Initialize USB UART
 */
void
init_vcdc(int config)
{
        cdc_init(NULL, NULL, &cdc);
        cdc_set_stdout(&cdc);
        timeout_add(&t, 50, send_cv, NULL);
}

/*
 * Reading PWM period between rising and falling edge
 */
int
main(void)
{
        SIM.scgc6.ftm0 = 1;     // turn on clock to FTM0
        ftm_input_init();       // initialize ftm for input
        timeout_init();         // initialize timer
        usb_init(&cdc_device);  // initialize usb
        sys_yield_for_frogs();  // enter the main loop
}
