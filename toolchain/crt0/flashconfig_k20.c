#include <stdint.h>

struct FTFL_FSEC_t {
        enum {
                FTFL_FSEC_SEC_UNSECURE = 2,
                FTFL_FSEC_SEC_SECURE = 3
        } sec : 2;
        enum {
                FTFL_FSEC_FSLACC_DENY = 1,
                FTFL_FSEC_FSLACC_GRANT = 3
        } fslacc : 2;
        enum {
                FTFL_FSEC_MEEN_DISABLE = 2,
                FTFL_FSEC_MEEN_ENABLE = 3
        } meen : 2;
        enum {
                FTFL_FSEC_KEYEN_DISABLE = 1,
                FTFL_FSEC_KEYEN_ENABLE = 2
        } keyen : 2;
} __packed;

struct FTFL_CONFIG_t {
        uint8_t key[8];
        uint8_t fprot[4];
        struct FTFL_FSEC_t fsec;
        uint8_t fopt;
        uint8_t feprot;
        uint8_t fdprot;
};


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
        .fopt = 0xff,
        .feprot = 0xff,
        .fdprot = 0xff
};
