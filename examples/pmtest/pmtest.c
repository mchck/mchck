#include <mchck.h>

static struct timeout_ctx t;

volatile unsigned int llwu_count = 0;
volatile unsigned int wakeup_count = 0;
volatile bool state = false;
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
        move_to_blpi();
        SIM.clkdiv1.raw = ((struct SIM_CLKDIV1_t) { .outdiv1 = 1, .outdiv2 = 1, .outdiv4 = 3 }).raw;
        SMC.pmprot.raw = ((struct SMC_PMPROT) { .avlls = 1, .alls = 1, .avlp = 1 }).raw;
        SMC.pmctrl.raw = ((struct SMC_PMCTRL) { .runm = RUNM_VLPR, .stopm = STOPM_VLPS }).raw;
        //SMC.pmctrl.raw = ((struct SMC_PMCTRL) { .runm = RUNM_RUN, .stopm = STOPM_VLPS }).raw;
        SMC.pmctrl.stopm = STOPM_LLS;
        onboard_led(1);
        while (PMC.regsc.regons != 0);
        while (SMC.pmstat.pmstat != PMSTAT_VLPR);
        onboard_led(0);
}

struct spiflash_transaction trans;
void powerdown_cb(void *cbdata) { }

void
sleep(void)
{
        __asm__("wfi");
        if (SMC.pmctrl.stopa) {
                gpio_write(GPIO_PTA2, 1);
        }
}

void
sleep_forever(void)
{
        for (;;) sleep();
}

void
sleep_until_cont(void)
{
        cont = false;
        while (!cont) sleep();
}

int
main(void)
{
        SIM.sopt1.usbregen = 0;
        SCB.scr.sleepdeep = 1;
        SMC.pmctrl.lpwui = 0;

        // pull up unused SPI FLASH pins
        pin_mode(PIN_PTC0, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC5, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC6, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
        pin_mode(PIN_PTC7, PIN_MODE_PULLUP | PIN_MODE_MUX_GPIO);
                
        enter_vlpr();

        if (0) {
                ftm_init();
                int_enable(IRQ_FTM0);
                FTM0.sc.toie = 1;
        }

        // LED (PTA18)
        pin_mode(PIN_PTA18, PIN_MODE_MUX_GPIO);
        gpio_dir(GPIO_PTA18, GPIO_OUTPUT);

        // STOPA output (PTA2)
        pin_mode(PIN_PTA2, PIN_MODE_MUX_GPIO);
        gpio_dir(GPIO_PTA2, GPIO_OUTPUT);

        // button (PTA4)
        pin_mode(PIN_PTA4, PIN_MODE_MUX_GPIO | PIN_MODE_PULLUP);
        gpio_dir(GPIO_PTA4, GPIO_INPUT);
        PORTA.pcr[4].irqc = PCR_IRQC_INT_FALLING;
        int_enable(IRQ_PORTA);
        int_enable(IRQ_LLWU);
        LLWU.wupe[0].wupe3 = LLWU_PE_FALLING;

        sleep_forever();
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
PORTA_Handler(void)
{
        PORTA.isfr = 0xffffffff;
        cont = true;
        state = !state;
        gpio_write(GPIO_PTA18, state);
        //onboard_led(state);
        long_task();
}

void
LLWU_Handler(void)
{
        llwu_count++;
        LLWU.wuf1 = 0xff;
        LLWU.wuf2 = 0xff;
        LLWU.mwuf = 0xff;
        onboard_led(-1);
        PORTA_Handler();
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
