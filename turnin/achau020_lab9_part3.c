/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerISR(){ TimerFlag = 1; }
void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff(){
	TCCR1B = 0x00;
}
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum states { start, init, zero, one, two } state;
enum states_2 { starts, inits, on, off } state_2;
enum states_3 { start_3, init_3, on_3, off_3 } state_3;
unsigned char threeLED = 0x00;
unsigned char blinkingLED = 0x00;
unsigned char speaker = 0x00;
void threeLEDsSM(){
	switch(state){
		case start:
			state = init;
		break;
		case init:
			state = zero;
		break;
		case zero:
			state = one;
		break;
		case one:
			state = two;
		break;
		case two:
			state = zero;
		break;
		default:
		break;
	}

	switch(state){
		case zero:
			threeLED = 0x01;
		break;
		case one:
			threeLED = 0x02;
		break;
		case two:
			threeLED = 0x04;
		break;
		default:
		break;
	}

}

void blinkingLEDSM(){
	switch(state_2){
		case starts:
			state_2 = inits;
		break;
		case inits:
			state_2 = off;
		break;
		case on:
			state_2 = off;
		break;
		case off:
			state_2 = on;
		break;
		default:
		break;
	}

	switch(state_2){
		case starts:
		break;
		case on:
			blinkingLED = 0x01;		
		break;
		case off:
			blinkingLED = 0x00;
		break;
		default:
		break;
	}

}

void speakerSM(){
	unsigned char button = (~PINA & 0x07);
	switch(state_3){
		case start_3:
			state_3 = init_3;
		break;
		case init_3:
			if(button == 0x04) {
	       			state_3 = on_3;
			} else {
				state_3 = init_3;
			}
		break;
		case on_3: 
			if(button == 0x00){
				state_3 = off_3;
			} else {
				state_3 = on_3;
			}
		break;
		case off_3:
			if(button == 0x04){
				state_3 = on_3;
			} else {
				state_3 = off_3;
			}
		break;
		default:
		break;
	}

	switch(state_3){
		case init_3:
		break;
		case on_3:
			speaker = 0x01;
		break;
		case off_3:
			speaker = 0x00;
		break;
		default:
		break;
	}
}

enum combineStates { cState } combineState;

void tick(){
	switch(combineState){
		case cState:
			combineState = cState;
		break;
		default:
		break;
	}

	switch(combineState) {
		case cState:
			PORTB = ((speaker << 4) | (blinkingLED << 3) | (threeLED));
		break;

		default:
		break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	unsigned long TL_elapsedTime = 300;
	unsigned long BL_elapsedTime = 1000;
	unsigned long SP_elapsedTime = 2;
	const unsigned long timerPeriod = 2;
	TimerSet(timerPeriod);
	TimerOn();
	state = start;
	state_2 = starts;
	state_3 = start_3;
	combineState = cState;
    /* Insert your solution below */
    while (1) {
	if(TL_elapsedTime >= 300) {
                threeLEDsSM();
                TL_elapsedTime = 0;
        }
        if(BL_elapsedTime >= 1000){
                blinkingLEDSM();
                BL_elapsedTime = 0;
        }
	if(SP_elapsedTime >= 2){
		speakerSM();
		SP_elapsedTime = 0;
	}
	tick();
	
	while(!TimerFlag);
	TimerFlag = 0;
	TL_elapsedTime += timerPeriod;
	BL_elapsedTime += timerPeriod;
	SP_elapsedTime += timerPeriod;
    }
    return 1;
}
