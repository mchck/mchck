#include <mchck.h>

struct GPIO_t {
        uint32_t pdor;
        uint32_t psor;
        uint32_t pcor;
        uint32_t ptor;
        uint32_t pdir;
        uint32_t pddr;
} ;
CTASSERT_SIZE_BYTE(struct GPIO_t, 24);

extern volatile struct GPIO_t GPIOA;
extern volatile struct GPIO_t GPIOB;
extern volatile struct GPIO_t GPIOC;
extern volatile struct GPIO_t GPIOD;
extern volatile struct GPIO_t GPIOE;
