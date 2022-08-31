//
// Created by Vitriol1744 on 30.08.2022.
//
#include "opcodes.h"
#include <stdio.h>

typedef word_t opcode_t;

static const byte_t cycles[256] =
{
    7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    2,5,2,5,4,4,4,4,2,4,2,5,4,4,4,4,
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7,
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    2,5,2,8,4,4,6,6,2,4,2,7,5,5,7,7
};

byte_t cpu6502_ReadByte(cpu6502_t*, word_t);
byte_t cpu6502_FetchByte(cpu6502_t*);
void cpu6502_Execute(cpu6502_t*, opcode_t);
void cpu6502_SetZNFlags(cpu6502_t*, word_t);

void cpu6502_ConnectBus(cpu6502_t* this, bus_t* bus)
{
    this->bus = bus;
}
void cpu6502_Reset(cpu6502_t* this)
{
    this->A = 0;

    this->X = 0;
    this->Y = 0;

    uint16_t PC_lo = cpu6502_ReadByte(this, 0xFFFC);
    uint16_t PC_hi = cpu6502_ReadByte(this, 0xFFFD);

    this->S = 0xFD;
    this->PC = (PC_hi << 8) | PC_lo;
    this->PC = 0x100;

    this->C = this->I = this->D = this->B = this->V = this->N;
    this->Z = this->U = 1;

    this->counter = 8;
}

void cpu6502_Run(cpu6502_t* this)
{
    register opcode_t opcode = 0x00;
    for (;;)
    {
        opcode = cpu6502_FetchByte(this);

        cpu6502_Execute(this, opcode);
        this->counter -= cycles[opcode];

        if (this->counter <= 0)
        {
            this->counter = 8;
        }
    }
}

byte_t cpu6502_ReadByte(cpu6502_t* this, word_t address)
{
    byte_t ret = bus_ReadByte(this->bus, address);

    return ret;
}
void cpu6502_WriteByte(cpu6502_t* this, byte_t data, word_t address)
{
    bus_WriteByte(this->bus, data, address);
}
void cpu6502_WriteWord(cpu6502_t* this, word_t data, word_t address)
{
    cpu6502_WriteByte(this, data & 0xFF, address);
    cpu6502_WriteByte(this, data >> 8, address + 1);
}
byte_t cpu6502_FetchByte(cpu6502_t* this)
{
    byte_t ret = cpu6502_ReadByte(this, this->PC++);

    return ret;
}
word_t cpu6502_FetchWord(cpu6502_t* this)
{
    word_t lo = cpu6502_FetchByte(this);
    word_t hi = cpu6502_FetchByte(this);

    return (hi << 8) | lo;
}
void cpu6502_Execute(cpu6502_t* this, opcode_t opcode)
{
    switch (opcode)
    {
        // ADC
        // Immediate Mode
        case 0x69:
        {
            word_t M = cpu6502_FetchByte(this);
            cpu6502_ADC(this, M);

            break;
        }
        // Zero Page
        case 0x65:
        {
            byte_t address = cpu6502_FetchByte(this);
            word_t M = cpu6502_ReadByte(this, address);
            cpu6502_ADC(this, M);

            break;
        }
        // Zero Page, X
        case 0x75:
        {
            byte_t address = cpu6502_FetchByte(this) + this->X;
            word_t M = cpu6502_ReadByte(this, address);
            cpu6502_ADC(this, M);

            break;
        }
        // Absolute
        case 0x6D:
        {
            word_t address = cpu6502_FetchWord(this);
            word_t M = cpu6502_ReadByte(this, address);
            cpu6502_ADC(this, M);

            break;
        }
        // Absolute, X
        case 0x7D:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            word_t M = cpu6502_ReadByte(this, address);
            cpu6502_ADC(this, M);

            break;
        }
        // Absolute, Y
        case 0x79:
        {
            word_t address = cpu6502_FetchWord(this) + this->Y;
            word_t M = cpu6502_ReadByte(this, address);
            cpu6502_ADC(this, M);

            break;
        }
        // Indirect, X
        case 0x61:
        // Indirect, Y
        case 0x71: break;
        // AND
        // Immediate Mode
        case 0x29:
        {
            cpu6502_AND(this, cpu6502_FetchByte(this));
            break;
        }
        // Zero Page
        case 0x25:
        {
            cpu6502_AND(this, cpu6502_ReadByte(this, cpu6502_FetchByte(this)));
            break;
        }
        // Zero Page, X
        case 0x35:
        {
            cpu6502_AND(cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X));
            break;
        }
        // Absolute
        case 0x2D:
        {
            word_t address = cpu6502_FetchWord(this);
            cpu6502_AND(this, cpu6502_ReadByte(this, address));
            break;
        }
        // Absolute, X
        case 0x3D:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            cpu6502_AND(this, cpu6502_ReadByte(this, address));
            break;
        }
        // Absolute, Y
        case 0x39:
        {
            word_t address = cpu6502_FetchWord(this) + this->Y;
            cpu6502_AND(this, cpu6502_ReadByte(this, address));
            break;
        }
        // Indirect, X
        case 0x21:
        {
            word_t ptr = cpu6502_FetchByte(this);

            word_t lo = cpu6502_ReadByte(this, ptr + this->X & 0x00FF);
            word_t hi = cpu6502_ReadByte(this, ptr + this->X + 1) & 0x00FF;

            cpu6502_AND(this, cpu6502_ReadByte(this, (hi << 8) | lo));
            break;
        }
        // Indirect, Y
        case 0x31:
        {
            word_t ptr = cpu6502_FetchByte(this);

            word_t lo = cpu6502_ReadByte(this, ptr + this->Y & 0x00FF);
            word_t hi = cpu6502_ReadByte(this, ptr + this->Y + 1) & 0x00FF;

            cpu6502_AND(this, cpu6502_ReadByte(this, (hi << 8) | lo));
            break;
        }

        // ASL
        // Immediate Mode
        case 0x0A:
        {
            this->A <<= 1;
            //TODO: Check Status Flag

            break;
        }
        case 0x06:
        case 0x16:
        case 0x0E:
        case 0x1E:

        // BCC
        case 0x90: if (this->C == 0) this->PC += cpu6502_FetchByte(this); break;

        // BCS
        case 0xB0: if (this->C != 0) this->PC += cpu6502_FetchByte(this); break;

        // BIT
        case 0x24:
        case 0x2C:

        // BMI
        case 0x30: if (this->N != 0) this->PC += cpu6502_FetchByte(this); break;

        // BNE
        case 0xD0: if (this->Z == 0) this->PC += cpu6502_FetchByte(this); break;

        // BPL
        case 0x10: if (this->N == 0) this->PC += cpu6502_FetchByte(this); break;

        // BRK
        case 0x00:

        // BVC
        case 0x50: if (this->V == 0) this->PC += cpu6502_FetchByte(this); break;

        // BVS
        case 0x70: if (this->V != 0) this->PC += cpu6502_FetchByte(this); break;

        // CLC
        case 0x18: this->C = 0; break;

        // CLD
        case 0xD8: this->D = 0; break;

        // CLI
        case 0x58: this->I = 0; break;

        // CLV
        case 0xB8: this->V = 0; break;

        // CMP
        // Immediate Mode
        case 0xC9:
        {
            byte_t M = cpu6502_FetchByte(this);
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Zero Page
        case 0xC5:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Zero Page, X
        case 0xD5:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X);
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Absolute
        case 0xCD:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Absolute, X
        case 0xDD:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X);
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Absolute, Y
        case 0xD9:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->Y);
            cpu6502_CMP(this, this->A, M);
            break;
        }
        // Indirect, X
        case 0xC1:
        {
            break;
        }
        // Indirect, Y
        case 0xD1:
        {
            break;
        }

        // CPX
        // Immediate Mode
        case 0xE0:
        {
            byte_t M = cpu6502_FetchByte(this);
            cpu6502_CMP(this, this->X, M);
            break;
        }
        // Zero Page
        case 0xE4:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            cpu6502_CMP(this, this->X, M);
            break;
        }
        // Absolute
        case 0xEC:
        {
            word_t M = cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            cpu6502_CMP(this, this->X, M);
            break;
        }

        // CPY
        case 0xC0:
        {
            byte_t M = cpu6502_FetchByte(this);
            cpu6502_CMP(this, this->Y, M);
            break;
        }
        case 0xC4:
        {
            byte_t M = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            cpu6502_CMP(this, this->Y, M);
            break;
        }
        case 0xCC:
        {
            word_t M = cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            cpu6502_CMP(this, this->Y, M);
            break;
        }

        // DEC
        // Zero Page
        case 0xC6:
        {
            word_t address = cpu6502_FetchByte(this);
            cpu6502_DEC(this, address);
            break;
        }
        // Zero Page, X
        case 0xD6:
        {
            word_t address = cpu6502_FetchByte(this) + this->X;
            cpu6502_DEC(this, address);
            break;
        }
        // Absolute
        case 0xCE:
        {
            word_t address = cpu6502_FetchWord(this);
            cpu6502_DEC(this, address);
            break;
        }
        // Absolute, X
        case 0xDE:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            cpu6502_DEC(this, address);
            break;
        }

        // DEX
        case 0xCA:
        {
            this->X--;
            cpu6502_SetZNFlags(this, this->X);

            break;
        }

        // DEY
        case 0x88:
        {
            this->Y--;
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }

        // EOR
        // Immediate Mode
        case 0x49:
        {
            cpu6502_EOR(this, cpu6502_FetchByte(this));
            break;
        }
        // Zero Page
        case 0x45:
        {
            cpu6502_EOR(this, cpu6502_ReadByte(this, cpu6502_FetchByte(this)));
            break;
        }
        // Zero Page, X
        case 0x55:
        {
            cpu6502_EOR(this, cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X));
            break;
        }
        // Absolute
        case 0x4D:
        {
            cpu6502_EOR(this, cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            break;
        }
        // Absolute, X
        case 0x5D:
        {
            cpu6502_EOR(this, cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X));
            break;
        }
        // Absolute, Y
        case 0x59:
        {
            cpu6502_EOR(this, cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->Y));
            break;
        }
        // Indirect, X
        case 0x41:
        {

        }
        // Indirect, Y
        case 0x51:
            break;

        // INC
        // Zero Page
        case 0xE6:
        {
            word_t address = cpu6502_FetchByte(this);
            cpu6502_INC(this, address);
            break;
        }
        // Zero Page, X
        case 0xF6:
        {
            word_t address = cpu6502_FetchByte(this) + this->X;
            cpu6502_INC(this, address);
            break;
        }
        // Absolute
        case 0xEE:
        {
            word_t address = cpu6502_FetchWord(this);
            cpu6502_INC(this, address);
            break;
        }
        // Absolute, X
        case 0xFE:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            cpu6502_INC(this, address);
            break;
        }

        // INX
        case 0xE8:
        {
            this->X++;
            cpu6502_SetZNFlags(this, this->X);

            break;
        }

        // INY
        case 0xC8:
        {
            this->Y++;
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }

        // JMP
        // Absolute
        case 0x4C:
        {
            byte_t lo = cpu6502_ReadByte(this, this->PC + 1);
            byte_t hi = cpu6502_ReadByte(this, this->PC + 2);
            this->PC = (hi << 8) | lo;

            break;
        }
        // Indirect
        case 0x6C:
        {
            word_t ptr_address = cpu6502_FetchWord(this);
            word_t address = (cpu6502_ReadByte(this, ptr_address + 1) << 8) | cpu6502_ReadByte(this, ptr_address);
            this->PC = (cpu6502_ReadByte(this, address + 1) << 8) | cpu6502_ReadByte(this, address);

            break;
        }

        // JSR
        case 0x20:
        {
            word_t subroutine_address = cpu6502_FetchWord(this);
            cpu6502_WriteWord(this, this->PC - 1, this->S++);

            //this->PC = subroutine_address;
            break;
        }
        // LDA
        // Immediate Mode
        case 0xA9:
        {
            printf("LDA\n");
            cpu6502_LD(this, &this->A, cpu6502_FetchByte(this));
            break;
        }
        // Zero Page
        case 0xA5:
        {
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, cpu6502_FetchByte(this)));
            break;
        }
        // Zero Page, X
        case 0xB5:
        {
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X));
            break;
        }
        // Absolute
        case 0xAD:
        {
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, cpu6502_FetchWord(this)));
            break;
        }
        // Absolute, X
        case 0xBD:
        {
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X));
            break;
        }
        // Absolute, Y
        case 0xB9:
        {
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->Y));
            break;
        }
        // Indirect, X
        case 0xA1:
        {
            word_t ptr_address = cpu6502_FetchWord(this) + this->X;
            word_t address = (cpu6502_ReadByte(this, ptr_address + 1) << 8) | cpu6502_ReadByte(this, ptr_address);
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, address));
            break;
        }
        // Indirect, Y
        case 0xB1:
        {
            word_t ptr_address = cpu6502_FetchWord(this) + this->Y;
            word_t address = (cpu6502_ReadByte(this, ptr_address + 1) << 8) | cpu6502_ReadByte(this, ptr_address);
            cpu6502_LD(this, &this->A, cpu6502_ReadByte(this, address));
            break;
        }

        // LDX
        // Immediate Mode
        case 0xA2:
        {
            cpu6502_LD(this, &this->X, cpu6502_FetchByte(this));
            break;
        }
        // Zero Page
        case 0xA6:
        {
            cpu6502_LD(this, &this->X, cpu6502_ReadByte(this, cpu6502_FetchByte(this)));
            break;
        }
        // Zero Page, Y
        case 0xB6:
        {
            cpu6502_LD(this, &this->X, cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->Y));
            break;
        }
        // Absolute
        case 0xAE:
        {
            cpu6502_LD(this, &this->X, cpu6502_ReadByte(this, cpu6502_FetchWord(this)));
            break;
        }
        // Absolute, Y
        case 0xBE:
        {
            cpu6502_LD(this, &this->X, cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->Y));
            break;
        }

        // LDY
        // Immediate Mode
        case 0xA0:
        {
            this->Y = cpu6502_FetchByte(this);
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }
        // Zero Page
        case 0xA4:
        {
            this->Y = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }
        // Zero Page, X
        case 0xB4:
        {
            this->Y = cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X);
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }
        // Absolute
        case 0xAC:
        {
            this->Y = cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }
        // Absolute, X
        case 0xBC:
        {
            this->Y = cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X);
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }

        // LSR
        // Accumulator
        case 0x4A:
        {
            word_t value = this->A >> 1;
            this->C = value > 255;
            this->Z = value == 0;
            this->A = value;

            break;
        }
        // Zero Page
        case 0x46:
        {
            word_t address = cpu6502_FetchByte(this);
            word_t value = cpu6502_ReadByte(this, address) >> 1;
            this->C = value > 255;
            this->Z = value == 0;
            cpu6502_WriteByte(this, value, address);

            break;
        }
        // Zero Page, X
        case 0x56:
        {
            word_t address = cpu6502_FetchByte(this) + this->X;
            word_t value = cpu6502_ReadByte(this, address) >> 1;
            this->C = value > 255;
            this->Z = value == 0;
            cpu6502_WriteByte(this, value, address);

            break;
        }
        // Absolute
        case 0x4E:
        {
            word_t address = cpu6502_FetchWord(this);
            word_t value = cpu6502_ReadByte(this, address) >> 1;
            this->C = value > 255;
            this->Z = value == 0;
            cpu6502_WriteByte(this, value, address);

            break;
        }
        // Absolute, X
        case 0x5E:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            word_t value = cpu6502_ReadByte(this, address) >> 1;
            this->C = value > 255;
            this->Z = value == 0;
            cpu6502_WriteByte(this, value, address);

            break;
        }

        // NOP
        case 0xEA: break;

        // ORA
        // Immediate
        case 0x09:
        {
            this->A |= cpu6502_FetchByte(this);
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Zero Page
        case 0x05:
        {
            this->A |= cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Zero Page, X
        case 0x15:
        {
            this->A |= cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X);
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Absolute
        case 0x0D:
        {
            this->A |= cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Absolute, X
        case 0x1D:
        {
            this->A |= cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X);
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Absolute, Y
        case 0x19:
        {
            this->A |= cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->Y);
            cpu6502_SetZNFlags(this, this->A);

            break;
        }
        // Indirect, X
        case 0x01:
        {

        }
        // Indirect, Y
        case 0x11:
        {
            break;
        }

        // PHA
        case 0x48: cpu6502_WriteByte(this, this->A, this->S++); break;

        // PHP
        case 0x08:
        {
            byte_t P = 0;
            P = this->C | this-> Z | this-> I | this->D | this->B | this->U | this->V | this->N;
            P |= CPU_FLAG_B | CPU_FLAG_U;
            cpu6502_WriteByte(this, P, this->S++);

            break;
        }

        // PLA
        case 0x68: this->A = cpu6502_ReadByte(this, this->S--); break;

        // PLP
        case 0x28:
        {
            byte_t P = cpu6502_ReadByte(this, this->S--);
            this->C = P & CPU_FLAG_C;
            this->Z = P & CPU_FLAG_Z;
            this->I = P & CPU_FLAG_I;
            this->D = P & CPU_FLAG_D;
            this->B = P & CPU_FLAG_B;
            this->U = P & CPU_FLAG_U;
            this->V = P & CPU_FLAG_V;
            this->N = P & CPU_FLAG_N;

            break;
        }

        // ROL
        case 0x2A:
        case 0x26:
        case 0x36:
        case 0x2E:
        case 0x3E:

        // ROR
        case 0x6A:
        case 0x66:
        case 0x76:
        case 0x6E:
        case 0x7E:

        // RTI
        case 0x40:
        {
            byte_t P = cpu6502_ReadByte(this, this->S--);
            this->C = P & CPU_FLAG_C;
            this->Z = P & CPU_FLAG_Z;
            this->I = P & CPU_FLAG_I;
            this->D = P & CPU_FLAG_D;
            this->B = P & CPU_FLAG_B;
            this->U = P & CPU_FLAG_U;
            this->V = P & CPU_FLAG_V;
            this->N = P & CPU_FLAG_N;

            this->PC = cpu6502_ReadByte(this, this->S--);

            break;
        }

        // RTS
        case 0x60:
        {
            this->PC = cpu6502_ReadByte(this, this->S - 1);
            cpu6502_WriteByte(this, this->PC, this->PC + 1);
            this->S--;

            break;
        }

        // SBC
        case 0xE9:
        case 0xE5:
        case 0xF5:
        case 0xED:
        case 0xFD:
        case 0xF9:
        case 0xE1:
        case 0xF1:

        // SEC
        case 0x38: this->C = 1; break;

        // SED
        case 0xF8: this->D = 1; break;

        // SEI
        case 0x78: this->I = 1; break;

        // STA
        // Zero Page
        case 0x85:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchByte(this));

            break;
        }
        // Zero Page, X
        case 0x95:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchByte(this) + this->X);

            break;
        }
        // Absolute
        case 0x8D:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchWord(this));

            break;
        }
        // Absolute, X
        case 0x9D:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchWord(this) + this->X);

            break;
        }
        // Absolute, Y
        case 0x99:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchWord(this) + this->Y);

            break;
        }
        // Indirect, X
        case 0x81:
        {

        }
        // Indirect, Y
        case 0x91:
        {
            break;
        }

        // STX
        // Zero Page
        case 0x86:
        {
            cpu6502_WriteByte(this, this->X, cpu6502_FetchByte(this));

            break;
        }
        // Zero Page, Y
        case 0x96:
        {
            cpu6502_WriteByte(this, this->X, cpu6502_FetchByte(this) + this->Y);

            break;
        }
        // Absolute
        case 0x8E:
        {
            cpu6502_WriteByte(this, this->X, cpu6502_FetchWord(this));

            break;
        }

        // STY
        // Zero Page
        case 0x84:
        {
            cpu6502_WriteByte(this, this->Y, cpu6502_FetchByte(this));

            break;
        }
        // Zero Page, X
        case 0x94:
        {
            cpu6502_WriteByte(this, this->Y, cpu6502_FetchByte(this) + this->X);

            break;
        }
        // Absolute
        case 0x8C:
        {
            cpu6502_WriteByte(this, this->Y, cpu6502_FetchWord(this));

            break;
        }

        // TAX
        case 0xAA:
        {
            this->X = this->A;
            cpu6502_SetZNFlags(this, this->X);

            break;
        }

        // TAY
        case 0xA8:
        {
            this->Y = this->A;
            cpu6502_SetZNFlags(this, this->Y);

            break;
        }

        // TSX
        case 0xBA:
        {
            this->X = this->S;
            cpu6502_SetZNFlags(this, this->X);

            break;
        }

        // TXA
        case 0x8A:
        {
            this->A = this->X;
            cpu6502_SetZNFlags(this, this->A);

            break;
        }

        // TXS
        case 0x9A: this->S = this->X; break;

        // TYA
        case 0x98:
        {
            this->A = this->Y;
            cpu6502_SetZNFlags(this, this->A);

            break;
        }

        default:
            //TODO: How to handle invalid opcode?
            break;
    }
}

void cpu6502_SetZNFlags(cpu6502_t* this, word_t value)
{
    this->Z = value == 0;
    this->N = (value & 0x80) > 0;
}