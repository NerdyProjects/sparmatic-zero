/*
 * programming.c
 *
 *  Created on: 24.01.2012
 *      Author: matthias
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "timer.h"
#include "control.h"
#include "lcd.h"

#define PROGRAM_DATA_BYTES_PER_WEEKDAY 24
#define PROGRAM_DATA_BYTES_PER_HOUR 1
#define PROGRAM_TIME_SLICE_MINUTES	15
#define PROGRAM_BITS	2
#define PROGRAM_BITS_SET (0x03)
/* stores program data information:
 * 2 bits for each 15 minute slot in each hour for each weekday.
 * layout:
 * weekday 0, 1, 2, 3, 4, 5, 6
 * each weekday: hour 0, 1, 2, 3, .. 23
 * one hour is 1 byte:
 * LSB  1    2    3     4    5     6    MSB
 * t0l t0h  t15l t15h  t30l t30h  t45l t45h
 */
uint8_t ProgramData[PROGRAM_DATA_BYTES_PER_WEEKDAY * 7];

/* corresponding programmed temperatures */
int16_t Temperatures[4] = { 2050, 1450, 1650, 2200 };

/* weekday == 7 -> no dismiss */
TIME dismissUntil = { 7, 0, 0, 0 };


static uint8_t getHourProgram(uint8_t weekday, uint8_t hour)
{
	uint8_t hourProgram = ProgramData[weekday * PROGRAM_DATA_BYTES_PER_WEEKDAY
				+ hour * PROGRAM_DATA_BYTES_PER_HOUR];
	return hourProgram;
}
/**
 * returns temperature index for given program.
 */
uint8_t getProgram(uint8_t weekday, uint8_t hour, uint8_t slice) {
	uint8_t hourProgram = getHourProgram(weekday, hour);
	uint8_t temperatureIndex = hourProgram >> (slice) * PROGRAM_BITS;

	temperatureIndex &= 0xFF << PROGRAM_BITS;

	return temperatureIndex;
}

void setProgram(uint8_t weekday, uint8_t hour, uint8_t slice, uint8_t temperatureIndex) {
	uint8_t hourProgram = getHourProgram(weekday, hour);

	hourProgram &= ~(PROGRAM_BITS_SET << slice * PROGRAM_BITS);
	hourProgram |= temperatureIndex << slice * PROGRAM_BITS;

	ProgramData[weekday * PROGRAM_DATA_BYTES_PER_WEEKDAY
					+ hour * PROGRAM_DATA_BYTES_PER_HOUR] = hourProgram;
}
/**
 * loads a program for the current time.
 */
void applyProgram(void) {
	TIME time = getTime();

	/* crappy weekday-overflow handling... */
	if (dismissUntil.weekday != 7
			&& (compareTime(time, dismissUntil) <= 0
					|| (time.weekday > 3 && dismissUntil.weekday < 3))) {
		displaySymbols(LCD_MANU, LCD_MANU | LCD_AUTO);
	} else {
		uint8_t temperatureIndex;
		dismissUntil.weekday = 7;
		temperatureIndex = getProgram(time.weekday, time.hour,
				time.minute / PROGRAM_TIME_SLICE_MINUTES);
		setNominalTemperature(Temperatures[temperatureIndex]);
		displaySymbols(LCD_AUTO, LCD_MANU | LCD_AUTO);
	}
}

void setTemperature(uint8_t num, uint16_t temperature)
{
	Temperatures[num] = temperature;
}

/**
 * disables program changing for given time.
 * @param quarterHours time in program-slices
 */
void dismissProgramChanges(uint16_t minutes) {
	TIME time = getTime();
	addToTime(time, 0, minutes);
	dismissUntil = time;
}
