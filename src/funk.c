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

static uint8_t Packet[32];

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

	headerToPacket(header);
	nRF24L01_wakeUp();
	nRF24L01_send(Packet, 3, 0);
}

