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
  audio_enabled = true;
  DAC->CTRLA.bit.ENABLE = 1;
  while (DAC->STATUS.bit.SYNCBUSY);
}

void Arduboy2Audio::off()
{
  audio_enabled = false;
  DAC->CTRLA.bit.ENABLE = 0;
  while (DAC->STATUS.bit.SYNCBUSY);
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
