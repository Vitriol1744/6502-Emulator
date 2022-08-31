//
// Created by Vitriol1744 on 31.08.2022.
//

#ifndef NES_EMULATOR_OPCODES_H
#define NES_EMULATOR_OPCODES_H

#include "cpu6502.h"

void cpu6502_ADC(cpu6502_t*, byte_t);
void cpu6502_AND(cpu6502_t*, byte_t);
void cpu6502_CMP(cpu6502_t*, byte_t, byte_t);
void cpu6502_DEC(cpu6502_t*, word_t);
void cpu6502_EOR(cpu6502_t*, byte_t);
void cpu6502_INC(cpu6502_t*, word_t);
void cpu6502_LD(cpu6502_t*, byte_t*, byte_t);

#endif //NES_EMULATOR_OPCODES_H
