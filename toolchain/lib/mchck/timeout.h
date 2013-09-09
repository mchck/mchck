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
