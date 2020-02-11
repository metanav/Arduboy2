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
static uint8_t *frameBuf = new uint8_t[frameBufLen];
static volatile bool usingSPI;

// Forward declarations
static void setWriteRegion(uint8_t x = (DISP_WIDTH-WIDTH)/2, uint8_t y = (DISP_HEIGHT-HEIGHT)/2, uint8_t width = WIDTH, uint8_t height = HEIGHT);
static void drawRegion(uint16_t color, uint8_t x = (DISP_WIDTH-WIDTH)/2, uint8_t y = (DISP_HEIGHT-HEIGHT)/2, uint8_t width = WIDTH, uint8_t height = HEIGHT);
static void drawBorder();
static void drawBorderFill();
static void drawBorderLines();
static void drawBorderGap();
static void drawLEDs();
static void initDMA();
static void startDMA(uint8_t *data, uint16_t n);

Arduboy2Core::Arduboy2Core() { }

void Arduboy2Core::boot()
{
  bootPins();
  bootSPI();
  bootDisplay();
  bootPowerSaving();
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
}

void Arduboy2Core::bootDisplay()
{
  pinMode(PIN_DISP_SS, OUTPUT);
  pinMode(PIN_DISP_DC, OUTPUT);
  digitalWrite(PIN_DISP_SS, HIGH);

  beginDisplaySPI();

  sendDisplayCommand(ST77XX_SWRESET);  // Software reset
  delayShort(150);

  sendDisplayCommand(ST77XX_SLPOUT);  // Bring out of sleep mode
  delayShort(150);

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

  endDisplaySPI();

  drawBorder();
  blank();
}

void Arduboy2Core::displayDataMode()
{
  *portOutputRegister(IO_PORT) |= MASK_DISP_DC;
}

void Arduboy2Core::displayCommandMode()
{
  *portOutputRegister(IO_PORT) &= ~MASK_DISP_DC;
}

// Initialize the SPI interface for the display
void Arduboy2Core::bootSPI()
{
  SPI.begin();
  initDMA();
}

void Arduboy2Core::beginDisplaySPI()
{
  acquireSPI();
  *portOutputRegister(IO_PORT) &= ~MASK_DISP_SS;
  SPI.beginTransaction(SPI_SETTINGS);

// The SPISettings class won't let the frequency go above 12MHz,
// so we take matters into our own hands ;)
#ifdef SPI_24MHZ_CLOCK
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);
  SPI_SERCOM->SPI.CTRLA.bit.ENABLE = 0;
  SPI_SERCOM->SPI.BAUD.reg = 0;  // 24 Mbps, per SAMD21 data sheet
  SPI_SERCOM->SPI.CTRLA.bit.ENABLE = 1;
  while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);
#endif
}

void Arduboy2Core::endDisplaySPI()
{
  SPI.endTransaction();
  *portOutputRegister(IO_PORT) |= MASK_DISP_SS;
  freeSPI();
}

void Arduboy2Core::acquireSPI()
{
  while (usingSPI);
  usingSPI = true;
}

void Arduboy2Core::freeSPI()
{
  usingSPI = false;
}

void Arduboy2Core::SPITransfer(uint8_t data)
{
  SPI.transfer(data);
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
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_SLPIN);
  endDisplaySPI();
  delayShort(150);
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  beginDisplaySPI();
  sendDisplayCommand(ST77XX_SLPOUT);
  endDisplaySPI();
  delayShort(150);
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
  startDMA(frameBuf, frameBufLen);

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
  startDMA(frameBuf, numBytes);
}

static uint8_t borderMarginX()
{
  return (DISP_WIDTH-borderWindowWidth)/2;
}

static uint8_t borderMarginY()
{
  return (DISP_HEIGHT-borderWindowHeight)/2;
}

// Note: this function reuses frameBuf since numBytes should always be less than frameBufLen
static void drawBorderFill()
{
  const uint8_t marginX = borderMarginX();
  const uint8_t marginY = borderMarginY();
  int numBytes;

  drawRegion(borderFillColor, 0, 0, DISP_WIDTH, marginY-1);
  drawRegion(borderFillColor, 0, DISP_HEIGHT-(marginY-1), DISP_WIDTH, marginY-1);
  drawRegion(borderFillColor, 0, marginY-1, marginX-1, borderWindowHeight+4);
  drawRegion(borderFillColor, DISP_WIDTH-(marginX-1), marginY-1, marginX-1, borderWindowHeight+4);
}

// Note: this function reuses frameBuf since numBytes should always be less than frameBufLen
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

// Note: this function reuses frameBuf since numBytes should always be less than frameBufLen
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
  endDisplaySPI();
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
  endDisplaySPI();
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
  endDisplaySPI();
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

  startDMA(frameBuf, numBytes);
}


/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  uint32_t btns = ~(*portInputRegister(IO_PORT));
  return (
    (((btns & MASK_BUTTON_A) != 0) << A_BUTTON_BIT) |
    (((btns & MASK_BUTTON_B) != 0) << B_BUTTON_BIT) |
    (((btns & MASK_BUTTON_UP) != 0) << UP_BUTTON_BIT) |
    (((btns & MASK_BUTTON_DOWN) != 0) << DOWN_BUTTON_BIT) |
    (((btns & MASK_BUTTON_LEFT) != 0) << LEFT_BUTTON_BIT) |
    (((btns & MASK_BUTTON_RIGHT) != 0) << RIGHT_BUTTON_BIT) |
    (((btns & MASK_BUTTON_START) != 0) << START_BUTTON_BIT) |
    (((btns & MASK_BUTTON_SELECT) != 0) << SELECT_BUTTON_BIT)
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


//  DMA code
// -------------------------------------------------------

typedef struct
{
  uint16_t btctrl;
  uint16_t btcnt;
  uint32_t srcaddr;
  uint32_t dstaddr;
  uint32_t descaddr;
} dmaDescriptor;

static volatile dmaDescriptor wrb[12] __attribute__ ((aligned(16)));
static dmaDescriptor descriptor_section[12] __attribute__ ((aligned(16)));
static dmaDescriptor descriptor __attribute__ ((aligned(16)));

static void initDMA()
{
  PM->AHBMASK.bit.DMAC_ = 1;
  PM->APBBMASK.bit.DMAC_ = 1;
  NVIC_EnableIRQ(DMAC_IRQn);

  // Assign descriptor/WRB addresses and enable DMA
  DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
  DMAC->WRBADDR.reg = (uint32_t)wrb;
  DMAC->CTRL.reg = (
    DMAC_CTRL_DMAENABLE |
    DMAC_CTRL_LVLEN(0xF)
  );
}

static void startDMA(uint8_t *data, uint16_t n)
{
  // Disable channel
  DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHAN);
  DMAC->CHCTRLA.bit.ENABLE = 0;

  // Reset and configure
  DMAC->CHCTRLA.bit.SWRST = 1;
  DMAC->SWTRIGCTRL.reg &= ~bit(DMA_CHAN);
  DMAC->CHCTRLB.reg = (
    DMAC_CHCTRLB_LVL(0) |
    DMAC_CHCTRLB_TRIGSRC(DMA_TRIGGER_SRC) |
    DMAC_CHCTRLB_TRIGACT_BEAT
  );
  DMAC->CHINTENSET.bit.TCMPL = 1;  // enable completion interrupt

  // Configure descriptor
  descriptor.descaddr = 0;
  descriptor.dstaddr = (uint32_t)&SPI_SERCOM->SPI.DATA.reg;
  descriptor.btcnt = n;
  descriptor.srcaddr = (uint32_t)data + n;
  descriptor.btctrl = (
    DMAC_BTCTRL_VALID |
    DMAC_BTCTRL_SRCINC
  );
  memcpy(&descriptor_section[DMA_CHAN], &descriptor, sizeof(dmaDescriptor));

  // Enable channel
  DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHAN);
  DMAC->CHCTRLA.bit.ENABLE = 1;
}

void DMAC_Handler()
{
  DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHAN);
  if (DMAC->CHINTFLAG.bit.TCMPL)
  {
    // Disable DMA
    DMAC->CHID.reg = DMAC_CHID_ID(DMA_CHAN);
    DMAC->CHCTRLA.bit.ENABLE = 0;

    Arduboy2Core::endDisplaySPI();

    // Clear interrupt flag
    DMAC->CHINTENCLR.bit.TCMPL = 1;
  }
}
