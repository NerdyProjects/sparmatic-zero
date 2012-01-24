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

#define PROGRAM_DATA_BYTES_PER_WEEKDAY 24
#define PROGRAM_DATA_BYTES_PER_HOUR 1
#define PROGRAM_TIME_SLICE_MINUTES	15
#define PROGRAM_BITS	2
/* stores program data information:
 * 2 bits for each 15 minute slot in each hour for each weekday.
 * layout:
 * weekday 0, 1, 2, 3, 4, 5, 6
 * each weekday: hour 0, 1, 2, 3, .. 23
 * one hour is 1 byte:
 * LSB  1    2    3     4    5     6    MSB
 * t0l t0h  t15l t15h  t30l t30h  t45l t45h
 */
uint8_t ProgramData[PROGRAM_DATA_BYTES_PER_WEEKDAY*7];

/* corresponding programmed temperatures */
int16_t Temperatures[4] = {2050, 1450, 1650, 2200};

/**
 * loads a program for the current time.
 */
void applyProgram(void)
{
	TIME time = getTime();
	uint8_t currentHourProgram = ProgramData[time.weekday *
	                                         PROGRAM_DATA_BYTES_PER_WEEKDAY +
	                                         time.hour *
	                                         PROGRAM_DATA_BYTES_PER_HOUR];

	uint8_t temperatureIndex = currentHourProgram >>
			(time.minute / PROGRAM_TIME_SLICE_MINUTES) * PROGRAM_BITS;
	temperatureIndex &= 0xFF << PROGRAM_BITS;

	setNominalTemperature(Temperatures[temperatureIndex]);

}
