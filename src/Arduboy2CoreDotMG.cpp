/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2CoreDotMG.h"
#include <SPI.h>

static uint16_t borderLineColor = COLOR_GRAY;
static uint16_t borderFillColor = COLOR_BLACK;
static uint16_t pixelColor = COLOR_WHITE;
static uint16_t bgColor = COLOR_BLACK;
static uint8_t MADCTL = ST77XX_MADCTL_MV | ST77XX_MADCTL_MY;
static uint8_t LEDs[] = {0, 0, 0};
static bool inverted = false;
static bool borderDrawn = false;
const uint8_t borderInnerGap = 1;
const uint8_t borderWindowWidth = WIDTH+borderInnerGap*2;
const uint8_t borderWindowHeight = HEIGHT+borderInnerGap*2;

#define BYTES_FOR_REGION(width, height) ((width)*(height)*12/8)  // 12 bits/px, 8 bits/byte
static const int frameBufLen = BYTES_FOR_REGION(WIDTH, HEIGHT);
static uint8_t frameBuf[frameBufLen];

static SPIClass dispSPI(
    &PERIPH_SPI_DISP,
    PIN_SPI_DISP_MISO,
    PIN_SPI_DISP_SCK,
    PIN_SPI_DISP_MOSI,
    PAD_SPI_DISP_TX,
    PAD_SPI_DISP_RX
);

// Forward declarations
static void setWriteRegion(uint8_t x = (DISP_WIDTH-WIDTH)/2, uint8_t y = (DISP_HEIGHT-HEIGHT)/2, uint8_t width = WIDTH, uint8_t height = HEIGHT);
static void drawRegion(uint16_t color, uint8_t x = (DISP_WIDTH-WIDTH)/2, uint8_t y = (DISP_HEIGHT-HEIGHT)/2, uint8_t width = WIDTH, uint8_t height = HEIGHT);
static void drawBorder();
static void drawBorderFill();
static void drawBorderLines();
static void drawBorderGap();
static void drawLEDs();

Arduboy2Core::Arduboy2Core() { }

void Arduboy2Core::boot()
{
  bootPins();
  bootDisplay();
}

void Arduboy2Core::bootPins()
{
  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);
  pinMode(PIN_BUTTON_UP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
  pinMode(PIN_BUTTON_SELECT, INPUT_PULLUP);

  pinMode(PIN_SPEAKER, OUTPUT);
}

void Arduboy2Core::bootDisplay()
{
  pinMode(PIN_DISP_DC, OUTPUT);
  pinMode(PIN_DISP_LED, OUTPUT);

  // Activate display SPI slave
  pinMode(PIN_SPI_DISP_SS, OUTPUT);
  digitalWrite(PIN_SPI_DISP_SS, LOW);

  dispSPI.begin();

  beginDisplaySPI();

  sendDisplayCommand(ST77XX_SWRESET);  // Software reset
  delayShort(150);

  displayOn();

  sendDisplayCommand(ST7735_FRMCTR1);  // Framerate ctrl - normal mode
  SPITransfer(0x01);               // Rate = fosc/(1x2+40) * (LINE+2C+2D)
  SPITransfer(0x2C);
  SPITransfer(0x2D);

  sendDisplayCommand(ST77XX_MADCTL);  // Set initial orientation
  SPITransfer(MADCTL);

  sendDisplayCommand(ST77XX_COLMOD);  // Set color mode (12-bit)
  SPITransfer(0x03);

  sendDisplayCommand(ST7735_GMCTRP1);  // Gamma Adjustments (pos. polarity)
  SPITransfer(0x02);
  SPITransfer(0x1c);
  SPITransfer(0x07);
  SPITransfer(0x12);
  SPITransfer(0x37);
  SPITransfer(0x32);
  SPITransfer(0x29);
  SPITransfer(0x2D);
  SPITransfer(0x29);
  SPITransfer(0x25);
  SPITransfer(0x2B);
  SPITransfer(0x39);
  SPITransfer(0x00);
  SPITransfer(0x01);
  SPITransfer(0x03);
  SPITransfer(0x10);

  sendDisplayCommand(ST7735_GMCTRN1);  // Gamma Adjustments (neg. polarity)
  SPITransfer(0x03);
  SPITransfer(0x1D);
  SPITransfer(0x07);
  SPITransfer(0x06);
  SPITransfer(0x2E);
  SPITransfer(0x2C);
  SPITransfer(0x29);
  SPITransfer(0x2D);
  SPITransfer(0x2E);
  SPITransfer(0x2E);
  SPITransfer(0x37);
  SPITransfer(0x3F);
  SPITransfer(0x00);
  SPITransfer(0x00);
  SPITransfer(0x02);
  SPITransfer(0x10);

  // Clear entire display
  setWriteRegion(0, 0, DISP_WIDTH, DISP_HEIGHT);
  for (int i = 0; i < DISP_WIDTH*DISP_HEIGHT/2; i++) {
    SPITransfer(bgColor >> 4);
    SPITransfer(((bgColor & 0xF) << 4) | (bgColor >> 8));
    SPITransfer(bgColor);
  }

  sendDisplayCommand(ST77XX_DISPON); //  Turn screen on
  delayShort(100);

  drawBorder();
}

void Arduboy2Core::displayDataMode()
{
  *portOutputRegister(PORT_DISP_DC_LED) |= MASK_DISP_DC;
}

void Arduboy2Core::displayCommandMode()
{
  *portOutputRegister(PORT_DISP_DC_LED) &= ~MASK_DISP_DC;
}

void Arduboy2Core::beginDisplaySPI()
{
  dispSPI.waitForTransfer();  // Block until any DMA transfers finish
  dispSPI.endTransaction();  // End any previous transaction
  dispSPI.beginTransaction(SPI_SETTINGS_DISP); // Start new transaction
}

void Arduboy2Core::SPITransfer(uint8_t data)
{
  dispSPI.transfer(data);
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

// Shut down the display
void Arduboy2Core::displayOff()
{
  *portOutputRegister(PORT_DISP_DC_LED) &= ~MASK_DISP_LED;
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_SLPIN);
  delayShort(150);
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_SLPOUT);
  delayShort(150);
  *portOutputRegister(PORT_DISP_DC_LED) |= MASK_DISP_LED;
}


/* Drawing */

uint16_t Arduboy2Core::getPixelColor()
{
  return pixelColor;
}

void Arduboy2Core::setPixelColor(uint16_t color)
{
  pixelColor = color;
}

uint16_t Arduboy2Core::getBackgroundColor()
{
  return bgColor;
}

void Arduboy2Core::setBackgroundColor(uint16_t color)
{
  bgColor = color;

  if (borderDrawn)
    drawBorderGap();
}

uint16_t Arduboy2Core::getBorderLineColor()
{
  return borderLineColor;
}

void Arduboy2Core::setBorderLineColor(uint16_t color)
{
  borderLineColor = color;

  if (borderDrawn)
    drawBorderLines();
}

uint16_t Arduboy2Core::getBorderFillColor()
{
  return borderFillColor;
}

void Arduboy2Core::setBorderFillColor(uint16_t color)
{
  borderFillColor = color;

  if (borderDrawn)
    drawBorderFill();
}

void Arduboy2Core::setColorTheme(Theme theme)
{
  setPixelColor(theme.pixelColor);
  setBackgroundColor(theme.backgroundColor);
  setBorderLineColor(theme.borderLineColor);
  setBorderFillColor(theme.borderFillColor);
}

void Arduboy2Core::paintScreen(const uint8_t *image)
{
  paintScreen((uint8_t *)image, false);
}

void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
  beginDisplaySPI();

  int b = 0;
  for (int y = 0; y < HEIGHT; y++)
  {
    int row = y >> 3;  // y / 8
    uint8_t rowMask = bit(y & 0x7);  // y % 8
    for (int x = 0; x < WIDTH; x += 2)
    {
      // Read next 2 pixels
      uint8_t *img = image + (x + row*WIDTH);
      uint16_t p0 = (img[0] & rowMask) ? pixelColor : bgColor;
      uint16_t p1 = (img[1] & rowMask) ? pixelColor : bgColor;

      // Write both as 12-bit pixels (3 bytes)
      frameBuf[b++] = p0 >> 4;
      frameBuf[b++] = ((p0 & 0xF) << 4) | (p1 >> 8);
      frameBuf[b++] = p1;
    }
  }

  setWriteRegion();
  dispSPI.transfer(frameBuf, NULL, frameBufLen, false);

  if (clear)
    memset(image, 0, WIDTH*HEIGHT/8);
}

void Arduboy2Core::blank()
{
  drawRegion(bgColor);
}

void Arduboy2Core::sendDisplayCommand(uint8_t command)
{
  displayCommandMode();
  SPITransfer(command);
  displayDataMode();
}

static void setWriteRegion(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
  Arduboy2Core::sendDisplayCommand(ST77XX_CASET);  //  Column addr set
  Arduboy2Core::SPITransfer(0);
  Arduboy2Core::SPITransfer(x);                    //  x start
  Arduboy2Core::SPITransfer(0);
  Arduboy2Core::SPITransfer(x + width - 1);        //  x end

  Arduboy2Core::sendDisplayCommand(ST77XX_RASET);  //  Row addr set
  Arduboy2Core::SPITransfer(0);
  Arduboy2Core::SPITransfer(y);                    //  y start
  Arduboy2Core::SPITransfer(0);
  Arduboy2Core::SPITransfer(y + height - 1);       //  y end

  Arduboy2Core::sendDisplayCommand(ST77XX_RAMWR);  //  Initialize write to display RAM
}

static void drawRegion(uint16_t color, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
  Arduboy2Core::beginDisplaySPI();

  int numBytes = BYTES_FOR_REGION(width, height);
  setWriteRegion(x, y, width, height);
  for (int i = 0; i < numBytes; i += 3)
  {
    frameBuf[i] = color >> 4;
    frameBuf[i+1] = ((color & 0xF) << 4) | (color >> 8);
    frameBuf[i+2] = color;
  }

  dispSPI.transfer(frameBuf, NULL, numBytes, false);
}

static uint8_t borderMarginX()
{
  return (DISP_WIDTH-borderWindowWidth)/2;
}

static uint8_t borderMarginY()
{
  return (DISP_HEIGHT-borderWindowHeight)/2;
}

static void drawBorderFill()
{
  const uint8_t marginX = borderMarginX();
  const uint8_t marginY = borderMarginY();
  drawRegion(borderFillColor, 0, 0, DISP_WIDTH, marginY-1);
  drawRegion(borderFillColor, 0, DISP_HEIGHT-(marginY-1), DISP_WIDTH, marginY-1);
  drawRegion(borderFillColor, 0, marginY-1, marginX-1, borderWindowHeight+4);
  drawRegion(borderFillColor, DISP_WIDTH-(marginX-1), marginY-1, marginX-1, borderWindowHeight+4);
}

static void drawBorderLines()
{
  const uint8_t marginX = borderMarginX();
  const uint8_t marginY = borderMarginY();
  int numBytes;

  drawRegion(borderLineColor, marginX-1, marginY-1, borderWindowWidth+2, 1);
  drawRegion(borderLineColor, marginX-1, DISP_HEIGHT-marginY, borderWindowWidth+2, 1);
  drawRegion(borderLineColor, marginX-1, marginY, 1, borderWindowHeight);
  drawRegion(borderLineColor, DISP_WIDTH-marginX, marginY, 1, borderWindowHeight);
}

static void drawBorderGap()
{
  const uint8_t marginX = borderMarginX();
  const uint8_t marginY = borderMarginY();
  int numBytes;

  drawRegion(bgColor, marginX, marginY, borderWindowWidth, borderInnerGap);
  drawRegion(bgColor, marginX, DISP_HEIGHT-marginY-borderInnerGap, borderWindowWidth, borderInnerGap);
  drawRegion(bgColor, marginX, marginY+borderInnerGap, borderInnerGap, HEIGHT);
  drawRegion(bgColor, DISP_WIDTH-marginX-borderInnerGap, marginY+borderInnerGap, borderInnerGap, HEIGHT);
}

static void drawBorder()
{
  drawBorderFill();
  drawBorderLines();
  drawBorderGap();

  borderDrawn = true;
}

// invert the display or set to normal
void Arduboy2Core::invert(bool inverse)
{
  if (inverse == inverted)
    return;

  inverted = inverse;

  uint16_t tmp = pixelColor;
  setPixelColor(bgColor);
  setBackgroundColor(tmp);
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void Arduboy2Core::allPixelsOn(bool on)
{
  beginDisplaySPI();
  sendDisplayCommand(on ? ST77XX_DISPOFF : ST77XX_DISPON);
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
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_MADCTL);
  SPITransfer(MADCTL);
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
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_MADCTL);
  SPITransfer(MADCTL);
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

static void drawLEDs()
{
  Arduboy2Core::beginDisplaySPI();

  int numBytes = BYTES_FOR_REGION(DISP_WIDTH, 4);
  setWriteRegion(0, (MADCTL & ST77XX_MADCTL_MX) ? 0 : DISP_HEIGHT-4, DISP_WIDTH, 4);

  for (int i = 0; i < numBytes; i += 3)
  {
    const uint16_t color = color444::from8BitRGB(LEDs[RED_LED], LEDs[GREEN_LED], LEDs[BLUE_LED]);

    // Reuse frameBuf since numBytes should be less than frameBufLen
    frameBuf[i] = color >> 4;
    frameBuf[i + 1] = ((color & 0xF) << 4) | (color >> 8);
    frameBuf[i + 2] = color;
  }

  dispSPI.transfer(frameBuf, NULL, numBytes, false);
}


/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  uint32_t st_sel_up_rt = ~(*portInputRegister(PORT_ST_SEL_UP_RT));
  uint32_t a_b_dn_lf = ~(*portInputRegister(PORT_A_B_DN_LF));
  return (
    (((a_b_dn_lf & MASK_BUTTON_A) != 0) << A_BUTTON_BIT) |
    (((a_b_dn_lf & MASK_BUTTON_B) != 0) << B_BUTTON_BIT) |
    (((a_b_dn_lf & MASK_BUTTON_DOWN) != 0) << DOWN_BUTTON_BIT) |
    (((a_b_dn_lf & MASK_BUTTON_LEFT) != 0) << LEFT_BUTTON_BIT) |
    (((st_sel_up_rt & MASK_BUTTON_UP) != 0) << UP_BUTTON_BIT) |
    (((st_sel_up_rt & MASK_BUTTON_RIGHT) != 0) << RIGHT_BUTTON_BIT) |
    (((st_sel_up_rt & MASK_BUTTON_START) != 0) << START_BUTTON_BIT) |
    (((st_sel_up_rt & MASK_BUTTON_SELECT) != 0) << SELECT_BUTTON_BIT)
  );
}

// delay in ms with 16 bit duration
void Arduboy2Core::delayShort(uint16_t ms)
{
  delay((unsigned long)ms);
}

void Arduboy2Core::exitToBootloader()
{
  noInterrupts();
  while (true);
}
