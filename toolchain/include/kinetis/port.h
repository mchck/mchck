#include <mchck.h>

/* Pin Control Register n */
struct PCR_t {
        UNION_STRUCT_START(32);
        enum {
                PCR_PULLDOWN = 0,
                PCR_PULLUP = 1
        } ps : 1;
        uint8_t pe : 1;
        uint8_t sre : 1;
        uint8_t _rsvd0 : 1;
        uint8_t pfe : 1;
        uint8_t ode : 1;
        uint8_t dse : 1;
        uint8_t _rsvd1 : 1;
        enum PCR_MUX_t {
                PCR_MUX_DISABLE = 0x0,
                PCR_MUX_ALT1 = 0x1, PCR_MUX_GPIO = 0x1,   /* aliases, ALT1 is always GPIO */
                PCR_MUX_ALT2 = 0x2,
                PCR_MUX_ALT3 = 0x3,
                PCR_MUX_ALT4 = 0x4,
                PCR_MUX_ALT5 = 0x5,
                PCR_MUX_ALT6 = 0x6,
                PCR_MUX_ALT7 = 0x7
        } mux : 3;
        uint8_t _rsvd2 : 4;
        uint8_t lk : 1;
        enum PCR_IRQC_t {
                PCR_IRQC_DISABLED    = 0x0,
                PCR_IRQC_DMA_RISING  = 0x1,
                PCR_IRQC_DMA_FALLING = 0x2,
                PCR_IRQC_DMA_EITHER  = 0x3,
                PCR_IRQC_INT_ZERO    = 0x8,
                PCR_IRQC_INT_RISING  = 0x9,
                PCR_IRQC_INT_FALLING = 0xA,
                PCR_IRQC_INT_EITHER  = 0xB,
                PCR_IRQC_INT_ONE     = 0xC
        } irqc : 4;
        uint8_t _rsvd3 : 4;
        uint8_t isf : 1;
        uint8_t _rsvd4 : 7;
        UNION_STRUCT_END;
};
_Static_assert(sizeof(struct PCR_t) == 4, "Size assertion failed");

/* Global Pin Control register */
struct GPCR_t {
        UNION_STRUCT_START(32);
        uint16_t gpwe;
        uint16_t gpwd;
        UNION_STRUCT_END;
};
_Static_assert(sizeof(struct GPCR_t) == 4, "Size assertion failed");

/* a single complete port register structure */
struct PORT_t {
        struct PCR_t pcr[32];
        struct GPCR_t gpclr;
        struct GPCR_t gpchr;
        uint32_t isfr;
};
_Static_assert(sizeof(struct PORT_t) == 140, "Size assertion failed");

/* port structures are not contiguous in memory, can't allocate them as an array */
extern volatile struct PORT_t PORTA;
extern volatile struct PORT_t PORTB;
extern volatile struct PORT_t PORTC;
extern volatile struct PORT_t PORTD;
extern volatile struct PORT_t PORTE;
