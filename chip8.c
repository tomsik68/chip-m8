#include "chip8.h"
#include "chip8_impl.h"
#include "chip8_cpu.h"
#include "chip8_opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* opcode that was executed on chip before this one(for debugging purposes) */
static unsigned short latest_opcode = 0;
/* parameters for currently executed opcode */
static opcode_params_t params;

/* chip8 fontset which is loaded in memory */
unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, 
  0x20, 0x60, 0x20, 0x20, 0x70, 
  0xF0, 0x10, 0xF0, 0x80, 0xF0, 
  0xF0, 0x10, 0xF0, 0x10, 0xF0, 
  0x90, 0x90, 0xF0, 0x10, 0x10, 
  0xF0, 0x80, 0xF0, 0x10, 0xF0, 
  0xF0, 0x80, 0xF0, 0x90, 0xF0, 
  0xF0, 0x10, 0x20, 0x40, 0x40, 
  0xF0, 0x90, 0xF0, 0x90, 0xF0, 
  0xF0, 0x90, 0xF0, 0x10, 0xF0, 
  0xF0, 0x90, 0xF0, 0x90, 0x90, 
  0xE0, 0x90, 0xE0, 0x90, 0xE0, 
  0xF0, 0x80, 0x80, 0x80, 0xF0, 
  0xE0, 0x90, 0x90, 0x90, 0xE0, 
  0xF0, 0x80, 0xF0, 0x80, 0xF0, 
  0xF0, 0x80, 0xF0, 0x80, 0x80  
};

/* loads default fontset into chip8 memory */
void chip8_load_fonts(chip8_t* chip){
	memcpy(chip->memory + CHIP_FONTS_OFFSET, chip8_fontset, sizeof(chip8_fontset));
}

/* initializes chip8 - e.g. prepares it for usage */
void chip8_init(chip8_t* chip){
	/* program counter starts at pre-defined offset(0x200) */
	chip->pc = CHIP_PROGRAM_OFFSET;
	/* stack pointer and index pointer point at 0 */
	chip->sp = 0;
	chip->I = 0;
	/* empty memory, keys array, stack and registers */
	memset(chip->memory, 0, sizeof(chip->memory));
	memset(chip->keys, 0, sizeof(chip->keys));
	memset(chip->stack, 0, sizeof(chip->stack));
	memset(chip->V, 0, sizeof(chip->V));
	/* clear screen */
	chip8_clear_screen(chip, NULL);
	/* restart timers */
	chip->delay_timer = 0;
	chip->sound_timer = 0;
	/* disable "waiting for keypress" status */
	chip->waiting_keypress = 0;
	/* load default fontset into memory */
	chip8_load_fonts(chip);
}

void chip8_load(chip8_t* chip, unsigned char* program, size_t program_length){
	memcpy(chip->memory + CHIP_PROGRAM_OFFSET, program, program_length);
}

/* loads various parameters from opcode */
void load_params(unsigned short opcode, opcode_params_t* out){
	out->nnn = 	opcode & 0x0FFF;
	out->nn = 	opcode & 0x00FF;
	out->x = 	(opcode & 0x0F00) >> 8;
	out->y = 	(opcode & 0x00F0) >> 4;
	out->n = 	opcode & 0x000F;
}

/* makes timers tick */
void chip8_update_timers(chip8_t* chip){
	if(chip->delay_timer > 0)
		chip->delay_timer -= 1;
	if(chip->sound_timer > 0)
		chip->sound_timer -= 1;
}

/* performs one CPU cycle */
void chip8_cycle(chip8_t* chip){
	/* if we're waiting for keypress, CPU is interrupted and nothing happens */
	if(chip->waiting_keypress == 1){
		return;
	} else if(chip->waiting_keypress == 2){
		/* once we get a keypress, we need to save the last key pressed */
		chip->V[(chip->opcode & 0x0F00) >> 8] = chip->last_pressed;
		chip->waiting_keypress = 0;
	}
	/* load raw opcode from chip8 memory */
	unsigned short raw_opcode;
	memcpy(&raw_opcode, chip->memory + chip->pc, sizeof(unsigned short));

	printf("PC before: %hx\n", chip->pc);

	/* move to next instruction */
	chip->pc += sizeof(unsigned short);
	/* switch opcode endianness */
	chip->opcode = 	((raw_opcode & 0x00FF) << 8) & 0xFF00;
	chip->opcode |= ((raw_opcode & 0xFF00) >> 8) & 0x00FF;

	/* load parameters from opcode */
	load_params(chip->opcode, &params);
	/* execute opcode (chip8_cpu.c) */
	chip8_execute_opcode(chip, &params);

	printf("PC after: %hx\n", chip->pc);

	/* tick timers */
	chip8_update_timers(chip);
	/* remember the latest opcode(for debugging purposes) */
	latest_opcode = chip->opcode;
}

void chip8_cleanup(chip8_t* chip){
}
