struct LPTMR {
        struct LPTMR_CSR {
                UNION_STRUCT_START(32);
                uint32_t ten    : 1;
                enum {
                        LPTMR_TMS_TIME    = 0,
                        LPTMR_TMS_PULSE   = 1
                } tms           : 1;
                uint32_t tfc    : 1;
                enum {
                        LPTMR_TPP_RISING  = 0,
                        LPTMR_TPP_FALLING = 1
                } tpp           : 1;
                uint32_t tps    : 2;
                uint32_t tie    : 1;
                uint32_t tcf    : 1;
                uint32_t _rsvd0 : 24;
                UNION_STRUCT_END;
        } csr;
        struct LPTMR_PSR {
                UNION_STRUCT_START(32);
                enum {
                        LPTMR_PCS_MCGIRCLK = 0b00,
                        LPTMR_PCS_LPO      = 0b01,
                        LPTMR_PCS_ERCLK32K = 0b10,
                        LPTMR_PCS_OSCERCLK = 0b11
                } pcs             : 2;
                uint32_t pbyp     : 1;
                uint32_t prescale : 4;
                uint32_t _rsvd0   : 25;
                UNION_STRUCT_END;
        } psr;
        uint16_t cmr;
        uint16_t _pad0;
        uint16_t cnr;
        uint16_t _pad1;
};
CTASSERT_SIZE_BYTE(struct LPTMR, 0xc+4);

extern volatile struct LPTMR LPTMR0;
