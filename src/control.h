/*
 * control.h
 *
 *  Created on: 24.01.2012
 *      Author: matthias
 */

#ifndef CONTROL_H_
#define CONTROL_H_

extern int16_t targetTemperature;

void control(void);
void setNominalTemperature(int16_t temperature);
#define getNominalTemperature() ((const int16_t) targetTemperature)

/**
 * all controller parameters will be scaled by 256.
 * this allows a full vent open/close just by I part.
 * -> p = 1 will give 1 at output for e = 2.56 degrees
 */
typedef struct {
	int16_t k_p;		/* scaled by 256 as noted above */
	int16_t k_d;		/* NOT scaled by 256 */
	int16_t k_i;		/* scaled by an additional factor of 16 */

	int16_t i_val;		/* this is normally scaled (256) */
	int16_t i_max;		/* will be taken as maximum and minimum */
	int16_t e_last;
	uint32_t t_last;
	uint8_t i_scale_off;	/* divide integral value additions by that when heater is off */
	uint8_t i_scale_p;		/* divide same for large p values */
	int8_t i_scale_p_lim;	/* (p) > (lim*256) activates i_scale_p */
} CONTROLLER;

extern CONTROLLER controller;

#endif /* CONTROL_H_ */
