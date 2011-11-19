/*
 * motor.c
 *
 *  Created on: 19.11.2011
 *      Author: matthias
 */

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "keys.h"
#include "motor.h"


#define MOTOR_PORT PORTE
#define MOTOR_DDR DDRE
#define MOTOR_PIN_L PE7
#define MOTOR_PIN_R PE6

#define MOTOR_SENSE_DDR DDRE
#define MOTOR_SENSE_PORT PORTE
#define MOTOR_SENSE_PIN PE2


#define ADC_CH_MOTOR (2)
#define ADC_CH_MOTOR_SENSE (0)

#define MOTOR_STOP (MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R)))
#define MOTOR_OPEN (MOTOR_PORT |= (1 << MOTOR_PIN_L))
#define MOTOR_CLOSE (MOTOR_PORT |= (1 << MOTOR_PIN_R))

#define MOTOR_SENSE_ON (MOTOR_SENSE_PORT |= (1 << MOTOR_SENSE_PIN))
#define MOTOR_SENSE_OFF (MOTOR_SENSE_PORT &= ~(1 << MOTOR_SENSE_PIN))

#define DIR_OPEN 1
#define DIR_CLOSE -1
#define DIR_STOP 0

/* hardcoded blocking current limit */
#define MOTOR_CURRENT_BLOCK 935

volatile static int8_t Direction = 0;
volatile static int16_t MotorPosition = 0;

/* Motorgeschwindigkeit Leerlauf ~ 60 Schritte pro Sekunde.
 * Motorstrom Leerlauf ~30 mA
 * Motorstrom Kurzschluss ~180 mA
 * Motorstrom starke Belastung
 */

void motorInit(void)
{
	  MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R));
	  MOTOR_DDR |= (1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R);

	  MOTOR_SENSE_DDR |= (1 << MOTOR_SENSE_PIN);
	  PCMSK0 |= (1 << PCINT1);
}

static uint16_t getCurrent(void)
{
	ADMUX = (1 << REFS0) | ADC_CH_MOTOR;
	ADCSRA |= (1 << ADEN) | (1 << ADSC);
	while(ADCSRA & (1 << ADSC))
		;
	return ADC;
}

static void motorMove(void)
{
	MOTOR_SENSE_ON;
	MOTOR_STOP;
	switch(Direction)
	{
	case DIR_OPEN:
		MOTOR_OPEN;
		break;
	case DIR_CLOSE:
		MOTOR_CLOSE;
		break;
	default:
		MOTOR_SENSE_OFF;
		break;
	}
}

void motorStopMove(void)
{
	MOTOR_STOP;
	_delay_ms(10);
	MOTOR_SENSE_OFF;
	Direction = DIR_STOP;
}

void motorStepOpen(void)
{
	Direction = DIR_OPEN;
	motorMove();
	while(key_state & KEY_ALL)
	{
		displayString("....");
		displayNumber(getCurrent());
		_delay_ms(50);
	}
	motorStopMove();
}

void motorStepClose(void)
{
	Direction = DIR_CLOSE;
	motorMove();
	while (key_state & KEY_ALL) {
		displayString("....");
		displayNumber(getCurrent());
		_delay_ms(50);
	}
	motorStopMove();
}

void motorDetect(void)
{

}

/**
 * called by interrupt
 */
void motorStep(void)
{
	MotorPosition += Direction;
}
