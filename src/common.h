//
// Created by Vitriol1744 on 31.08.2022.
//

#ifndef NES_EMULATOR_COMMON_H
#define NES_EMULATOR_COMMON_H

#include <stdint.h>

#define BIT(n) (1 << n)
#define CONSTEXPR(n, val) enum { n = val }

typedef uint8_t  byte_t;
typedef uint16_t word_t;

#endif //NES_EMULATOR_COMMON_H
