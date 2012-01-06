/*
 * high layer rf protocol.
 */

#include <stdint.h>
#include "nRF24L01.h"
#include "funk.h"
#include "ntc.h"
#include "timer.h"
#include "lcd.h"
#include "motor.h"

/* from main */
extern uint16_t BatteryMV;


void funkRxDataAvailable(void)
{
	static uint16_t readCounter = 0;
	uint8_t rxData[32];
	uint8_t readLen;
	readLen = nRF24L01_get_data(rxData);

	readCounter++;
	displayWeekday(rxData[0]);
	displayNumber(readCounter);
}


/**
 * initializes lower level rf protocol, sets high level address.
 * @param ownAddress high level address: 2-255
 */
void funkInit(void)
{
	nRF24L01_init();
	nRF24L01_set_RADDR_01(0, ThermostatAdr);
	nRF24L01_enable_RPIPE(0);
	nRF24L01_set_TADDR(ThermostatAdr);
	nRF24L01_set_rx_callback(&funkRxDataAvailable);
}

/*
void txPacket(uint8_t adr, MESSAGE_TYPE type, uint8_t *data)
{
	nRF24L01_wakeUp(0);
	nRF24L01_send(data, 32, 1);
}
*/

void funkSend(void)
{
	MSG_FROM_THRM msg;
	msg.info.temperatureActual = getNtcTemperature();
	msg.info.valve = getMotorPosition();
	msg.info.battery = BatteryMV;
	//msg.info.temperatureNominal =
	msg.time.day = Weekday;
	msg.time.hour = Hours;
	msg.time.minute = Minutes;

	nRF24L01_send(&msg, sizeof(msg), 0);
}
