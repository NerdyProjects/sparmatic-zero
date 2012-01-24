/*
 * timer.c
 *
 *  Created on: 20.11.2011
 *      Author: matthias
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

typedef void (*TimerCallback)(void);

static volatile uint8_t Seconds;
static uint8_t Minutes;
static uint8_t Hours;
static uint8_t Weekday;
volatile uint8_t Timer0H;
volatile uint32_t SystemTime;

static volatile TIME Time;

static TimerCallback TimeoutCallback;

ISR(TIMER0_OVF_vect) {
	++Timer0H;
}

ISR(TIMER0_COMP_vect) {
	if(TimeoutCallback) {
		TimeoutCallback();
		TimeoutCallback = 0;
		TIMSK0 &= ~(1 << OCIE0A);
	}

}

/* used for waking up the device periodically */
ISR(TIMER2_OVF_vect)
{
	TIME tTime = Time;

	tTime.second += 8;
	SystemTime += 8;
	if(tTime.second > 59) {
		tTime.second -= 60;
		tTime.minute += 1;
		if(tTime.minute > 59) {
			tTime.minute = 0;
			tTime.hour += 1;
			if(tTime.hour > 23) {
				tTime.hour = 0;
				tTime.weekday += 1;
				if(tTime.weekday > 6) {
					tTime.weekday = 0;
				}
			}
		}
	}

	Time = tTime;
}

void timerInit(void)
{
	ASSR |= (1 << AS2);
	TCCR2A = (1 << CS20) | (1 << CS21) | (1 << CS22);	/* normal mode, 32768/1024 = 32 Hz, 1/8 Hz Interrupt rate */
	TCNT2 = 0;
	OCR2A = 0;
	while(ASSR & ((1 << TCN2UB) | (1 << OCR2UB) | (1 << TCR2UB)))
		;
	TIFR2 = (1 << OCF2A) | (1 << TOV2);
	TIMSK2 = (1 << TOIE2);

	TCCR0A = (1 << CS00) | (1 << CS02);	/* CLK / 1024 -> 1ms */
	TIMSK0 = (1 << TOIE0);
}

void enableTimeout(TimerCallback cbk, uint8_t timeout)
{
	TimeoutCallback = cbk;
	OCR0A = TCNT0 + timeout;
	TIFR0 |= (1 << OCF0A);
	TIMSK0 |= (1 << OCIE0A);
}

void setTimeout(uint8_t timeout)
{
	OCR0A = TCNT0 + timeout;
}

void disableTimeout()
{
	TIMSK0 &= ~(1 << OCIE1A);
	TimeoutCallback = 0;
}

void setTime(uint8_t weekday, uint8_t hour, uint8_t minute)
{
	cli();
	Minutes = minute;
	Hours = hour;
	Weekday = weekday;
	sei();
}

TIME getTime(void)
{
	TIME res;
	cli();
	res = Time;
	sei();
	return res;
}
