#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


#include "nRF24L01_ll.h"
#include "funk.h"
#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"
#include "timer.h"
#include "nRF24L01.h"
#include "adc.h"
#include "control.h"
#include "programming.h"

#define ADC_CH_REF 30
#define ADC_REF_MV 1100


#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

#define POWERLOSS_PORTIN PINE
#define POWERLOSS_PIN PE0

/* keypad changes temperature by this (1/100 degrees) */
#define MANUAL_TEMPERATURE_STEP		(50)

/* various input functions block for that time, not longer.
 * additionally, an okay press returns immediately.
 * time 256 ms steps (TIMER0 Overflow used).
 */
#define INPUT_TIMEOUT	(50)
#define TIMEOUT_READ		(Timer0H)

#define TIMEOUT_RENEW	timeout = TIMEOUT_READ + INPUT_TIMEOUT
#define TIMEOUT_INIT	uint8_t TIMEOUT_RENEW
#define TIMEOUT_OKAY	(timeout - TIMEOUT_READ < (INPUT_TIMEOUT + 1))

uint16_t BatteryMV;

typedef enum {
	MENU_SELECTOR, MENU_MAIN, MENU_FIRST, MENU_PID, MENU_PROGRAM, MENU_OTA_UPDATE, MENU_LAST
} MENU;

const char MenuText[][5] PROGMEM = {" PID", "PROG", "OTAU"};
const char PIDText[][5] PROGMEM = {" K-P", " K-D", " K-I", " INT", "IMAX", "ELST"};

static void updateBattery(void)
{
	uint16_t adc = getAdc(ADC_CH_REF);
	/* Uin = scale/fullscale * Uref
	 * -> here: Uref = ??; Uin = const
	 * Uref = Uin/scale*fullscale
	 * calculate at 32 bit
	 */
	BatteryMV = (ADC_REF_MV*1024UL / adc);
}


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

/* vent initialisation UI
 * returns 1 on error */
static uint8_t ventInit(void)
{
	if(motorFullOpen()) {
		displayString("EI1");
		return 1;
	}
	displayString("INST");
	while(!get_key_press(1 << KEY_OK))
		;
	displayString("ADAP");
	if(motorAdapt()) {
		displayString("EI2");
		return 1;
	}
	return 0;
}

/**
 * requests the user to enter a number.
 * blocks until timeout has elapsed or user presses OK key.
 * over/underflowing minimum/maximum value wraps around.
 * @param min minimum input number
 * @param max maximum input number
 * @param init initial value
 * @param step step size
 * @param displayLength output that many characters on the display
 */
static int16_t inputNumber(int16_t min, int16_t max, int16_t init, int16_t step, uint8_t displayLength)
{
	TIMEOUT_INIT;
	int16_t value = init;
	while(TIMEOUT_OKAY)
	{
		int8_t valueChange = get_key_increment();
		/* exit on OK */
		if(get_key_press((1 << KEY_OK)))
			break;

		if(valueChange != 0)
			TIMEOUT_RENEW;

		displayNumber(value, displayLength);
		value += valueChange * step;
		if(value > max)
			value -= max - min;
		else if(value < min)
			value += max - min;
	}

	return value;
}

/**
 * UI for selecting a text message.
 * @param text: pointer to pointer to chars (array of strings) in program space
 * @param min/max selection bounds
 * @param init: start value
 * @param offset: array-Index = value - offset (signed)
 */
static uint8_t inputSelector(const char text[][5], uint8_t min, uint8_t max, uint8_t init, int8_t offset) {
	TIMEOUT_INIT;
	int16_t value = init;
	while (TIMEOUT_OKAY) {
		char buf[5];
		int8_t valueChange = get_key_increment();
		/* exit on OK */
		if (get_key_press((1 << KEY_OK)))
			break;

		if (valueChange != 0)
			TIMEOUT_RENEW;

		buf[4] = 0;
		buf[0] = pgm_read_byte(&(text[value - offset][0]));
		buf[1] = pgm_read_byte(&(text[value - offset][1]));
		buf[2] = pgm_read_byte(&(text[value - offset][2]));
		buf[3] = pgm_read_byte(&(text[value - offset][3]));
		displayString(buf);
		value += valueChange;
		if (value > max)
			value -= max - min;
		else if (value < min)
			value += max - min;
	}

	return value;
}

static void menu(void) {
	static MENU currentMenu = MENU_MAIN;
	static uint8_t menuData = 0; /* number of screen in menu */

	uint8_t valueChange;
	switch (currentMenu) {
	case MENU_SELECTOR: {
		uint8_t nextMenu = menuData;

		nextMenu = inputSelector(MenuText, MENU_FIRST + 1, MENU_LAST - 1,
				nextMenu, MENU_FIRST + 1);
		menuData = 0;
		currentMenu = nextMenu;
		break;
	}
	case MENU_MAIN: {
		if (menuData == 0)
			displayNumber(getNtcTemperature(), 4);
		else if (menuData == 1)
			displayNumber(getNominalTemperature(), 4);

		displaySymbols(LCD_DP, LCD_DP);

		valueChange = get_key_increment();
		if (valueChange != 0) {
			setNominalTemperature(
					getNominalTemperature()
							+ valueChange * MANUAL_TEMPERATURE_STEP);
			menuData = 1; /* display this */
		}

		if (get_key_press(1 << KEY_CLOCK)) { /* disable program for X hours */
			int16_t hours;
			displayAsciiDigit('H', 3); /* suffix: hour */
			hours = inputNumber(0, 995, 5, 5, 3);
			dismissProgramChanges(hours * 6);
		}
		break;
	}
	case MENU_PID:
	{
		int16_t value;
		if(get_key_short(1 << KEY_MENU)) {
			menuData = inputSelector(PIDText, 0, sizeof(PIDText), menuData, 0);
		}
		switch (menuData) {
		case 0:
			value = controller.k_p;
			break;
		case 1:
			value = controller.k_d;
			break;
		case 2:
			value = controller.k_i;
			break;
		case 3:
			value = controller.i_val / 100;
			break;
		case 4:
			value = controller.i_max / 10;
			break;
		case 5:
			value = controller.e_last;
			break;
		default:
			value = -999;
		}
		if (get_key_increment()) {
			value = inputNumber(-999, 9999, value, 1, 4);
			switch (menuData) {
			case 0:
				controller.k_p = value;
				break;
			case 1:
				controller.k_d = value;
				break;
			case 2:
				controller.k_i = value;
				break;
			case 3:
				controller.i_val = value * 100;
				break;
			case 4:
				controller.i_max = value * 10;
				break;
			case 5:
				controller.e_last = value;
				break;
			}
		}
		break;
	}
	}
}

int main(void)
{
	_delay_ms(50);
	timerInit();
	pwrInit();
	ioInit();
	motorInit();
	lcdInit();
	keyInit();
	ntcInit();
	funkInit();
	sei();

	if(ventInit())
	{
		while(1)	/* we do not want to operate with an incorrect setup */
			sysSleep();
			;
	}

	while(1)
	{
		menu();

		updateNtcTemperature();
		updateBattery();
		funkSend();

		sysSleep();
	}

	return -1;
}
