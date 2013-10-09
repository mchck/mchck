enum pin_port_id {
        PIN_PORTA = 1 << 16,
        PIN_PORTB = 2 << 16,
        PIN_PORTC = 3 << 16,
        PIN_PORTD = 4 << 16,
};

enum pin_id {
        PIN_PTA0 = PIN_PORTA | 0,
        PIN_PTA1 = PIN_PORTA | 1,
        PIN_PTA2 = PIN_PORTA | 2,
        PIN_PTA3 = PIN_PORTA | 3,
        PIN_PTA4 = PIN_PORTA | 4,
        PIN_PTA18 = PIN_PORTA | 18,
        PIN_PTA19 = PIN_PORTA | 19,
        PIN_PTB0 = PIN_PORTB | 0,
        PIN_PTB1 = PIN_PORTB | 1,
        PIN_PTB2 = PIN_PORTB | 2,
        PIN_PTB3 = PIN_PORTB | 3,
        PIN_PTB16 = PIN_PORTB | 16,
        PIN_PTB17 = PIN_PORTB | 17,
        PIN_PTC0 = PIN_PORTC | 0,
        PIN_PTC1 = PIN_PORTC | 1,
        PIN_PTC2 = PIN_PORTC | 2,
        PIN_PTC3 = PIN_PORTC | 3,
        PIN_PTC4 = PIN_PORTC | 4,
        PIN_PTC5 = PIN_PORTC | 5,
        PIN_PTC6 = PIN_PORTC | 6,
        PIN_PTC7 = PIN_PORTC | 7,
        PIN_PTD0 = PIN_PORTD | 0,
        PIN_PTD1 = PIN_PORTD | 1,
        PIN_PTD2 = PIN_PORTD | 2,
        PIN_PTD3 = PIN_PORTD | 3,
        PIN_PTD4 = PIN_PORTD | 4,
        PIN_PTD5 = PIN_PORTD | 5,
        PIN_PTD6 = PIN_PORTD | 6,
        PIN_PTD7 = PIN_PORTD | 7,
};

enum pin_mode {
        PIN_MODE__SLEW            =                         2 << 2,
        PIN_MODE_SLEW_FAST        = PIN_MODE__SLEW       | (0 << 2),
        PIN_MODE_SLEW_SLOW        = PIN_MODE__SLEW       | (1 << 2),
        PIN_MODE__SLEW_MASK       = PIN_MODE__SLEW       | (1 << 2),

        PIN_MODE__PULL            =                         4 << 4,
        PIN_MODE_PULL_OFF         = PIN_MODE__PULL       | (0 << 4),
        PIN_MODE_PULLUP           = PIN_MODE__PULL       | (1 << 4),
        PIN_MODE_PULLDOWN         = PIN_MODE__PULL       | (2 << 4),
        PIN_MODE__PULL_MASK       = PIN_MODE__PULL       | (3 << 4),

        PIN_MODE__DRIVE           =                         2 << 7,
        PIN_MODE_DRIVE_LOW        = PIN_MODE__DRIVE      | (0 << 7),
        PIN_MODE_DRIVE_HIGH       = PIN_MODE__DRIVE      | (1 << 7),
        PIN_MODE__DRIVE_MASK      = PIN_MODE__DRIVE      | (1 << 7),

        PIN_MODE__FILTER          =                         2 << 9,
        PIN_MODE_FILTER_OFF       = PIN_MODE__FILTER     | (0 << 9),
        PIN_MODE_FILTER_ON        = PIN_MODE__FILTER     | (1 << 9),
        PIN_MODE__FILTER_MASK     = PIN_MODE__FILTER     | (1 << 9),

        PIN_MODE__OPEN_DRAIN      =                         2 << 11,
        PIN_MODE_OPEN_DRAIN_OFF   = PIN_MODE__OPEN_DRAIN | (0 << 11),
        PIN_MODE_OPEN_DRAIN_ON    = PIN_MODE__OPEN_DRAIN | (1 << 11),
        PIN_MODE__OPEN_DRAIN_MASK = PIN_MODE__OPEN_DRAIN | (1 << 11),

        PIN_MODE__MUX             =                         8 << 13,
        PIN_MODE_MUX_ALT0         = PIN_MODE__MUX        | (0 << 13),
        PIN_MODE_MUX_ANALOG       = PIN_MODE__MUX        | (0 << 13),
        PIN_MODE_MUX_ALT1         = PIN_MODE__MUX        | (1 << 13),
        PIN_MODE_MUX_GPIO         = PIN_MODE__MUX        | (1 << 13),
        PIN_MODE_MUX_ALT2         = PIN_MODE__MUX        | (2 << 13),
        PIN_MODE_MUX_ALT3         = PIN_MODE__MUX        | (3 << 13),
        PIN_MODE_MUX_ALT4         = PIN_MODE__MUX        | (4 << 13),
        PIN_MODE_MUX_ALT5         = PIN_MODE__MUX        | (5 << 13),
        PIN_MODE_MUX_ALT6         = PIN_MODE__MUX        | (6 << 13),
        PIN_MODE_MUX_ALT7         = PIN_MODE__MUX        | (7 << 13),
        PIN_MODE__MUX_MASK        = PIN_MODE__MUX        | (7 << 13),

        PIN_MODE_RESET            =                         1 << 31
};


static inline enum pin_port_id
pin_port_from_pin(enum pin_id pin)
{
        return (pin & 0xf0000);
}

static inline int
pin_portnum_from_pin(enum pin_id pin)
{
        return (pin_port_from_pin(pin) >> 16);
}

static inline int
pin_physpin_from_pin(enum pin_id pin)
{
        return (pin & 0xffff);
}

volatile struct PORT_t *pin_physport_from_pin(enum pin_id pin);
void pin_mode(enum pin_id pin, enum pin_mode mode);
