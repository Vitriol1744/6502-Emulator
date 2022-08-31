#include <stdio.h>
#include <stdlib.h>

#include "cpu6502.h"

void load_program(bus_t* bus, void* machine_code, size_t size)
{
    word_t address = 0x100;
    byte_t* ptr = (byte_t*)machine_code;

    while (size > 0)
    {
        bus_WriteByte(bus, *ptr++, address++);
        size--;
    }
    printf("0xFFFC: %x, 0xFFFD: %x", bus_ReadByte(bus, 0x100), bus_ReadByte(bus, 0x101));
}

int main(int argc, char** argv)
{
    /*byte_t p[] = { 0xA9, 0x01, 0x20, 0xFC, 0xFF };
    FILE* f = fopen("K:/program.nes", "w+b");
    fwrite(p, 1, 5, f);
    fclose(f);
    return 5;*/

    printf("Hello, World!\n");
    FILE* file;
    file = fopen("K:/super_mario.nes", "rb");

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    byte_t* program = malloc(file_size + 1);
    fread(program, file_size, 1, file);
    fclose(file);

    bus_t bus;
    bus_Initialize(&bus);
    load_program(&bus, program, file_size);

    cpu6502_t cpu;
    cpu6502_ConnectBus(&cpu, &bus);
    cpu6502_Reset(&cpu);
    cpu6502_Run(&cpu);

    return 0;
}
