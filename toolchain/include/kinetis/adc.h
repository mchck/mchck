struct ADC_SC1 {
        UNION_STRUCT_START(32);
        enum ADC_ADCH {
                ADC_ADCH_TEMP = 0b11010,
                ADC_ADCH_BANDGAP = 0b11011,
                ADC_ADCH_VREFSH = 0b11101,
                ADC_ADCH_VREFSL = 0b11110,
                ADC_ADCH_DISABLED = 0b11111
        } adch : 5;
        uint32_t diff : 1;
        uint32_t aien : 1;
        uint32_t coco : 1;
        uint32_t _rsvd0 : 24;
        UNION_STRUCT_END;
};

struct ADC {
        struct ADC_SC1 sc1a;
        struct ADC_SC1 sc1b;
        struct ADC_CFG1 {
                UNION_STRUCT_START(32);
                enum {
                        ADC_CLK_BUS = 0b00,
                        ADC_CLK_BUS_HALF = 0b01,
                        ADC_CLK_ALTCLK = 0b10,
                        ADC_CLK_ADACK = 0b11
                } adiclk : 2;
                enum {
                        ADC_BIT_8 = 0b00,
                        ADC_BIT_12 = 0b01,
                        ADC_BIT_10 = 0b10,
                        ADC_BIT_16 = 0b11
                } mode : 2;
                uint32_t adlsmp : 1;
                enum {
                        ADC_DIV_1 = 0b00,
                        ADC_DIV_2 = 0b01,
                        ADC_DIV_4 = 0b10,
                        ADC_DIV_8 = 0b11
                } adiv : 2;
                uint32_t adlpc : 1;
                uint32_t _rsvd0 : 24;
                UNION_STRUCT_END;
        } cfg1;
        struct ADC_CFG2 {
                UNION_STRUCT_START(32);
                enum {
                        ADC_SAMPLE_CLKS_24 = 0b00,
                        ADC_SAMPLE_CLKS_16 = 0b01,
                        ADC_SAMPLE_CLKS_10 = 0b10,
                        ADC_SAMPLE_CLKS_6 = 0b11
                } adlsts : 2;
                uint32_t adhsc : 1;
                uint32_t adacken : 1;
                uint32_t muxsel : 1;
                uint32_t _rsvd0 : 27;
                UNION_STRUCT_END;
        } cfg2;
        uint32_t ra;
        uint32_t rb;
        uint32_t cv1;
        uint32_t cv2;
        struct ADC_SC2 {
                UNION_STRUCT_START(32);
                enum {
                        ADC_REF_DEFAULT = 0b00,
                        ADC_REF_ALTERNATE = 0b01
                } refsel : 2;
                uint32_t dmaen : 1;
                uint32_t acren : 1;
                uint32_t acfgt : 1;
                uint32_t acfe : 1;
                uint32_t adtrg : 1;
                uint32_t adact : 1;
                uint32_t _rsvd0 : 24;
                UNION_STRUCT_END;
        } sc2;
        struct ADC_SC3 {
                UNION_STRUCT_START(32);
                enum {
                        ADC_AVG_4 = 0b00,
                        ADC_AVG_8 = 0b01,
                        ADC_AVG_16 = 0b10,
                        ADC_AVG_32 = 0b11
                } avgs : 2;
                uint32_t avge : 1;
                uint32_t adco : 1;
                uint32_t _rsvd0 : 2;
                uint32_t calf : 1;
                uint32_t cal : 1;
                uint32_t _rsvd1 : 24;
                UNION_STRUCT_END;
        } sc3;
        uint32_t ofs;
        uint32_t pg;
        uint32_t mg;
        uint32_t clp[7];
        uint32_t _pad0;
        uint32_t clm[7];
};
CTASSERT_SIZE_BYTE(struct ADC, 0x6c+4);

extern volatile struct ADC ADC0;
