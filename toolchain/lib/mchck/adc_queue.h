/*
 * adc queue
 *
 * This is a high-level interface to the ADC which manages queuing
 * of multiple sampling requests. If you want to do your own queueing,
 * see adc.h
 *
 */

struct adc_queue_ctx {
        enum adc_channel channel;
        enum adc_mode mode;
        adc_result_cb_t *cb;
        void *cbdata;
        struct adc_queue_ctx *next;
};

void adc_queue_sample(struct adc_queue_ctx *ctx,
                      enum adc_channel channel, enum adc_mode mode,
                      adc_result_cb_t *cb, void *cbdata);

/* This should be called in lieu of adc_init() */
void adc_queue_init(void);
