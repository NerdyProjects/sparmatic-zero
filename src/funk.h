/*
 * funk.h
 *
 *  Created on: 17.11.2011
 *      Author: matthias
 */

#ifndef FUNK_H_
#define FUNK_H_

static const uint8_t *BroadcastAdr = {0xA7, 0x5B, 0xCD, 0x14};

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

#pragma packed
struct PACKET_HEADER {
	uint8_t destination;
	uint8_t source;
	enum MESSAGE_TYPE type;
	uint16_t crc;
};
// #pragma unpacked



#endif /* FUNK_H_ */
