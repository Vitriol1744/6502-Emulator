cmake_minimum_required(VERSION 3.0)
project(NES_Emulator C)

set(CMAKE_C_STANDARD 17)

file (GLOB_RECURSE SOURCES "src/*.c" "src/*/*.c")

add_executable(NES_Emulator ${SOURCES} src/cpu6502.c src/cpu6502.h src/bus.c src/bus.h src/common.h)
