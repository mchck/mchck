struct NVIC {
        uint32_t iser[32];
        uint32_t icer[32];
        uint32_t ispr[32];
        uint32_t icpr[32];
        uint32_t iabr[32];
        uint8_t ipr[1024];
};

extern volatile struct NVIC NVIC;
