/*
 * lcd.c
 *
 *  Created on: 18.11.2011
 *      Author: matthias
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "lcd.h"

#define NUM_DIGITS 4

/*
 * font 0-H taken from TravelRec., converted to hex
 * 	msb	mlkjih	 ggfedcba lsb
 */
#define FONT_ASCII_OFFSET '-'
static const uint16_t Font[] PROGMEM = {
			0x00C0, // -
			0, // .
			0, // /
			0x003F,		//0
			0x0406,		//1
			0x00DB,		//2
			0x008F,		//3
			0x00E6,		//4
			0x00ED,		//5
			0x00FD,		//6
			0x1401,		//7
			0x00FF,		//8
			0x00EF,		//9
			0x1200,		//:
			0x0C00,		// //
			0x2400,		// <
			0x00C1,		//=
			0x0900,		//>
			0x1421,		//?
			0x2D3F,		//@
			0x00F7,		//A
			0x128F,		//B
			0x0039,		//C
			0x120F,		//D
			0x00F9,		//E a f e d g1 g2
			0x00F1,		//F a f e g1 g2
			0x00BD,		//G a f e d c g2
			0x00F6,		//H f e g1 g2 b c
			0x1209,		// I a i l d
			0x001E,		// J b c d e
			0x2470,		// K f e g1 j m
			0x0038,		// L f e d
			0x0536, 	// M f e h j b c
			0x2136,		// N f e h m c b
			0x003F,		// O a f e d b c
			0x00F3,		// P f e a b g1 g2
			0x203F,		// Q a f e d b c m
			0x20F3,		// R a f e b g1 g2 m
			0x018D,		// S a h g2 c d
			0x1201,		// T i l a
			0x003E,		// U f e d b c
			0x0C30,		// V f e k j
			0x2836,		// W f e k m c b
			0x2D00,		// X h m k j
			0x1500,		// Y h j l
			0x0C09		// Z a j k d

};

/*
 * ;		a	b	c	d	e	f	g1	g2	h	i	j	k	l	m
 * taken from TravelRec.
 */
#define SEGMENTS_PER_DIGIT 14
static const uint8_t Segments[] PROGMEM = {

		126,124, 44,  5,  7,127, 47, 85, 87, 86,125, 6, 46,  45,	//left Digit
		123,121,  1,  2,  4, 84, 43, 81, 83, 82,122, 3, 42,  41,	//middle left Digit
		137,139, 59, 18, 16,136, 56, 98, 96, 97,138, 17, 57, 58,	//middle right Digit
		140,142, 22, 21, 19, 99, 60,102,100,101,141, 20, 61, 62		//right Digit
};

static const uint8_t BargraphSegments[] PROGMEM = { 88, 48, 8, 9, 49, 89, 90, 50, 10,
		11, 51, 91, 92, 52, 12, 13, 53, 93, 134, 94, 54, 14, 15, 55 };


/* Mo - So, LCDDR16 */
static const uint8_t WeekdaySegments[] PROGMEM = {128, 129, 130, 131, 132, 133, 95};

/* enum LCD_SYMBOLS */
static const uint8_t SymbolSegments[] PROGMEM = {80, 120, 40, 23, 24, 64, 104, 144, 103, 143, 135, 0};

static void segmentOn(uint8_t segment)
{
	uint8_t volatile *segmentBase = &LCDDR0;
	segmentBase[segment / 8] |= (1 << segment % 8);
}

static void segmentOff(uint8_t segment)
{
	uint8_t volatile *segmentBase = &LCDDR0;
	segmentBase[segment / 8] &= ~(1 << segment % 8);
}

static void segmentSwitch(uint8_t segment, uint8_t on)
{
	if(on)
		segmentOn(segment);
	else
		segmentOff(segment);
}

static void displayDigit(uint16_t segments, uint8_t pos)
{
	uint8_t segmentOffset = pos * SEGMENTS_PER_DIGIT;
	uint8_t i;
	uint8_t segment;
	for(i = 0; i < SEGMENTS_PER_DIGIT; ++i)
	{
		segment = pgm_read_byte(&Segments[segmentOffset + i]);
		segmentSwitch(segment, (segments & (1 << i)) > 0);
	}
}

/**
 * Displays the bargraph.
 * @param bargraphOn each bit represents one hour on the bargraph.
 */
void displayBargraph(uint32_t bargraphOn)
{
	uint8_t i;
	for(i = 0; i < sizeof(BargraphSegments); ++i)
	{
		segmentSwitch(pgm_read_byte(&BargraphSegments[i]), (bargraphOn & (1 << i)) > 0);
	}
}

/**
 * Displays the weekday bar.
 * @param dayOn each bit represents one day to display
 */
void displayWeekday(uint8_t dayOn)
{
	uint8_t i;
	for(i = 0; i < 7; ++i)
	{
		segmentSwitch(pgm_read_byte(&WeekdaySegments[i]), dayOn & (1 << i));
	}
}

/**
 * Displays a symbol on the LCD. Only masked ones are taken into account.
 * @param on turn on these symbols
 * @param mask only touch symbols listed here
 */
void displaySymbols(LCD_SYMBOLS on, LCD_SYMBOLS mask)
{
	uint8_t i;
	for(i = 0; i < sizeof(SymbolSegments); ++i)
	{
		if(mask & (1 << i))
			segmentSwitch(pgm_read_byte(&SymbolSegments[i]), (on & (1 << i)) > 0);
	}

}
/**
 * Outputs one digit at given position.
 * @param c ASCII character to display
 * @param pos position on display, 0-NUM_DIGITS
 */
void displayAsciiDigit(char c, uint8_t pos)
{
	if(pos >= NUM_DIGITS)
		return;
	uint8_t fontIndex = c - FONT_ASCII_OFFSET;
	uint16_t segments = pgm_read_word(&Font[fontIndex]);
	displayDigit(segments, pos);
}

/**
 * Outputs a string on the display, beginning on the left digit.
 * @param str string to display, it is displayed up to NUM_DIGITS
 */
void displayString(char *str)
{
	uint8_t pos = 0;
	while (str[pos] && pos < NUM_DIGITS)
	{
		displayAsciiDigit(str[pos], pos);
		pos++;
	}
}

/*
 *  display a number consisting of up to four characters.
 *  a minus sign is provided for negative numbers. (so we have
 *  three remaining characters). Output will be left adjusted.
 *  @param width leading with zeroes so we have given width
 *  todo: filling width will be done with sign character...
 */
void displayNumber(int16_t num, int8_t width)
{
	char buf[NUM_DIGITS + 1];
	uint8_t i = NUM_DIGITS;
	char sign = ' ';
	if(num < 0)
	{
		sign = '-';
		num = -num;
	}

	buf[NUM_DIGITS] = 0;
	while(i && num != 0)
	{
		buf[--i] = '0' + (num % 10);
		num /= 10;
	}
	width -= NUM_DIGITS-i;	/* subtract already written chars */
	while(i && width > 0)
	{
		buf[--i] = sign;	/* leading sign chars */
		--width;
	}
	displayString(buf[i]);	/* begin output at first wanted character */
}


void lcdInit(void)
{
LCDCRB = (1<<LCDCS)|(0<<LCD2B)|(1<<LCDMUX1)|(1<<LCDMUX0)|(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM0);
/*
      (1<<LCDCS)                            // Das LCD wird im asynchronen Modus (LCDCS-Bit=1)
                                               mit der Frequenz des Quarzes TOSC1 = 32.768Hz als LCD Clock betrieben.
      |(0<<LCD2B)                           // 1/3 bias is used
      |(1<<LCDMUX1)|(1<<LCDMUX0)            // 1/4 Duty; COM0:3;
      |(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM0); // SEG0:24
*/

LCDFRR = (0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)|(0<<LCDCD2)|(0<<LCDCD1)|(1<<LCDCD0);
/*
      (0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)    // N = 16
      |(0<<LCDCD2)|(0<<LCDCD1)|(1<<LCDCD0);  // D = 2
      // ergo f(frame) = 128Hz
      eventuell D=1, N=64 (LCDCD0 = 0) LCDPS0=1 für 64 Hz, ausprobieren
*/

LCDCCR = (1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)|(/*config.lcd_contrast*/ 10 << LCDCC0);
/*
      (1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)   // 575 µs
      // 3,1V
      |(config.lcd_contrast << LCDCC0);     // Set the initial LCD contrast level
*/

LCDCRA = (1<<LCDEN)|(1<<LCDAB)|(0<<LCDIE)|(0<<LCDBL);
/*
      (1<<LCDEN)    // Enable LCD
      |(1<<LCDAB)   // Low Power Waveform
      |(0<<LCDIE)   // disable Interrupt
      |(0<<LCDBL);  // No Blanking
*/
}

void lcdOff(void)
{
	LCDCRA |= (1 << LCDBL);
}
