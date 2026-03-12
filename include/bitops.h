#pragma once
#include <stdint.h>

#define BIT(n)              (1u << (n))
#define SET_BIT(reg, n)     ((reg) |= BIT(n))
#define CLR_BIT(reg, n)     ((reg) &= (uint8_t)~BIT(n))
#define TOG_BIT(reg, n)     ((reg) ^= BIT(n))
#define READ_BIT(reg, n)    (((reg) >> (n)) & 1u)