#include <mchck.h>

struct PIT_t {
        struct PIT_MCR_t {
                UNION_STRUCT_START(32);
                uint32_t frz:1;
                uint32_t mdis:1;
                uint32_t _pad:30;
                UNION_STRUCT_END;
        } mcr;
        uint32_t _pad[63];
        struct PIT_TIMER_t {
                uint32_t ldval;
                uint32_t cval;
                struct PIT_TCTRL_t {
                        UNION_STRUCT_START(32);
                        uint32_t ten:1;
                        uint32_t tie:1;
                        uint32_t _pad:30;
                        UNION_STRUCT_END;
                } tctrl;
                struct PIT_TFLG_t {
                        UNION_STRUCT_START(32);
                        uint32_t tif:1;
                        uint32_t _pad:31;
                        UNION_STRUCT_END;
                } tflg;
        } timer[4];
};

CTASSERT_SIZE_BYTE(struct PIT_t, 256 + 64);

extern volatile struct PIT_t PIT;
