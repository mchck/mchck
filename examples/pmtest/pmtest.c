#include <mchck.h>

volatile bool cont = false;

static void
move_to_blpi()
{
        // FEI to FBI (ref manual pg. 454)
        MCG.c1.irclken = 0;
        MCG.sc.fcrdiv = 0x3; // divide IRC by 8
        MCG.c1.irclken = 1;
        MCG.c2.ircs = MCG_IRCS_FAST;
        while (MCG.s.ircst != MCG_IRCST_FAST);
        MCG.c1.clks = MCG_CLKS_INTERNAL;
        while (MCG.s.clkst != MCG_CLKST_INTERNAL);

        // FBI to BLPI (ref manual pg. 461)
        MCG.c6.plls = 0;
        MCG.c2.lp = 1;
}

static void
enter_vlpr()
{
        // Enable very low power modes
        SIM.clkdiv1.raw = ((struct SIM_CLKDIV1_t) { .outdiv1 = 1, .outdiv2 = 1, .outdiv4 = 3 }).raw;

        SMC.pmctrl.raw = ((struct SMC_PMCTRL) { .runm = RUNM_VLPR, .stopm = STOPM_VLPS }).raw;

        if (0) {
                onboard_led(1);
                while (PMC.regsc.regons != 0);
                while (SMC.pmstat.pmstat != PMSTAT_VLPR);
                onboard_led(0);
        }
}

unsigned int blink_count = 1000000;
void
blink_once()
{
        onboard_led(1);
        for (volatile int i=0; i<blink_count; i++);
        onboard_led(0);
        for (volatile int i=0; i<blink_count; i++);
}

void
blink(unsigned int n)
{
        for (unsigned int i=0; i < n; i++)
                blink_once();
        pin_mode(PIN_PTB16, PIN_MODE_MUX_ANALOG); /* detach led */
        SIM.scgc5.portb = 0;                      /* disable port clk */
}

void
sleep(void)
{
        __asm__("wfi");
        /* if (SMC.pmctrl.stopa) { */
        /*         gpio_write(GPIO_PTA18, 1); */
        /* } */
}

void
sleep_until_cont(void)
{
        cont = false;
        while (!cont) sleep();
}

volatile unsigned int acc = 0;

void long_task(void)
{
        bool vfpr = true;
        unsigned int count = vfpr ? 50000 : 5000000;
        for (unsigned int i=0; i<count; i++) {
                //if (i % 100000 == 0) onboard_led(-1);
                acc += i;
        }
}

void
spin(void)
{
        cont = false;
        while (!cont);
}

void
set_sleepdeep(bool sleepdeep)
{
        SCB.scr.sleepdeep = sleepdeep;
        volatile bool unused = SCB.scr.sleepdeep;
}

uint32_t *const state = (uint32_t*) 0x4003e000;

int
main(void)
{
        // enable low-power modes
        SMC.pmprot.raw = ((struct SMC_PMPROT) { .avlls = 1, .alls = 1, .avlp = 1 }).raw;
        // ensure we stay in VLPR
        SMC.pmctrl.lpwui = 0;
        // but we want to be able to run after be awoken
        SCB.scr.sleeponexit = 0;

        /* float JTAG/SWD pins */
        gpio_dir(PIN_PTA0, GPIO_DISABLE);
        gpio_dir(PIN_PTA1, GPIO_DISABLE);
        gpio_dir(PIN_PTA2, GPIO_DISABLE);
        gpio_dir(PIN_PTA3, GPIO_DISABLE);

        // setup button (PTA4)
        pin_mode(PIN_PTA4, PIN_MODE_MUX_GPIO | PIN_MODE_PULLUP);
        gpio_dir(GPIO_PTA4, GPIO_INPUT);
        LLWU.wupe[0].wupe3 = LLWU_PE_FALLING;
        int_enable(IRQ_LLWU);

        pin_change_init();

        // Acknowledge isolation
        PMC.regsc.ackiso = 1;

        /**
         * All measurements done on external ~3.3V power, nothing
         * connected except for button on A4; USB disconnected.
         *
         * Measurements:
         *
         * a. with stock setup: JTAG/SWD pins unmodified (i.e. default
         *    pull up/down), LED pulled low, PORTC active & SPI lines
         *    pulled high, A18 pulled low.  This is represents a
         *    typical setup with some IO lines active.
         *
         * b. with USB Vreg disconnected from 3.3V (cut SJ6), same
         *    peripheral setup as above.
         *
         * c. with USB Vreg disconnected from 3.3V (cut SJ6), no
         *    peripherals/pins/ports active, except for A4 pulled
         *    high.  This represents the practical low limit.
         *
         * All measurements were only taken ONCE on ONE SAMPLE.
         */

        switch (*state) {
        default:
        case 0:
                /**
                 * 1. RUN (busy)
                 *
                 * a. 13.61mA
                 * b. 13.08mA
                 * c. 12.77mA
                 */
                *state = 1;
                blink(1);
                spin();

                /**
                 * 2. RUN (busy, USB regulator disabled from now on)
                 *
                 * a. 13.59mA
                 * b. 13.05mA
                 * c. 12.81mA
                 */
                blink(2);
                SIM.sopt1.usbregen = 0;
                spin();

                /**
                 * 3. WAIT
                 *
                 * a. 8.81mA
                 * b. 8.26mA
                 * c. 8.13mA
                 */
                blink(3);
                set_sleepdeep(0);
                sleep_until_cont();

                /**
                 * 4. STOP
                 *
                 * a. 814uA
                 * b. 335uA
                 * c. 335uA
                 */
                blink(4);
                set_sleepdeep(1);
                sleep_until_cont();

                /**
                 * 5. BLPI RUN (busy)
                 *
                 * a. 1950uA
                 * b. 1540uA
                 * c. 1540uA
                 */
                blink(5);
                move_to_blpi();
                blink_count /= 100;
                spin();

                /**
                 * 6. BLPI STOP
                 *
                 * a. 814uA
                 * b. 335uA
                 * c. 335uA
                 */
                blink(6);
                set_sleepdeep(1);
                sleep_until_cont();

                /**
                 * 7. VLP RUN (busy)
                 *
                 * a. 598uA
                 * b. 95.7uA
                 * c. 94.0uA
                 */
                blink(7);
                enter_vlpr();
                spin();

                /**
                 * 8. VLP WAIT
                 *
                 * a. 574uA
                 * b. 69.8uA
                 * c. 68.2uA
                 */
                blink(8);
                set_sleepdeep(0);
                sleep_until_cont();

                /**
                 * 9. VLP STOP
                 *
                 * a. 515uA
                 * b. 4.7uA
                 * c. 3.3uA
                 */
                blink(9);
                set_sleepdeep(1);
                sleep_until_cont();

                /**
                 * 10. LLS
                 *
                 * a. 514.2uA
                 * b. 3.9uA
                 * c. 2.5uA
                 */
                blink(10);
                SMC.pmctrl.stopm = STOPM_LLS;
                set_sleepdeep(1);
                sleep_until_cont();

        case 1:
                /**
                 * 11. VLLS3
                 *
                 * a. 513.7uA
                 * b. 3.2uA
                 * c. 1.8uA
                 */
                *state = 2;
                blink(11);
                SMC.pmctrl.stopm = STOPM_VLLS;
                SMC.vllsctrl.vllsm = 3;
                set_sleepdeep(1);
                sleep_until_cont();

        case 2:
                /**
                 * 12. VLLS2
                 *
                 * a. 513.7uA
                 * b. 3.1uA
                 * c. 1.7uA
                 */
                *state = 3;
                blink(12);
                SMC.pmctrl.stopm = STOPM_VLLS;
                SMC.vllsctrl.vllsm = 2;
                set_sleepdeep(1);
                sleep_until_cont();

        case 3:
                /**
                 * 13. VLLS1
                 *
                 * a. 513uA
                 * b. 2.5uA
                 * c. 0.9uA
                 */
                *state = 4;
                blink(13);
                SMC.pmctrl.stopm = STOPM_VLLS;
                SMC.vllsctrl.vllsm = 1;
                set_sleepdeep(1);
                sleep_until_cont();

        case 4:
                /**
                 * 14. VLLS0 with PoR circuit enabled
                 *
                 * a. 513uA
                 * b. 2.5uA
                 * c. 0.6uA
                 */
                *state = 5;
                blink(14);
                SMC.pmctrl.stopm = STOPM_VLLS;
                SMC.vllsctrl.vllsm = 0;
                SMC.vllsctrl.porpo = 0; /* default */
                set_sleepdeep(1);
                sleep_until_cont();

        case 5:
                /**
                 * 13. VLLS0 with PrR circuit disabled
                 *
                 * a. -
                 * b. -
                 * c. 0.4uA
                 */
                *state = 0;
                blink(15);
                SMC.pmctrl.stopm = STOPM_VLLS;
                SMC.vllsctrl.vllsm = 0;
                SMC.vllsctrl.porpo = 1;
                set_sleepdeep(1);
                sleep_until_cont();
        }

        // reset state
        *state = 0;

        // For identifying clock rate
        if (0) {
                ftm_init();
                int_enable(IRQ_FTM0);
                FTM0.sc.toie = 1;
        }

        // done
        onboard_led(1);
        while (true)
                for (;;) sleep();
}

void
button_handler(void *cbdata)
{
        cont = true;
        //onboard_led(-1);
}
PIN_DEFINE_CALLBACK(PIN_PTA4, PIN_CHANGE_FALLING, button_handler, NULL);

void
LLWU_Handler(void)
{
        LLWU.wuf1 = 0xff;
        LLWU.wuf2 = 0xff;
        LLWU.mwuf = 0xff;
        cont = true;
}

volatile unsigned int n = 0;
void
FTM0_Handler(void)
{
        FTM0.sc.tof = 0;
        if (n % 10 == 0)
                onboard_led(-1);
        n++;
}
