#include <string.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "common.h"
#include "rs485.h"

// This is the program code for the 30 channel (10 channel RGB) DMX controlled
// dimmer made for OHM2013 by SA007.

// See https://ohm2013.org/wiki/DMX_lighting for interfacing specs.

// Hardware defines:
// Outputs:
// 	Red	Green	Blue
// X1	PF1	PF2	PF3
// X2	PA0	PA1	PA2
// X3	PA3	PA4	PA5
// X4	PA6	PA7	PG2
// X5	PC7	PC6	PC5
// X6	PC4	PC3	PC2
// X7	PC1	PC0	PG1
// X8	PG0	PD7	PD6
// X9	PD5	PD4	PD3
// X10	PD2	PD1	PD0
// If you don't like this, complain to Atmel, this is the chip's physical layout.

// Green led (ON) : PE6
// Red   led (ACT): PE3

// Inputs:
// 9 bit DMX address: PB0 .. PB7, PG3.
// DMX in: PE0 (RXD)

void inithw(void);

// PWM values
void pwminterrupt(void);
unsigned char pwmdata[PWMCHANNELS];
unsigned char *datatouse, *databeingused;
unsigned char pwmstep;
// There are two datasets, so that the current PWM and data receive don't consufe each other.
// receive shouldn't touch 'databeingused' and signals to the PWM with 'datatouse' is the data is complete.

// ^^ not actually used, but it was a nice idea :)

// DMX values
unsigned char breakReceived; // This will turn one on after the first break ever, this makes sure that the first data received is correct.
unsigned int dmxAddress; // Current address receiving on
unsigned int dmxListening; // Current address listening on, is updated every break from the dipswitches
unsigned char uartStatus, uartData;

int main(void) {
  inithw();
  init_rs485();

  databeingused = datatouse = pwmdata;

  memset(pwmdata, 0, PWMCHANNELS);

  ONON;

  while(1) {
    // UART is explicitely not done in interrupts because
    // it is hardware buffered and can stand being interrupted
    // The PWM does not like being interrupted :)
    pwminterrupt();
    if (recv && payload_length == PWMCHANNELS)
    {
        cli();

        recv = 0;
        memcpy((void*)pwmdata, (const void*)buffer, PWMCHANNELS);

        sei();
    }
  }
}

void inithw(void) {
  DDRA = 0xFF;
  DDRB = 0x0;
  DDRC = 0xFF;
  DDRD = 0xFF;
  DDRE = 0x48;
  DDRF = 0x0E;

  // Pull-ups
  PORTB = 0xFF;
  PORTG = 0x08;
}

void pwminterrupt() {
  unsigned char pwmcount;
  // This does the PWM (d0h).
  // Main qualities of this code are:
  // Fast, should run very often to gat a smooth pwm
  // Staggered, so the PWM current is limited

  // Staggering is done by shifting the PWM value trough
  // the outputs, using the ROL instruction, which is
  // strangly 'optimized' to adc by the compiler which is identical

#define pwm(CHAN, PORT, VAL) if (databeingused[CHAN] > pwmcount) { PORT |= VAL; } else { PORT &= ~VAL; asm("nop"::); }
//#define rol() asm("rol %0" : "=r" (pwmcount) : "0" (pwmcount))
#define rol() pwmcount += 8;

  pwmstep++;
  pwmcount = pwmstep;

  pwm(0,PORTF,0x02);
  rol();
  pwm(1,PORTF,0x04);
  rol();
  pwm(2,PORTF,0x08);
  rol();
  pwm(3,PORTA,0x01);
  rol();
  pwm(4,PORTA,0x02);
  rol();
  pwm(5,PORTA,0x04);
  rol();
  pwm(6,PORTA,0x08);
  rol();
  pwm(7,PORTA,0x10);
  rol();
  pwm(8,PORTA,0x20);
  rol();
  pwm(9,PORTA,0x40);
  rol();
  pwm(10,PORTA,0x80);
  rol();
  pwm(11,PORTG,0x04);
  rol();
  pwm(12,PORTC,0x80);
  rol();
  pwm(13,PORTC,0x40);
  rol();
  pwm(14,PORTC,0x20);
  rol();
  pwm(15,PORTC,0x10);
  rol();
  pwm(16,PORTC,0x08);
  rol();
  pwm(17,PORTC,0x04);
  rol();
  pwm(18,PORTC,0x02);
  rol();
  pwm(19,PORTC,0x01);
  rol();
  pwm(20,PORTG,0x02);
  rol();
  pwm(21,PORTG,0x01);
  rol();
  pwm(22,PORTD,0x80);
  rol();
  pwm(23,PORTD,0x40);
  rol();
  pwm(24,PORTD,0x20);
  rol();
  pwm(25,PORTD,0x10);
  rol();
  pwm(26,PORTD,0x08);
  rol();
  pwm(27,PORTD,0x04);
  rol();
  pwm(28,PORTD,0x02);
  rol();
  pwm(29,PORTD,0x01);

  if (pwmstep == 0xFF) databeingused = datatouse;
}
