struct VREF_TRM_t {
        UNION_STRUCT_START(8);
        uint8_t trim : 6;
        uint8_t chopen : 1;
        uint8_t _rsvd : 1;
        UNION_STRUCT_END;
};

struct VREF_SC_t {
        UNION_STRUCT_START(8);
        uint8_t mode_lv : 2;
        uint8_t vrefst : 1;
        uint8_t _rsvd : 2;
        uint8_t icompen : 1;
        uint8_t regen : 1;
        uint8_t vrefen : 1;
        UNION_STRUCT_END;
};

struct VREF {
        struct VREF_TRM_t trm;
        struct VREF_SC_t sc;
};

extern volatile struct VREF VREF;

