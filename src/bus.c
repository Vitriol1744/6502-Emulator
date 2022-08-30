//
// Created by Vitriol1744 on 31.08.2022.
//
#include "bus.h"

#include <stdlib.h>

void bus_Initialize(bus_t* this)
{
    this->memory = malloc(0x200000);
}

byte_t bus_Read(bus_t* this, word_t address)
{
    return this->memory[address];
}

void bus_Write(bus_t* this, byte_t data, word_t address)
{
    this->memory[address] = data;
}
