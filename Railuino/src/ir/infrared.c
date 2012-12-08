#include <avr/interrupt.h>

// ===================================================================
// === Low-level IR stuff ============================================
// ===================================================================

/**
 * This is taken more or less verbatim from Ken Shirriff's awesome IR
 * library.
 *
 * http://www.arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * I stripped it down to what is needed here and made it work with
 * RC5 extended code. I decided against using his full code because
 * of Arduino's inability to deal with libraries that use other
 * libraries. I decided in favor of "repackaging" so people can use
 * his full library in parallel with Railuino.
 */ 

#define SYSCLOCK 16000000  // main Arduino clock
#define RC5_T1      889
#define TOPBIT 0x80000000

void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  TCCR2A |= _BV(COM2B1); // Enable pin 3 PWM output
  delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TCCR2A &= ~(_BV(COM2B1)); // Disable pin 3 PWM output
  delayMicroseconds(time);
}

void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.

  
  // Disable the Timer2 Interrupt (which is used for receiving IR)
  TIMSK2 &= ~_BV(TOIE2); //Timer2 Overflow Interrupt
  
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW); // When not sending PWM, we want it low
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);

  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  OCR2A = SYSCLOCK / 2 / khz / 1000;
  OCR2B = OCR2A / 3; // 33% duty cycle
}

// Note: first bit must be a one (start bit)
void sendRC5(unsigned long data, int nbits, bool extended)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC5_T1); // First start bit
    if (extended) {
        mark(RC5_T1); // Second start bit
        space(RC5_T1); // Second start bit
    } else {
  space(RC5_T1); // Second start bit
  mark(RC5_T1); // Second start bit
    }
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); // 1 is space, then mark
      mark(RC5_T1);
    } 
    else {
      mark(RC5_T1);
      space(RC5_T1);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}
