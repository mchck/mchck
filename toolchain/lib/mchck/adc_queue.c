#include <mchck.h>

static struct adc_queue_ctx *head;

/* this starts out as true until calibration has finished */
static volatile bool adc_busy = true;

static void adc_queue_start(void);

static void
adc_queue_sample_done(uint16_t codepoint, int error, void* cbdata)
{
        struct adc_queue_ctx *ctx = head;
        head = head->next;
        ctx->cb(codepoint, error, ctx->cbdata);
        adc_busy = false;
        adc_queue_start();
}

static void
adc_queue_start(void)
{
        if (adc_busy)
                return;

        crit_enter();
        if (head) {
                adc_busy = true;
                adc_sample_prepare(head->mode);
                adc_sample_start(head->channel, adc_queue_sample_done, NULL);
        }
        crit_exit();
}

void
adc_queue_sample(struct adc_queue_ctx *ctx,
                 enum adc_channel channel, enum adc_mode mode,
                 adc_result_cb_t *cb, void *cbdata)
{
        *ctx = (struct adc_queue_ctx){
                .channel = channel,
                .mode = mode,
                .cb = cb,
                .cbdata = cbdata,
                .next = NULL
        };

        crit_enter();
        for (struct adc_queue_ctx **c = &head; ; c = &(*c)->next) {
                if (*c == NULL) {
                        *c = ctx;
                        if (c == &head)
                                adc_queue_start();
                        break;
                }
        }
        crit_exit();
}

#pragma weak adc_calibration_done
void
adc_calibration_done(void)
{
        adc_busy = false;
        adc_queue_start();
}

void
adc_queue_init(void)
{
        adc_busy = true;
        adc_init();
}
