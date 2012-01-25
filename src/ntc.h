/*
 * ntc.h
 *
 *  Created on: 19.11.2011
 *      Author: matthias
 */

#ifndef NTC_H_
#define NTC_H_

void ntcInit(void);
void updateNtcTemperature(void);

extern int16_t Temperature;
extern int8_t NTCOffset;
#define getNtcTemperature(x) ((const int16_t)Temperature)

#endif /* NTC_H_ */
