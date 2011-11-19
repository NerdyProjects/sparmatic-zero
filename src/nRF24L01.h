/*
 * nRF24L01.h
 *
 *  Created on: 12.11.2011
 *      Author: matthias
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

// Mirf settings

#define DISABLE_ENHANCED_SHOCKBURST 1

/* Not implemented so far. Future: Callback out of Interupt for incoming packet */
#define RX_CALLBACK 0

/* 2 Byte CRC */
#define DEF_CONFIG     ( (1<<EN_CRC) | (0<<CRCO) | (1 << MASK_MAX_RT) | (RX_CALLBACK << MASK_RX_DR) )

/* 0.75ms retransmission delay, up to 0 retransmissions */
#define DEF_SETUP_RETR ((2 << ARD) | (0 << ARC))

/* Channel Offset in MHz*/
/* ISM is 2,4-2,5 GHz, WLAN g */
#define DEF_RF_CH         96

/* Data Rate 2 MBps, maximum output power */
#define DEF_RF_SETUP ((1 << RF_DR_HIGH) | (3 << RF_PWR))

/* Dynamic Payload Length, Payload with Ack */
#define DEF_FEATURE ((1 << EN_DPL) | (1 << EN_ACK_PAY))

/* Dynamic Payload on all data pipes */
#define DEF_DYNPD ((1 << DPL_P5) | (1 << DPL_P4) | (1 << DPL_P3) | (1 << DPL_P2) | (1 << DPL_P1) | (1 << DPL_P0))



// Public standart functions
void nRF24L01_init(void);
void nRF24L01set_RADDR_01(uint8_t pipe, uint8_t * addr);
void nRF24L01_set_RADDR(uint8_t pipe, uint8_t addr);
void nRF24L01_set_TADDR(uint8_t * addr);
void nRF24L01_enable_RPIPE(uint8_t pipe);
uint8_t nRF24L01_get_data(uint8_t * data);
void nRF24L01_send(uint8_t * data, uint8_t len, uint8_t rxAfterTx);
void nRF24L01_wakeUp(void);
void nRF24L01_sleep(void);

void nRF24L01IRQ(void);

#endif /* NRF24L01_H_ */
