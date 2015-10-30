union VBAT {
        uint8_t  b[32];
        uint16_t s[16];
        uint32_t l[8];
};
CTASSERT_SIZE_BYTE(union VBAT, 32);

extern union VBAT VBAT;
