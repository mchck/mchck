#include <mchck.h>

static struct rtc_alarm_ctx *alarm_head = NULL;

void
rtc_init(void)
{
        SIM.scgc6.rtc = 1;
        RTC.cr.osce = 1;
}

int
rtc_start_counter(void)
{
        if (RTC.sr.tif)
                return 1;
        RTC.sr.tce = 1;
        return 0;
}

uint32_t
rtc_get_time()
{
        return RTC.tsr;
}

void
rtc_set_time(uint32_t seconds)
{
        int started = RTC.sr.tce;
        RTC.sr.tce = 0;
        RTC.tsr = seconds;
        RTC.sr.tce = started;
}

static void
rtc_alarm_update()
{
        if (alarm_head) {
                int_enable(IRQ_RTC_alarm);
                RTC.tar = alarm_head->time;
        } else {
                int_disable(IRQ_RTC_alarm);
        }
}

void
rtc_alarm_add(struct rtc_alarm_ctx *ctx, uint32_t time,
              rtc_alarm_cb *cb, void *cbdata)
{
        ctx->time = time;
        ctx->cb = cb;
        ctx->cbdata = cbdata;

        crit_enter();
        if (alarm_head) {
                struct rtc_alarm_ctx **last_next = &alarm_head;
                struct rtc_alarm_ctx *tail = alarm_head;
                while (tail && time > tail->time)
                        tail = tail->next;
                ctx->next = tail;
                *last_next = ctx;
        } else {
                alarm_head = ctx;
        }
        rtc_alarm_update();
        crit_exit();
}

void
rtc_alarm_cancel(struct rtc_alarm_ctx *ctx)
{
        crit_enter();
        struct rtc_alarm_ctx *tail = alarm_head;
        while (tail) {
                if (tail->next == ctx) {
                        tail->next = ctx->next;
                        break;
                }
                tail = tail->next;
        }
        crit_exit();
}

void
RTC_alarm_Handler(void)
{
        uint32_t time = rtc_get_time();
        while (alarm_head) {
                if (alarm_head->time <= time) {
                        struct rtc_alarm_ctx *ctx = alarm_head;
                        alarm_head = alarm_head->next;
                        rtc_alarm_update();
                        ctx->cb(ctx->cbdata);
                } else {
                        break;
                }
        }
}
