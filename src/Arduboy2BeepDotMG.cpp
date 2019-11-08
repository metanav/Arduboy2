/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2BeepDotMG.h"

uint8_t BeepPin1::duration = 0;
bool pin1Value = false;

void BeepPin1::begin()
{
  TCCR1A = 0;
  TCCR1B =bit(WGM12) | bit(CS11);  // CTC mode, clk/8
}

void BeepPin1::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  if (!bitRead(DDR_CS_SPK_SEL_ST, BIT_SPEAKER))
    return;  // audio was disabled at boot

  duration = dur;
  OCR1A = count;
  bitSet(TIMSK1, OCIE1A);  // enable OCR1A interrupt
}

void BeepPin1::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepPin1::noTone()
{
  duration = 0;
  pin1Value = false;
  bitClear(TIMSK1, OCIE1A);  // disable OCR1A interrupt
}


uint8_t BeepPin2::duration = 0;
bool pin2Value = false;

void BeepPin2::begin()
{
  TCCR2A = bit(WGM21);             // CTC mode
  TCCR2B = bit(CS22) | bit(CS20);  // clk/128
}

void BeepPin2::tone(uint8_t count)
{
  tone(count, 0);
}

void BeepPin2::tone(uint8_t count, uint8_t dur)
{
  if (!bitRead(DDR_CS_SPK_SEL_ST, BIT_SPEAKER))
    return;  // audio was disabled at boot

  duration = dur;
  OCR2A = count;
  bitSet(TIMSK2, OCIE2A);  // enable OCR2A interrupt
}

void BeepPin2::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepPin2::noTone()
{
  duration = 0;
  pin2Value = false;
  bitClear(TIMSK2, OCIE2A);  // disable OCR2A interrupt
}


ISR(TIMER1_COMPA_vect)
{
  pin1Value = !pin1Value;
  bitWrite(PORT_CS_SPK_SEL_ST, BIT_SPEAKER, pin1Value | pin2Value);
}

ISR(TIMER2_COMPA_vect)
{
  pin2Value = !pin2Value;
  bitWrite(PORT_CS_SPK_SEL_ST, BIT_SPEAKER, pin1Value | pin2Value);
}
