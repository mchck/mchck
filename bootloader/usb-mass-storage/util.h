#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

uint32_t betoh32(uint32_t *);
uint16_t betoh16(uint16_t *);

void htobe32(uint32_t, uint32_t *);
void htobe16(uint16_t, uint16_t *);

#endif
