/*
 * control.c
 *
 *  Created on: 24.01.2012
 *      Author: matthias
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "ntc.h"
#include "timer.h"
#include "motor.h"
#include "control.h"


int16_t targetTemperature;

/* a controller result of 0 will set vent to halve open */
CONTROLLER controller = {
		150,		/* k_p */
		10,			/* k_d 50: +0.5 deg in 60 seconds -> ud~ -50 */
		2,			/* k_i */
		-15000,		/* i_val */
		20000,		/* i_max */
		0,			/* e_last */
		0,			/* t_last */
		5,			/* i_scale_off factor */
		2,			/* i_scale_p factor*/
		100			/* i_scale_p_lim aware: not scaled!*/
};



/**
 * controller main function. Has to be called some times.
 * exact timing does not matter, but once a minute should be the minimum!
 * @pre system status variables (temperature) should be up2date!
 */
void control(void)
{
	CONTROLLER ctrl = controller;
	uint32_t systemTime = SystemTime;
	uint8_t deltaTime = systemTime - controller.t_last;
	int16_t e = targetTemperature - getNtcTemperature();
	int16_t up = ctrl.k_p * e;
	/* scale differential part here so we can use it easier */
	int16_t ud = (((int16_t)ctrl.k_d * (e - ctrl.e_last)) / deltaTime) * 256;
	int16_t ui;
	int16_t i_change;
	int16_t result;

	i_change = ctrl.k_i * e * deltaTime / 16;
	/* todo:
	 *   // dampen effect of turned of heater
	 * if(i_change > 0 && heaterStatusOff)
	 *   i_change /= ctrl.i_scale_off;
	 *
	 */
	/* slower change of integral value for large p parts.
	 * this hopefully dampens overshoots on system start
	 */
	if(i_change > 0 && (up)/256 > ctrl.i_scale_p_lim)
		i_change /= ctrl.i_scale_p;

	ctrl.i_val += i_change;

	if(ctrl.i_val > ctrl.i_max)
		ctrl.i_val = ctrl.i_max;
	else if(ctrl.i_val < -ctrl.i_max)
		ctrl.i_val = -ctrl.i_max;

	ui = ctrl.i_val;

	result = up + ud + ui;
	motorMoveTo((128 + result/256));


	ctrl.e_last = e;
	ctrl.t_last = systemTime;
	controller = ctrl;
}

/**
 * should be called to set nominal temperature.
 * @param temperature
 */
void setNominalTemperature(int16_t temperature)
{
	targetTemperature = temperature;
}
