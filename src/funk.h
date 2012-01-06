/*
 * funk.h
 *
 *  Created on: 17.11.2011
 *      Author: matthias
 */

#ifndef FUNK_H_
#define FUNK_H_

/* Maybe only the 3 LSB are used */
static const uint8_t ThermostatAdr[] = {0xA7, 0x5A, 0xCD, 0x14, 0x15};

/* fixed comma s12.4 temperature */
typedef int16_t TEMPERATURE;

typedef enum {
	MSG_NULL	= 0, /* No special message */
	MSG_PROGRAM = 1, /* program from base to thermostat */
	MSG_TIME_SET = 2, /* set time on thermostat */
	MSG_TEMPERATURE = 3 /* set temperatures */
} MSG_TO_THRM_TYPE;

typedef struct {
	uint8_t day; /* Weekday for that this program is specified */
	uint8_t programData[24]; /* two bits for each 15 minute segment for each hour */
} MSG_PROGRAM_PART;

typedef struct {
	unsigned day:3;
	unsigned hour:5;
	uint8_t minute;
} MSG_TIME;

typedef struct {
	TEMPERATURE temperatures[4];
} MSG_TEMPERATURES;

typedef union {
	MSG_PROGRAM_PART program;	/* 25 bytes */
	MSG_TEMPERATURES temperature;
} MSG_TO_THRM_DATA;

typedef struct {
	MSG_TIME time;	/* 2 bytes */
	uint8_t type;	/* MSG_TO_THRM_TYPE, but C standard says >= 16 bit for enums */
	MSG_TO_THRM_DATA data; /* 25 bytes */
} MSG_TO_THRM;

typedef struct {
	TEMPERATURE temperatureActual;
	TEMPERATURE temperatureNominal;
	uint16_t valve;
	uint16_t battery;
} MSG_THRM_INFO;

typedef struct {
	MSG_TIME time;
	MSG_THRM_INFO info;
} MSG_FROM_THRM;



void funkInit(void);
void funkSend(void);

#endif /* FUNK_H_ */
