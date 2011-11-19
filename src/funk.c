/*
 * high layer rf protocol.
 */

#include <stdint.h>
#include "nRF24L01.h"
#include "funk.h"

static uint8_t myAddress = 0;
/* global header/union with date field of 32 byte */

/**
 * initializes lower level rf protocol, sets high level address.
 * @param ownAddress high level address: 2-255
 */
void funkInit(uint8_t ownAddress)
{
	nRF24L01_init();
	nRF24L01_set_RADDR_(0, BroadcastAdr);
	nRF24L01_set_TADDR(BroadcastAdr);
	myAddress = ownAddress;
}


static void txPacket(uint8_t adr, MESSAGE_TYPE type, uint8_t *data)
{
	struct PACKET_HEADER header;
	header.source = myAddress;
	header.destination = adr;
	header.type = MSG_THERMOSTAT_INFO;
	header.crc = 0;

}
