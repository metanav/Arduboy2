/**
 * @file Arduboy2Audio.cpp
 * \brief
 * The Arduboy2Audio class for speaker and sound control.
 */

#include "Arduboy2DotMG.h"
#include "Arduboy2AudioDotMG.h"

bool Arduboy2Audio::audio_enabled = false;

void Arduboy2Audio::on()
{
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->CTRLA.bit.ENABLE = 0;     // disable DAC

  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->DACCTRL[DAC_CH_SPEAKER].bit.ENABLE = 1;  // enable channel

  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->CTRLA.bit.ENABLE = 1;     // enable DAC

  while (!DAC_READY);
  while (DAC_DATA_BUSY);
  DAC->DATA[DAC_CH_SPEAKER].reg = 0;
  delay(10);

  audio_enabled = true;
}

void Arduboy2Audio::off()
{
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->CTRLA.bit.ENABLE = 0;     // disable DAC

  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->DACCTRL[DAC_CH_SPEAKER].bit.ENABLE = 0;  // disable channel

  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
  DAC->CTRLA.bit.ENABLE = 1;     // enable DAC

  audio_enabled = false;
}

void Arduboy2Audio::toggle()
{
  if (audio_enabled)
    off();
  else
    on();
}

void Arduboy2Audio::saveOnOff()
{
  EEPROM.update(EEPROM_AUDIO_ON_OFF, audio_enabled);
}

void Arduboy2Audio::begin()
{
  if (EEPROM.read(EEPROM_AUDIO_ON_OFF))
    on();
  else
    off();
}

bool Arduboy2Audio::enabled()
{
  return audio_enabled;
}
