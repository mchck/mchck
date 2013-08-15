struct SCB_t {
        struct SCB_CPUID_t {
                UNION_STRUCT_START(32);
                uint32_t revision : 4;
                uint32_t partno : 12;
                uint32_t _rsvd0 : 4;
                uint32_t variant : 4;
                uint32_t implementer : 8;
                UNION_STRUCT_END;
        } cpuid;
        struct SCB_ICSR_t {
                UNION_STRUCT_START(32);
                const uint32_t vectactive : 9;
                const uint32_t _rsvd0 : 2;
                const uint32_t rettobase : 1;
                const uint32_t vectpending : 9;
                const uint32_t _rsvd1 : 1;
                const uint32_t isrpending : 1;
                const uint32_t isrpreempt : 1;
                const uint32_t _rsvd2 : 1;
                uint32_t pendstclr : 1;
                uint32_t pendstset : 1;
                uint32_t pendsvclr : 1;
                uint32_t pendsvset : 1;
                const uint32_t _rsvd3 : 2;
                uint32_t nmipendset : 1;
                UNION_STRUCT_END;
        } icsr;
        uint32_t vtor;
        struct SCB_AIRCR_t {
                UNION_STRUCT_START(32);
                uint32_t vectreset : 1;
                uint32_t vectclractive : 1;
                uint32_t sysresetreq : 1;
                const uint32_t _rsvd0 : 5;
                uint32_t prigroup : 3;
                const uint32_t _rsvd1 : 4;
                const uint32_t endianess : 1;
                enum {
                        SCB_AIRCR_VECTKEY = 0x05fa
                } vectkey;
                UNION_STRUCT_END;
        } aircr;
        struct SCB_SCR_t {
                UNION_STRUCT_START(32);
                const uint32_t _rsvd0 : 1;
                uint32_t sleeponexit : 1;
                uint32_t sleepdeep : 1;
                const uint32_t _rsvd1 : 1;
                uint32_t sevonpend : 1;
                const uint32_t _rsvd2 : 23;
                UNION_STRUCT_END;
        } scr;
        struct SCB_CCR_t {
                UNION_STRUCT_START(32);
                uint32_t nonbasethrdena : 1;
                uint32_t usersetmpend : 1;
                const uint32_t _rsvd0 : 1;
                uint32_t unalign_trp : 1;
                uint32_t div_0_trp : 1;
                const uint32_t _rsvd1 : 3;
                uint32_t bfhfnmign : 1;
                uint32_t stkalign : 1;
                const uint32_t _rsvd2 : 22;
                UNION_STRUCT_END;
        } ccr;
        struct SCB_SHPR1_t {
                UNION_STRUCT_START(32);
                uint32_t pri_memmanage : 8;
                uint32_t pri_busfault : 8;
                uint32_t pri_usagefault : 8;
                uint32_t pri_7 : 8;
                UNION_STRUCT_END;
        } shpr1;
        struct SCB_SHPR2_t {
                UNION_STRUCT_START(32);
                uint32_t pri_8 : 8;
                uint32_t pri_9 : 8;
                uint32_t pri_10 : 8;
                uint32_t pri_svcall : 8;
                UNION_STRUCT_END;
        } shpr2;
        struct SCB_SHPR3_t {
                UNION_STRUCT_START(32);
                uint32_t pri_debugmonitor : 8;
                uint32_t pri_13 : 8;
                uint32_t pri_pendsv : 8;
                uint32_t pri_systick : 8;
                UNION_STRUCT_END;
        } shpr3;
        struct SCB_SHCSR_t {
                UNION_STRUCT_START(32);
                uint32_t memfaultact : 1;
                uint32_t busfaultact : 1;
                uint32_t usgfaultact : 1;
                uint32_t svcallact : 1;
                uint32_t monitoract : 1;
                uint32_t pendsvact : 1;
                uint32_t systickact : 1;
                uint32_t usgfaultpended : 1;
                uint32_t memfaultpended : 1;
                uint32_t busfaultpended : 1;
                uint32_t svcallpended : 1;
                uint32_t memfaultena : 1;
                uint32_t busfaultena : 1;
                uint32_t usgfaultena : 1;
                const uint32_t _rsvd0 : 13;
                UNION_STRUCT_END;
        } shcsr;
        struct SCB_CFSR_t {
                UNION_STRUCT_START(32);
                struct SCB_MMFSR_t {
                        UNION_STRUCT_START(8);
                        uint8_t iaccviol : 1;
                        uint8_t daccviol : 1;
                        const uint8_t _rsvd0 : 1;
                        uint8_t munstkerr : 1;
                        uint8_t mstkerr : 1;
                        const uint8_t _rsvd1 : 2;
                        uint8_t mmarvalid : 1;
                        UNION_STRUCT_END;
                } mmfsr;
                struct SCB_BFSR_t {
                        UNION_STRUCT_START(8);
                        uint8_t ibuserr : 1;
                        uint8_t preciserr : 1;
                        uint8_t impreciserr : 1;
                        uint8_t unstkerr : 1;
                        uint8_t stkerr : 1;
                        const uint8_t _rsvd0 : 2;
                        uint8_t bfarvalid : 1;
                        UNION_STRUCT_END;
                } bfsr;
                struct SCB_UFSR_t {
                        UNION_STRUCT_START(16);
                        uint16_t undefinstr : 1;
                        uint16_t invstate : 1;
                        uint16_t invpc : 1;
                        uint16_t nocp : 1;
                        const uint16_t _rsvd0 : 4;
                        uint16_t unaligned : 1;
                        uint16_t divbyzero : 1;
                        const uint16_t _rsvd1 : 6;
                        UNION_STRUCT_END;
                } ufsr;
                UNION_STRUCT_END;
        } cfsr;
        struct SCB_HFSR_t {
                UNION_STRUCT_START(32);
                const uint32_t _rsvd0 : 1;
                uint32_t vecttbl : 1;
                const uint32_t _rsvd1 : 28;
                uint32_t forced : 1;
                uint32_t debugevt : 1;
                UNION_STRUCT_END;
        } hfsr;
        uint32_t dfsr;
        uint32_t mmfar;
        uint32_t bfar;
        uint32_t afsr;
        const uint32_t _pad0[(0xd88-0xd3c)/4-1];
        uint32_t cpacr;
        const uint32_t _pad1[(0xf00-0xd88)/4-1];
        uint32_t stir;
};

extern volatile struct SCB_t SCB;
