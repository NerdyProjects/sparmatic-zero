/*
 * high layer rf protocol.
 */

#include <stdint.h>
#include "nRF24L01.h"
#include "funk.h"
#include "lcd.h"

static uint8_t myAddress = 0;

#define TX_RESENDS 5
#define TX_ACK_DELAY 100

static uint8_t Packet[32] = { 0x10, 0x13, 0x17, 0x1E,
		0x20, 0x23, 0x27, 0x2E,
		0x30, 0x33, 0x37, 0x3E,
		0x40, 0x43, 0x47, 0x4E,
		0x50, 0x53, 0x57, 0x5E,
		0x60, 0x63, 0x67, 0x6E,
		0x70, 0x73, 0x77, 0x7E,
		0x80, 0x83, 0x87, 0x8E };

void funkRxDataAvailable(void)
{
	static uint16_t readCounter = 0;
	uint8_t rxData[32];
	uint8_t readLen;
	readLen = nRF24L01_get_data(rxData);

	readCounter++;
	//displayBargraph(readCounter);
	displayNumber(Packet[0]);
}


/**
 * initializes lower level rf protocol, sets high level address.
 * @param ownAddress high level address: 2-255
 */
void funkInit(uint8_t ownAddress)
{
	nRF24L01_init();
	nRF24L01_set_RADDR_01(0, BroadcastAdr);
	nRF24L01_enable_RPIPE(0);
	nRF24L01_set_TADDR(BroadcastAdr);
	nRF24L01_set_rx_callback(&funkRxDataAvailable);
	myAddress = ownAddress;
}


static void headerToPacket(struct PACKET_HEADER header)
{
	Packet[0] = header.destination;
	Packet[1] = header.source;
	Packet[2] = header.type;
}

void txPacket(uint8_t adr, MESSAGE_TYPE type, uint8_t *data)
{
	struct PACKET_HEADER header;
	header.source = myAddress;
	header.destination = adr;
	header.type = type;
	header.crc = 0;	/* todo */

	Packet[3] = adr;
	nRF24L01_wakeUp();
	nRF24L01_send(Packet, 32, 0);
}

