//
// Created by Vitriol1744 on 31.08.2022.
//

#ifndef NES_EMULATOR_BUS_H
#define NES_EMULATOR_BUS_H

#include "common.h"

typedef struct
{
    byte_t* memory;
} bus_t;

void bus_Initialize(bus_t*);
byte_t bus_Read(bus_t*, word_t address);
void bus_Write(bus_t*, byte_t data, word_t address);

#endif // NES_EMULATOR_BUS_H
