#ifndef __CHIP8_CPU_H__
#define __CHIP8_CPU_H__

#include "chip8.h"

void chip8_execute_opcode(chip8_t* chip, opcode_params_t* params);
#endif
