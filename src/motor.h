/*
 * motor.h
 *
 *  Created on: 19.11.2011
 *      Author: matthias
 */

#ifndef MOTOR_H_
#define MOTOR_H_


void motorInit(void);
void motorStepOpen(void);
void motorStepClose(void);
uint8_t motorStep(void);
void motorDetectFullOpen(void);
uint8_t motorIsRunning(void);
void motorTimer(void);

#endif /* MOTOR_H_ */
