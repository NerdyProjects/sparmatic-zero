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
#include "timer.h"


#define MOTOR_PORT PORTE
#define MOTOR_DDR DDRE
#define MOTOR_PIN_L PE7
#define MOTOR_PIN_R PE6

#define MOTOR_SENSE_DDR DDRE
#define MOTOR_SENSE_PORT PORTE
#define MOTOR_SENSE_PORT_IN PINE
#define MOTOR_SENSE_PIN PE1
#define MOTOR_SENSE_LED_PIN PE2


#define ADC_CH_MOTOR (2)
#define ADC_CH_MOTOR_SENSE (0)

#define MOTOR_STOP (MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R)))
#define MOTOR_OPEN (MOTOR_PORT |= (1 << MOTOR_PIN_L))
#define MOTOR_CLOSE (MOTOR_PORT |= (1 << MOTOR_PIN_R))

#define MOTOR_SENSE_ON (MOTOR_SENSE_PORT |= (1 << MOTOR_SENSE_LED_PIN))
#define MOTOR_SENSE_OFF (MOTOR_SENSE_PORT &= ~(1 << MOTOR_SENSE_LED_PIN))

#define DIR_OPEN 1
#define DIR_CLOSE -1
#define DIR_STOP 0

/* hardcoded blocking current limit. Currents are in ADC digits.
 * For real current the formular is as follows:
 * I = (1024 - ADCval) * Ubat / 2.2
 * The loaded motor behaves very much resistive so Ubat can be eliminated. Result wont be
 * a current but an indicator for motor load. */
#define MOTOR_CURRENT_BLOCK 940
/* all speeds are given in ms per tick.
 * A tick is a period on motor sense pin: LH for Forward, HL for Backwards.
 */
/* block: Stop if no pulse is received in that interval */
#define MOTOR_SPEED_BLOCK 95

volatile static int8_t Direction = 0;
volatile static int16_t MotorPosition = 0;
/* used as stop condition, initialized to very slow speed stop/block stop */
static uint8_t MotorTimeout = 95;
static uint16_t CurrentLimit = 940;

/* Motorgeschwindigkeit Leerlauf ~ 70-90 Schritte pro Sekunde
 * Motorgeschwindigkeit Last ~40-80 Schritte pro Sekunde
 * Motorstrom Leerlauf ~30 mA
 * Motorstrom Kurzschluss ~180 mA
 * Motorstrom starke Belastung
 */

void motorInit(void)
{
	  MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R));
	  MOTOR_DDR |= (1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R);

	  MOTOR_SENSE_DDR |= (1 << MOTOR_SENSE_LED_PIN);
	  PCMSK0 |= (1 << PCINT1);
}

static uint16_t getCurrent(void)
{
	ADMUX = (1 << REFS0) | ADC_CH_MOTOR;
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2);
	while(ADCSRA & (1 << ADSC))	/* Conversion time will be about 400 µs at first conversion, 200 µs any other */
		;
	return ADC;
}

void motorStopMove(void)
{
	disableTimeout();
	MOTOR_STOP;
	_delay_ms(800);
	MOTOR_SENSE_OFF;
	Direction = DIR_STOP;
}

static void motorMove(int8_t dir)
{
	MOTOR_SENSE_ON;
	MOTOR_STOP;
	enableTimeout(motorStopMove, 255);	/* first timeout long to allow startup */
	switch(dir)
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
	Direction = dir;
}



void motorStepOpen(void)
{
	motorMove(DIR_OPEN);
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
	motorMove(DIR_CLOSE);
	while (key_state & KEY_ALL) {
		displayString("....");
		displayNumber(getCurrent());
		_delay_ms(50);
	}
	motorStopMove();
}

static void motorStopMoveAtBlock(void)
{
	uint8_t timerOld = TCNT0;
	int16_t position = MotorPosition;
	int16_t positionTemp;
	uint8_t currentLimitOk;
	uint8_t timeLimitOk;
	do {
		currentLimitOk = getCurrent() > MOTOR_CURRENT_BLOCK;
		timeLimitOk = ((uint8_t)(TCNT0 - timerOld)) < MOTOR_SPEED_BLOCK;
		positionTemp = MotorPosition;
		if(positionTemp != position) {
			position = positionTemp;
			timerOld = TCNT0;
		}
	} while(currentLimitOk && timeLimitOk);

	if(!currentLimitOk)
		displaySymbols(LCD_AUTO, LCD_AUTO);
	if(!timeLimitOk)
		displaySymbols(LCD_MANU, LCD_MANU);

	motorStopMove();
}

static int16_t detectValveOpen(void) {

}

/*
 * Fully opens the motor to detect end position. Closes until it detects touching the vent.
 */
void motorDetectFullOpen(void)
{
	uint16_t currentNormal;
	int16_t positionValveOpen;
	motorMove(DIR_OPEN);
	motorStopMoveAtBlock();
	MotorPosition = 1000;
	motorMove(DIR_CLOSE);
	while(MotorPosition > 990)
		;
	currentNormal = getCurrent();
	positionValveOpen = detectValveOpen();

	motorStopMove();
	displayString("...");
	displayNumber(MotorPosition);
}

/**
 * called by interrupt
 */
uint8_t motorStep(void)
{
	uint8_t state = MOTOR_SENSE_PORT_IN & (1 << MOTOR_SENSE_PIN);
	if(((Direction == DIR_OPEN) && state) || ((Direction == DIR_CLOSE) && !state)) {
		setTimeout(MotorTimeout);
		MotorPosition += Direction;
		return 1;
	}
	return 0;
}

uint8_t motorIsRunning(void)
{
	return (Direction != DIR_STOP);
}

/**
 * periodic callback as long as the motor is running.
 * used for current limit or stop condition tests.
 */
void motorTimer(void)
{
	uint16_t current = getCurrent();
	if(current < CurrentLimit) {
		motorStopMove();
	}
}
