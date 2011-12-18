/*
 * ntc.h
 *
 *  Created on: 19.11.2011
 *      Author: matthias
 */

#ifndef NTC_H_
#define NTC_H_

void ntcInit(void);
int16_t UpdateNtcTemperature(void);
uint16_t getNtcAdc(void);

const int16_t Temperature;

#define getNtcTemperature(x) (Temperature)

#endif /* NTC_H_ */
