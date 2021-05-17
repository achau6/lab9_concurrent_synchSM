/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	Demo: https://youtu.be/P7o360zqCr8
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
unsigned char threeLED = 0x00;
int blinkingLED = 0;
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
			PORTB = threeLED;
		break;
		case one:
			threeLED = 0x02;
			PORTB = threeLED;
		break;
		case two:
			threeLED = 0x04;
			PORTB = threeLED;
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
			PORTB = threeLED | 0x08;
		break;
		case off:
			//PORTB = threeLED;
		break;
		default:
		break;
	}

}

void tick(){
	threeLEDsSM();
	blinkingLEDSM();
	
}
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	TimerSet(1000);
	TimerOn();
	state = start;
	state_2 = starts;
    /* Insert your solution below */
    while (1) {
	tick();
	//PORTB = 0x0F;
	while(!TimerFlag);
	TimerFlag = 0;
	
    }
    return 1;
}
