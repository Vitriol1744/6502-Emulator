//
// Created by Vitriol1744 on 31.08.2022.
//
#include "opcodes.h"

void cpu6502_SetZNFlags(cpu6502_t*, word_t);
byte_t cpu6502_ReadByte(cpu6502_t*, word_t);
void cpu6502_WriteByte(cpu6502_t*, byte_t, word_t);

void cpu6502_ADC(cpu6502_t* this, byte_t M)
{
    word_t value = this->A + M + this->C;

    this->C = value > 255;
    cpu6502_SetZNFlags(this, value);
    this->V = (~((word_t)this->A ^ (word_t)M) & ((word_t)this->A ^ (word_t)value)) & 0x0080;
    this->A = value & 0x00FF;
}
void cpu6502_AND(cpu6502_t* this, byte_t M)
{
    word_t val = this->A & M;
    cpu6502_SetZNFlags(this, val);
    this->A = val;
}
void cpu6502_CMP(cpu6502_t* this, byte_t R, byte_t M)
{
    word_t result = R - M;
    cpu6502_SetZNFlags(this, result);
    this->C = result > 255;
}
void cpu6502_DEC(cpu6502_t* this, word_t address)
{
    word_t value = cpu6502_ReadByte(this, address) - 1;
    cpu6502_WriteByte(this, value, address);
    cpu6502_SetZNFlags(this, value);
}
void cpu6502_EOR(cpu6502_t* this, byte_t M)
{
    word_t value = this->A ^ M;
    cpu6502_SetZNFlags(this, value);
    this->A = value;
}
void cpu6502_INC(cpu6502_t* this, word_t address)
{
    word_t value = cpu6502_ReadByte(this, address) + 1;
    cpu6502_WriteByte(this, value, address);
    cpu6502_SetZNFlags(this, value);
}
void cpu6502_LD(cpu6502_t* this, byte_t* dst, byte_t src)
{
    *dst = src;
    cpu6502_SetZNFlags(this, *dst);
}