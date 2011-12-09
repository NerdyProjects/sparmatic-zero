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
	DDR_SPI &= ~((1 << DD_MOSI) | (1 << DD_MISO) | (1 << DD_SCK));
	// Define the following pins as output
	DDR_SPI |= ((1 << DD_MOSI) | (1 << DD_SCK));

	SPCR = ((1 << SPE) | // SPI Enable
			(0 << SPIE) | // SPI Interupt Enable
			(0 << DORD) | // Data Order (0:MSB first / 1:LSB first)
			(1 << MSTR) | // Master/Slave select
			(0 << SPR1) | (0 << SPR0) | // SPI Clock Rate: fOsc / 2 for minimum active time
			(0 << CPOL) | // Clock Polarity (0:SCK low / 1:SCK hi when idle)
			(0 << CPHA)); // Clock Phase (0:leading / 1:trailing edge sampling)

	SPSR = (1 << SPI2X); // Double Clock Rate

	if(SPSR & (1 << SPIF))
		SPDR;	/* Clear SPIF */

}

void spi_rw(const uint8_t * dataout, uint8_t * datain, uint8_t len)
// Shift full array through target device
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		datain[i] = spi_rw1(dataout[i]);
	}
}

void spi_w(const uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
{
	uint8_t i;
	for (i = 0; i < len; i++) {
		spi_rw1(dataout[i]);
	}
}



