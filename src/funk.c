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
#include "adc.h"

/* from main */
extern uint16_t BatteryMV;


void funkRxDataAvailable(void)
{
	uint8_t rxData[32];
	uint8_t readLen;
	readLen = nRF24L01_get_data(rxData);
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
	TIME time = getTime();
	msg.info.temperatureActual = getNtcTemperature();
	msg.info.valve = getMotorPosition();
	msg.info.battery = getBatteryVoltage();
	//msg.info.temperatureNominal =
	msg.time.day = time.weekday;
	msg.time.hour = time.hour;
	msg.time.minute = time.minute;

	nRF24L01_send((uint8_t *)&msg, sizeof(msg), 0);
}
