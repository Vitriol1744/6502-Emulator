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
    this->PC = 0xFFFC;

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
    word_t ret = cpu6502_FetchByte(this) | (cpu6502_FetchByte(this) << 8);

    return ret;
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
            word_t temp = (uint16_t)this->A + (uint16_t)fetched + (uint16_t) this->C;

            this->C = temp > 255;
            this->Z = (temp & 0x00FF) == 0;
            this->V = (~((uint16_t)this->A ^ (uint16_t)fetched) & ((uint16_t)this->A ^ (uint16_t)temp)) & 0x0080;
            this->N = temp & 0x80;
            this->A = temp & 0x00FF;

            break;
        }
            // AND
        case 0x29:
        case 0x25:
        case 0x35:
        case 0x2D:
        case 0x3D:
        case 0x39:
        case 0x21:
        case 0x31:

            // ASL
        case 0x0A:
        case 0x06:
        case 0x16:
        case 0x0E:
        case 0x1E:

            // BCC
        case 0x90:

            // BCS
        case 0xB0:

            // BIT
        case 0x24:
        case 0x2C:

            // BMI
        case 0x30:

            // BNE
        case 0xD0:

            // BPL
        case 0x10:
            if (this->N) this->PC++;
            else ;
            break;

            // BRK
        case 0x00:

            // BVC
        case 0x50:

            // BVS
        case 0x70:

            // CLC
        case 0x18:

            // CLI
        case 0x58:

            // CLV
        case 0xB8:

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
        case 0xC6:
        case 0xD6:
        case 0xCE:
        case 0xDE:

            // DEX
        case 0xCA:

            // DEY
        case 0x88:

            // EOR
        case 0x49:
        case 0x45:
        case 0x55:
        case 0x4D:
        case 0x5D:
        case 0x59:
        case 0x41:
        case 0x51:

            // INC
        case 0xE6:
        case 0xF6:
        case 0xEE:
        case 0xFE:

            // INX
        case 0xE8:

            // INY
        case 0xC8:

            // JMP
        case 0x4C:
        case 0x6C:

            // JSR
        case 0x20:
        {
            word_t subroutine_address = cpu6502_FetchWord(this);
            cpu6502_WriteWord(this, this->PC - 1, this->S);

            this->PC = subroutine_address;
            break;
        }
            // LDA
        case 0xA9:
        {
            printf("LDA\n");
            this->A = cpu6502_FetchByte(this);
            this->Z = (this->A == 0);
            this->N = (this->A & 0xb10000000) > 0;
        }
        case 0xA5:
        case 0xB5:
        case 0xAD:
        case 0xBD:
        case 0xB9:
        case 0xA1:
        case 0xB1:

            // LDX
        case 0xA2:
        case 0xA6:
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
        case 0x48:

            // PHP
        case 0x08:

            // PLA
        case 0x68:

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
        case 0x38:

            // SED
        case 0xF8:

            // SEI
        case 0x78:

            // STA
        case 0x85:
        case 0x95:
        case 0x8D:
        case 0x9D:
        case 0x99:
        case 0x81:
        case 0x91:

            // STX
        case 0x86:
        case 0x96:
        case 0x8E:

            // STY
        case 0x84:
        case 0x94:
        case 0x8C:

            // TAX
        case 0xAA:

            // TAY
        case 0xA8:

            // TSX
        case 0xBA:

            // TXA
        case 0x8A:

            // TXS
        case 0x9A:

            // TYA
        case 0x98:
            break;

        default:
            //TODO: How to handle invalid opcode?
            break;
    }
}