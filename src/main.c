#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>


#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"
#include "timer.h"

#define ADC_CH_BAT (14)


#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

#define POWERLOSS_PORTIN PINE
#define POWERLOSS_PIN PE0

static void sysShutdown(void);
static volatile uint8_t MotorStepTimes[256];
static volatile uint8_t MotorStepWrite;

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

ISR(PCINT0_vect)
{
	/* emergency wakeup on power loss, motor step counter */
	if(POWERLOSS_PORTIN & (1 << POWERLOSS_PIN))
		sysShutdown();

	/* any other case is motor step */
	if(motorStep())
		MotorStepTimes[MotorStepWrite++] = TCNT0;
}


static void sysShutdown(void)
{
	lcdOff();
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
  PRR = (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0); // disable some hardware
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  PCMSK0 |= (1 << PCINT0);	/* emergency power loss IRQ */
  DDRE &= ~(1 << PE0);
  EIMSK |= (1 << PCIE0);
}

void ioInit(void)
{
  DIDR0 = 0xFF;	/* Disable digital inputs on Port F */
}



void adcInit(void)
{
  ADCSRA = (1 << ADPS2);	// Fclk/16
  ADMUX = (1 << REFS0);
}

int main(void)
{
	uint8_t motorStepRead = 0;
	uint8_t flag = 1;
	timerInit();
	pwrInit();
	ioInit();
	motorInit();
	lcdInit();
	keyInit();
	adcInit();
	ntcInit();
	displayAsciiDigit('H', 0);
	displayAsciiDigit('A', 1);
	displayAsciiDigit('L', 2);
	displayAsciiDigit('O', 3);
	sei();

	while(1)
	{
		if (get_key_press(1 << KEY_PLUS)) {
			motorStepOpen();
		}
		if (get_key_press(1 << KEY_MINUS)) {
			motorStepClose();
		}
		if(get_key_short(1 << KEY_MENU)) {
			++motorStepRead;
			flag = 1;
		}
		if(get_key_long(1 << KEY_MENU)) {
			motorDetectFullOpen();
		}
		if(get_key_press(1 << KEY_CLOCK)) {
			--motorStepRead;
			flag = 1;
		}
		if(get_key_press(1 << KEY_OK)) {
			for(motorStepRead = 0; motorStepRead < 255; motorStepRead++)
				MotorStepTimes[motorStepRead] = 0;
			motorStepRead = 0;
			MotorStepWrite = 0;
			flag = 1;
		}
		if(flag) {
			flag = 0;
			displayBargraph(motorStepRead);
			displayString("....");
			displayNumber(MotorStepTimes[motorStepRead]);
		}

		sysSleep();
	}

	return -1;
}
