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
CTASSERT_SIZE_BIT(struct PCR_t, 32);

/* Global Pin Control register */
struct GPCR_t {
        UNION_STRUCT_START(32);
        uint16_t gpwe;
        uint16_t gpwd;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct GPCR_t, 32);

/**
 * The digital filter registers are undocumented.
 * Information found in K11P80M50SF4RM.
 */
/* Digital Filter Clock register */
struct PORT_DFCR_t {
        UNION_STRUCT_START(32);
        enum {
                PORT_CS_BUS = 0,
                PORT_CS_LPO = 1
        } cs : 1;
        unsigned _rsvd0 : 31;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct PORT_DFCR_t, 32);

struct PORT_DFWR_t {
        UNION_STRUCT_START(32);
        unsigned filt : 5;
        unsigned _rsvd0 : 27;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct PORT_DFWR_t, 32);

/* a single complete port register structure */
struct PORT_t {
        struct PCR_t pcr[32];
        struct GPCR_t gpclr;
        struct GPCR_t gpchr;
        uint32_t _pad0[(0xa0 - 0x84)/4 - 1];
        uint32_t isfr;
        uint32_t _pad1[(0xc0 - 0xa0)/4 - 1];
        uint32_t dfer;
        struct PORT_DFCR_t dfcr;
        struct PORT_DFWR_t dfwr;
};
CTASSERT_SIZE_BYTE(struct PORT_t, 0xc8+4);

/* port structures are not contiguous in memory, can't allocate them as an array */
extern volatile struct PORT_t PORTA;
extern volatile struct PORT_t PORTB;
extern volatile struct PORT_t PORTC;
extern volatile struct PORT_t PORTD;
extern volatile struct PORT_t PORTE;
