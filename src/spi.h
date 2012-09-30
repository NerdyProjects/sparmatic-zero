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

#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>


extern void spi_init(void);
extern void spi_rw(uint8_t * data, uint8_t len);
extern void spi_w (const uint8_t * dataout, uint8_t len);

// SPI Interupt Enable
// Data Order (0:MSB first / 1:LSB first)
// Master/Slave select
// SPI Clock Rate: fOsc / 2 for minimum active time
// Clock Polarity (0:SCK low / 1:SCK hi when idle)
// Clock Phase (0:leading / 1:trailing edge sampling))
#define SPI_MSTR	{SPCR = ((1 << SPE) | \
			(0 << SPIE) | \
			(0 << DORD) |\
			(1 << MSTR) |  \
			(0 << SPR1) | (0 << SPR0) | \
			(0 << CPOL) |  \
			(0 << CPHA)); SPSR; SPDR; }

static inline uint8_t spi_rw1 (uint8_t data)
// Clocks only one byte to target device and returns the received one
{
    SPDR = data;
    while((SPSR & (1<<SPIF))==0)
    	;
    return SPDR;
}

#endif /* _SPI_H_ */
