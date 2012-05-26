#ifndef __MCHCK_INTERNAL_H
#define __MCHCK_INTERNAL_H

#ifndef __XTAL
#define __XTAL  8000000L
#endif

#ifndef HSE_VALUE
#define HSE_VALUE       __XTAL
#endif

#define STM32L151C8
#define STM32L1XX_MD
#define __STM32L1XX_MD

#define assert_param(x) do {} while (0)

#endif
