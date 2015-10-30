struct SPI {
        struct SPI_MCR {
                UNION_STRUCT_START(32);
                unsigned halt      : 1;
                unsigned _rsvd0    : 7;
                enum {
                        SPI_SMPL_PT_0 = 0b00,
                        SPI_SMPL_PT_1 = 0b01,
                        SPI_SMPL_PT_2 = 0b10
                } smpl_pt          : 2;
                unsigned clr_rxf   : 1;
                unsigned clr_txf   : 1;
                unsigned dis_rxf   : 1;
                unsigned dis_txf   : 1;
                unsigned mdis      : 1;
                unsigned doze      : 1;
                unsigned pcsis     : 5;
                unsigned _rsvd1    : 3;
                unsigned rooe      : 1;
                unsigned _rsvd2    : 1;
                unsigned mtfe      : 1;
                unsigned frz       : 1;
                enum {
                        SPI_DCONF_SPI = 0b00
                } dconf            : 2;
                unsigned cont_scke : 1;
                unsigned mstr      : 1;
                UNION_STRUCT_END;
        } mcr;
        uint32_t _pad0;
        uint16_t _pad1;
        uint16_t tcr;
        union {
                struct SPI_CTAR {
                        UNION_STRUCT_START(32);
                        unsigned br     : 4;
                        unsigned dt     : 4;
                        unsigned asc    : 4;
                        unsigned cssck  : 4;
                        unsigned pbr    : 2;
                        unsigned pdt    : 2;
                        unsigned pasc   : 2;
                        unsigned pcssck : 2;
                        unsigned lsbfe  : 1;
                        unsigned cpha   : 1;
                        unsigned cpol   : 1;
                        unsigned fmsz   : 4;
                        unsigned dbr    : 1;
                        UNION_STRUCT_END;
                } ctar[2];
                struct SPI_CTAR_SLAVE {
                        UNION_STRUCT_START(32);
                        unsigned _rsvd0 : 24;
                        unsigned cpha   : 1;
                        unsigned cpol   : 1;
                        unsigned fmsz   : 5;
                        UNION_STRUCT_END;
                } ctar_slave;
        };
        uint32_t _pad2[(0x2c - 0x10)/4 - 1];
        struct SPI_SR {
                UNION_STRUCT_START(32);
                unsigned popnxtpr : 4;
                unsigned rxctr    : 4;
                unsigned txnxtptr : 4;
                unsigned txctr    : 4;
                unsigned _rsvd0   : 1;
                unsigned rfdf     : 1;
                unsigned _rsvd1   : 1;
                unsigned rfof     : 1;
                unsigned _rsvd2   : 5;
                unsigned tfff     : 1;
                unsigned _rsvd3   : 1;
                unsigned tfuf     : 1;
                unsigned eoqf     : 1;
                unsigned _rsvd4   : 1;
                unsigned txrxs    : 1;
                unsigned tcf      : 1;
                UNION_STRUCT_END;
        } sr;
        struct SPI_RSER {
                UNION_STRUCT_START(32);
                unsigned _rsvd0    : 16;
                unsigned rfdf_dirs : 1;
                unsigned rfdf_re   : 1;
                unsigned _rsvd1    : 1;
                unsigned rfof_re   : 1;
                unsigned _rsvd2    : 4;
                unsigned tfff_dirs : 1;
                unsigned tfff_re   : 1;
                unsigned _rsvd3    : 1;
                unsigned tfuf_re   : 1;
                unsigned eoqf_re   : 1;
                unsigned _rsvd4    : 2;
                unsigned tcf_re    : 1;
                UNION_STRUCT_END;
        } rser;
        struct SPI_PUSHR {
                UNION_STRUCT_START(32);
                unsigned txdata : 16;
                unsigned pcs    : 6;
                unsigned _rsvd0 : 4;
                unsigned ctcnt  : 1;
                unsigned eoq    : 1;
                unsigned ctas   : 3;
                unsigned cont   : 1;
                UNION_STRUCT_END;
        } pushr;
        const uint32_t popr;
        const struct SPI_PUSHR txfr[4];
        uint32_t _pad3[(0x7c - 0x48) / 4 - 1];
        const uint32_t rxfr[4];
};
CTASSERT_SIZE_BYTE(struct SPI, 0x88+4);

extern volatile struct SPI SPI0;
