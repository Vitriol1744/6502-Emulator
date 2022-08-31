//
// Created by Vitriol1744 on 30.08.2022.
//

#ifndef NES_EMULATOR_CPU6502_H
#define NES_EMULATOR_CPU6502_H

#include <stdint.h>

#include "bus.h"

CONSTEXPR(CPU_FLAG_C, BIT(0));
CONSTEXPR(CPU_FLAG_Z, BIT(1));
CONSTEXPR(CPU_FLAG_I, BIT(2));
CONSTEXPR(CPU_FLAG_D, BIT(3));
CONSTEXPR(CPU_FLAG_B, BIT(4));
CONSTEXPR(CPU_FLAG_U, BIT(5));
CONSTEXPR(CPU_FLAG_V, BIT(6));
CONSTEXPR(CPU_FLAG_N, BIT(7));

typedef struct
{
    // CPU Registers
    byte_t A;    // Accumulator
    byte_t X, Y; // Indices

    byte_t S;    // Stack Pointer
    word_t PC;  // Program Counter

    // Status flag

    byte_t C: 1; // Carry Flag
    byte_t Z: 1; // Zero
    byte_t I: 1; // Interrupt Disable
    byte_t D: 1; // Decimal Mode
    byte_t B: 1; // Break
    byte_t U: 1; // Unused
    byte_t V: 1; // Overflow
    byte_t N: 1; // Negative

    uint32_t counter;
    bus_t* bus;
} cpu6502_t;

void cpu6502_ConnectBus(cpu6502_t*, bus_t*);
void cpu6502_Reset(cpu6502_t*);
void cpu6502_Run(cpu6502_t*);

#endif // NES_EMULATOR_CPU6502_H
