#include <mchck.h>

struct MCG_t {
        union {
                struct {
                        uint8_t irefsten : 1;
                        uint8_t irclken : 1;
                        uint8_t irefs : 1;
                        uint8_t frdiv : 3;
                        enum {
                                MCG_CLKS_FLLPLL = 0,
                                MCG_CLKS_INTERNAL = 1,
                                MCG_CLKS_EXTERNAL = 2
                        } clks : 2;
                } __packed;
                uint8_t c1;
        };
        union {
                struct {
                        enum {
                                MCG_IRCS_SLOW = 0,
                                MCG_IRCS_FAST = 1
                        } ircs : 1;
                        uint8_t lp : 1;
                        enum {
                                MCG_EREF_CLOCK = 0,
                                MCG_EREF_OSC = 1
                        } erefs0 : 1;
                        uint8_t hgo0 : 1;
                        enum {
                                MCG_RANGE_LOW = 0,
                                MCG_RANGE_HIGH = 1,
                                MCG_RANGE_VERYHIGH = 2
                        } range0 : 2;
                        uint8_t _rsvd0 : 1;
                        enum mcg_lore {
                                MCG_LORE_INTR = 0,
                                MCG_LORE_RESET = 1
                        } locre0 : 1;
                } __packed;
                uint8_t c2;
        };
        uint8_t sctrim;
        union {
                struct {
                        uint8_t scftrim : 1;
                        uint8_t fctrim : 4;
                        enum {
                                MCG_DRST_DRS_LOW = 0,
                                MCG_DRST_DRS_MID = 1,
                                MCG_DRST_DRS_MIDHIGH = 2,
                                MCG_DRST_DRS_HIGH = 3
                        } drst_drs : 2;
                        uint8_t dmx32 : 1;
                } __packed;
                uint8_t c4;
        };
        union {
                struct {
                        uint8_t prdiv0 : 5;
                        uint8_t pllsten0 : 1;
                        uint8_t pllclken0 : 1;
                        uint8_t _rsvd1 : 1;
                } __packed;
                uint8_t c5;
        };
        union {
                struct {
                        uint8_t vdiv0 : 5;
                        uint8_t cme0 : 1;
                        enum {
                                MCG_PLLS_FLL = 0,
                                MCG_PLLS_PLL = 1
                        } plls : 1;
                        uint8_t lolie0 : 1;
                } __packed;
                uint8_t c6;
        };
        union {
                struct {
                        enum {
                                MCG_IRCST_SLOW = 0,
                                MCG_IRCST_FAST = 1
                        } ircst : 1;
                        uint8_t oscinit0 : 1;
                        enum {
                                MCG_CLKST_FLL = 0,
                                MCG_CLKST_INTERNAL = 1,
                                MCG_CLKST_EXTERNAL = 2,
                                MCG_CLKST_PLL = 3
                        } clkst : 2;
                        enum {
                                MCG_IREFST_EXTERNAL = 0,
                                MCG_IREFST_INTERNAL = 1
                        } irefst : 1;
                        enum {
                                MCG_PLLST_FLL = 0,
                                MCG_PLLST_PLL = 1
                        } pllst : 1;
                        uint8_t lock0 : 1;
                        uint8_t lols0 : 1;
                } __packed;
                uint8_t s;
        };
        uint8_t _rsvd2;
        union {
                struct {
                        uint8_t locs0 : 1;
                        uint8_t fcrdiv : 3;
                        uint8_t fltprsrv : 1;
                        uint8_t atmf : 1;
                        uint8_t atms : 1;
                        uint8_t atme : 1;
                } __packed;
                uint8_t sc;
        };
        uint8_t _rsvd3;
        uint8_t atcvh;
        uint8_t atcvl;
        union {
                struct {
                        enum {
                                MCG_OSCSEL_OSCCLK = 0,
                                MCG_OSCSEL_RTC = 1
                        } oscsel : 1;
                        uint8_t _rsvd4 : 7;
                } __packed;
                uint8_t c7;
        };
        union {
                struct {
                        uint8_t locs1 : 1;
                        uint8_t _rsvd5 : 4;
                        uint8_t cme1 : 1;
                        enum mcg_lore lolre : 1;
                        enum mcg_lore locre1 : 1;
                } __packed;
                uint8_t c8;
        };
} __packed;
CTASSERT_SIZE_BYTE(struct MCG_t, 0xe);

extern volatile struct MCG_t MCG;
