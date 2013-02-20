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

#if defined(__UNO__)

// defines for timer1 (16 bits)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR1A |= _BV(COM1A1))
#define TIMER_DISABLE_PWM    (TCCR1A &= ~(_BV(COM1A1)))
#define TIMER_ENABLE_INTR    (TIMSK1 = _BV(OCIE1A))
#define TIMER_DISABLE_INTR   (TIMSK1 = 0)
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR1A = _BV(WGM11); \
  TCCR1B = _BV(WGM13) | _BV(CS10); \
  ICR1 = pwmval; \
  OCR1A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR1A = 0; \
  TCCR1B = _BV(WGM12) | _BV(CS10); \
  OCR1A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT1 = 0; \
})

#elif defined(__LEONARDO__)

// defines for timer3 (16 bits)
#define TIMER_ENABLE_PWM     (TCCR3A |= _BV(COM3A1))
#define TIMER_DISABLE_PWM    (TCCR3A &= ~(_BV(COM3A1)))
#define TIMER_ENABLE_INTR    (TIMSK3 = _BV(OCIE3A))
#define TIMER_DISABLE_INTR   (TIMSK3 = 0)
#define TIMER_INTR_NAME      TIMER3_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR3A = _BV(WGM31); \
  TCCR3B = _BV(WGM33) | _BV(CS30); \
  ICR3 = pwmval; \
  OCR3A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR3A = 0; \
  TCCR3B = _BV(WGM32) | _BV(CS30); \
  OCR3A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT3 = 0; \
})

#else
#error Oops...
#endif

void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  TIMER_ENABLE_PWM;
  delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TIMER_DISABLE_PWM;
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
  TIMER_DISABLE_INTR;

  pinMode(9, OUTPUT);
  digitalWrite(9, LOW); // When not sending PWM, we want it low

  TIMER_CONFIG_KHZ(khz);
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
