/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2BeepDotMG.h"

uint8_t BeepChan1::duration = 0;
static volatile bool value1 = false;

void BeepChan1::begin()
{
  // Enable GCLK0 for TC4
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0 |
    GCLK_CLKCTRL_ID(GCM_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Reset TC4
  TC4->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
  while (TC4->COUNT16.CTRLA.bit.SWRST);

  // Set to 16-bit counter, match frequency mode, clk/16 prescaler
  TC4->COUNT16.CTRLA.reg = (
    TC_CTRLA_MODE_COUNT16 |
    TC_CTRLA_WAVEGEN_MFRQ |
    TC_CTRLA_PRESCALER_DIV16
  );
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

  // Configure interrupt request
  NVIC_DisableIRQ(TC4_IRQn);
  NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);
  NVIC_EnableIRQ(TC4_IRQn);

  // Enable interrupt request
  TC4->COUNT16.INTENSET.bit.MC0 = 1;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void BeepChan1::tone(float freq)
{
  tone(freq, 0);
}

void BeepChan1::tone(float freq, uint8_t dur)
{
  duration = dur;

  // Set counter based on desired frequency
  TC4->COUNT16.CC[0].reg = (uint16_t)((F_CPU / 16 / 2 / freq) - 1);

  // Enable counter
  TC4->COUNT16.CTRLA.bit.ENABLE = 1;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void BeepChan1::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepChan1::noTone()
{
  duration = 0;
  value1 = false;

  // Disable counter
  TC4->COUNT16.CTRLA.bit.ENABLE = 0;
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}


uint8_t BeepChan2::duration = 0;
static volatile bool value2 = false;

void BeepChan2::begin()
{
  // Enable GCLK0 for TC5
  GCLK->CLKCTRL.reg =
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK0 |
    GCLK_CLKCTRL_ID(GCM_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Reset TC5
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
  while (TC5->COUNT16.CTRLA.bit.SWRST);

  // Set to 16-bit counter, match frequency mode, clk/16 prescaler
  TC5->COUNT16.CTRLA.reg = (
    TC_CTRLA_MODE_COUNT16 |
    TC_CTRLA_WAVEGEN_MFRQ |
    TC_CTRLA_PRESCALER_DIV16
  );
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);

  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);

  // Enable interrupt request
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}

void BeepChan2::tone(float freq)
{
  tone(freq, 0);
}

void BeepChan2::tone(float freq, uint8_t dur)
{
  duration = dur;

  // Set counter based on desired frequency
  TC5->COUNT16.CC[0].reg = (uint16_t)((F_CPU / 16 / 2 / freq) - 1);

  // Enable counter
  TC5->COUNT16.CTRLA.bit.ENABLE = 1;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}

void BeepChan2::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepChan2::noTone()
{
  duration = 0;
  value2 = false;

  // Disable counter
  TC5->COUNT16.CTRLA.bit.ENABLE = 0;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}


void TC4_Handler()
{
  value1 = !value1;
  DAC->DATA.reg = (value1 ? 511 : 0) + (value2 ? 511 : 0);

  // Clear the interrupt
  TC4->COUNT16.INTFLAG.bit.MC0 = 1;
}

void TC5_Handler()
{
  value2 = !value2;
  DAC->DATA.reg = (value1 ? 511 : 0) + (value2 ? 511 : 0);

  // Clear the interrupt
  TC5->COUNT16.INTFLAG.bit.MC0 = 1;
}
