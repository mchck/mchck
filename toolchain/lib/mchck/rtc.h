/* Enable access to the RTC and starts the oscillator. */
void rtc_init();

/* Start the RTC counter
 *
 * This can fail if the counter value is invalid, as is the case
 * after power-on. This is resolved by setting the counter value
 * (e.g. with rtc_set_time).
 *
 * One should allow around one second between calling rtc_init and
 * starting the counter to allow the oscillator to stabilize.
 */
int rtc_start_counter();

/* Get the current time in seconds */
uint32_t rtc_get_time();

/* Set the current time in seconds */
void rtc_set_time(uint32_t seconds);

typedef void (rtc_alarm_cb)(void *cbdata);

struct rtc_alarm_ctx {
        struct rtc_alarm_ctx *next;
        uint32_t time;
        rtc_alarm_cb *cb;
        void *cbdata;
};

void rtc_alarm_add(struct rtc_alarm_ctx *ctx, uint32_t time,
                   rtc_alarm_cb *cb, void *cbdata);

void rtc_alarm_cancel(struct rtc_alarm_ctx *ctx);
