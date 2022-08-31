//
// Created by Vitriol1744 on 30.08.2022.
//
#include "cpu6502.h"

#include <string.h>
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
        case 0x69:
        case 0x65:
        case 0x75:
        case 0x6D:
        case 0x7D:
        case 0x79:
        case 0x61:
        case 0x71:
        {
            word_t fetched = cpu6502_FetchByte(this);

            // Add is performed in 16-bit domain for emulation to capture any
            // carry bit, which will exist in bit 8 of the 16-bit word
            word_t temp = (word_t)this->A + (word_t)fetched + (word_t)this->C;

            this->C = temp > 255;
            this->Z = (temp & 0x00FF) == 0;
            this->V = (~((word_t)this->A ^ (word_t)fetched) & ((word_t)this->A ^ (word_t)temp)) & 0x0080;
            this->N = temp & 0x80;
            this->A = temp & 0x00FF;

            break;
        }
        // AND
        // Immediate Mode
        case 0x29:
        {
            this->A &= cpu6502_FetchByte(this);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Zero Page
        case 0x25:
        {
            this->A &= cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Zero Page, X
        case 0x35:
        {
            this->A &= cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute
        case 0x2D:
        {
            word_t address = cpu6502_FetchWord(this);
            this->A &= cpu6502_ReadByte(this, address);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute, X
        case 0x3D:
        {
            word_t address = cpu6502_FetchWord(this);
            this->A &= cpu6502_ReadByte(this, address + this->X);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute, Y
        case 0x39:
        {
            word_t address = cpu6502_FetchWord(this);
            this->A &= cpu6502_ReadByte(this, address + this->Y);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Indirect, X
        case 0x21:
        {
            /*word_t ptr_lo = cpu6502_FetchByte(this);
            word_t ptr_hi = cpu6502_FetchByte(this);

            word_t ptr_address = (ptr_hi << 8) | ptr_lo;
            word_t hi = cpu6502_ReadByte(this, ptr_lo == 0x00FF ? ptr_address & 0xFF00 : ptr_address + 1);
            word_t lo = cpu6502_ReadByte(this, ptr_address);

            word_t address = (hi << 8) | lo;
            this->A &= cpu6502_ReadByte(this, address + this->X);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;*/
            word_t ptr = cpu6502_FetchByte(this);

            word_t lo = cpu6502_ReadByte(this, ptr + this->X & 0x00FF);
            word_t hi = cpu6502_ReadByte(this, ptr + this->X + 1) & 0x00FF;

            this->A &= cpu6502_ReadByte(this, (hi << 8) | lo);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Indirect, Y
        case 0x31:
        {
            word_t ptr = cpu6502_FetchByte(this);

            word_t lo = cpu6502_ReadByte(this, ptr + this->Y & 0x00FF);
            word_t hi = cpu6502_ReadByte(this, ptr + this->Y + 1) & 0x00FF;

            this->A &= cpu6502_ReadByte(this, (hi << 8) | lo);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

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
        case 0xC9:
        case 0xC5:
        case 0xD5:
        case 0xCD:
        case 0xDD:
        case 0xD9:
        case 0xC1:
        case 0xD1:

        // CPX
        case 0xE0:
        case 0xE4:
        case 0xEC:

        // CPY
        case 0xC0:
        case 0xC4:
        case 0xCC:

        // DEC
        // Zero Page
        case 0xC6:
        {
            word_t address = cpu6502_FetchByte(this);
            word_t value = cpu6502_ReadByte(this, address) - 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Zero Page, X
        case 0xD6:
        {
            word_t address = cpu6502_FetchByte(this) + this->X;
            word_t value = cpu6502_ReadByte(this, address) - 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Absolute
        case 0xCE:
        {
            word_t address = cpu6502_FetchWord(this);
            word_t value = cpu6502_ReadByte(this, address) - 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Absolute, X
        case 0xDE:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            word_t value = cpu6502_ReadByte(this, address) - 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }

        // DEX
        case 0xCA:
        {
            this->X--;
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }

        // DEY
        case 0x88:
        {
            this->Y--;
            this->Z = this->Y == 0;
            this->N = (this->Y & 0xb10000000) > 0;

            break;
        }

        // EOR
        // Immediate Mode
        case 0x49:
        {
            this->A ^= cpu6502_FetchByte(this);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Zero Page
        case 0x45:
        {
            this->A ^= cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Zero Page, X
        case 0x55:
        {
            this->A ^= cpu6502_ReadByte(this, cpu6502_FetchByte(this) + this->X);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute
        case 0x4D:
        {
            this->A ^= cpu6502_ReadByte(this, cpu6502_FetchWord(this));
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute, X
        case 0x5D:
        {
            this->A ^= cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->X);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Absolute, Y
        case 0x59:
        {
            this->A ^= cpu6502_ReadByte(this, cpu6502_FetchWord(this) + this->Y);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Indirect, X
        case 0x41:
        {

        }
        // Indirect, Y
        case 0x51:

        // INC
        // Zero Page
        case 0xE6:
        {
            word_t address = cpu6502_FetchByte(this);
            byte_t value = cpu6502_ReadByte(this, address) + 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Zero Page, X
        case 0xF6:
        {
            word_t address = cpu6502_FetchByte(this) + this->X;
            byte_t value = cpu6502_ReadByte(this, address) + 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Absolute
        case 0xEE:
        {
            word_t address = cpu6502_FetchWord(this);
            byte_t value = cpu6502_ReadByte(this, address) + 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }
        // Absolute, X
        case 0xFE:
        {
            word_t address = cpu6502_FetchWord(this) + this->X;
            byte_t value = cpu6502_ReadByte(this, address) + 1;
            cpu6502_WriteByte(this, value, address);

            this->Z = value == 0;
            this->N = (value & 0xb10000000) > 0;

            break;
        }

        // INX
        case 0xE8:
        {
            this->X++;
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }

        // INY
        case 0xC8:
        {
            this->Y++;
            this->Z = this->Y == 0;
            this->N = (this->Y & 0xb10000000) > 0;

            break;
        }

        // JMP
        // Absolute
        case 0x4C: //this->PC = cpu6502_FetchWord(this); break;
        case 0x6C:

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
            this->A = cpu6502_FetchByte(this);
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        // Zero Page
        case 0xA5:
        {
            this->A = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }
        case 0xB5:
        case 0xAD:
        case 0xBD:
        case 0xB9:
        case 0xA1:
        case 0xB1:

        // LDX
        // Immediate Mode
        case 0xA2:
        {
            this->X = cpu6502_FetchByte(this);
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }
        // Zero Page
        case 0xA6:
        {
            this->X = cpu6502_ReadByte(this, cpu6502_FetchByte(this));
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }
        case 0xB6:
        case 0xAE:
        case 0xBE:

        // LDY
        case 0xA0:
        case 0xA4:
        case 0xB4:
        case 0xAC:
        case 0xBC:

        // LSR
        case 0x4A:
        case 0x46:
        case 0x56:
        case 0x4E:
        case 0x5E:

        // NOP
        case 0xEA:

        // ORA
        case 0x09:
        case 0x05:
        case 0x15:
        case 0x0D:
        case 0x1D:
        case 0x19:
        case 0x01:
        case 0x11:

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
        case 0x85:
        {
            cpu6502_WriteByte(this, this->A, cpu6502_FetchByte(this));

            break;
        }
        case 0x95:
        case 0x8D:
        case 0x9D:
        case 0x99:
        case 0x81:
        case 0x91:

        // STX
        case 0x86:
        {
            cpu6502_WriteByte(this, this->X, cpu6502_FetchByte(this));

            break;
        }
        case 0x96:
        case 0x8E:

        // STY
        case 0x84:
        {
            cpu6502_WriteByte(this, this->Y, cpu6502_FetchByte(this));

            break;
        }
        case 0x94:
        case 0x8C:

        // TAX
        case 0xAA:
        {
            this->X = this->A;
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }

        // TAY
        case 0xA8:
        {
            this->Y = this->A;
            this->Z = this->Y == 0;
            this->N = (this->Y & 0xb10000000) > 0;

            break;
        }

        // TSX
        case 0xBA:
        {
            this->X = this->S;
            this->Z = this->X == 0;
            this->N = (this->X & 0xb10000000) > 0;

            break;
        }

        // TXA
        case 0x8A:
        {
            this->A = this->X;
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }

        // TXS
        case 0x9A: this->S = this->X; break;

        // TYA
        case 0x98:
        {
            this->A = this->Y;
            this->Z = this->A == 0;
            this->N = (this->A & 0xb10000000) > 0;

            break;
        }

        default:
            //TODO: How to handle invalid opcode?
            break;
    }
}