#ifndef THEMES_H
#define THEMES_H

#include "colors.h"

// Builds a theme from color values, for use with `Arduboy2Core::setTheme()`
#define THEME(pixelColor, backgroundColor, borderLineColor, borderFillColor) \
  (pixelColor), \
  (backgroundColor), \
  (borderLineColor), \
  (borderFillColor)


// -------------
//    THEMES
// -------------

// Default, high contrast theme
#define THEME_DEFAULT THEME(COLOR_WHITE, COLOR_BLACK, COLOR_GRAY, COLOR_BLACK)

// --------------------------------------------------------------------------

// Theme reminiscent of the original Nintendo Game Boy
#define THEME_DMG     THEME(0x003, 0xAD8, 0x777, COLOR_GRAY)

// --------------------------------------------------------------------------

// Futuristic dark theme
#define THEME_SOLARIZED_DARK    THEME( \
  color444::fromHex(0xD33682), \
  color444::fromHex(0x002B36), \
  color444::fromHex(0x268BD2), \
  color444::fromHex(0x073642) \
)

// --------------------------------------------------------------------------

// Add more themes here...

#endif
