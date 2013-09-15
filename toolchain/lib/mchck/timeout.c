#include <mchck.h>

static struct timeout_ctx *timeout_queue;

static union timeout_time timeout_lazy_now;
struct timeout_ctx overflow;


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

static void
timeout_empty(void *data)
{
}

/* call with crit_active() */
static void
timeout_reschedule(void)
{
        if (timeout_queue == NULL) {
                LPTMR0.csr.raw &= ~((struct LPTMR_CSR){ .ten = 1, .tie = 1 }).raw;
                return;
        }
        /* will we have to wrap the epoch before? */
        if (timeout_queue->time.count > timeout_lazy_now.count &&
            timeout_queue->time.epoch > timeout_lazy_now.epoch) {
                /* we get triggered at the end of the period, so -1 */
                overflow.time.time = timeout_lazy_now.time - 1;
                overflow.time.epoch += 1;
                overflow.cb = timeout_empty;
                overflow.next = timeout_queue;
                timeout_queue = &overflow;
        }
        LPTMR0.cmr = timeout_queue->time.count;
        LPTMR0.csr.raw |= ((struct LPTMR_CSR){
                        .ten = 1,
                                .tie = 1,
                                .tcf = 1
                                }).raw;
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
        struct timeout_ctx *t = timeout_queue;
        if (t == NULL) {
                crit_exit();
                return;
        }
        timeout_queue = t->next;
        timeout_reschedule();
        crit_exit();
        t->cb(t->cbdata);
}
