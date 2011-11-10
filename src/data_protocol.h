/*
 * data_protocol.h
 *
 *  Created on: 10.11.2011
 *      Author: matthias
 */

#ifndef DATA_PROTOCOL_H_
#define DATA_PROTOCOL_H_

enum MESSAGE_TYPE {
	MSG_ACK,	/* Empty Message, just dst type and crc */
	MSG_THERMOSTAT_INFO, /* regular message from thermostat to base */
	MSG_PROGRAM, /* program from base to thermostat */
	MSG_GET_PROGRAMS, /* Empty Message, request all programs from base to thermostat */
	MSG_PROGRAM_DETAILS, /* program from thermostat to base, one message for each program */
	MSG_PROGRAM_DELETE, /* delete one program from base to thermostat */
	MSG_TIME_SET, /* set time on thermostat, from base to thermostat */
	MSG_HEAT, /* "manual" temperature change from base to thermostat */
	MSG_TEMPERATURE_OFF /* temperature on no active program, from base to thermostat */
};

/* high resolution temperature: 12.4 */
typedef signed short TEMPERATURE_H;
/* low resolution temperature: 6.2 */
typedef unsigned char TEMPERATURE_L;
typedef unsigned char VALVE;

struct TIME {
	unsigned day:3;
	unsigned hour:5;
	unsigned char minute;
};


struct THERMOSTAT_INFO {
	TEMPERATURE_H actual;
	TEMPERATURE_H target;
	VALVE valve;
	struct TIME now;
};

struct PROGRAM {
	/* weekday: Bit 7..0 Monday, Tuesday, Wednesday, ..., Sunday, ONE TIME
	 * empty weekday is invalid program*/
	unsigned char weekday;
	unsigned char start;
	unsigned char end;
	TEMPERATURE_L temperature;
};

struct PROGRAM_DETAILS {
	struct PROGRAM program;
	unsigned char num;
};

struct PROGRAM_DELETE {
	unsigned char num;
};

struct TIME_SET {
	struct TIME now;
};

struct HEAT {
	TEMPERATURE_L;
	/* 5.3 hour.minute; hour 24: change current target temperature, stay in program */
	unsigned char offTime;
};

struct TEMPERATURE_OFF {
	TEMPERATURE_L temperature;
};

/* on the air: do not use this union! use the raw data */
union MSG_DATA {
	struct THERMOSTAT_INFO thermostatInfo;
	struct PROGRAM program;
	struct PROGRAM_DETAILS programDetails;
	struct PROGRAM_DELETE programDelete;
	struct TIME_SET timeSet;
	struct HEAT heat;
	struct TEMPERATURE_OFF temperatureOff;
};

struct MESSAGE {
	/* 0: broadcast, 1: master, 2-n: slave */
	unsigned char destination;
	enum MESSAGE_TYPE type;
	/* directly use type! sizeof(union) is biggest member */
	union MSG_DATA data;
	/* use CRC-CCITT:
	 * public static int CRC16CCITT(byte[] bytes) {
          int crc = 0xFFFF;          // initial value
          int polynomial = 0x1021;   // 0001 0000 0010 0001  (0, 5, 12)

          for (byte b : bytes) {
              for (int i = 0; i < 8; i++) {
                  boolean bit = ((b   >> (7-i) & 1) == 1);
                  boolean c15 = ((crc >> 15    & 1) == 1);
                  crc <<= 1;
                  if (c15 ^ bit) crc ^= polynomial;
               }
          }

          crc &= 0xffff;
          //System.out.println("CRC16-CCITT = " + Integer.toHexString(crc));
          return crc;
      }
	 */
	unsigned short crc;



};

#endif /* DATA_PROTOCOL_H_ */
