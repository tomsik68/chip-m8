#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define SCREEN_WIDTH 10 * CHIP_GFX_WIDTH
#define SCREEN_HEIGHT 10 * CHIP_GFX_HEIGHT

typedef struct {
	SDL_Keycode 	sdl_key;
	unsigned char 	chip_key;
} keybinding_t;

/* SDL environment */
static SDL_Window* 	window;
static SDL_Event 	event;
static SDL_Renderer* 	renderer;
static SDL_Surface*	chipScreen;
static SDL_Surface*	windowSurface;
static SDL_Texture*	windowTexture;
static keybinding_t bindings[] = {
	{SDLK_1, 1}, {SDLK_2, 2}, {SDLK_3, 3}, {SDLK_4, 4}, {SDLK_5, 5}, {SDLK_6, 6},
	{SDLK_7, 7}, {SDLK_8, 8}, {SDLK_9, 9}, {SDLK_a, 0xA}, {SDLK_b, 0xB}, {SDLK_c, 0xC}, 
	{SDLK_c, 0xC}, {SDLK_d, 0xD}, {SDLK_e, 0xE}, {SDLK_f, 0xF}
};

/* the chip */
static chip8_t chip;

void sync_screen();

size_t load_program(char* filename, unsigned char* whereToLoad){
	size_t result;
	FILE* file = fopen(filename, "rb");
	result = fread(whereToLoad, sizeof(char), 512, file);
	fclose(file);
	return result;
}

int main(int argc, char** argv){
	if(argc < 2){
		fprintf(stderr, "Please specify a filename");
		return 1;
	}
	unsigned char* program = malloc(512 * sizeof(char));
	size_t program_length = load_program(argv[1], program);

	/* initialize SDL */
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		fprintf(stderr, "Error: Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}
	
	window = SDL_CreateWindow("chipm8", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if(window == NULL){
		fprintf(stderr, "Error: Unable to create window: %s", SDL_GetError());
		return 1;
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
	if(renderer == NULL){
		fprintf(stderr, "Error: Unable to create renderer: %s", SDL_GetError());
		return 1;
	}
	
	chipScreen = SDL_CreateRGBSurface(0, CHIP_GFX_WIDTH, CHIP_GFX_HEIGHT, 32, 0, 0, 0, 0);	
	windowSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
	windowTexture = NULL;
	/* initialize the chip */
	chip8_init(&chip);
	chip8_load(&chip, program, program_length);
	/* after we've loaded from the program, we can free the memory */
	free(program);
	
	int running = 1;
	while(running){
		/* update input status */
		while(SDL_PollEvent(&event) != 0){
			if(event.type == SDL_KEYDOWN){
				if(event.key.keysym.sym ==  SDLK_ESCAPE){
					running = 0;
				} else {
					unsigned char i;
					for(i = 0; i < sizeof(bindings); ++i){
						if(event.key.keysym.sym == bindings[i].sdl_key){
							chip.keys[bindings[i].chip_key] = 1;
							if(chip.waiting_keypress == 1){
								chip.last_pressed = bindings[i].chip_key;
								chip.waiting_keypress = 2;
							}
						}
					}
				}
			} else if(event.type == SDL_KEYUP){
				unsigned char i;
				for(i = 0; i < sizeof(bindings); ++i){
					if(event.key.keysym.sym == bindings[i].sdl_key){
						chip.keys[bindings[i].chip_key] = 1;
					}
				}
			} else if(event.type == SDL_QUIT){
				running = 0;
			} /* else, do nothing */
		}
		
		/* do one cycle on chip */
		chip8_cycle(&chip);
		

		/* draw the current screen */
		SDL_RenderClear(renderer);
		sync_screen();
		SDL_RenderCopy(renderer, windowTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		/* ensure delay <= 60 Hz */
		SDL_Delay(1000 / 60);
	}
	chip8_cleanup(&chip);
	SDL_FreeSurface(chipScreen);
	SDL_FreeSurface(windowSurface);
	SDL_DestroyTexture(windowTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

static int x, y, i;
static SDL_Rect dest = {0, 0, SCREEN_WIDTH / CHIP_GFX_WIDTH, SCREEN_HEIGHT / CHIP_GFX_HEIGHT};

#define COLOR_OFF	0x000000FF
#define COLOR_ON	0xFF00FFFF
int get_color(unsigned char pixel){
	if(pixel == 1)
		return COLOR_ON;
	else
		return COLOR_OFF;
}

void sync_screen(){
	/* chip device -> small SDL_Surface */
	for(i = 0; i < CHIP_GFX_HEIGHT * CHIP_GFX_WIDTH; ++i){
		int* pixel = ((int*)(chipScreen->pixels + sizeof(int) * i));
		*pixel = get_color(chip.gfx[i]);
	}
	/* small SDL_Surface -> big SDL_Surface */
	SDL_BlitScaled(chipScreen, NULL, windowSurface, NULL );
	/* big SDL_Surface -> windowTexture */
	windowTexture = SDL_CreateTextureFromSurface(renderer, windowSurface);
}
