/*
 * programming.h
 *
 *  Created on: 24.01.2012
 *      Author: matthias
 */

#ifndef PROGRAMMING_H_
#define PROGRAMMING_H_

void applyProgram(void);
void dismissProgramChanges(uint16_t minutes);
uint8_t getProgram(uint8_t weekday, uint8_t hour, uint8_t slice);
void setProgram(uint8_t weekday, uint8_t hour, uint8_t slice, uint8_t temperatureIndex);
void setTemperature(uint8_t num, uint16_t temperature);


#endif /* PROGRAMMING_H_ */
