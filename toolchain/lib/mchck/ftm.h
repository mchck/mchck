#define FTM_NUM_CH 8

enum FTM_CH_t {
    FTM_CH0 = 0x01,
    FTM_CH1 = 0x02,
    FTM_CH2 = 0x04,
    FTM_CH3 = 0x08,
    FTM_CH4 = 0x10,
    FTM_CH5 = 0x20,
    FTM_CH6 = 0x40,
    FTM_CH7 = 0x80,

    FTM_CH_ALL = 0xFF,
} ;

void ftm_init();
void ftm_set_raw(enum FTM_CH_t channels, uint16_t duty);

static inline void
ftm_set(enum FTM_CH_t channels, fract duty)
{
    ftm_set_raw(channels, duty * FTM0.mod);
}
