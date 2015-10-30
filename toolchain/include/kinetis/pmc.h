struct PMC {
        struct PMC_LVDSC1_t {
                UNION_STRUCT_START(8);
                enum {
                        PMC_LVDV_LOW = 0b00,
                        PMC_LVDV_HIGH = 0b01
                } lvdv : 2;
                uint8_t _rsvd0 : 2;
                uint8_t lvdre : 1;
                uint8_t lvdie : 1;
                uint8_t lvdack : 1;
                uint8_t lvdf : 1;
                UNION_STRUCT_END;
        } lvdsc1;
        struct PMC_LVDSC2_t {
                UNION_STRUCT_START(8);
                enum {
                        PMC_LVWV_LOW = 0b00,
                        PMC_LVWV_MID1 = 0b01,
                        PMC_LVWV_MID2 = 0b10,
                        PMC_LVWV_HIGH = 0b11
                } lvwv : 2;
                uint8_t _rsvd0 : 3;
                uint8_t lvwie : 1;
                uint8_t lvwack : 1;
                uint8_t lvwf : 1;
                UNION_STRUCT_END;
        } lvdsc2;
        struct PMC_REGSC_t {
                UNION_STRUCT_START(8);
                uint8_t bgbe : 1;
                uint8_t _rsvd0 : 1;
                uint8_t regons : 1;
                uint8_t ackiso : 1;
                uint8_t bgen : 1;
                uint8_t _rsvd1 : 3;
                UNION_STRUCT_END;
        } regsc;
};
CTASSERT_SIZE_BYTE(struct PMC, 3);

extern volatile struct PMC PMC;
