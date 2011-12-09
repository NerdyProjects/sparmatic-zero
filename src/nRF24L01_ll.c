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

    changed 2011 by Matthias Larisch
    Low Level nRF24L01+ access for Atmel AVR devices
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "nRF24L01_ll.h"
#include "spi.h"

void _nRF24L01_init(void)
// Initializes pins and interrupt to communicate with the MiRF module
// Should be called in the early initializing phase at startup.
{
    // Define CSN and CE as Output and set them to default
	mirf_CE_lo;
	mirf_CSN_hi;
	CE_DDR |= (1 << CE);
    CSN_DDR |= (1 << CSN);
    IRQ_PORT |= (1 << IRQ_PIN);


#ifdef IRQ_MODE_INT0
    // Initialize external interrupt 0 (PD2)
    MCUCR = ((1<<ISC11)|(0<<ISC10)|(1<<ISC01)|(0<<ISC00)); // Set external interupt on falling edge
    GICR  = ((0<<INT1)|(1<<INT0));                         // Activate INT0
#endif // INT0

#ifdef IRQ_MODE_PINCHANGE0
    // Initialize external interrupt
    IRQ_DDR &= ~(1<<IRQ_PIN);
    PCMSK0 |= (1<<IRQ_PIN);
    EIMSK  |= (1<<PCIE0);
#endif

#ifdef IRQ_MODE_PINCHANGE2
    // Initialize external interrupt
    IRQ_DDR &= ~(1<<IRQ_PIN);
    PCMSK2 |= (1<<IRQ_PIN);
    EIMSK  |= (1<<PCIE2);
#endif

    // Initialize spi module
    spi_init();
}



/*
#ifdef IRQ_MODE_INT0
ISR(INT0_vect)
#endif
#ifdef IRQ_MODE_PINCHANGE2
ISR(PCINT2_vect)
#endif
#ifdef IRQ_MODE_PINCHANGE0
ISR(PCINT0_vect)
#endif
// Interrupt handler
{
    nRF24L01IRQ();
}
*/
