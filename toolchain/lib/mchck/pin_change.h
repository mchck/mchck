#include <mchck.h>

/* Pin change interrupt handling */
enum pin_change_polarity {
        PIN_CHANGE_ZERO     = 0x8,
        PIN_CHANGE_ONE      = 0xC,
        PIN_CHANGE_FALLING  = 0xA,
        PIN_CHANGE_RISING   = 0x9,
        PIN_CHANGE_EITHER   = 0xB
};

typedef void (*pin_change_cb)(void *);

struct pin_change_handler {
        enum pin_id pin_id;
        enum pin_change_polarity polarity;
        pin_change_cb cb;
        void *cbdata;
};

/*
 * Register a pin change callback.
 *
 * Note that _pin should be a PIN_* constant, not GPIO_*. Things will
 * unfortunately fail silently if this is violated
 *
 */
#define PIN_DEFINE_CALLBACK(_pin, _polarity, _cb, _cbdata)              \
        const struct pin_change_handler _CONCAT(_CONCAT(_CONCAT(pin_handler_, _pin), _), _polarity) \
                __attribute__((__section__(".pin_hooks." _STR(_pin)), __used__)) = { \
                .pin_id = _pin,                                         \
                .polarity = _polarity,                                  \
                .cb = _cb,                                              \
                .cbdata = _cbdata                                       \
        };

void pin_change_init(void);
