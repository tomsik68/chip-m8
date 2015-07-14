#include "chip8_impl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void chip8_clear_screen(chip8_t* chip, opcode_params_t* params){
	memset(chip->gfx, 0, sizeof(chip->gfx));
}

void chip8_subroutine_return(chip8_t* chip, opcode_params_t* params){
	/* set program counter to previous location */
	chip->pc = chip->stack[chip->sp];
	/* decrement stack pointer */
	--(chip->sp);
}

void chip8_jump(chip8_t* chip, opcode_params_t* params){
	/* set program counter to desired value */
	chip->pc = params->nnn;
}

void chip8_callsub(chip8_t* chip, opcode_params_t* params){
	/* increment stack pointer */
	++(chip->sp);
	/* remember program counter value to stack */
	chip->stack[chip->sp] = chip->pc;
	/* set program counter to nnn */
	chip8_jump(chip, params);
}

void chip8_skipifvx(chip8_t* chip, opcode_params_t* params){
	/* if VX = NN, skip the next instruction */
	if(chip->V[params->x] == params->nn) { 
		chip->pc += 2;
	}
}

void chip8_skipifnvx(chip8_t* chip, opcode_params_t* params){
	/* if VX != NN, skip the next instruction */
	if(chip->V[params->x] != params->nn) { 
		chip->pc += 2;
	}
}

void chip8_skipifxy(chip8_t* chip, opcode_params_t* params){
	/* if VX == VY, skip the next instruction */
	if(chip->V[params->x] == chip->V[params->y]){
		chip->pc += 2;
	}
}

void chip8_setvx(chip8_t* chip, opcode_params_t* params){
	/* set VX = nn */
	chip->V[params->x] = params->nn;
}

void chip8_addvx(chip8_t* chip, opcode_params_t* params){
	/* set VX = VX + nn */
	chip->V[params->x] += params->nn;
}

void chip8_setvxvy(chip8_t* chip, opcode_params_t* params){
	/* set VX = VY */
	chip->V[params->x] = chip->V[params->y];
}

void chip8_orvxvy(chip8_t* chip, opcode_params_t* params){
	/* set VX = VX or VY */
	chip->V[params->x] |= chip->V[params->y];
}

void chip8_andvxvy(chip8_t* chip, opcode_params_t* params){
	/* VX = VX and VY */
	chip->V[params->x] &= chip->V[params->y];
}

void chip8_xorvxvy(chip8_t* chip, opcode_params_t* params){
	/* VX = VX xor VY */
	chip->V[params->x] ^= chip->V[params->y];
}

void chip8_addvxvy(chip8_t* chip, opcode_params_t* params){
	/* handle the carry bit */
	if(chip->V[params->x] + chip->V[params->y] > 255){
		chip->V[0xF] = 1;
	} else {
		chip->V[0xF] = 0;
	}
	/* VX = VX + VY */
	chip->V[params->x] += chip->V[params->y];
}

void chip8_subvxvy(chip8_t* chip, opcode_params_t* params){
	/* handle the carry bit */
	if(chip->V[params->x] > chip->V[params->y]){
		chip->V[0xF] = 1;
	} else {
		chip->V[0xF] = 0;
	}
	/* VX = VX - VY */
	chip->V[params->x] -= chip->V[params->y];
}

void chip8_shrvx(chip8_t* chip, opcode_params_t* params){
	/* handle the carry bit */
	if(chip->V[params->x] & (1 << 0)){
		chip->V[0xF] = 1;
	} else {
		chip->V[0xF] = 0;
	}
	chip->V[params->x] = chip->V[params->x] >> 1;
}

void chip8_subnvxvy(chip8_t* chip, opcode_params_t* params){
	/* handle the carry bit */
	if(chip->V[params->x] < chip->V[params->y]){
		chip->V[0xF] = 1;
	} else {
		chip->V[0xF] = 0;
	}
	chip->V[params->x] = chip->V[params->y] - chip->V[params->x];
}

void chip8_shlvx(chip8_t* chip, opcode_params_t* params){
	/* handle the carry bit */
	if(chip->V[params->x] & (1 << 7)){
		chip->V[0xF] = 1;
	} else {
		chip->V[0xF] = 0;
	}
	chip->V[params->x] = chip->V[params->x] << 1;
}

void chip8_skipifnvxvy(chip8_t* chip, opcode_params_t* params){
	if(chip->V[params->x] != chip->V[params->y]){
		chip->pc += 2;
	}
}

void chip8_seti(chip8_t* chip, opcode_params_t* params){
	/* set the index register to nnn */
	chip->I = params->nnn;
}

void chip8_jumpr(chip8_t* chip, opcode_params_t* params){
	/* jump relatively to VX */
	chip->pc = params->nnn + chip->V[params->x];
}

void chip8_rand(chip8_t* chip, opcode_params_t* params){
	/* set VX = random number & nn */
	chip->V[params->x] = ((unsigned char)(rand() % 255)) & params->nn;
}

void chip8_draw(chip8_t* chip, opcode_params_t* params){
	unsigned short vx = chip->V[params->x];
	unsigned short vy = chip->V[params->y];
	unsigned short xOnSprite = 0;
	unsigned short yOnSprite = 0;
	const unsigned short width = 8;
	unsigned short height = params->n;
	unsigned short destX, destY;
	
	chip->V[0xF] = 0;
	
	/* iterate over sprite pixels */
	for(yOnSprite = 0; yOnSprite < height; ++yOnSprite){
		for(xOnSprite = 0; xOnSprite < width; ++xOnSprite) {
			destX = (xOnSprite + vx) % CHIP_GFX_WIDTH;
			destY = (yOnSprite + vy) % CHIP_GFX_HEIGHT;
			/* obtain a pixel from sprite */
			unsigned char currentSpritePixel = (chip->memory[chip->I + yOnSprite]) & (0x80 >> xOnSprite);
			if(currentSpritePixel != 0){
				/* obtain a screen pixel */
				unsigned char screenPixel = chip->gfx[CHIP_GFX_WIDTH * destY + destX];
				/* xor them together */
				chip->gfx[CHIP_GFX_WIDTH * destY + destX] ^= 1;
				if(chip->gfx[CHIP_GFX_WIDTH * destY + destX] == 0 && screenPixel == 1){
					chip->V[0xF] = 1;
				}
			}
		}
	}
}

void chip8_skipkeydown(chip8_t* chip, opcode_params_t* params){
	if(chip->keys[chip->V[params->x]] == 1){
		chip->pc += 2;
	}
}

void chip8_skipkeyup(chip8_t* chip, opcode_params_t* params){
	if(chip->keys[chip->V[params->x]] == 0){
		chip->pc += 2;
	}
}

void chip8_setvxdt(chip8_t* chip, opcode_params_t* params){
	chip->V[params->x] = chip->delay_timer;
}

void chip8_waitkeypress(chip8_t* chip, opcode_params_t* params){
	chip->waiting_keypress = 1;
}

void chip8_setdtvx(chip8_t* chip, opcode_params_t* params){
	chip->delay_timer = chip->V[params->x];
}

void chip8_setstvx(chip8_t* chip, opcode_params_t* params){
	chip->sound_timer = chip->V[params->x];
}

void chip8_addivx(chip8_t* chip, opcode_params_t* params){
	chip->I += chip->V[params->x];
}

void chip8_bcdvx(chip8_t* chip, opcode_params_t* params){
	/* memory[I] will contain amount of hundreds */
	chip->memory[chip->I	] = chip->V[params->x] / 100;
	/* memory[I + 1] will contain number of tens */
	chip->memory[chip->I + 1] = (chip->V[params->x] / 10) % 10;
	/* memory[I + 2] will contain number of ones */
	chip->memory[chip->I + 2] = (chip->V[params->x] % 100) % 10;
}

void chip8_writereg(chip8_t* chip, opcode_params_t* params){
	memcpy(chip->memory + chip->I, chip->V, params->x);
}

void chip8_loadreg(chip8_t* chip, opcode_params_t* params){
	memcpy(chip->V, chip->memory + chip->I, params->x);
}

/* set I to location in memory where sprite for number VX is located */
void chip8_digisprite(chip8_t* chip, opcode_params_t* params){
	chip->I = CHIP_FONTS_OFFSET + 5 * chip->V[params->x];
}
