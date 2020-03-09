/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2BeepDotMG.h"
#include "Arduboy2CoreDotMG.h"

static void timer_init(Tc *TCx, unsigned int clkId, IRQn_Type irqn)
{
  // Enable GCLK for timer
  GCLK->PCHCTRL[clkId].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  // Disable counter
  TCx->COUNT16.CTRLA.bit.ENABLE = 0;
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);

  // Reset counter
  TCx->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);
  while (TCx->COUNT16.CTRLA.bit.SWRST);

  // Set to match frequency mode
  TCx->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;

  // Set to 16-bit counter, clk/16 prescaler
  TCx->COUNT16.CTRLA.reg = (
    TC_CTRLA_MODE_COUNT16 |
    TC_CTRLA_PRESCALER_DIV16
  );
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);

  // Configure interrupt request
  NVIC_DisableIRQ(irqn);
  NVIC_ClearPendingIRQ(irqn);
  NVIC_SetPriority(irqn, 0);
  NVIC_EnableIRQ(irqn);

  // Enable interrupt request
  TCx->COUNT16.INTENSET.bit.MC0 = 1;
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);
}

static void timer_tone(Tc *TCx, float freq, uint16_t dur)
{
  // Set counter based on desired frequency
  TCx->COUNT16.CC[0].reg = (uint16_t)((F_CPU / 16 / 2 / freq) - 1);

  // Enable counter
  TCx->COUNT16.CTRLA.bit.ENABLE = 1;
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);
}

static void timer_stop(Tc *TCx)
{
  // Disable counter
  TCx->COUNT16.CTRLA.bit.ENABLE = 0;
  while (TCx->COUNT16.SYNCBUSY.bit.ENABLE);
}


uint16_t BeepChan1::duration = 0;
static volatile bool value1 = false;

#define TIMER1         TC1
#define TIMER_GCLK_ID1 TC1_GCLK_ID
#define TIMER_IRQ1     TC1_IRQn
#define TIMER1_HANDLER void TC1_Handler()

void BeepChan1::begin()
{
  timer_init(TIMER1, TIMER_GCLK_ID1, TIMER_IRQ1);
}

void BeepChan1::tone(float freq)
{
  tone(freq, 0);
}

void BeepChan1::tone(float freq, uint16_t dur)
{
  duration = dur;
  timer_tone(TIMER1, freq, dur);
}

void BeepChan1::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepChan1::noTone()
{
  timer_stop(TIMER1);
  duration = 0;
  value1 = false;
}


uint16_t BeepChan2::duration = 0;
static volatile bool value2 = false;

#define TIMER2         TC2
#define TIMER_GCLK_ID2 TC2_GCLK_ID
#define TIMER_IRQ2     TC2_IRQn
#define TIMER2_HANDLER void TC2_Handler()

void BeepChan2::begin()
{
  timer_init(TIMER2, TIMER_GCLK_ID2, TIMER_IRQ2);
}

void BeepChan2::tone(float freq)
{
  tone(freq, 0);
}

void BeepChan2::tone(float freq, uint16_t dur)
{
  duration = dur;
  timer_tone(TIMER2, freq, dur);
}

void BeepChan2::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepChan2::noTone()
{
  timer_stop(TIMER2);
  duration = 0;
  value2 = false;
}


// ----------------------------------------------

inline static void toggle(volatile bool &value)
{
  if (!DAC->DACCTRL[DAC_CH_SPEAKER].bit.ENABLE)
    return;

  while (!DAC_READY);
  while (DAC_DATA_BUSY);
  value = !value;
  DAC->DATA[DAC_CH_SPEAKER].reg = (value1 ? 2047 : 0) + (value2 ? 2047 : 0);
}

TIMER1_HANDLER
{
  toggle(value1);
  TIMER1->COUNT16.INTFLAG.bit.MC0 = 1;  // Clear interrupt
}

TIMER2_HANDLER
{
  toggle(value2);
  TIMER2->COUNT16.INTFLAG.bit.MC0 = 1;  // Clear interrupt
}
