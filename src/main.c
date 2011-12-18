#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>


#include "nRF24L01_ll.h"
#include "funk.h"
#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"
#include "timer.h"
#include "nRF24L01.h"

#define ADC_CH_BAT (14)


#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

#define POWERLOSS_PORTIN PINE
#define POWERLOSS_PIN PE0


static void sysShutdown(void)
{
	//lcdOff();
}

ISR(PCINT1_vect)
{
/* used for waking up the device by key press*/
	LCDCRA |= (1 << LCDIE);
}

/* occurs at each new LCD frame , ~128 ms*/
ISR(LCD_vect)
{
	static uint8_t cnt = 0;
	uint8_t motorRunning = motorIsRunning();
	if(motorRunning) {
		motorTimer();
	}
	keyPeriodicScan();
	if(key_state & KEY_ALL)
		cnt = 0;
	else
		cnt ++;

	if(cnt > 4 && !motorRunning)
	{	/* TODO this simple check may lead to missed keys when they are pressed in the moment the motor is stopped */
		cnt = 0;
		LCDCRA &= ~(1 << LCDIE);	/* disable LCD Interrupt when keys are handled */
	}
}

#define PCINT0_PORTIN PINE
ISR(PCINT0_vect)
{
	static unsigned char lastState = (1 << IRQ_PIN);	// init to defaults
	unsigned char newState = PCINT0_PORTIN;
	unsigned char changed = newState ^ lastState;
	lastState = newState;

	/* emergency wakeup on power loss, motor step counter */
	if(newState & (1 << POWERLOSS_PIN))
		sysShutdown();

	if(~newState & changed & (1 << IRQ_PIN))
	{
		/* this may happen more often than the real IRQ */
		nRF24L01_IRQ();
	}

	/* any other case is motor step */
	if(changed & (1 << MOTOR_SENSE_PIN))
	{
		motorStep();
	}
}


static void sysSleep(void) {
	OCR2A = 0;
	ADCSRA &= ~(1 << ADEN); // Disable ADC
	displaySymbols(LCD_BATTERY, LCD_BATTERY);
	while (ASSR & (1 << OCR2UB))
		/* wait at least one asynchronous clock cycle for interrupt logic to reset */
		;
	sleep_mode();
	displaySymbols(0, LCD_BATTERY);
}

void pwrInit(void)
{
  PRR = (1 << PRTIM1) | (1 << PRUSART0); // disable some hardware
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  PCMSK0 |= (1 << PCINT0);	/* emergency power loss IRQ */
  DDRE &= ~(1 << PE0);
  EIMSK |= (1 << PCIE0);
}

void ioInit(void)
{
  DIDR0 = 0xFF;	/* Disable digital inputs on Port F */
}

int main(void)
{
	uint8_t cnt = 0;
	_delay_ms(50);
	timerInit();
	pwrInit();
	ioInit();
	motorInit();
	lcdInit();
	keyInit();
	ntcInit();
	funkInit();
	displayAsciiDigit('H', 0);
	displayAsciiDigit('A', 1);
	displayAsciiDigit('L', 2);
	displayAsciiDigit('O', 3);
	sei();

	while(1)
	{
		if(get_key_long(1 << KEY_MENU)) {
			uint8_t err;
			displaySymbols(LCD_DP, LCD_DP);
			err = motorAdapt();
			if(err) {
				displayNumber(err);
			} else {
				displaySymbols(0, LCD_DP);
			}
		}

		UpdateNtcTemperature();

		funkSend();

		sysSleep();
	}

	return -1;
}
