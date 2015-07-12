#define OPCODE_DECODE_MASK 	0xF000

/* jump to address */
#define JUMP			0x1000
/* call subroutine **/
#define CALLSUB			0x2000
/* skip next instruction if vx is equal to specified value**/
#define SKIPIFVX		0x3000
/* skip if vx is not equal to the value**/
#define SKIPIFNVX		0x4000
/* skip if vx == vy **/
#define SKIPIFXY		0x5000
/* sets vx **/
#define SETVX			0x6000
/* vx += nn **/
#define ADDVX			0x7000
/* weird function **/
#define MANIPULATE		0x8000
/* skip if vx != vy **/
#define SKIPIFNE		0x9000
/* sets I to nnn **/
#define SETI			0xA000
/* jump to nnn + v0 **/
#define JUMPR			0xB000
/* sets vx to random number masked by nn **/
#define RANDOM			0xC000
/* draws the sprite **/
#define DRAW			0xD000
/* skips instruction if key is pressed **/
#define KEY				0xE000
/*  **/
#define MANIPULATE2		0xF000
