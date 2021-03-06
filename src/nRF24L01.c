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

#include <stdint.h>

#include "nRF24L01.h"
#include "nRF24L01_reg.h"
#include "nRF24L01_ll.h"
#include "spi.h"
#include "wait.h"

// Defines for setting the MiRF registers for transmitting or receiving mode
#define TX nRF24L01_write_register(CONFIG, DEF_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) )
#define RX nRF24L01_write_register(CONFIG, DEF_CONFIG | ( (1<<PWR_UP) | (1<<PRIM_RX) ) )

/*
 * 0: No TX in progress.
 * 1: TX in progress.
 * 2: TX in progress, switch to RX on completion.
 *
 * general behaviour: != 0 -> TX in progress
 */
static volatile uint8_t TxActive;

static void (*rxCallback)(void);

/**
 *@param cmd command
 *@param data not modified, only sent
 */
static uint8_t nRF24L01_command(uint8_t cmd, const uint8_t *data, uint8_t len)
{
	uint8_t sreg;
	SPI_MSTR;
	mirf_CSN_lo;
	sreg = spi_rw1(cmd);
	spi_w(data, len);
	mirf_CSN_hi;
	return sreg;
}
/**
 *@param cmd command
 *@param data output and input
 */
static uint8_t nRF24L01_commandR(uint8_t cmd, uint8_t *data, uint8_t len)
{
	uint8_t sreg;
	SPI_MSTR;
	mirf_CSN_lo;
	sreg = spi_rw1(cmd);
	spi_rw(data, len);
	mirf_CSN_hi;
	return sreg;
}

static uint8_t nRF24L01_read_register(uint8_t reg)
// Reads an array of bytes from the given start position in the MiRF registers.
{
	uint8_t value;
	nRF24L01_commandR(R_REGISTER | (REGISTER_MASK & reg), &value, 1);
	return value;
}

static void nRF24L01_write_register(uint8_t reg, uint8_t value)
{
	nRF24L01_command(W_REGISTER | (REGISTER_MASK & reg), &value, 1);
}

static void nRF24L01_write_register_long(uint8_t reg, const uint8_t *data, uint8_t len)
{
	nRF24L01_command(W_REGISTER | (REGISTER_MASK & reg), data, len);
}


static void nRF24L01_config(void)
// Sets the important registers in the MiRF module
{
    // Set RF channel
	nRF24L01_write_register(CONFIG, 0);
	nRF24L01_write_register(RF_CH, DEF_RF_CH);
	nRF24L01_write_register(RF_SETUP, DEF_RF_SETUP);
	nRF24L01_write_register(SETUP_AW, (ADDRESS_WIDTH - 2));
	nRF24L01_write_register(SETUP_RETR, DEF_SETUP_RETR);
	nRF24L01_write_register(FEATURE, DEF_FEATURE);
	nRF24L01_write_register(DYNPD, DEF_DYNPD);
	nRF24L01_write_register(EN_RXADDR, 1);	/* disable all rx pipes except first one*/
	//nRF24L01_write_register(EN_AA, (1 << ENAA_P0));	/* default: all ENAA set*/
	nRF24L01_command(FLUSH_RX, 0, 0);
	nRF24L01_command(FLUSH_TX, 0, 0);
	nRF24L01_write_register(STATUS, (1 << RX_DR | (1 << TX_DS) | (1 << MAX_RT)));
}

/**
 * Initializes nRF24L01 by initializing mcu specific low level controls and sending configuration
 * to the radio.
 */
void nRF24L01_init(void)
{
	_nRF24L01_init();
	nRF24L01_config();
}


/**
 * Sets receive address of pipe.
 * @param pipe Pipe 0 or 1
 * @param addr address of that pipe
 */
void nRF24L01_set_RADDR_01(uint8_t pipe, const uint8_t * addr)
// Sets the receiving address
{
	if(pipe < 2)
		nRF24L01_write_register_long(RX_ADDR_P0 + pipe, addr, ADDRESS_WIDTH);
}

/**
 * Sets receive address of pipes 2-5.
 * @param pipe Pipe 2, 3, 4 or 5
 * @param addr low byte of pipe address. High bytes equal pipe 1.
 */
void nRF24L01_set_RADDR(uint8_t pipe, uint8_t addr)
{
	if(pipe < 6 && pipe > 1)
		nRF24L01_write_register(RX_ADDR_P0 + pipe, addr);
}

void nRF24L01_set_RXPW(uint8_t pipe, uint8_t len)
{
	if(pipe < 6)
		nRF24L01_write_register(RX_PW_P0 + pipe, len);
}

/**
 * Sets transmit address for following transmissions.
 * Waits for current transmission to complete.
 * @param addr target address
 */
void nRF24L01_set_TADDR(const uint8_t * addr)
{
    while(TxActive)
    	;
	nRF24L01_write_register_long(TX_ADDR, addr, ADDRESS_WIDTH);
}

/**
 * Enables given RX pipe.
 * @param pipe number of pipe to enable
 */
void nRF24L01_enable_RPIPE(uint8_t pipe)
{
	uint8_t en_rxaddr;
	en_rxaddr = nRF24L01_read_register(EN_RXADDR);
	en_rxaddr |= (1 << pipe);
	nRF24L01_write_register(EN_RXADDR, en_rxaddr);
}


/**
 * reads top fifo payload.
 * @param data array to put data in. must have up to 32 bytes!
 * @return number of bytes received
 */
uint8_t nRF24L01_get_data(uint8_t * data)
{
    uint8_t len;
    uint8_t status;
    status = nRF24L01_command(R_RX_PL_WID, &len, 1);

    if(len > 32)
    {
    	nRF24L01_command(FLUSH_RX, 0, 0);
    	return 0;
    }
    nRF24L01_commandR(R_RX_PAYLOAD, data, len);
    nRF24L01_write_register(STATUS, (1 << RX_DR));

    return len;
}


/**
 * sends a packet to previously defined address.
 * waits for completion of previous packet.
 * @param data data packet (will be destroyed)!
 * @param len number of bytes
 * @param rxAfterTx switch to RX mode right after transmission
 */
void nRF24L01_send(const uint8_t * data, uint8_t len, uint8_t rxAfterTx)
{

    while(TxActive)
    	;

    TX;
    nRF24L01_command(W_TX_PAYLOAD, data, len);

    mirf_CE_hi;                     // Start transmission
    if(rxAfterTx)
    	TxActive = 2;
    else
    	TxActive = 1;
    wait10us();
    mirf_CE_lo;
}

/* wakes the RF chip up. Delays by 5 ms */
void nRF24L01_wakeUp(uint8_t rx)
{
	mirf_CE_lo;
	nRF24L01_write_register(CONFIG, DEF_CONFIG | PWR_UP);
	waitms(5);
	if(rx) {
		RX;
		mirf_CE_hi;
	}
}

/* put RF to deep power down. A wakeup is needed before the next transfer */
void nRF24L01_sleep(void)
{
	mirf_CE_lo;
	nRF24L01_write_register(CONFIG, DEF_CONFIG);
}

void nRF24L01_IRQ(void)
{
	uint8_t status;
	status = nRF24L01_command(NOP, 0, 0);

	if(status & ((1 << TX_DS) | (1 << MAX_RT)))
	{	/* doesn't matter if packet is transmitted successfully or not */
		nRF24L01_command(FLUSH_TX, 0, 0);	/* on MAX_RT, packet is still there... */

		if(TxActive == 2)
		{
			RX;
			mirf_CE_hi;
		}

		TxActive = 0;
		/* clear status flags. as we do same action on both we may clear both. */
		nRF24L01_write_register(STATUS, (1 << TX_DS) | (1 << MAX_RT));
	}
#ifdef RX_CALLBACK
	if(status & (1 << RX_DR))
	{
		rxCallback();
	}
#endif
	}

uint8_t nRF24L01_isTransmitting(void)
{
	return TxActive;
}

uint8_t nRF24L01_rxDataAvailable(void)
{
	uint8_t fifoStatus;
	fifoStatus = nRF24L01_read_register(FIFO_STATUS);
	return !(fifoStatus & (1 << RX_EMPTY));
}

void nRF24L01_set_rx_callback(void (*f)(void))
{
	rxCallback = f;
}
