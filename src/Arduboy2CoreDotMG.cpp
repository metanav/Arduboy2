/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2CoreDotMG.h"

#include <avr/wdt.h>

uint16_t Arduboy2Core::borderLineColor = COLOR(0xF, 0x1F, 0xF);
uint16_t Arduboy2Core::borderFillColor = ST77XX_BLACK;
uint16_t Arduboy2Core::pixelColor = ST77XX_WHITE;
uint8_t Arduboy2Core::MADCTL = ST77XX_MADCTL_MY;
uint8_t Arduboy2Core::LEDs[] = {0, 0, 0};
bool Arduboy2Core::inverted = false;
bool Arduboy2Core::borderDrawn = false;

Arduboy2Core::Arduboy2Core() { }

void Arduboy2Core::boot()
{
  bootPins();
  bootSPI();
  bootTFT();
  bootPowerSaving();
}

// Pins are set to the proper modes and levels for the specific hardware.
// This routine must be modified if any pins are moved to a different port
void Arduboy2Core::bootPins()
{
  // Set inputs
  DDR_RT_LFT_DN_UP_B_A_TX_RX &= ~(
    bit(BIT_BUTTON_RIGHT) |
    bit(BIT_BUTTON_LEFT)  |
    bit(BIT_BUTTON_DOWN)  |
    bit(BIT_BUTTON_UP)    |
    bit(BIT_BUTTON_B)     |
    bit(BIT_BUTTON_A)
  );
  DDR_CS_SPK_SEL_ST &= ~(
    bit(BIT_BUTTON_SELECT) |
    bit(BIT_BUTTON_START)
  );

  // Set input pullups
  PORT_RT_LFT_DN_UP_B_A_TX_RX |= (
    bit(BIT_BUTTON_RIGHT) |
    bit(BIT_BUTTON_LEFT)  |
    bit(BIT_BUTTON_DOWN)  |
    bit(BIT_BUTTON_UP)    |
    bit(BIT_BUTTON_B)     |
    bit(BIT_BUTTON_A)
  );
  PORT_CS_SPK_SEL_ST |= (
    bit(BIT_BUTTON_SELECT) |
    bit(BIT_BUTTON_START)
  );

  // Set outputs
  DDR_CS_SPK_SEL_ST |= (
    bit(BIT_TFT_CS)
  );

  DDR_SCK_MISO_MOSI_SS_DC_RST |= (
    bit(BIT_SPI_SCK)  |
    bit(BIT_SPI_MOSI) |
    bit(BIT_SPI_SS)   |
    bit(BIT_TFT_DC)   |
    bit(BIT_TFT_RST)
  );
}

void Arduboy2Core::bootTFT()
{
  // Reset display
  delayShort(5);  // Let display stay in reset
  bitSet(PORT_SCK_MISO_MOSI_SS_DC_RST, BIT_TFT_RST);  // Bring out of reset
  delayShort(5);

  // run our customized boot-up command sequence against the
  // TFT to initialize it properly for Arduboy
  LCDCommandMode();

  sendLCDCommand(ST77XX_SWRESET);  // Software reset
  delayShort(150);

  sendLCDCommand(ST77XX_SLPOUT);  // Bring out of sleep mode
  delayShort(500);

  sendLCDCommand(ST7735_FRMCTR1);  // Framerate ctrl - normal mode
  SPItransfer(0x01);               // Rate = fosc/(1x2+40) * (LINE+2C+2D)
  SPItransfer(0x2C);
  SPItransfer(0x2D);

  sendLCDCommand(ST77XX_MADCTL);  // Set initial orientation
  SPItransfer(MADCTL);

  sendLCDCommand(ST77XX_COLMOD);  // Set color mode (16-bit)
  SPItransfer(0x05);

  sendLCDCommand(ST7735_GMCTRP1);  // Gamma Adjustments (pos. polarity)
  SPItransfer(0x02);
  SPItransfer(0x1c);
  SPItransfer(0x07);
  SPItransfer(0x12);
  SPItransfer(0x37);
  SPItransfer(0x32);
  SPItransfer(0x29);
  SPItransfer(0x2D);
  SPItransfer(0x29);
  SPItransfer(0x25);
  SPItransfer(0x2B);
  SPItransfer(0x39);
  SPItransfer(0x00);
  SPItransfer(0x01);
  SPItransfer(0x03);
  SPItransfer(0x10);

  sendLCDCommand(ST7735_GMCTRN1);  // Gamma Adjustments (neg. polarity)
  SPItransfer(0x03);
  SPItransfer(0x1D);
  SPItransfer(0x07);
  SPItransfer(0x06);
  SPItransfer(0x2E);
  SPItransfer(0x2C);
  SPItransfer(0x29);
  SPItransfer(0x2D);
  SPItransfer(0x2E);
  SPItransfer(0x2E);
  SPItransfer(0x37);
  SPItransfer(0x3F);
  SPItransfer(0x00);
  SPItransfer(0x00);
  SPItransfer(0x02);
  SPItransfer(0x10);

  setWriteRegion(0, 0, TFT_WIDTH, TFT_HEIGHT);
  for (int i = 0; i < TFT_WIDTH*TFT_HEIGHT; i++) {
    SPItransfer(highByte(ST77XX_BLACK));
    SPItransfer(lowByte(ST77XX_BLACK));
  }

  sendLCDCommand(ST77XX_DISPON); //  Turn screen on
  delayShort(100);

  LCDDataMode();

  drawBorder();
}

void Arduboy2Core::LCDDataMode()
{
  bitSet(PORT_SCK_MISO_MOSI_SS_DC_RST, BIT_TFT_DC);
}

void Arduboy2Core::LCDCommandMode()
{
  bitClear(PORT_SCK_MISO_MOSI_SS_DC_RST, BIT_TFT_DC);
}

// Initialize the SPI interface for the display
void Arduboy2Core::bootSPI()
{
  // master, mode 0, MSB first, CPU clock / 2 (8MHz)
  SPCR = bit(SPE) | bit(MSTR);
  SPSR = bit(SPI2X);
}

// Write to the SPI bus (MOSI pin)
void Arduboy2Core::SPItransfer(uint8_t data)
{
  bitClear(PORT_CS_SPK_SEL_ST, BIT_TFT_CS);
  SPDR = data;
  /*
   * The following NOP introduces a small delay that can prevent the wait
   * loop form iterating when running at the maximum speed. This gives
   * about 10% more speed, even if it seems counter-intuitive. At lower
   * speeds it is unnoticed.
   */
  asm volatile("nop");
  while (!(SPSR & bit(SPIF)));  // wait
  bitSet(PORT_CS_SPK_SEL_ST, BIT_TFT_CS);
}

void Arduboy2Core::safeMode()
{
  if (buttonsState() == UP_BUTTON)
  {
    digitalWriteRGB(RED_LED, RGB_ON);
    while (true);
  }
}

/* Power Management */

void Arduboy2Core::idle()
{
  SMCR = bit(SE); // select idle mode and enable sleeping
  sleep_cpu();
  SMCR = 0; // disable sleeping
}

void Arduboy2Core::bootPowerSaving()
{
  // NOP
}

// Shut down the display
void Arduboy2Core::displayOff()
{
  sendLCDCommand(ST77XX_SLPIN);
  delayShort(150);
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  sendLCDCommand(ST77XX_SLPOUT);
  delayShort(150);
}


/* Drawing */

uint16_t Arduboy2Core::getBorderLineColor()
{
  return borderLineColor;
}

void Arduboy2Core::setBorderLineColor(uint16_t color)
{
  borderLineColor = color;

  if (borderDrawn)
    drawBorder();
}

uint16_t Arduboy2Core::getBorderFillColor()
{
  return borderFillColor;
}

void Arduboy2Core::setBorderFillColor(uint16_t color)
{
  borderFillColor = color;

  if (borderDrawn)
    drawBorder();
}

uint16_t Arduboy2Core::getPixelColor()
{
  return pixelColor;
}

void Arduboy2Core::setPixelColor(uint16_t color)
{
  pixelColor = color;
}

void Arduboy2Core::paint8Pixels(uint8_t pixels)
{
  for (int b = 0; b < 8; b++)
  {
    const uint16_t val = (pixels & 1) ? pixelColor : ST77XX_BLACK;
    pixels = pixels >> 1;
    SPItransfer(highByte(val));
    SPItransfer(lowByte(val));
  }
}

void Arduboy2Core::paintScreen(const uint8_t *image)
{
  setWriteRegion();
  const uint8_t numRows = HEIGHT/8;
  for (int c = 0; c < WIDTH; c++)
  {
    for (int r = 0; r < numRows; r++)
    {
      uint8_t pixels = pgm_read_byte(image + r*WIDTH+c);
      paint8Pixels(pixels);
    }
  }
}

void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
  setWriteRegion();
  const uint8_t numRows = HEIGHT/8;
  for (int c = 0; c < WIDTH; c++)
  {
    for (int r = 0; r < numRows; r++)
    {
      uint8_t pixels = image[r*WIDTH+c];
      paint8Pixels(pixels);
    }
  }

  if (clear)
    memset(image, 0, WIDTH*HEIGHT/8);
}

void Arduboy2Core::blank()
{
  setWriteRegion();
  for (int i = 0; i < WIDTH*HEIGHT; i++)
  {
    SPItransfer(highByte(ST77XX_BLACK));
    SPItransfer(lowByte(ST77XX_BLACK));
  }
}

void Arduboy2Core::sendLCDCommand(uint8_t command)
{
  LCDCommandMode();
  SPItransfer(command);
  LCDDataMode();
}

void Arduboy2Core::setWriteRegion(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
  sendLCDCommand(ST77XX_CASET);  //  Column addr set
  SPItransfer(0);
  SPItransfer(y);                //  x start
  SPItransfer(0);
  SPItransfer(y + height - 1);   //  x end

  sendLCDCommand(ST77XX_RASET);  //  Row addr set
  SPItransfer(0);
  SPItransfer(x);                //  y start
  SPItransfer(0);
  SPItransfer(x + width - 1);    //  y end

  sendLCDCommand(ST77XX_RAMWR);  //  Write to display RAM
}

void Arduboy2Core::drawBorder()
{
  const uint8_t windowWidth = WIDTH+2;
  const uint8_t windowHeight = HEIGHT+2;
  const uint8_t marginX = (TFT_WIDTH-windowWidth)/2;
  const uint8_t marginY = (TFT_HEIGHT-windowHeight)/2;

  // draw border fill

  setWriteRegion(0, 0, TFT_WIDTH, marginY-1);
  for (int i = 0; i < TFT_WIDTH*(marginY-1); i++)
  {
    SPItransfer(highByte(borderFillColor));
    SPItransfer(lowByte(borderFillColor));
  }

  setWriteRegion(0, TFT_HEIGHT-(marginY-1), TFT_WIDTH, marginY-1);
  for (int i = 0; i < TFT_WIDTH*(marginY-1); i++)
  {
    SPItransfer(highByte(borderFillColor));
    SPItransfer(lowByte(borderFillColor));
  }

  setWriteRegion(0, marginY-1, marginX-1, windowHeight+4);
  for (int i = 0; i < (marginX-1)*(windowHeight+4); i++)
  {
    SPItransfer(highByte(borderFillColor));
    SPItransfer(lowByte(borderFillColor));
  }

  setWriteRegion(TFT_WIDTH-(marginX-1), marginY-1, marginX-1, windowHeight+4);
  for (int i = 0; i < (marginX-1)*(windowHeight+4); i++)
  {
    SPItransfer(highByte(borderFillColor));
    SPItransfer(lowByte(borderFillColor));
  }

  // draw border lines

  setWriteRegion(marginX-1, marginY-1, windowWidth+2, 1);
  for (int i = 0; i < windowWidth+2; i++)
  {
    SPItransfer(highByte(borderLineColor));
    SPItransfer(lowByte(borderLineColor));
  }

  setWriteRegion(marginX-1, TFT_HEIGHT-marginY, windowWidth+2, 1);
  for (int i = 0; i < windowWidth+2; i++)
  {
    SPItransfer(highByte(borderLineColor));
    SPItransfer(lowByte(borderLineColor));
  }

  setWriteRegion(marginX-1, marginY, 1, windowHeight);
  for (int i = 0; i < windowHeight; i++)
  {
    SPItransfer(highByte(borderLineColor));
    SPItransfer(lowByte(borderLineColor));
  }

  setWriteRegion(TFT_WIDTH-marginX, marginY, 1, windowHeight);
  for (int i = 0; i < windowHeight; i++)
  {
    SPItransfer(highByte(borderLineColor));
    SPItransfer(lowByte(borderLineColor));
  }

  // clear main draw area
  setWriteRegion(marginX, marginY, windowWidth, windowHeight);
  for (int i = 0; i < windowWidth*windowHeight; i++) {
    SPItransfer(highByte(ST77XX_BLACK));
    SPItransfer(lowByte(ST77XX_BLACK));
  }

  borderDrawn = true;
}

// invert the display or set to normal
// when inverted, a pixel set to 0 will be on
void Arduboy2Core::invert(bool inverse)
{
  if (inverse == inverted)
    return;

  inverted = inverse;

  // keep LED bar color agnostic of inversion
  drawLEDs();

  sendLCDCommand(inverse ? ST77XX_INVON : ST77XX_INVOFF);
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void Arduboy2Core::allPixelsOn(bool on)
{
  sendLCDCommand(on ? ST77XX_DISPOFF : ST77XX_DISPON);
  delayShort(100);
}

// flip the display vertically or set to normal
void Arduboy2Core::flipVertical(bool flipped)
{
  if (flipped)
  {
    MADCTL |= ST77XX_MADCTL_MX;
  }
  else
  {
    MADCTL &= ~ST77XX_MADCTL_MX;
  }
  sendLCDCommand(ST77XX_MADCTL);
  SPItransfer(MADCTL);
}

// flip the display horizontally or set to normal
void Arduboy2Core::flipHorizontal(bool flipped)
{
  if (flipped)
  {
    MADCTL &= ~ST77XX_MADCTL_MY;
  }
  else
  {
    MADCTL |= ST77XX_MADCTL_MY;
  }
  sendLCDCommand(ST77XX_MADCTL);
  SPItransfer(MADCTL);
}


/* RGB LED */

void Arduboy2Core::setRGBled(uint8_t red, uint8_t green, uint8_t blue)
{
  LEDs[RED_LED] = red;
  LEDs[GREEN_LED] = green;
  LEDs[BLUE_LED] = blue;
  drawLEDs();
}

void Arduboy2Core::setRGBled(uint8_t color, uint8_t val)
{
  LEDs[color] = val;
  drawLEDs();
}

void Arduboy2Core::freeRGBled()
{
  // NOP
}

void Arduboy2Core::digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  LEDs[RED_LED] = (red == RGB_ON ? 0xFF : 0);
  LEDs[GREEN_LED] = (green == RGB_ON ? 0xFF : 0);
  LEDs[BLUE_LED] = (blue == RGB_ON ? 0xFF : 0);
  drawLEDs();
}

void Arduboy2Core::digitalWriteRGB(uint8_t color, uint8_t val)
{
  LEDs[color] = (val == RGB_ON ? 0xFF : 0);
  drawLEDs();
}

void Arduboy2Core::drawLEDs()
{
  const uint8_t red = inverted ? 0xFF - LEDs[RED_LED] : LEDs[RED_LED];
  const uint8_t green = inverted ? 0xFF - LEDs[GREEN_LED] : LEDs[GREEN_LED];
  const uint8_t blue = inverted ? 0xFF - LEDs[BLUE_LED] : LEDs[BLUE_LED];

  setWriteRegion(0, (MADCTL & ST77XX_MADCTL_MX) ? 0 : TFT_HEIGHT-4, TFT_WIDTH, 4);
  for (int i = 0; i < TFT_WIDTH*5; i++)
  {
    const uint16_t color = COLOR((red*0x1F)/0xFF, (green*0x3F)/0xFF , (blue*0x1F)/0xFF);
    SPItransfer(highByte(color));
    SPItransfer(lowByte(color));
  }
}


/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  return (
    (~PORTIN_RT_LFT_DN_UP_B_A_TX_RX & (
      bit(BIT_BUTTON_RIGHT) |
      bit(BIT_BUTTON_LEFT)  |
      bit(BIT_BUTTON_DOWN)  |
      bit(BIT_BUTTON_UP)    |
      bit(BIT_BUTTON_B)     |
      bit(BIT_BUTTON_A)
    )) >> 2
  ) | (
    (~PORTIN_CS_SPK_SEL_ST & (
      bit(BIT_BUTTON_SELECT) |
      bit(BIT_BUTTON_START)
    )) << 6
  );
}

// delay in ms with 16 bit duration
void Arduboy2Core::delayShort(uint16_t ms)
{
  delay((unsigned long)ms);
}

void Arduboy2Core::exitToBootloader()
{
  cli();
  while(true) {}
}
