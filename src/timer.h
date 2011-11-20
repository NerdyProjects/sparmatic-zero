/*
 * timer.h
 *
 *  Created on: 20.11.2011
 *      Author: matthias
 */

#ifndef TIMER_H_
#define TIMER_H_

void timerInit(void);

/*
 * 1/32th second per step.
 * TODO this wont work within one timer step after wake up from sleep!
 * We will get old value before going to sleep!
 */
#define timerGet() (TCNT2)

extern volatile uint8_t Timer0H;

typedef void (*TimerCallback)(void);

void enableTimeout(TimerCallback cbk, uint8_t timeout);
void setTimeout(uint8_t timeout);
void disableTimeout(void);

#endif /* TIMER_H_ */
