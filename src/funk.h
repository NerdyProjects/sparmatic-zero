/*
 * funk.h
 *
 *  Created on: 17.11.2011
 *      Author: matthias
 */

#ifndef FUNK_H_
#define FUNK_H_

static const uint8_t BroadcastAdr[] = {0xA7, 0x5B, 0xCD, 0x14};

typedef enum {
	MSG_ACK,	/* Empty Message, just dst type and crc */
	MSG_THERMOSTAT_INFO, /* regular message from thermostat to base */
	MSG_PROGRAM, /* program from base to thermostat */
	MSG_GET_PROGRAMS, /* Empty Message, request all programs from base to thermostat */
	MSG_PROGRAM_DETAILS, /* program from thermostat to base, one message for each program */
	MSG_PROGRAM_DELETE, /* delete one program from base to thermostat */
	MSG_TIME_SET, /* set time on thermostat, from base to thermostat */
	MSG_HEAT, /* "manual" temperature change from base to thermostat */
	MSG_TEMPERATURE_OFF /* temperature on no active program, from base to thermostat */
} MESSAGE_TYPE;

#pragma pack(1)
struct PACKET_HEADER {
	uint8_t destination;
	uint8_t source;
	MESSAGE_TYPE type;
	uint16_t crc;
};
// #pragma unpacked

void funkInit(uint8_t ownAddress);
void txPacket(uint8_t adr, MESSAGE_TYPE type, uint8_t *data);

#endif /* FUNK_H_ */
