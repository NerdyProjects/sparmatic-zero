#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdint.h>

#include "../nRF24L01.h"
#include "../nRF24L01_ll.h"
#include "../lcd.h"

#include "bootloader.h"

#define KEY_DDR         DDRB
#define KEY_PORT        PORTB
#define KEY_PLUS PB0
#define KEY_MINUS PB7
#define KEY_CLOCK PB5
#define KEY_OK PB6
#define KEY_MENU PB4
#define KEY_ALL ((1 << KEY_PLUS) | (1 << KEY_MINUS) | (1 << KEY_CLOCK) | (1 << KEY_MENU) | (1 << KEY_OK))

#define TIMER_VAL_MS(ms, prescaler) (ms*(F_CPU/1000)/prescaler)


typedef enum {
	BL_NOT_OPERATING, BL_FAILED, BL_OPERATING
} BOOTLOADER_STATUS;

BOOTLOADER_STATUS BlStatus;

static void            (*start)( void ) = 0x0000;
/* Maybe only the 3 LSB are used */
const uint8_t BootloaderAdr[] = {0xA7, 0x5B, 0xCA, 0x4C, 0x15};

uint8_t pageBuf[SPM_PAGESIZE];
uint8_t rxBuf[32];

uint16_t TValNextRequestCommand = 0;
/* ~ 1 second */
#define WAIT_BETWEEN_REQUESTS (10000UL)
#define UPDATE_TVAL_NEXT_REQUEST_COMMAND (TValNextRequestCommand = TCNT1)

#define SPM_BYTEADDR_MASK (SPM_PAGESIZE - 1)

#define CMD_PROGPAGE	0x01	/* <CMD> <addr_h> <addr_l> <up to 29 data bytes> */
#define CMD_FINISHED 0x7F	/* <CMD> */
#define CMD_REQUEST 0x81	/* <CMD> <8 bit request packet counter, reset on answer received> */
#define CMD_FAILURE 0x82	/* <CMD> [<add. failure code data>] */

#define FAIL_WRONG_ADDRESS 0x01
#define FAIL_NO_DATA 0x02


/* code snippet taken from avr-libc documentation, shortened by some parts. */
void boot_program_page (uint16_t page, uint8_t *buf)
{
    uint8_t i;

    eeprom_busy_wait ();

    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.

    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.

        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill (page + i, w);
    }

    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
}

void sendDataRequest(uint8_t resetCnt) {
	static uint8_t cnt = 0;
	uint8_t buf[2];
	if(resetCnt)
		cnt = 0;

	buf[0] = CMD_REQUEST;
	buf[1] = cnt++;
	nRF24L01_send(buf, 2, 0);
}

void sendFailure(uint8_t failureCode) {
	uint8_t buf[2];
	buf[0] = CMD_FAILURE;
	buf[1] = failureCode;
	nRF24L01_send(buf, 1, 0);
}

void rxData(void) {
	uint8_t len;
	len = nRF24L01_get_data(rxBuf);		/* len will _always_ be >=1 */
	switch(rxBuf[0]) {
	case CMD_PROGPAGE:
	{
		static uint16_t addrExpected = 0;
		static uint8_t pageBytesReceived = 0;
		uint16_t addr;
		if(len >= 4) {	/* CMD + 2x ADDR + >=1 DATA */
			uint8_t i;
			if(BlStatus != BL_OPERATING) {
				BlStatus = BL_OPERATING;
			}

			addr = rxBuf[1] << 8 | rxBuf[2];
			if((addrExpected & SPM_BYTEADDR_MASK) && addr != addrExpected) {
				/* page begun, but this data does not match next page data */
				sendFailure(FAIL_WRONG_ADDRESS);
				BlStatus = BL_FAILED;
				return;
			} else if((addr & SPM_BYTEADDR_MASK) == 0) {
				/* new page */
				addrExpected = addr;
			}
			i = 3;	/* first data byte in */
			while(i < len) {
				pageBuf[(addrExpected++) & SPM_BYTEADDR_MASK] = rxBuf[i++];
			}
			pageBytesReceived += len - 3;
			if(pageBytesReceived == SPM_PAGESIZE && (addrExpected & SPM_BYTEADDR_MASK) == 0) {
				/* we finished the page! */
				boot_program_page(addrExpected - SPM_PAGESIZE, pageBuf);
			}
			sendDataRequest(1);
			UPDATE_TVAL_NEXT_REQUEST_COMMAND;
		} else {
			sendFailure(FAIL_NO_DATA);
			BlStatus = BL_FAILED;
		}
		break;
	}
	case CMD_FINISHED:
		BlStatus = BL_NOT_OPERATING;
		break;
	default:
		break;
	}




}

static void checkAndExecNrfIRQ(void)
{
	static uint8_t i = 0;

	if(~IRQ_PORT_IN & (1 << IRQ_PIN))
	{
		++i;
		nRF24L01_IRQ();

	}
	displayAsciiDigit('0' + i, 0);
}

int main(void) {
	CLKPR = (1 << CLKPCE);
	CLKPR = (1 << CLKPS1) | (1 << CLKPS0);
	TCNT1 = 0;
	TCCR1B = (1 << CS10) | (1 << CS11);		/* CLOCK / 64 -> wait 4,19 seconds @ 1 MHz */
	TIFR1 = 1 << TOV1;
	lcdInit();
	KEY_DDR &= ~KEY_ALL;                // configure key port for input
	KEY_PORT |= KEY_ALL;                // and turn on pull up resistors
	while(TCNT1 < TIMER_VAL_MS(100, 64))		/* startup delay for nRF */
		;
	nRF24L01_init();
	nRF24L01_set_TADDR(BootloaderAdr);
	nRF24L01_set_RADDR_01(0, BootloaderAdr);

	nRF24L01_set_rx_callback(&rxData);
	cli();		/* just in case... */


	sendDataRequest(1);
	UPDATE_TVAL_NEXT_REQUEST_COMMAND;
	while(!(TIFR1 & (1 << TOV1)) || (BlStatus == BL_OPERATING)) {
		if((((uint32_t)TCNT1 + 65536 - TValNextRequestCommand) % 65536) > WAIT_BETWEEN_REQUESTS && BlStatus != BL_FAILED && !nRF24L01_isTransmitting())
		{
			sendDataRequest(0);
			UPDATE_TVAL_NEXT_REQUEST_COMMAND;
		}
		checkAndExecNrfIRQ();

	}


	boot_rww_enable();

	TCCR1B = 0;
	lcdOff();
	SET_BOOTLOADER_EXECUTED;

	/* jump to reset */
	start();
	return 0;
}


