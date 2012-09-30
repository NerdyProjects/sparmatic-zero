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

#ifndef _MIRF_H_
#define _MIRF_H_

#include <avr/io.h>

// Pin definitions for chip select and chip enabled of the MiRF module
#define IRQ_MODE_PINCHANGE0
/* #define IRQ_MODE_PINCHANGE2 */
/* #define IRQ_MODE_INT0 */
#define CE_PORT		PORTF
#define CSN_PORT	PORTE
#define CE_DDR		DDRF
#define CSN_DDR		DDRE
#define IRQ_DDR		DDRE
#define IRQ_PORT	PORTE
#define IRQ_PORT_IN	PINE
#define IRQ_PIN		PE3
#define CE  PF7
#define CSN PE4

// Definitions for selecting and enabling MiRF module
#define mirf_CSN_hi     CSN_PORT |=  (1<<CSN);
#define mirf_CSN_lo     CSN_PORT &= ~(1<<CSN);
#define mirf_CE_hi      CE_PORT |=  (1<<CE);
#define mirf_CE_lo      CE_PORT &= ~(1<<CE);


void _nRF24L01_init(void);

#endif /* _MIRF_H_ */
