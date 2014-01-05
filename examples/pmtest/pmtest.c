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
        SMC.pmprot.raw = ((struct SMC_PMPROT) { .avlls = 1, .alls = 1, .avlp = 1 }).raw;

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
}

void
sleep(void)
{
        __asm__("wfi");
        if (SMC.pmctrl.stopa) {
                gpio_write(GPIO_PTA18, 1);
        }
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
        volatile bool sync = SCB.scr.sleepdeep;
}

int
main(void)
{
        // ensure we stay in VLPR
        SMC.pmctrl.lpwui = 0;
        // but we want to be able to run after be awoken
        SCB.scr.sleeponexit = 0;

        // pull up unused SPI FLASH pins
        pin_mode(PIN_PTC0, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC5, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC6, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC7, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);

        // setup button (PTA4)
        pin_mode(PIN_PTA4, PIN_MODE_MUX_GPIO | PIN_MODE_PULLUP);
        gpio_dir(GPIO_PTA4, GPIO_INPUT);
        PORTA.pcr[4].irqc = PCR_IRQC_INT_FALLING;
        int_enable(IRQ_PORTA);
        LLWU.wupe[0].wupe3 = LLWU_PE_FALLING;
        int_enable(IRQ_LLWU);

        // Extra LED (PTA18)
        pin_mode(PIN_PTA18, PIN_MODE_MUX_GPIO);
        gpio_dir(GPIO_PTA18, GPIO_OUTPUT);

        // stage 1: baseline RUN
        blink(1);
        spin();

        // stage 2: RUN (USB regulator disabled)
        blink(2);
        SIM.sopt1.usbregen = 0;
        spin();

        // stage 3: wait
        blink(3);
        set_sleepdeep(0);
        sleep_until_cont();

        // stage 4: stop
        blink(4);
        set_sleepdeep(1);
        sleep_until_cont();

        // stage 5: BLPI RUN
        blink(5);
        move_to_blpi();
        blink_count /= 100;
        spin();

        // stage 6: BLPI STOP
        blink(6);
        set_sleepdeep(1);
        sleep_until_cont();

        // stage 7: VLP RUN
        blink(7);
        enter_vlpr();
        spin();

        // stage 8: VLP WAIT
        blink(8);
        set_sleepdeep(0);
        sleep_until_cont();

        // stage 9: VLP STOP
        blink(9);
        set_sleepdeep(1);
        sleep_until_cont();

        // stage 10: LLS
        blink(10);
        SMC.pmctrl.stopm = STOPM_LLS;
        set_sleepdeep(1);
        sleep_until_cont();
        
        // stage 11: VLLS3
        /*
        blink(11);
        SMC.pmctrl.stopm = STOPM_VLLS;
        SMC.vllsctrl.vllsm = 3;
        set_sleepdeep(1);
        sleep_until_cont();
        */

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
PORTA_Handler(void)
{
        PORTA.isfr = 0xffffffff;
        cont = true;
        //onboard_led(-1);
}

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
