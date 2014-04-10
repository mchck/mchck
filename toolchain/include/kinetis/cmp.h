struct CMP_CR0_t {
        UNION_STRUCT_START(8);
        uint8_t hystctr : 2;
        uint8_t _rsvd0 : 2;
        uint8_t filter_cnt : 3;
        uint8_t _rsvd1 : 1;
        UNION_STRUCT_END;
};
CTASSERT_SIZE_BIT(struct CMP_CR0_t, 8);

struct CMP_CR1_t {
        UNION_STRUCT_START(8);
        uint8_t en : 1;
        uint8_t ope : 1;
        uint8_t cos : 1;
        uint8_t inv : 1;
        uint8_t pmode : 1;
        uint8_t _rsvd0 : 1;
        uint8_t we : 1;
        uint8_t se : 1;
        UNION_STRUCT_END;
};

struct CMP_SCR_t {
        UNION_STRUCT_START(8);
        uint8_t cout : 1;
        uint8_t cff : 1;
        uint8_t cfr : 1;
        uint8_t ief : 1;
        uint8_t ier : 1;
        uint8_t _rsvd0 : 1;
        uint8_t dmaen : 1;
        uint8_t _rsvd1 : 1;
        UNION_STRUCT_END
};

struct CMP_DACCR_t {
        UNION_STRUCT_START(8);
        uint8_t vosel : 6;
        uint8_t vrsel : 1;
        uint8_t dacen : 1;
        UNION_STRUCT_END;
};

struct CMP_MUXCR_t {
        UNION_STRUCT_START(8);
        uint8_t msel : 3;
        uint8_t psel : 3;
        uint8_t _rsvd0 : 2;
        UNION_STRUCT_END;
};

struct CMP {
        struct CMP_CR0_t cr0;
        struct CMP_CR1_t cr1;
        uint8_t fpr;
        struct CMP_SCR_t scr;
        struct CMP_DACCR_t daccr;
        struct CMP_MUXCR_t muxcr;
};

extern volatile struct CMP CMP0;
extern volatile struct CMP CMP1;
