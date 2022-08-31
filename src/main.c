#include <stdio.h>

#include "cpu6502.h"

void load_program(bus_t* bus, void* machine_code, size_t size)
{
    word_t address = 0xFFFC;
    byte_t* ptr = (byte_t*)machine_code;

    while (size > 0)
    {
        bus_WriteByte(bus, *ptr++, address++);
        size--;
    }
    printf("0xFFFC: %x, 0xFFFD: %x", bus_ReadByte(bus, 0xFFFC), bus_ReadByte(bus, 0xFFFD));
}

int main()
{
    printf("Hello, World!\n");

    byte_t program[] =
    {
        0xA9, 0x01, // LDA 0x01
        0x20, 0xFC, 0xFF // JSR 0xFFFC
    };

    bus_t bus;
    bus_Initialize(&bus);
    load_program(&bus, program, sizeof(program) / sizeof(program[0]));

    cpu6502_t cpu;
    cpu6502_ConnectBus(&cpu, &bus);
    cpu6502_Reset(&cpu);
    cpu6502_Run(&cpu);

    return 0;
}
