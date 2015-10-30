typedef void (timeout_cb_t)(void *);

union timeout_time {
        uint32_t time;
        struct {
                uint16_t count;
                uint16_t epoch;
        };
};

struct timeout_ctx {
        union timeout_time time;
        struct timeout_ctx *next;
        timeout_cb_t *cb;
        void *cbdata;
};

void timeout_init(void);
void timeout_add(struct timeout_ctx *t, uint32_t ms, timeout_cb_t *cb, void *cbdata);
int timeout_cancel(struct timeout_ctx *t);

/*
 * Get time
 *
 * The timeout subsystem provides a clock with a 1 millisecond
 * tick. timeout_get_time() exposes this timebase. However, as the
 * clock is disabled when not in use one must take a reference to the
 * clock with timeout_get when it's needed. The reference can be
 * dropped with timeout_put.
 */
void timeout_get_ref();
void timeout_put_ref();
union timeout_time timeout_get_time();
