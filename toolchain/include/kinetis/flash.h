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

struct FTFL_FSTAT_t {
        union {
                struct {
                        uint8_t mgstat0 : 1;
                        uint8_t _rsvd0 : 3;
                        uint8_t fpviol : 1;
                        uint8_t accerr : 1;
                        uint8_t rdcolerr : 1;
                        uint8_t ccif : 1;
                } __packed;
                uint8_t fstat;
        };
} __packed;

struct FTFL_FCNFG_t {
        union {
                struct {
                        uint8_t eeerdy : 1;
                        uint8_t ramrdy : 1;
                        uint8_t pflsh : 1;
                        uint8_t _rsvd0 : 1;
                        uint8_t erssusp : 1;
                        uint8_t ersareq : 1;
                        uint8_t rdcollie : 1;
                        uint8_t ccie : 1;
                } __packed;
                uint8_t fcnfg;
        } __packed;
} __packed;

/**
 * The FCOOB is a weird register file, because it is double big endian,
 * which makes for odd gaps and for some data that is big endian, and for
 * some that is little endian.
 */
union FTFL_FCCOB_t {
        struct ftfl_generic {
                uint32_t addr : 24;
                enum FTFL_FCMD {
                        FTFL_FCMD_READ_1s_BLOCK = 0x00,
                        FTFL_FCMD_READ_1s_SECTION = 0x01,
                        FTFL_FCMD_PROGRAM_CHECK = 0x02,
                        FTFL_FCMD_READ_RESOURCE = 0x03,
                        FTFL_FCMD_PROGRAM_LONGWORD = 0x06,
                        FTFL_FCMD_ERASE_BLOCK = 0x08,
                        FTFL_FCMD_ERASE_SECTOR = 0x09,
                        FTFL_FCMD_PROGRAM_SECTION = 0x0b,
                        FTFL_FCMD_READ_1s_ALL_BLOCKS = 0x40,
                        FTFL_FCMD_READ_ONCE = 0x41,
                        FTFL_FCMD_PROGRAM_ONCE = 0x43,
                        FTFL_FCMD_ERASE_ALL_BLOCKS = 0x44,
                        FTFL_FCMD_VERIFY_KEY = 0x45,
                        FTFL_FCMD_PROGRAM_PARTITION = 0x80,
                        FTFL_FCMD_SET_FLEXRAM = 0x81
                } fcmd : 8;
                uint8_t data_be[8];
        } __packed generic;
        struct {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
                uint8_t _rsvd0[3];
                enum FTFL_MARGIN_CHOICE {
                        FTFL_MARGIN_NORMAL = 0x00,
                        FTFL_MARGIN_USER = 0x01,
                        FTFL_MARGIN_FACTORY = 0x02
                } margin : 8;
        } __packed read_1s_block;
        struct ftfl_data_num_words {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
                uint8_t _rsvd0;
                enum FTFL_MARGIN_CHOICE margin : 8;
                uint16_t num_words;
        } __packed read_1s_section;
        struct {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
                uint8_t _rsvd0[3];
                enum FTFL_MARGIN_CHOICE margin : 8;
                uint8_t data_be[4];
        } __packed program_check;
        struct {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
                uint32_t data;
                uint8_t _rsvd0[3];
                enum FTFL_RESOURCE_SELECT {
                        FTFL_RESOURCE_IFR = 0x00,
                        FTFL_RESOURCE_VERSION = 0x01
                } resource_select : 8;
        } __packed read_resource;
        struct {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
                uint8_t data_be[4];
        } __packed program_longword;
        struct {
                uint32_t addr : 24;
                enum FTFL_FCMD fcmd : 8;
        } __packed erase;
        struct ftfl_data_num_words program_section;
        struct {
                uint8_t _rsvd0[2];
                enum FTFL_MARGIN_CHOICE margin : 8;
                enum FTFL_FCMD fcmd : 8;
        } __packed read_1s_all_blocks;
        struct ftfl_cmd_once {
                uint8_t _rsvd0[2];
                uint8_t idx;
                enum FTFL_FCMD fcmd : 8;
                uint8_t data_be[4];
        } __packed read_once;
        struct ftfl_cmd_once program_once;
        struct {
                uint8_t _rsvd0[3];
                enum FTFL_FCMD fcmd : 8;
        } __packed erase_all;
        struct {
                uint8_t _rsvd0[3];
                enum FTFL_FCMD fcmd : 8;
                uint8_t key_be[8];
        } __packed verify_key;
        struct {
                uint8_t _rsvd0[3];
                enum FTFL_FCMD fcmd : 8;
                uint8_t _rsvd1[2];
                enum FTFL_FLEXNVM_PARTITION {
                        FTFL_FLEXNVM_DATA_32_EEPROM_0 = 0x00,
                        FTFL_FLEXNVM_DATA_24_EEPROM_8 = 0x01,
                        FTFL_FLEXNVM_DATA_16_EEPROM_16 = 0x02,
                        FTFL_FLEXNVM_DATA_8_EEPROM_24 = 0x05,
                        FTFL_FLEXNVM_DATA_0_EEPROM_32 = 0x03
                } flexnvm_partition : 8;
                enum FTFL_EEPROM_SIZE {
                        FTFL_EEPROM_SIZE_0 = 0x3f,
                        FTFL_EEPROM_SIZE_32 = 0x39,
                        FTFL_EEPROM_SIZE_64 = 0x38,
                        FTFL_EEPROM_SIZE_128 = 0x37,
                        FTFL_EEPROM_SIZE_256 = 0x36,
                        FTFL_EEPROM_SIZE_512 = 0x35,
                        FTFL_EEPROM_SIZE_1024 = 0x34,
                        FTFL_EEPROM_SIZE_2048 = 0x33
                } eeprom_size : 8;
        } __packed program_partition;
        struct {
                uint8_t _rsvd0[2];
                enum FTFL_FLEXRAM_FUNCTION {
                        FTFL_FLEXRAM_EEPROM = 0x00,
                        FTFL_FLEXRAM_RAM = 0xff
                } flexram_function : 8;
                enum FTFL_FCMD fcmd : 8;
        } __packed set_flexram;
} __packed;

struct FTFL_t {
        struct FTFL_FSTAT_t fstat;
        struct FTFL_FCNFG_t fcnfg;
        struct FTFL_FSEC_t fsec;
        uint8_t fopt;
        union FTFL_FCCOB_t fccob;
        uint8_t fprot_be[4];
        uint8_t feprot;
        uint8_t fdprot;
} __packed;

struct FTFL_CONFIG_t {
        uint8_t key[8];
        uint8_t fprot[4];
        struct FTFL_FSEC_t fsec;
        uint8_t fopt;
        uint8_t feprot;
        uint8_t fdprot;
};

extern volatile struct FTFL_t FTFL;
