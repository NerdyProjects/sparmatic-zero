/*
 * nRF24L01.h
 *
 *  Created on: 12.11.2011
 *      Author: matthias
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

// Mirf settings

//#define DISABLE_ENHANCED_SHOCKBURST

/* Not implemented so far. Future: Callback out of Interupt for incoming packet */
#define RX_CALLBACK 1

/* 2 Byte CRC */
#define DEF_CONFIG     ( (1<<EN_CRC) | (0<<CRCO))

/* N Byte addresses (3-5) */
#define ADDRESS_WIDTH 3

/* ((ARD+1) * 0,25ms) retransmission delay, up to 8 retransmissions */
#define DEF_SETUP_RETR ((5 << ARD) | (8 << ARC))

/* Channel Offset in MHz*/
/* ISM is 2,4-2,47xx MHz */
#define DEF_RF_CH         65

/* Data Rate 250 kBps, maximum output power */
#define DEF_RF_SETUP ((1 << RF_DR_LOW) | (0 << RF_DR_HIGH) | (3 << RF_PWR))

/* Dynamic Payload Length, Payload with Ack */
#define DEF_FEATURE ((1 << EN_DPL) | (1 << EN_ACK_PAY) | (1 << EN_DYN_ACK))

/* Static Payload on all data pipes */
#define DEF_DYNPD (1 << DPL_P0)



// Public standart functions
void nRF24L01_init(void);
void nRF24L01_set_RADDR_01(uint8_t pipe, const uint8_t * addr);
void nRF24L01_set_RADDR(uint8_t pipe, uint8_t addr);
void nRF24L01_set_TADDR(const uint8_t * addr);
void nRF24L01_enable_RPIPE(uint8_t pipe);
uint8_t nRF24L01_get_data(uint8_t * data);
void nRF24L01_send(const uint8_t * data, uint8_t len, uint8_t rxAfterTx);
void nRF24L01_wakeUp(uint8_t rx);
void nRF24L01_sleep(void);
void nRF24L01_set_rx_callback(void (*f)(void));
void nRF24L01_set_RXPW(uint8_t pipe, uint8_t len);
uint8_t nRF24L01_rxDataAvailable(void);

void nRF24L01_IRQ(void);

#endif /* NRF24L01_H_ */
