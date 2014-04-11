#include <mchck.h>

static struct timeout_ctx *timeout_queue;

static union timeout_time timeout_lazy_now;
static struct timeout_ctx overflow;

static unsigned int timeout_ref_count = 0;

/* Here we use timeout_ctx.cb == NULL to indicate an unqueued context */

/* call with crit_active() */
static void
timeout_update_time(void)
{
        /* tell ctr to latch */
        LPTMR0.cnr = 0;
        uint16_t now = LPTMR0.cnr;
        if (timeout_lazy_now.count > now)
                timeout_lazy_now.epoch++;
        timeout_lazy_now.count = now;
}

union timeout_time
timeout_get_time()
{
        crit_enter();
        timeout_update_time();
        crit_exit();
        return timeout_lazy_now;
}

static void
timeout_empty(void *data)
{
}

/* call with crit_active() */
static void
timeout_schedule_wrap(void)
{
        /* we get triggered at the end of the period, so -1 */
        overflow.time.time = timeout_lazy_now.time - 1;
        overflow.time.epoch += 1;
        overflow.cb = timeout_empty;
        overflow.next = timeout_queue;
        timeout_queue = &overflow;
}

/* call with crit_active() */
static void
timeout_reschedule(void)
{
        if (timeout_queue == NULL) {
                if (timeout_ref_count > 0) {
                        /* the queue is empty but we still need to
                           keep the timebase running */
                        timeout_schedule_wrap();
                } else {
                        /* we can stop the timebase */
                        LPTMR0.csr.raw &= ~((struct LPTMR_CSR){ .ten = 1, .tie = 1 }).raw;
                }
                return;
        }

        /* will we have to wrap the epoch before the next timeout? */
        if (timeout_queue->time.count > timeout_lazy_now.count &&
            timeout_queue->time.epoch > timeout_lazy_now.epoch) {
                timeout_schedule_wrap();
        }
        LPTMR0.cmr = timeout_queue->time.count;
        LPTMR0.csr.raw |= ((struct LPTMR_CSR){
                        .ten = 1,
                                .tie = 1,
                                .tcf = 1
                                }).raw;
}

void
timeout_get_ref()
{
        crit_enter();
        timeout_ref_count++;
        timeout_reschedule();
        crit_exit();
}

void
timeout_put_ref()
{
        crit_enter();
        if (timeout_ref_count == 0)
                panic("timeout_put_ref");
        timeout_ref_count--;
        timeout_reschedule();
        crit_exit();
}

void
timeout_init(void)
{
        SIM.scgc5.lptimer = 1;
        LPTMR0.psr.raw = ((struct LPTMR_PSR){
                        .prescale = 0,
                                .pbyp = 1,
                                .pcs = LPTMR_PCS_LPO
                                }).raw;
        LPTMR0.csr.raw = ((struct LPTMR_CSR){
                        .tcf = 1,
                        .tfc = 1,
                                .tms = LPTMR_TMS_TIME,
                                }).raw;
        int_enable(IRQ_LPT);
}

void
timeout_add(struct timeout_ctx *t, uint32_t ms, timeout_cb_t *cb, void *cbdata)
{
        crit_enter();
        timeout_update_time();

        // Ensure this context isn't already queued
        if (t->cb) {
                crit_exit();
                return;
        }

        *t = (struct timeout_ctx){
                .time.time = ms + timeout_lazy_now.time + 1,
                .cb = cb,
                .cbdata = cbdata,
        };

        /* XXX what if this traversal takes >= one timer tick? */
        struct timeout_ctx **p;
        for (p = &timeout_queue; *p != NULL; p = &(*p)->next) {
                if ((*p)->time.time > t->time.time)
                        break;
        }
        t->next = *p;
        *p = t;

        if (timeout_queue == t)
                timeout_reschedule();
        crit_exit();
}

int
timeout_cancel(struct timeout_ctx *t)
{
        crit_enter();
        struct timeout_ctx **p;
        for (p = &timeout_queue; *p != NULL; p = &(*p)->next) {
                if (*p == t)
                        break;
                if ((*p)->time.time > t->time.time)
                        return (-1);
        }
        *p = t->next;
        t->cb = NULL;
        if (*p == timeout_queue)
                timeout_reschedule();
        crit_exit();
        return (0);
}

void
LPT_Handler(void)
{
        crit_enter();
        timeout_update_time();
        if (timeout_queue == NULL) {
                // there are no tasks to run, schedule the next overflow
                timeout_reschedule();
                crit_exit();
                return;
        }

        // for each task whose time has past...
        while (timeout_queue->time.time <= timeout_lazy_now.time) {
                struct timeout_ctx *t = timeout_queue;
                timeout_queue = t->next;
                timeout_cb_t *cb = t->cb;
                t->cb = NULL;
                cb(t->cbdata);

                timeout_update_time();
                if (!timeout_queue)
                        break;
        }
        timeout_reschedule();
        crit_exit();
}
