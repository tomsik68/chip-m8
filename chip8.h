#ifndef __CHIP8_H__
#define __CHIP8_H__
#include <stdlib.h>

#define CHIP_PROGRAM_OFFSET	0x200
#define CHIP_FONTS_OFFSET	0x0
#define CHIP_MEMORY_SIZE 	4096
#define CHIP_GFX_WIDTH 		64
#define CHIP_GFX_HEIGHT 	32
#define CHIP_REGISTER_COUNT 	16
#define CHIP_STACK_DEPTH 	16
#define CHIP_KEYS_COUNT		16

typedef struct {
	/* memory array */
	unsigned char memory[CHIP_MEMORY_SIZE];
	/* registers - V0 - VF */
	unsigned char V[CHIP_REGISTER_COUNT];
	/* index register */
	unsigned short I;
	/* program counter */
	unsigned short pc;
	/* graphics memory */
	unsigned char gfx[CHIP_GFX_WIDTH * CHIP_GFX_HEIGHT];
	/* delay timer */
	unsigned char delay_timer;
	/* sound timer */
	unsigned char sound_timer;
	/* jump call stack */
	unsigned short stack[CHIP_STACK_DEPTH];
	/* jump call stack pointer */
	unsigned short sp;
	/* keys state */
	unsigned char keys[CHIP_KEYS_COUNT];
	/* current opcode */
	unsigned short opcode;
	/* 1 if the machine is currently waiting for a key press, otherwise 0 */
	unsigned char waiting_keypress;
	/* the key that was pressed last time */
	unsigned char last_pressed;
} chip8_t;

typedef struct {
	unsigned short nnn;
	unsigned short nn;
	unsigned short n, x, y;
} opcode_params_t;

/* initializes the machine */
void chip8_init(chip8_t* chip);

/* load program into memory */
void chip8_load(chip8_t* chip, unsigned char* program, size_t program_length);

/* perform one cycle */
void chip8_cycle(chip8_t* chip);

/* cleans up the struct */
void chip8_cleanup(chip8_t* chip);
#endif
