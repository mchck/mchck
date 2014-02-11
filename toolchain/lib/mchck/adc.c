#include <mchck.h>

struct adc_ctx {
        volatile struct {
                adc_result_cb_t *cb;
                void *cbdata;
                int active;
        } stat_a, stat_b;
        unsigned long fract scale;
};

static struct adc_ctx adc_ctx;

static void
adc_calibrate_cb(uint16_t val, int error, void *cbdata)
{
        /* disable band-gap buffer */
        PMC.regsc.bgbe = 1;

        /* base ADC calibration */
        if (error || ADC0.sc3.calf)
                return;

        uint32_t calib = 0;
        for (int i = sizeof(ADC0.clp)/sizeof(*ADC0.clp) - 1; i >= 0; --i)
                calib += ADC0.clp[i];
        ADC0.pg = (calib >> 1) | 0x8000;

        calib = 0;
        for (int i = sizeof(ADC0.clm)/sizeof(ADC0.clm) - 1; i >= 0; --i)
                calib += ADC0.clm[i];
        ADC0.mg = (calib >> 1) | 0x8000;

        adc_calibrate_voltage(0);
}

static void
adc_calibrate_voltage_cb(uint16_t val, int error, void *cbdata)
{
        if (error)
                return;

        /**
         * The value we read is V_BG = 1V.  This allows us to
         * calibrate our voltage scale, even if we're running
         * unregulated from battery.  Do not mistake the V_BG bandgap
         * voltage with VREF's bandgap voltage, which is 1.2V.  Also,
         * V_REFH is not VREF's output voltage, but the ADC reference
         * voltage, by default tied to AVDD.
         *
         * (1) bg = V_BG / V_REFH
         * (2) x = V_x / V_REFH
         *     V_x = x * V_REFH
         *     V_x = x * V_BG / bg       (with (1))
         *     V_x = x / bg              (with V_BG = 1V)
         * (3) scale = 1 / bg
         * (4) V_x = x * scale           (with (3))
         *
         * We just store scale and use it in subsequent conversions (4).
         */

        adc_ctx.scale = 1.0lR / val;
}

void
adc_calibrate_voltage(unsigned accum reference)
{
        if (reference != 0) {
                adc_ctx.scale = (unsigned long accum)reference >> 16;
        } else {
                adc_sample_start(ADC_BANDGAP, adc_calibrate_voltage_cb, NULL);
        }
}

unsigned accum
adc_as_voltage(uint16_t val)
{
        return (val * adc_ctx.scale);
}

void
adc_init(void)
{
        /**
         * Enable bandgap buffer.  We need this later to calibrate our
         * reference scale.  However, we start it now, so that it will
         * have time to stabilize. */
        PMC.regsc.bgbe = 1;

        /* enable clock */
        SIM.scgc6.adc0 = 1;

        /* enable interrupt handler */
        int_enable(IRQ_ADC0);

        /* setup ADC calibration */
        adc_sample_prepare(ADC_MODE_SAMPLE_LONG | ADC_AVG_32);
        adc_ctx.stat_a.cb = adc_calibrate_cb;
        adc_ctx.stat_a.active = 1;

        /* enable interrupt */
        ADC0.sc1a.raw = ((struct ADC_SC1){
                        .aien = 1,
                                .diff = 0,
                                .adch = ADC_ADCH_DISABLED, /* do not start ADC */
                                }).raw;

        /* start calibration */
        ADC0.sc3.cal = 1;
}

void
adc_sample_prepare(enum adc_mode mode)
{
        ADC0.cfg1.raw = ((struct ADC_CFG1){
                        .adlpc = !!(mode & ADC_MODE_POWER_LOW),
                                .adiv = ADC_DIV_1,
                                .adlsmp = !!(mode & ADC_MODE_SAMPLE_LONG),
                                .mode = ADC_BIT_16,
                                .adiclk = ADC_CLK_ADACK,
                                }).raw;
        ADC0.cfg2.raw = ((struct ADC_CFG2){
                        .muxsel = 1, /* we only have b channels on the K20 */
                                .adacken = !!(mode & ADC_MODE_KEEP_CLOCK),
                                .adhsc = !!(mode & ADC_MODE_SPEED_HIGH),
                                }).raw;
        ADC0.sc2.raw = ((struct ADC_SC2){
                        .adtrg = 0,
                                .acfe = 0,
                                .dmaen = 0,
                                .refsel = ADC_REF_DEFAULT,
                                }).raw;
        ADC0.sc3.raw = ((struct ADC_SC3){
                        .adco = !!(mode & ADC_MODE_CONTINUOUS),
                                .avge = (mode & ADC_MODE__AVG_MASK) / ADC_MODE__AVG / 2, /* XXX ugly */
                                }).raw;
}

int
adc_sample_start(enum adc_channel channel, adc_result_cb_t *cb, void *cbdata)
{
        if (adc_ctx.stat_a.active)
                return (-1);

        /* XXX check for previous conversion running */
        adc_ctx.stat_a.cb = cb;
        adc_ctx.stat_a.cbdata = cbdata;

        /* trigger conversion */
        ADC0.sc1a.raw = ((struct ADC_SC1){
                        .aien = 1,
                                .diff = 0, /* XXX */
                                .adch = channel
                                }).raw;
        return (0);
}

int
adc_sample_abort(void)
{
        crit_enter();
        ADC0.sc1a.raw = ((struct ADC_SC1){.adch = ADC_ADCH_DISABLED}).raw;
        if (adc_ctx.stat_a.active) {
                adc_result_cb_t *cb = adc_ctx.stat_a.cb;
                void *cbdata = adc_ctx.stat_a.cbdata;

                adc_ctx.stat_a.active = 0;
                crit_exit();
                cb(0, 1, cbdata);
                return (0);
        } else {
                crit_exit();
                return (-1);
        }
}

void
ADC0_Handler(void)
{
        if (ADC0.sc1a.coco) {
                adc_result_cb_t *cb;
                void *cbdata;
                uint16_t val;

                crit_enter();
                cb = adc_ctx.stat_a.cb;
                cbdata = adc_ctx.stat_a.cbdata;
                if (!ADC0.sc3.adco)
                        adc_ctx.stat_a.active = 0;
                val = ADC0.ra;  /* clears interrupt */
                crit_exit();

                cb(val, 0, cbdata);
        }
        /* XXX repeat for sb1 */
}
