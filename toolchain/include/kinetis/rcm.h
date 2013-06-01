#include <mchck.h>

struct RCM_t {
        struct RCM_SRS0_t {
                UNION_STRUCT_START(8);
                uint8_t wakeup : 1;
                uint8_t lvd : 1;
                uint8_t loc : 1;
                uint8_t lol : 1;
                uint8_t _rsvd0 : 1;
                uint8_t wdog : 1;
                uint8_t pin : 1;
                uint8_t por : 1;
                UNION_STRUCT_END;
        } srs0;
        struct RCM_SRS1_t {
                UNION_STRUCT_START(8);
                uint8_t jtag : 1;
                uint8_t lockup : 1;
                uint8_t sw : 1;
                uint8_t mdm_ap : 1;
                uint8_t ezpt : 1;
                uint8_t sackerr : 1;
                uint8_t _rsvd0 : 2;
                UNION_STRUCT_END;
        } srs1;
        uint8_t _pad0[2];
        struct RCM_RPFC_t {
                UNION_STRUCT_START(8);
                enum {
                        RCM_RSTFLTSRW_DISABLED = 0,
                        RCM_RSTFLTSRW_BUS = 1,
                        RCM_RSTFLTSRW_LPO = 2
                } rstfltsrw : 2;
                uint8_t rstfltss : 1;
                uint8_t _rsvd0 : 5;
                UNION_STRUCT_END;
        } rpfc;
        struct RCM_RPFW_t {
                UNION_STRUCT_START(8);
                uint8_t rstfltsel : 5;
                uint8_t _rsvd0 : 3;
                UNION_STRUCT_END;
        } rpfw;
        uint8_t _pad1;
        struct RCM_MR_t {
                uint8_t _rsvd0 : 1;
                uint8_t ezp_ms : 1;
                uint8_t _rsvd1 : 6;
        } mr;
};
CTASSERT_SIZE_BYTE(struct RCM_t, 8);

extern volatile struct RCM_t RCM;
