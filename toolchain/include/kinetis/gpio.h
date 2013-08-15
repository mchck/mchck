#include <mchck.h>

struct GPIO_t {
        uint32_t pdor;
        uint32_t psor;
        uint32_t pcor;
        uint32_t ptor;
        uint32_t pdir;
        uint32_t pddr;
} ;
_Static_assert(sizeof(struct GPIO_t) == 24, "Size assertion failed");

extern volatile struct GPIO_t GPIOA;
extern volatile struct GPIO_t GPIOB;
extern volatile struct GPIO_t GPIOC;
extern volatile struct GPIO_t GPIOD;
extern volatile struct GPIO_t GPIOE;
