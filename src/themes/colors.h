#ifndef COLORS_H
#define COLORS_H

// Useful 12-bit 444-formatted color values

#define COLOR_BLACK      0x0000
#define COLOR_WHITE      0x0FFF
#define COLOR_GRAY       0x0AAA
#define COLOR_RED        0x0F00
#define COLOR_GREEN      0x00F0
#define COLOR_BLUE       0x000F
#define COLOR_CYAN       0x00FF
#define COLOR_MAGENTA    0x0F0F
#define COLOR_YELLOW     0x0FF0
#define COLOR_ORANGE     0x0F40

// Utilities for working with 12-bit 444-formatted colors
namespace color444 {

  // Builds a 12-bit 444-formatted color value from individual 4-bit RGB channels
  static uint16_t from4BitRGB(uint8_t r, uint8_t g, uint8_t b)
  {
    return (
      ((r & 0x0F) << 8) |
      ((g & 0x0F) << 4) |
      ((b & 0x0F) << 0)
    );
  }

  // Builds a 12-bit 444-formatted color value from individual 8-bit RGB channels
  static uint16_t from8BitRGB(uint8_t r, uint8_t g, uint8_t b)
  {
    return from4BitRGB(
      (r*0x0F)/0xFF,
      (g*0x0F)/0xFF,
      (b*0x0F)/0xFF
    );
  }

  // Builds a 12-bit 444-formatted color value from a standard 24-bit hex color value
  static uint16_t fromHex(uint32_t hexColor)
  {
    return from8BitRGB(
      (hexColor & 0xFF0000) >> 16,
      (hexColor & 0x00FF00) >> 8,
      (hexColor & 0x0000FF) >> 0
    );
  }

  // Returns the complement of a 12-bit 444-formatted color
  static uint16_t complement(uint16_t color)
  {
    return COLOR_WHITE - (color & 0x0FFF);
  }
}

#endif
