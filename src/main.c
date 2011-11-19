#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>


#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"

#define ADC_CH_BAT (14)


#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

#define POWERLOSS_PORTIN PINE
#define POWERLOSS_PIN PE0

static void sysShutdown(void);

volatile uint32_t SystemTimeSeconds;

ISR(TIMER2_OVF_vect)
{
	/* used for waking up the device periodically */
	SystemTimeSeconds += 8;
}

ISR(PCINT1_vect)
{
/* used for waking up the device by key press*/
	LCDCRA |= (1 << LCDIE);
}

/* occurs at each new LCD frame , ~128 ms*/
ISR(LCD_vect)
{
	static uint32_t icnt = 0;
	static uint8_t cnt = 0;
	icnt ++;
	displayBargraph(icnt);
	keyPeriodicScan();
	if(key_state & KEY_ALL)
		cnt = 0;
	else
		cnt ++;

	if(cnt > 4)
	{
		cnt = 0;
		LCDCRA &= ~(1 << LCDIE);	/* disable LCD Interrupt when keys are handled */
	}
}

ISR(PCINT0_vect)
{
	displaySymbols(LCD_TOWER, LCD_TOWER);
	/* emergency wakeup on power loss, motor step counter */
	if(POWERLOSS_PORTIN & (1 << POWERLOSS_PIN))
		sysShutdown();

	/* any other case is motor step */
	motorStep();
}


static void sysShutdown(void)
{
	lcdOff();
}

static void sysSleep(void)
{
	OCR2A = 0;
  ADCSRA &= (1 << ADEN);		// Disable ADC
  displaySymbols(LCD_BATTERY, LCD_BATTERY);
  while(ASSR & (1 << OCR2UB))	/* wait at least one asynchronous clock cycle for interrupt logic to reset */
	  ;
  sleep_mode();
  displaySymbols(0, LCD_BATTERY);
}

void pwrInit(void)
{
  PRR = (1 << PRTIM1) | (1 << PRSPI); // | (1 << PRUSART0); // disable some hardware
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  PCMSK0 |= (1 << PCINT0);	/* emergency power loss IRQ */
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

void timerInit(void)
{
	ASSR |= (1 << AS2);
	TCCR2A = (1 << CS20) | (1 << CS21) | (1 << CS22);	/* normal mode, 32768/1024 = 32 Hz, 1/8 Hz Interrupt rate */
	TCNT2 = 0;
	OCR2A = 0;
	while(ASSR & ((1 << TCN2UB) | (1 << OCR2UB) | (1 << TCR2UB)))
		;
	TIFR2 = (1 << OCF2A) | (1 << TOV2);
	TIMSK2 = (1 << TOIE2);
}

int main(void)
{
	int16_t i = 0;
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

		displaySymbols(key_state, KEY_ALL);
		sysSleep();
	}

	return -1;
}
