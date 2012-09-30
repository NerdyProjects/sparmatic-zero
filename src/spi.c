/*
 Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use, copy,
 modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.

 $Id$
 */

#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define PORT_SPI    PORTB
#define DDR_SPI     DDRB
#define DD_MISO     DDB3
#define DD_MOSI     DDB2
#define DD_OWN_SS	DDB0		/* Sparmatic: This pin is used as button '+' or rotary input */
#define DD_SCK      DDB1



void spi_init()
// Initialize pins for spi communication
{
	SPSR = (1 << SPI2X); // Double Clock Rate

	SPI_MSTR;
}

void spi_rw(uint8_t * data, uint8_t len)
// Shift full array through target device
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		SPDR = data[i];
		while((SPSR & (1<<SPIF))==0)
			;
		data[i] = SPDR;
	}
}

void spi_w(const uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		SPDR = dataout[i];
		while((SPSR & (1<<SPIF))==0)
			;
	}
}



