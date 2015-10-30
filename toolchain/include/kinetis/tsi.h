struct TSI {
    struct TSI_GENCS {
        UNION_STRUCT_START(32);
        uint32_t stpe       : 1;
        uint32_t stm        : 1;
        uint32_t _rsvd0     : 2;
        uint32_t esor       : 1;
        uint32_t erie       : 1;
        uint32_t tsiie      : 1;
        uint32_t tsien      : 1;
        uint32_t swts       : 1;
        uint32_t scnip      : 1;
        uint32_t _rsvd1     : 2;
        uint32_t ovrf       : 1;
        uint32_t outrgf     : 1;
        uint32_t exterf     : 1;
        uint32_t eosf       : 1;
        enum {
            TSI_OSC_PS_1    = 0b000,
            TSI_OSC_PS_2    = 0b001,
            TSI_OSC_PS_4    = 0b010,
            TSI_OSC_PS_8    = 0b011,
            TSI_OSC_PS_16   = 0b100,
            TSI_OSC_PS_32   = 0b101,
            TSI_OSC_PS_64   = 0b110,
            TSI_OSC_PS_128  = 0b111
        } ps                : 3;
        uint32_t nscn       : 5;
        uint32_t lpscnitv   : 4;
        enum {
            TSI_LP_LPOCLCK  = 0b0,
            TSI_LP_VLPOSCCLK= 0b1
        } lpclks            : 1;
        uint32_t _rsvd2     : 3;
        UNION_STRUCT_END;
    } gencs;
    struct TSI_SCANS {
        UNION_STRUCT_START(32);
        enum {
            TSI_AM_PS_1     = 0b000,
            TSI_AM_PS_2     = 0b001,
            TSI_AM_PS_4     = 0b010,
            TSI_AM_PS_8     = 0b011,
            TSI_AM_PS_16    = 0b100,
            TSI_AM_PS_32    = 0b101,
            TSI_AM_PS_64    = 0b110,
            TSI_AM_PS_128   = 0b111
        } ampsc             : 3;
        enum {
            TSI_AM_LPOSCLCK = 0b00,
            TSI_AM_MCGIRCLK = 0b01,
            TSI_AM_OSCERCLK = 0b10
        } amclks            : 2;
        uint32_t _rsvd0     : 3;
        uint32_t smod       : 8;
        uint32_t extchrg    : 4;
        uint32_t _rsvd1     : 4;
        uint32_t refchrg    : 4;
        uint32_t _rsvd2     : 4;
        UNION_STRUCT_END;
    } scanc;
    struct TSI_PEN {
        UNION_STRUCT_START(32);
        uint32_t pen        : 16;
        uint32_t lpsp       : 4;
        uint32_t _rsvd0     : 12;
        UNION_STRUCT_END;
    } pen;
    struct TSI_WUCNTR {
        UNION_STRUCT_START(32);
        uint32_t wucnt      : 16;
        uint32_t _rsvd0     : 16;
        UNION_STRUCT_END;
    } wucntr;
    uint32_t _pad0[(0x100 - 0xc)/4 - 1];
    struct TSI_CNTR {
        UNION_STRUCT_START(32);
        uint32_t ctn1       : 16;
        uint32_t ctn        : 16;
        UNION_STRUCT_END;
    } cntr[8];
    struct TSI_THRESHOLD {
        UNION_STRUCT_START(32);
        uint32_t hthh       : 16;
        uint32_t lthh       : 16;
        UNION_STRUCT_END;
    } threshold;
};

CTASSERT_SIZE_BYTE(struct TSI, 0x124);

extern volatile struct TSI TSI0;
