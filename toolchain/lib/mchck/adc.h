/*
 * low-level adc interface
 *
 * This is a low-level interface to the ADC. Note that this does not
 * make any attempt at queuing multiple sample requests. See
 * adc_queue.h for a high-level interface which manages this for you.
 *
 * If you would prefer to use the low-level interface, you should
 * define a function adc_calibration_done callback which will be
 * called after adc_init once calibration has completed.
 *
 */

enum adc_mode {
        ADC_MODE_POWER_NORMAL  = 0 << 0,
        ADC_MODE_POWER_LOW     = 1 << 0,
        ADC_MODE_SAMPLE_NORMAL = 0 << 1,
        ADC_MODE_SAMPLE_LONG   = 1 << 1,
        ADC_MODE_SPEED_NORMAL  = 0 << 2,
        ADC_MODE_SPEED_HIGH    = 1 << 2,
        ADC_MODE_SINGLE        = 0 << 3,
        ADC_MODE_CONTINUOUS    = 1 << 3,
        ADC_MODE_AVG_OFF       = 0 << 4,
        ADC_MODE__AVG          = 1 << 4,
        ADC_MODE_AVG_4         = (ADC_AVG_4 << 5) | ADC_MODE__AVG,
        ADC_MODE_AVG_8         = (ADC_AVG_8 << 5) | ADC_MODE__AVG,
        ADC_MODE_AVG_16        = (ADC_AVG_16 << 5) | ADC_MODE__AVG,
        ADC_MODE_AVG_32        = (ADC_AVG_32 << 5) | ADC_MODE__AVG,
        ADC_MODE__AVG_MASK     = 0b11 << 5,
        ADC_MODE_KEEP_CLOCK    = 1 << 7,
};

enum adc_channel {
        ADC__DIFFERENTIAL = 0x80,
        ADC_DIFF0 = 0 | ADC__DIFFERENTIAL,
        ADC_DP0 = 0,
        ADC_PTB0 = 8,
        ADC_PTB1 = 9,
        ADC_PTB2 = 12,
        ADC_PTB3 = 13,
        ADC_PTC0 = 14,
        ADC_PTC1 = 15,
        ADC_PTC2 = 4,
        ADC_PTD1 = 5,
        ADC_PTD5 = 6,
        ADC_PTD6 = 7,
        ADC_DM0  = 19,
        ADC_TEMP = ADC_ADCH_TEMP,
        ADC_BANDGAP = ADC_ADCH_BANDGAP,
        ADC_VREFH = ADC_ADCH_VREFSH,
        ADC_VREFL = ADC_ADCH_VREFSL,
        ADC__MASK = 0x1f,
};

typedef void (adc_result_cb_t)(uint16_t val, int error, void *cbdata);

void adc_init(void);
void adc_sample_prepare(enum adc_mode mode);
int adc_sample_start(enum adc_channel channel, adc_result_cb_t *cb, void *cbdata);
int adc_sample_abort(void);
void adc_calibrate_voltage(unsigned accum reference);
unsigned accum adc_as_voltage(uint16_t val);

/* Weakly defined by adc_queue. Override if adc_queue is not used. */
void adc_calibration_done(void);
