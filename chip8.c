#include "chip8.h"
#include "chip8_impl.h"
#include "chip8_opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void chip8_load_fonts(chip8_t* chip){
	memcpy(chip->memory + CHIP_FONTS_OFFSET, chip8_fontset, sizeof(chip8_fontset));
}

void chip8_init(chip8_t* chip){
	chip->pc = CHIP_PROGRAM_OFFSET;
	chip->sp = 0;
	chip->I = 0;
	memset(chip->memory, 0, sizeof(chip->memory));
	memset(chip->keys, 0, sizeof(chip->keys));
	memset(chip->stack, 0, sizeof(chip->stack));
	memset(chip->V, 0, sizeof(chip->V));
	chip8_clear_screen(chip, NULL);
	chip->delay_timer = 0;
	chip->sound_timer = 0;
	chip->waiting_keypress = 0;
	chip8_load_fonts(chip);
}

void chip8_load(chip8_t* chip, unsigned char* program, size_t program_length){
	memcpy(chip->memory + CHIP_PROGRAM_OFFSET, program, program_length);
}

static void chip8_manipulate(chip8_t* chip, opcode_params_t* params);
static void chip8_manipulate2(chip8_t* chip, opcode_params_t* params);

static void load_params(unsigned short opcode, opcode_params_t* out){
	out->nnn = 	opcode & 0x0FFF;
	out->nn = 	opcode & 0x00FF;
	out->x = 	(opcode & 0x0F00) >> 8;
	out->y = 	(opcode & 0x00F0) >> 4;
	out->n = 	opcode & 0x000F;
}

static void chip8_update_timers(chip8_t* chip){
	if(chip->delay_timer > 0)
		chip->delay_timer -= 1;
	if(chip->sound_timer > 0)
		chip->sound_timer -= 1;
}

static opcode_params_t params;

void chip8_cycle(chip8_t* chip){
	if(chip->waiting_keypress == 1){
		return;
	} else if(chip->waiting_keypress == 2){
		chip->V[(chip->opcode & 0x0F00) >> 8] = chip->last_pressed;
		chip->waiting_keypress = 0;
	}
	unsigned short raw_opcode;
	memcpy(&raw_opcode, chip->memory + chip->pc, sizeof(unsigned short));
	
	chip->opcode = 0;
	chip->opcode = (raw_opcode & 0xFF00) >> 8;
	chip->opcode |= (raw_opcode & 0x00FF) << 8;
	
	
	load_params(chip->opcode, &params);
	/* decode and execute opcode */
	switch(chip->opcode & OPCODE_DECODE_MASK){
		case 0x0000:
			if(chip->opcode == 0x00E0){
				chip8_clear_screen(chip, &params);
			} else if (chip->opcode == 0x00EE) {
				chip8_subroutine_return(chip, &params);
			} else {
				/*printf("Unknown instruction(0) %hxy\n", chip->opcode);*/
				printf("Unknown instruction(0) %hx on addr %hu\n", chip->opcode, chip->pc);
				return;
			}
			break;
		case JUMP:
			chip8_jump(chip, &params);
			break;
		case CALLSUB:
			chip8_callsub(chip, &params);
			break;
		case SKIPIFVX:
			chip8_skipifvx(chip, &params);
			break;
		case SKIPIFNVX:
			chip8_skipifnvx(chip, &params);
			break;
		case SKIPIFXY:
			chip8_skipifxy(chip, &params);
			break;
		case SETVX:
			chip8_setvx(chip, &params);
			break;
		case ADDVX:
			chip8_addvx(chip, &params);
			break;
		case MANIPULATE:
			chip8_manipulate(chip, &params);
			break;
		case SKIPIFNE:
			if(params.n == 0){
				chip8_skipifnvxvy(chip, &params);
				printf("Unknown instruction(SKIPIFNE) %hx on addr %hu\n", chip->opcode, chip->pc);
			} else {
			}
			break;
		case SETI:
			chip8_seti(chip, &params);
			break;
		case JUMPR:
			chip8_jumpr(chip, &params);
			break;
		case RANDOM:
			chip8_rand(chip, &params);
			break;
		case DRAW:
			chip8_draw(chip, &params);
			break;
		case KEY:
			if(params.nn == 0x9E){
				chip8_skipkeydown(chip, &params);
			} else if(params.nn == 0xA1){
				chip8_skipkeyup(chip, &params);
			} else {
				printf("Unknown instruction(key) %hx on addr %hu\n", chip->opcode, chip->pc);
			}
			break;
		case MANIPULATE2:
			chip8_manipulate2(chip, &params);
			break;
		default:
				printf("Unknown instruction(bigswitch) %hx on addr %hu\n", chip->opcode, chip->pc);
	}
	chip8_update_timers(chip);
}

static void chip8_manipulate(chip8_t* chip, opcode_params_t* params){
	/* the last 4 bits from opcode are stored in n */
	switch(params->n){
		case 0x0:
			chip8_setvxvy(chip, params);
			break;
		case 0x1:
			chip8_orvxvy(chip, params);
			break;
		case 0x2:
			chip8_andvxvy(chip, params);
			break;
		case 0x3:
			chip8_xorvxvy(chip, params);
			break;
		case 0x4:
			chip8_addvxvy(chip, params);
			break;
		case 0x5:
			chip8_subvxvy(chip, params);
			break;
		case 0x6:
			chip8_shrvx(chip, params);
			break;
		case 0x7:
			chip8_subnvxvy(chip, params);
			break;
		case 0xE:
			chip8_shlvx(chip, params);
			break;
		default:
			printf("Unknown instruction(manipulate) %hx on addr %hu\n", chip->opcode, chip->pc);
			break;
	}
}

static void chip8_manipulate2(chip8_t* chip, opcode_params_t* params){
	switch(params->nn){
		case 0x07:
			chip8_setvxdt(chip, params);
			break;
		case 0x0A:
			chip8_waitkeypress(chip, params);
			break;
		case 0x15:
			chip8_setdtvx(chip, params);
			break;
		case 0x18:
			chip8_setstvx(chip, params);
			break;
		case 0x1E:
			chip8_addivx(chip, params);
			break;
		case 0x29:
			chip8_digisprite(chip, params);
			break;
		case 0x33:
			chip8_bcdvx(chip, params);
			break;
		case 0x55:
			chip8_writereg(chip, params);
			break;
		case 0x65:
			chip8_loadreg(chip, params);
			break;
		default:
			printf("Unknown instruction(manipulate2) %hx on addr %hu\n", chip->opcode, chip->pc);
			break;
	}
}

void chip8_cleanup(chip8_t* chip){
}
