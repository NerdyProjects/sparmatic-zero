/*
 * wait.h
 *
 *  Created on: 13.11.2011
 *      Author: matthias
 */

#ifndef WAIT_H_
#define WAIT_H_

#ifdef __AVR__
#include <util/delay.h>
#define wait10us(x)

static inline void waitms(uint8_t ms)
{
	while(ms--)
		_delay_ms(1);
}

#endif


#endif /* WAIT_H_ */
