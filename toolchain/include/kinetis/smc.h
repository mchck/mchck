struct SMC {
        struct SMC_PMPROT {
                UNION_STRUCT_START(8);
                uint8_t _rsvd0   : 1;
                uint8_t avlls    : 1;
                uint8_t _rsvd1   : 1;
                uint8_t alls     : 1;
                uint8_t _rsvd2   : 1;
                uint8_t avlp     : 1;
                uint8_t _rsvd3   : 2;
                UNION_STRUCT_END;
        } pmprot;
        struct SMC_PMCTRL {
                UNION_STRUCT_START(8);
                enum {
                        STOPM_STOP = 0b000,
                        STOPM_VLPS = 0b010,
                        STOPM_LLS  = 0b011,
                        STOPM_VLLS = 0b100,
                } stopm          : 3;
                uint8_t stopa    : 1;
                uint8_t _rsvd0   : 1;
                enum {
                        RUNM_RUN   = 0b00,
                        RUNM_VLPR  = 0b10,
                } runm           : 2;
                uint8_t lpwui    : 1;
                UNION_STRUCT_END;
        } pmctrl;
        struct SMC_VLLSCTRL {
                UNION_STRUCT_START(8);
                enum {
                        VLLSM_VLLS0 = 0b000,
                        VLLSM_VLLS1 = 0b001,
                        VLLSM_VLLS2 = 0b010,
                        VLLSM_VLLS3 = 0b011,
                } vllsm          : 3;
                uint8_t _rsvd0   : 2;
                uint8_t porpo    : 1;
                uint8_t _rsvd1   : 2;
                UNION_STRUCT_END;
        } vllsctrl;
        struct SMC_PMSTAT {
                UNION_STRUCT_START(8);
                enum {
                        PMSTAT_RUN   = 1 << 0,
                        PMSTAT_STOP  = 1 << 1,
                        PMSTAT_VLPR  = 1 << 2,
                        PMSTAT_VLPW  = 1 << 3,
                        PMSTAT_VLPS  = 1 << 4,
                        PMSTAT_LLS   = 1 << 5,
                        PMSTAT_VLLS  = 1 << 6,
                } pmstat         : 7;
                uint8_t _rsvd0   : 1;
                UNION_STRUCT_END;
        } pmstat;
};
CTASSERT_SIZE_BYTE(struct SMC, 0x03+1);

extern volatile struct SMC SMC;
