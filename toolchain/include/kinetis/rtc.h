struct RTC {
        uint32_t tsr;
        uint16_t tpr;
        uint16_t _pad0;
        uint32_t tar;
        uint8_t tcr;
        uint8_t cir;
        uint8_t tcv;
        uint8_t cic;
        struct RTC_CR {
                UNION_STRUCT_START(32);
                unsigned swr       : 1;
                unsigned wpe       : 1;
                unsigned sup       : 1;
                unsigned um        : 1;
                unsigned _rsvd0    : 4;
                unsigned osce      : 1;
                unsigned clko      : 1;
                unsigned sc16p     : 1;
                unsigned sc8p      : 1;
                unsigned sc4p      : 1;
                unsigned sc2p      : 1;
                unsigned _rsvd1    : 18;
                UNION_STRUCT_END;
        } cr;
        struct RTC_SR {
                UNION_STRUCT_START(32);
                unsigned tif       : 1;
                unsigned tof       : 1;
                unsigned taf       : 1;
                unsigned _rsvd0    : 1;
                unsigned tce       : 1;
                unsigned _rsvd1    : 27;
                UNION_STRUCT_END;
        } sr;
        struct RTC_LR {
                UNION_STRUCT_START(32);
                unsigned _rsvd0    : 3;
                unsigned tcl       : 1;
                unsigned crl       : 1;
                unsigned srl       : 1;
                unsigned lrl       : 1;
                unsigned _rsvd1    : 25;
                UNION_STRUCT_END;
        } lr;
        struct RTC_IER {
                UNION_STRUCT_START(32);
                unsigned tiie      : 1;
                unsigned toie      : 1;
                unsigned taie      : 1;
                unsigned _rsvd0    : 1;
                unsigned tsie      : 1;
                unsigned _rsvd1    : 3;
                unsigned _rsvd2    : 24;
                UNION_STRUCT_END;
        } ier;
        uint32_t _pad1[(0x800 - 0x1c) / 4 - 1];
        struct RTC_WAR {
                UNION_STRUCT_START(32);
                unsigned tsrw      : 1;
                unsigned tprw      : 1;
                unsigned tarw      : 1;
                unsigned tcrw      : 1;
                unsigned crw       : 1;
                unsigned srw       : 1;
                unsigned lrw       : 1;
                unsigned ierw      : 1;
                unsigned _rsvd0    : 24;
                UNION_STRUCT_END;
        } war;
        struct RTC_RAR {
                UNION_STRUCT_START(32);
                unsigned tsrr      : 1;
                unsigned tprr      : 1;
                unsigned tarr      : 1;
                unsigned tcrr      : 1;
                unsigned crr       : 1;
                unsigned srr       : 1;
                unsigned lrr       : 1;
                unsigned ierr      : 1;
                unsigned _rsvd0    : 24;
                UNION_STRUCT_END;
        } rar;
};
CTASSERT_SIZE_BYTE(struct RTC, 0x804+4);

extern volatile struct RTC RTC;
