#define NTC_PORT PORTF
#define NTC_DDR DDRF
#define NTC_PIN (PF3)

#define MOTOR_PORT PORTE
#define MOTOR_DDR DDRE
#define MOTOR_PIN-L PE7
#define MOTOR_PIN-R PE6

#define MOTOR_SENSE_DDR DDRE
#define MOTOR_SENSE_PORT PORTE
#define MOTOR_SENSE_PIN PE2

#define SWITCH_PORT PORTB
#define SWITCH_DDR DDRB
#define SWITCH_PLUS PB0
#define SWITCH_MINUS PB7
#define SWITCH_MENU PB5
#define SWITCH_OK PB6
#define SWITCH_CLOCK PB4
#define SWITCH_ALL ((1 << SWITCH_PLUS) | (1 << SWITCH_MINUS) | (1 << SWITCH_CLOCK) | (1 << SWITCH_MENU) | (1 << SWITCH_OK))


#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

void sysSleep(void)
{
  SMCR = SLEEP | (1 << SE)
  ADCSRA &= (1 << ADEN)		// Disable ADC
  // execute sleep *TODO*
}

void pwrInit(void)
{
  PRR = (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0);
}

void ioInit(void)
{
  NTC_DDR |= (1 << NTC_PIN);
 
  MOTOR_PORT &= (1 << MOTOR_PIN-L) | (1 << MOTOR_PIN-R)
  MOTOR_DDR = (1 << MOTOR_PIN-L) | (1 << MOTOR_PIN-R);
  
  MOTOR_SENSE_DDR = (1 << MOTOR_SENSE_PIN);

  SWITCH_PORT = SWITCH_ALL; // Pullups
  
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
