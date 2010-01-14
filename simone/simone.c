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
 *	- Jan 10, 20010 - jesse
 *		created.
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


/* audio stuff - beware, these song tables take up much memory! (so comment out unused tables) */


//
// a few notes from a well known classical melody (J.S. Bach, Cantata BWV 147)
//
byte ClassicalIntroSong[] = {
N_E4,N_8TH,
N_C4,N_8TH,
N_D4,N_8TH,
N_E4,N_8TH,
N_G4,N_8TH,
N_F4,N_8TH,
N_F4,N_8TH,
N_A4,N_8TH,
N_G4,N_8TH,
N_G4,N_8TH,
N_C5,N_8TH,
N_B4,N_8TH,
N_C5,N_8TH,
N_G4,N_8TH,
N_E4,N_8TH,
N_C4,N_HALF,

N_END,
};


/* colors */
#define DIRECTION_A 0
#define DIRECTION_B	1
#define DIRECTION_C	2
#define DIRECTION_D	3




static byte arrows[100] = {DIRECTION_B};
//static byte[100] colors;
//static byte[100] sounds;


//
// crude delay of 1 to 255 us
//
void
delay_us(byte usec)
{
	usec++;
	
	while (--usec) {
		_delay_us(1);		// get 1us delay from library macro (see <util/delay.h>)
	}
}


//
// crude delay of 1 to 255 ms
//
void
delay_ms(uint8_t ms)
{
	
	ms++;
	
	while (--ms) {
		_delay_ms(1);		// get 1ms delay from library macro (see <util/delay.h>)
	}
}


//
// crude "sleep" function for 0 to 255 seconds
//
void
delay_sec(uint8_t sec)
{
	uint8_t i;
	for (i = 0; i < sec; i++) {
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
	}
}


#define XMAX (XSCREEN-1)
#define YMAX (YSCREEN-1)


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


int
main(void)
{
	int cnt;
	byte btnDown = 0;
	byte level = 1;

    avrinit();

	initswapbuffers();
	swapinterval(10);		// note: display refresh is 100hz (lower number speeds up game)
	cleardisplay();

	start_timer1();			// this starts display refresh and audio processing
	
	button_init();

	initaudio();			// XXX eventually, we remove this!

	//setwavetable(WT_SINE);
	playsong(ClassicalIntroSong);	// test audio

	startup_screen();

	delay_sec(1);


	//
	// and now, the game
	//
nextlevel:
    cleardisplay();
    for(cnt=0; cnt<level; cnt++) {
	    draw_arrow(arrows[cnt], GREEN);
	    delay_ms(200);
	    delay_ms(200);
	    delay_ms(200);
	    cleardisplay();		
    }
    
    cnt = 0;

	while(1) {

		cleardisplay();
		handlebuttons();
		
		
		if (!btnDown) {

			if (ButtonA || ButtonB || ButtonC || ButtonD) {
				btnDown = 1;			
			}

	
			if (ButtonA) {
				draw_arrow(DIRECTION_A, RED);
				delay_ms(100);
				if (arrows[cnt] == DIRECTION_A) {
					cnt++;
				}
				else {
					goto gameover;
				}
	
			}
			if (ButtonB) {
				draw_arrow(DIRECTION_B, RED);
				delay_ms(100);
				if (arrows[cnt] == DIRECTION_B) {
					cnt++;
				}
				else {
					goto gameover;
				}
	
			}
	
			if (ButtonC) {
				draw_arrow(DIRECTION_C, RED);
				delay_ms(100);
				if (arrows[cnt] == DIRECTION_C) {
					cnt++;
				}
				else {
					goto gameover;
				}
	
			}
			if (ButtonD) {
				draw_arrow(DIRECTION_D, RED);
				delay_ms(100);
				if (arrows[cnt] == DIRECTION_D) {
					cnt++;
				}
				else {
					goto gameover;
				}
			
			}
			

		
			if (cnt == level) {
				level++;
				arrows[cnt] = DIRECTION_D;
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


gameover:
	cleardisplay();
	return (0);
}
