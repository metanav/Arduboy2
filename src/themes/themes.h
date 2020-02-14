#ifndef THEMES_H
#define THEMES_H

#include "colors.h"

// Represents the configurable colors for drawing to the screen. Colors must
// be 12-bit 444-formatted RGB color values.
typedef struct {
  uint16_t pixelColor;
  uint16_t backgroundColor;
  uint16_t borderLineColor;
  uint16_t borderFillColor;
} Theme;


// -------------
//    THEMES
// -------------

// Default, high contrast theme
#define THEME_DEFAULT { \
  COLOR_WHITE, \
  COLOR_BLACK, \
  COLOR_GRAY, \
  COLOR_BLACK, \
}

// --------------------------------------------------------------------------

// Theme reminiscent of the original Nintendo Game Boy
#define THEME_DMG { \
  0x003, \
  0xAD8, \
  0x777, \
  COLOR_GRAY, \
}

// --------------------------------------------------------------------------

// Futuristic dark theme
#define THEME_ELEKTRONIK { \
  COLOR_CYAN, \
  0x035, \
  COLOR_MAGENTA, \
  0x303, \
}

// --------------------------------------------------------------------------

// Dark, ominous theme
#define THEME_DOOM { \
  0xF11, \
  0x110, \
  0x888, \
  0x421, \
}

// --------------------------------------------------------------------------

// Add more themes here...

#endif
