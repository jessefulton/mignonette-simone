/*
 *	simone.c - "Simon" game concept using Mignonette Graphics Library (miggl) - v0.1
 *
 *	author(s): jesse fulton (jesse.fulton at gmail dot com | http://jessefulton.com) 
 * 				(c) 2010 - Some Rights Reserved
 *
 *
 *	Note: This source code is licensed under a Creative Commons License, CC-by-nc-sa.
 *		(attribution, non-commercial, share-alike)
 *  	see http://creativecommons.org/licenses/by-nc-sa/3.0/ for details.
 *
 *
 *	hardware requirements:
 *		- Mignonette v1.0
 *
 *
 *	instructions:
 *		Based upon Simon. Hit the correct buttons based upon the arrows displayed
 *
 *	revision history:
 *
 *  - Feb 16, 2010 - jesse
 *      cleaning up code for "official" release
 *
 *  - Jan 16, 2010 - jesse
 *      added more music & sound effects
 *      added random number generator
 *
 *	- Jan 13, 20010 - jesse
 *		created. (Used "Munch" as a template)
 *
 *
 */

#include <inttypes.h>
#include <avr/io.h>			/* this takes care of definitions for our specific AVR */
#include <avr/pgmspace.h>	/* needed for printf_P, etc */
#include <avr/interrupt.h>	/* for interrupts, ISR macro, etc. */
#include <stdio.h>			// for sprintf, etc.
#include <stdlib.h>			// for abs(), etc.
//#include <string.h>			// for strcpy, etc.

#include "uart.h"			// we keep this here only to define F_CPU (uart.c not needed)

// for _delay_us() macro  (note: this gets F_CPU define from uart.h)
#include <util/delay.h>

#include "mydefs.h"
#include "iodefs.h"

#include "miggl.h"		/* Mignonette Game Library */



//=============================================================
// Time functions by Rolf Van Widenfelt & Mitch Altman (munch)
//=============================================================


/**
 * crude delay of 1 to 255 us
 */
void delay_us(byte usec)
{
	usec++;
	
	while (--usec) {
		_delay_us(1);		// get 1us delay from library macro (see <util/delay.h>)
	}
}


/**
 * crude delay of 1 to 255 ms
 */
void delay_ms(uint8_t ms) {
	ms++;
	
	while (--ms) {
		_delay_ms(1);		// get 1ms delay from library macro (see <util/delay.h>)
	}
}


/**
 * crude delay of 1 to 255 s
 */
void delay_sec(uint8_t sec) {
	uint8_t i;
	for (i = 0; i < sec; i++) {
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
	}
}



//============================================
// Random number generator by Jegge (Tri2s)
//============================================

static uint8_t RandomSeedA = 0x11;
static uint8_t RandomSeedB = 0x0D;

/**
 * Generates a pseudo random number from 0 to max 
 */
uint8_t next_random (uint8_t max) {
	RandomSeedA = 0x7F * (RandomSeedA & 0x0F) + (RandomSeedA >> 4);
	RandomSeedB = 0x3C * (RandomSeedB & 0x0F) + (RandomSeedB >> 4);
 	return ((RandomSeedA << 4) + RandomSeedB) % max;
}

void init_random (void) {
	uint8_t *addr = 0;
	for (addr = 0; addr < (uint8_t*)0xFFFF; addr++) 
		RandomSeedB += (*addr);	
}



//============================================
// Sounds!
//============================================

static byte SONG_INTRO[] = {
	N_C4,N_8TH,
	N_E4,N_8TH,
	N_F4,N_HALF,
	N_REST,N_QUARTER,
	N_C4,N_8TH,
	N_E4,N_8TH,
	N_G4,N_HALF,
	N_REST,N_QUARTER,
	N_F4,N_8TH,
	N_E4,N_8TH,
	N_C4,N_HALF,
	N_END,
};

static byte SONG_TAPS[] = {
	N_G3, N_HALF,
	N_G3, N_8TH,
	N_C4, N_WHOLE,
	N_G3, N_HALF,
	N_C4, N_8TH,
	N_E4, N_WHOLE,
	N_END
};

static byte SONG_WIN[] = {
	N_C5, N_16TH, N_D5, N_16TH, N_E5, N_16TH, 
	N_C5, N_16TH, N_D5, N_16TH, N_E5, N_16TH, 
	N_C5, N_16TH, N_D5, N_16TH, N_E5, N_16TH, 
	N_END
};


static byte DIRECTION_A_NOISE[] = {N_F4, N_16TH, N_END};
static byte DIRECTION_B_NOISE[] = {N_D4, N_16TH, N_END};
static byte DIRECTION_C_NOISE[] = {N_E4, N_16TH, N_END};
static byte DIRECTION_D_NOISE[] = {N_G4, N_16TH, N_END};

static byte CORRECT_NOISE[] = {N_C5, N_16TH, N_D5, N_16TH, N_E5, N_16TH, N_END};




//============================================
// Arrows!
//============================================

/* Arrow Directions */
#define DIRECTION_A 0
#define DIRECTION_B	1
#define DIRECTION_C	2
#define DIRECTION_D	3


static byte DIRECTIONS[4] = {
	DIRECTION_A,
	DIRECTION_B,
	DIRECTION_C,
	DIRECTION_D
};

/* Array to hold arrow history */
static byte arrows[100];



/**
 * Displays the arrows on the screen
 */
void draw_arrow(byte dir, byte clr) {
	setcolor(clr);
	if (dir == DIRECTION_A) {
		//POINTS UP AND LEFT
		drawpoint(0, 0);
		drawpoint(0, 1);
		drawpoint(0, 2);
		drawpoint(1, 0);
		drawpoint(2, 0);
		drawpoint(1, 1);
		drawpoint(2, 2);
		drawpoint(3, 3);
		drawpoint(4, 4);
		//drawpoint(5, 5);
	}
	else if (dir == DIRECTION_B) {
		//POINTS DOWN AND LEFT
		drawpoint(0, 4);
		drawpoint(0, 3);
		drawpoint(0, 2);
		drawpoint(1, 4);
		drawpoint(2, 4);
		drawpoint(1, 3);
		drawpoint(2, 2);
		drawpoint(3, 1);
		drawpoint(4, 0);
		//drawpoint(0, 5);
	}
	else if (dir == DIRECTION_C) {
		//POINTS DOWN AND RIGHT
		drawpoint(6, 4);
		drawpoint(6, 3);
		drawpoint(6, 2);
		drawpoint(5, 4);
		drawpoint(4, 4);
		drawpoint(5, 3);
		drawpoint(4, 2);
		drawpoint(3, 1);
		drawpoint(2, 0);
		//drawpoint(0, 5);

	}
	else if (dir == DIRECTION_D) {

		//POINTS UP AND RIGHT
		drawpoint(6, 0);
		drawpoint(6, 1);
		drawpoint(6, 2);
		drawpoint(5, 0);
		drawpoint(4, 0);
		drawpoint(5, 1);
		drawpoint(4, 2);
		drawpoint(3, 3);
		drawpoint(2, 4);
		//drawpoint(0, 5);
	}
}

/**
 * Draws an arrow to the screen and plays the appropriate noise
 */
void show_next_arrow(int cnt) {
	byte *noise;
	byte dir = arrows[cnt];

	if (dir == DIRECTION_A) {
		noise = DIRECTION_A_NOISE;
	}
	else if (dir == DIRECTION_B) {
		noise = DIRECTION_B_NOISE;
	}
	else if (dir == DIRECTION_C) {
		noise = DIRECTION_C_NOISE;
	}
	else {
		noise = DIRECTION_D_NOISE;
	}
	
	
	draw_arrow(arrows[cnt], GREEN);
	delay_ms(200);
	playsong(noise);
	delay_ms(200);
	delay_ms(200);
	cleardisplay();	
	delay_ms(200);

	
}


//============================================
// Helper functions drawing large digits
//============================================

void draw_digit_vert_right(int x_shift) {
	int col3 = 2 + x_shift;
	drawpoint(col3, 0);
	drawpoint(col3, 1);
	drawpoint(col3, 2);
	drawpoint(col3, 3);
	drawpoint(col3, 3);
	drawpoint(col3, 4);
}

void draw_digit_vert_left(int x_shift) {
	int col1 = 0 + x_shift;
	drawpoint(col1, 0);
	drawpoint(col1, 1);
	drawpoint(col1, 2);
	drawpoint(col1, 3);
	drawpoint(col1, 3);
	drawpoint(col1, 4);
}


void draw_digit_horiz_top(int x_shift) {
	int col1 = 0 + x_shift;
	int col2 = 1 + x_shift;
	int col3 = 2 + x_shift;
	drawpoint(col1, 0);
	drawpoint(col2, 0);
	drawpoint(col3, 0);
}


void draw_digit_horiz_mid(int x_shift) {
	int col1 = 0 + x_shift;
	int col2 = 1 + x_shift;
	int col3 = 2 + x_shift;
	drawpoint(col1, 2);
	drawpoint(col2, 2);
	drawpoint(col3, 2);
}
void draw_digit_horiz_bot(int x_shift) {
	int col1 = 0 + x_shift;
	int col2 = 1 + x_shift;
	int col3 = 2 + x_shift;
	drawpoint(col1, 4);
	drawpoint(col2, 4);
	drawpoint(col3, 4);
}


//============================================
// Number Screen
//============================================
void draw_number(int number, int x_shift) {
	int col1 = 0 + x_shift;
	int col2 = 1 + x_shift;
	int col3 = 2 + x_shift;
	
	if (number == 0) {
		draw_digit_vert_left(x_shift);
		draw_digit_vert_right(x_shift);
		draw_digit_horiz_top(x_shift);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 1) {
		drawpoint(col2, 0);
		drawpoint(col2, 1);
		drawpoint(col2, 2);
		drawpoint(col2, 3);
		drawpoint(col2, 4);
	}
	else if (number == 2) {
		draw_digit_horiz_top(x_shift);
		drawpoint(col3, 1);
		draw_digit_horiz_mid(x_shift);
		drawpoint(col1, 3);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 3) {
		draw_digit_vert_right(x_shift);
		draw_digit_horiz_top(x_shift);
		draw_digit_horiz_mid(x_shift);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 4) {
		drawpoint(col1, 0);
		drawpoint(col1, 1);
		draw_digit_vert_right(x_shift);
		draw_digit_horiz_mid(x_shift);

	}
	else if (number == 5) {
		draw_digit_horiz_top(x_shift);
		drawpoint(col1, 1);
		draw_digit_horiz_mid(x_shift);
		drawpoint(col3, 3);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 6) {
		draw_digit_vert_left(x_shift);
		draw_digit_horiz_mid(x_shift);
		drawpoint(col1, 3);
		drawpoint(col3, 3);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 7) {
		draw_digit_vert_right(x_shift);
		draw_digit_horiz_top(x_shift);
	}
	else if (number == 8) {
		draw_digit_vert_left(x_shift);
		draw_digit_vert_right(x_shift);
		draw_digit_horiz_top(x_shift);
		draw_digit_horiz_mid(x_shift);
		draw_digit_horiz_bot(x_shift);
	}
	else if (number == 9) {
		draw_digit_vert_left(x_shift);
		drawpoint(col1, 1);
		drawpoint(col3, 1);
		draw_digit_horiz_mid(x_shift);
		draw_digit_horiz_bot(x_shift);
	}

}



//============================================
// Game Screens
//============================================

/**
 * Shows the startup screen
 */
void startup_screen() {
	draw_arrow(DIRECTION_A, GREEN);
	delay_ms(200);
	delay_ms(200);
	delay_ms(200);
	cleardisplay();
	draw_arrow(DIRECTION_B, GREEN);
	delay_ms(200);
	delay_ms(200);
	delay_ms(200);
	cleardisplay();
	draw_arrow(DIRECTION_C, GREEN);
	delay_ms(200);
	delay_ms(200);
	delay_ms(200);
	cleardisplay();
	draw_arrow(DIRECTION_D, GREEN);
	delay_ms(200);
	delay_ms(200);
	delay_ms(200);
	cleardisplay();
}


/**
 * Shows the game over screen
 */
void gameover_screen(int level) {
	setcolor(RED);
	draw_number((level % 10), 4);
	draw_number((level / 10), 0);
}




//============================================
// Main game logic
//============================================

int main(void) {
	init_random();
	avrinit();
	int cnt;
	byte btnDown = 0;
	byte level = 1;
	arrows[0] = DIRECTIONS[next_random(4)];

	initswapbuffers();
	swapinterval(10);		// note: display refresh is 100hz (lower number speeds up game)
	cleardisplay();
	start_timer1();			// this starts display refresh and audio processing
	button_init();
	initaudio();			// XXX eventually, we remove this!

	playsong(SONG_INTRO);
	startup_screen();
	delay_sec(1);


	//~~~ GOTO: NEXT LEVEL ~~~
	nextlevel:
		cleardisplay();
		
		//show all of the arrows in our history
		for(cnt=0; cnt<level; cnt++) {
			show_next_arrow(cnt);
		}
		
		cnt = 0;
	
		while(1) {
			cleardisplay();
			handlebuttons();
			
			//look for button presses and compare to history
			if (!btnDown) {

				//make sure we only count long button presses once
				if (ButtonA || ButtonB || ButtonC || ButtonD) {
					btnDown = 1;			
				}
	
		
				if (ButtonA) {
					draw_arrow(DIRECTION_A, YELLOW);
					playsong(DIRECTION_A_NOISE);
					delay_ms(100);
					if (arrows[cnt] == DIRECTION_A) {
						cnt++;
					}
					else {
						goto gameover;
					}
		
				}
				if (ButtonB) {
					draw_arrow(DIRECTION_B, YELLOW);
					playsong(DIRECTION_B_NOISE);
					delay_ms(100);
					if (arrows[cnt] == DIRECTION_B) {
						cnt++;
					}
					else {
						goto gameover;
					}
		
				}
		
				if (ButtonC) {
					draw_arrow(DIRECTION_C, YELLOW);
					playsong(DIRECTION_C_NOISE);
					delay_ms(100);
					if (arrows[cnt] == DIRECTION_C) {
						cnt++;
					}
					else {
						goto gameover;
					}
		
				}
				if (ButtonD) {
					draw_arrow(DIRECTION_D, YELLOW);
					playsong(DIRECTION_D_NOISE);
					delay_ms(100);
					if (arrows[cnt] == DIRECTION_D) {
						cnt++;
					}
					else {
						goto gameover;
					}
				
				}
				
	
				//this is the last arrow in the history, add another to the list?
				if (cnt == level) {
					if (level == 99) {
						goto gamewin;
					}
					cleardisplay();
					delay_ms(200);
					playsong(CORRECT_NOISE);
					level++;
					arrows[cnt] = DIRECTIONS[next_random(4)];
					delay_ms(200);
					delay_ms(200);
					goto nextlevel;
				}
	
			}
			else {
				if (!ButtonA && !ButtonB && !ButtonC && !ButtonD) {
					btnDown = 0;
				}
			}
	
			
	
		}
		
	//~~~ GOTO: GAME WIN ~~~
	gamewin:
		cleardisplay();
		//do something;
		playsong(SONG_WIN);
		gameover_screen(level);
		return (0);
	
	//~~~ GOTO: GAME OVER ~~~
	gameover:
		cleardisplay();
		delay_ms(200);
		delay_ms(200);
		playsong(SONG_TAPS);
		gameover_screen(level);
		return (0);
}
