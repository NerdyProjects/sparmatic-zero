/*
 * timer.h
 *
 *  Created on: 20.11.2011
 *      Author: matthias
 */

#ifndef TIMER_H_
#define TIMER_H_
#include "timerdef.h"

/*
 * 1/32th second per step.
 * TODO this wont work within one timer step after wake up from sleep!
 * We will get old value before going to sleep!
 */
#define timerGet() (TCNT2)

extern const volatile uint8_t Seconds;
extern const volatile uint8_t Minutes;
extern const volatile uint8_t Hours;
extern const volatile uint8_t Weekday;

extern const volatile uint8_t Timer0H;

void timerInit(void);
void enableTimeout(TimerCallback cbk, uint8_t timeout);
void setTimeout(uint8_t timeout);
void disableTimeout(void);
void setTime(uint8_t weekday, uint8_t hour, uint8_t minute);


#endif /* TIMER_H_ */
