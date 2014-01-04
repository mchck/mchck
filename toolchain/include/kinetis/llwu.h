enum llwu_pin_mode {
        LLWU_PE_DISABLED = 0b00,
        LLWU_PE_RISING   = 0b01,
        LLWU_PE_FALLING  = 0b10,
        LLWU_PE_BOTH     = 0b11
};

struct LLWU_FILT {
        UNION_STRUCT_START(8);
        uint8_t filtsel : 4;
        uint8_t _rsvd   : 1;
        enum llwu_filter_mode {
                LLWU_FILTER_DISABLED = 0b00,
                LLWU_FILTER_POSEDGE  = 0b01,
                LLWU_FILTER_NEGEDGE  = 0b10,
                LLWU_FILTER_BOTH     = 0b11
        } filte         : 2;
        uint8_t filtf   : 1;
        UNION_STRUCT_END;
};

struct LLWU {
        struct {
                UNION_STRUCT_START(8);
                enum llwu_pin_mode wupe0  : 2;
                enum llwu_pin_mode wupe1  : 2;
                enum llwu_pin_mode wupe2  : 2;
                enum llwu_pin_mode wupe3  : 2;
                UNION_STRUCT_END;
        } wupe[4];

        uint8_t wume;
        uint8_t wuf1;
        uint8_t wuf2;
        uint8_t mwuf;

        struct LLWU_FILT filt1;
        struct LLWU_FILT filt2;
        struct {
                UNION_STRUCT_START(8);
                uint8_t rstfilt : 1;
                uint8_t llrste  : 1;
                uint8_t _rsvd0  : 6;
                UNION_STRUCT_END;
        } rst;
};
CTASSERT_SIZE_BYTE(struct LLWU, 0x0a+1);

extern volatile struct LLWU LLWU;
