/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05	/* 7 bit channel offset in MHz */
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define RPD          0x09
#define RX_ADDR_P0  0x0A	/* SETUP_AW size */
#define RX_ADDR_P1  0x0B	/* SETUP_AW size */
#define RX_ADDR_P2  0x0C	/* Only LSB, MSB taken from RX_ADDR_P1 */
#define RX_ADDR_P3  0x0D	/* Only LSB, MSB taken from RX_ADDR_P1 */
#define RX_ADDR_P4  0x0E	/* Only LSB, MSB taken from RX_ADDR_P1 */
#define RX_ADDR_P5  0x0F	/* Only LSB, MSB taken from RX_ADDR_P1 */
#define TX_ADDR     0x10
#define RX_PW_P0    0x11	/* Payload lengths for each */
#define RX_PW_P1    0x12	/*  RX pipe */
#define RX_PW_P2    0x13	/* 0: unused */
#define RX_PW_P3    0x14	/* 1-32: no. of bytes */
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD		0x1C	/* Dynamic Payload */
#define FEATURE		0x1D

/* Bit Mnemonics */
/* CONFIG 7 reserved -> 0*/
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0

/* EN_AA 7,6 reserved -> 00 */
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0

/* EN_RXADDR 7,6 reserved -> 00 */
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0

/* SETUP_AW 7:2 reserved -> 000000 */
#define AW          0 /* 01 -> 3, 10 -> 4, 11 -> 5 bytes */

/* SETUP_RETR */
#define ARD         4 /* 0..15 250-4000 µs */
#define ARC         0 /* 0..15 0-15 Retransmissions */

/* RF_SETUP 6 reserved, 7 and 4 for test, all 0. 0 dont care */
#define CONT_WAVE	7 /* continuous carrier transmission */
#define RF_DR_LOW	5
#define PLL_LOCK    4 /* force lock: only for test */
#define RF_DR_HIGH  3 /* together with bit 5: 250 kBps possible */
#define RF_PWR      1 /* 0..3 -18, -12, -6, 0 dBm */
#define LNA_HCURR   0 /* Obsolete in nRF24L01+ */

/* STATUS: 7 reserved, 0 */
#define RX_DR		6
#define TX_DS		5
#define MAX_RT		4
#define RX_P_NO		1	/* 0-5: Pipe no, 6 unused, 7 fifo empty */
#define TX_FULL		0
#define STATUS_RX_PIPE (7 << RX_P_NO)
#define STATUS_RX_PIPE_EMPTY (7 << RX_P_NO)

/* OBSERVE_TX */
#define PLOS_CNT	4	/* Loss counter, does not overflow */
#define ARC_CNT		0	/* Retransmission counter, reset on new packet */

/* RPD 7:1 reserved 0*/
#define RPD_BIT		0

/* FIFO_STATUS */
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0

/* DYNPD 7,6 reserved -> 0 */
#define DPL_P5		5
#define DPL_P4		4
#define DPL_P3		3
#define DPL_P2		2
#define DPL_P1		1
#define DPL_P0		0

/* FEATURE 7:3 reserved -> 0 */
#define EN_DPL		2
#define EN_ACK_PAY	1
#define EN_DYN_ACK	0	/* W_TX_PAYLOAD_NOACK cmd enabled */

/* Instruction Mnemonics */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define R_RX_PL_WID   0x60
#define W_ACK_PAYLOAD 0xA8
#define W_TX_PAYLOAD_NO_ACK 0xB0
#define NOP           0xFF
