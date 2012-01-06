/*
 * adc.h
 *
 *  Created on: 06.01.2012
 *      Author: matthias
 */

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

static inline uint16_t getAdc(uint8_t channel)
{

	ADMUX = (1 << REFS0) | (channel & 0x1F);
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2);
	while (ADCSRA & (1 << ADSC))
		/* Conversion time will be about 400 µs at first conversion, 200 µs any other */
		;
	return ADC;
}



#endif /* ADC_H_ */
