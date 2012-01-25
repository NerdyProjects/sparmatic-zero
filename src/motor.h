/*
 * motor.h
 *
 *  Created on: 19.11.2011
 *      Author: matthias
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define MOTOR_SENSE_PIN PE1

void motorInit(void);
void motorStepOpen(void);
void motorStepClose(void);
uint8_t motorStep(void);
void motorMoveTo(uint8_t valve);
uint8_t motorFullOpen(void);
uint8_t motorAdapt(void);
uint8_t motorIsRunning(void);
void motorTimer(void);


extern volatile int16_t MotorPosition;

#define getMotorPosition(x) ((const volatile int16_t)MotorPosition)

#endif /* MOTOR_H_ */
