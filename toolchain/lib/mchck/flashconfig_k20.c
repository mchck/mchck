#include <mchck.h>

const static __attribute__((section(".flash_config"), used))
struct FTFL_CONFIG_t flash_config = {
        .key = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff},
        .fprot = {0xff, 0xff, 0xff, 0xff},
        .fsec = {
                .sec = FTFL_FSEC_SEC_UNSECURE,
                .fslacc = FTFL_FSEC_FSLACC_GRANT,
                .meen = FTFL_FSEC_MEEN_ENABLE,
                .keyen = FTFL_FSEC_KEYEN_ENABLE
        },
        .fopt = {
                .nmi_dis = 0,
                .ezport_dis = 1,
                .lpboot = 1,
        },
        .feprot = 0xff,
        .fdprot = 0xff
};
