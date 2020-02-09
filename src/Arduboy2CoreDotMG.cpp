/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2CoreDotMG.h"
#include <SPI.h>

static uint16_t borderLineColor = ST77XX_GRAY;
static uint16_t borderFillColor = ST77XX_BLACK;
static uint16_t pixelColor = ST77XX_WHITE;
static uint16_t bgColor = ST77XX_BLACK;
static uint8_t MADCTL = ST77XX_MADCTL_MV | ST77XX_MADCTL_MY;
static uint8_t LEDs[] = {0, 0, 0};
static bool inverted = false;
static bool borderDrawn = false;

static const int frameBufLen = WIDTH*HEIGHT*12/8; // 12 bits/px, 8 bits/byte
static uint8_t *frameBuf = new uint8_t[frameBufLen];
static volatile bool usingSPI;

// Forward declarations
static void setWriteRegion(uint8_t x = (TFT_WIDTH-WIDTH)/2, uint8_t y = (TFT_HEIGHT-HEIGHT)/2, uint8_t width = WIDTH, uint8_t height = HEIGHT);
static void drawBorder();
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
  pinMode(PIN_TFT_CS, OUTPUT);
  pinMode(PIN_TFT_DC, OUTPUT);
  pinMode(PIN_TFT_RST, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);

  // Reset display
  delayShort(5);  // Let display stay in reset
  digitalWrite(PIN_TFT_RST, HIGH); // Bring out of reset
  delayShort(5);

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
  setWriteRegion(0, 0, TFT_WIDTH, TFT_HEIGHT);
  for (int i = 0; i < TFT_WIDTH*TFT_HEIGHT/2; i++) {
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
  *portOutputRegister(IO_PORT) |= MASK_TFT_DC;
}

void Arduboy2Core::displayCommandMode()
{
  *portOutputRegister(IO_PORT) &= ~MASK_TFT_DC;
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
  *portOutputRegister(IO_PORT) &= ~MASK_TFT_CS;
  SPI.beginTransaction(SPI_SETTINGS);

  // TODO: set to SPI clock to 24MHz?
  // SPI_SERCOM->SPI.BAUD.reg = 0;  // 24 Mbps
}

void Arduboy2Core::endDisplaySPI()
{
  SPI.endTransaction();
  *portOutputRegister(IO_PORT) |= MASK_TFT_CS;
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

void Arduboy2Core::idle()
{
  // Not implemented
}

void Arduboy2Core::bootPowerSaving()
{
  // Not implemented
}

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

uint16_t Arduboy2Core::getBackgroundColor()
{
  return bgColor;
}

void Arduboy2Core::setBackgroundColor(uint16_t color)
{
  bgColor = color;

  if (borderDrawn)
    drawBorder();
}

void Arduboy2Core::paint8Pixels(uint8_t pixels)
{
  // Not implemented
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
  // endDisplaySPI() called by IRQ handler after DMA completes

  if (clear)
    memset(image, 0, WIDTH*HEIGHT/8);
}

void Arduboy2Core::blank()
{
  beginDisplaySPI();

  for (int i = 0; i < frameBufLen; i += 3)
  {
    frameBuf[i] = bgColor >> 4;
    frameBuf[i + 1] = ((bgColor & 0xF) << 4) | (bgColor >> 8);
    frameBuf[i + 2] = bgColor;
  }

  setWriteRegion();
  startDMA(frameBuf, frameBufLen);
  // endDisplaySPI() called by IRQ handler after DMA completes
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

static void drawBorder()
{
  const uint8_t innerGap = 1;
  const uint8_t windowWidth = WIDTH+innerGap*2;
  const uint8_t windowHeight = HEIGHT+innerGap*2;
  const uint8_t marginX = (TFT_WIDTH-windowWidth)/2;
  const uint8_t marginY = (TFT_HEIGHT-windowHeight)/2;

  // draw border fill

  Arduboy2Core::beginDisplaySPI();

  setWriteRegion(0, 0, TFT_WIDTH, marginY-1);
  for (int i = 0; i < (TFT_WIDTH*(marginY-1))/2; i++)
  {
    Arduboy2Core::SPITransfer(borderFillColor >> 4);
    Arduboy2Core::SPITransfer(((borderFillColor & 0xF) << 4) | (borderFillColor >> 8));
    Arduboy2Core::SPITransfer(borderFillColor);
  }

  setWriteRegion(0, TFT_HEIGHT-(marginY-1), TFT_WIDTH, marginY-1);
  for (int i = 0; i < (TFT_WIDTH*(marginY-1))/2; i++)
  {
    Arduboy2Core::SPITransfer(borderFillColor >> 4);
    Arduboy2Core::SPITransfer(((borderFillColor & 0xF) << 4) | (borderFillColor >> 8));
    Arduboy2Core::SPITransfer(borderFillColor);
  }

  setWriteRegion(0, marginY-1, marginX-1, windowHeight+4);
  for (int i = 0; i < ((marginX-1)*(windowHeight+4))/2; i++)
  {
    Arduboy2Core::SPITransfer(borderFillColor >> 4);
    Arduboy2Core::SPITransfer(((borderFillColor & 0xF) << 4) | (borderFillColor >> 8));
    Arduboy2Core::SPITransfer(borderFillColor);
  }

  setWriteRegion(TFT_WIDTH-(marginX-1), marginY-1, marginX-1, windowHeight+4);
  for (int i = 0; i < ((marginX-1)*(windowHeight+4))/2; i++)
  {
    Arduboy2Core::SPITransfer(borderFillColor >> 4);
    Arduboy2Core::SPITransfer(((borderFillColor & 0xF) << 4) | (borderFillColor >> 8));
    Arduboy2Core::SPITransfer(borderFillColor);
  }

  // draw border lines

  setWriteRegion(marginX-1, marginY-1, windowWidth+2, 1);
  for (int i = 0; i < (windowWidth+2)/2; i++)
  {
    Arduboy2Core::SPITransfer(borderLineColor >> 4);
    Arduboy2Core::SPITransfer(((borderLineColor & 0xF) << 4) | (borderLineColor >> 8));
    Arduboy2Core::SPITransfer(borderLineColor);
  }

  setWriteRegion(marginX-1, TFT_HEIGHT-marginY, windowWidth+2, 1);
  for (int i = 0; i < (windowWidth+2)/2; i++)
  {
    Arduboy2Core::SPITransfer(borderLineColor >> 4);
    Arduboy2Core::SPITransfer(((borderLineColor & 0xF) << 4) | (borderLineColor >> 8));
    Arduboy2Core::SPITransfer(borderLineColor);
  }

  setWriteRegion(marginX-1, marginY, 1, windowHeight);
  for (int i = 0; i < windowHeight/2; i++)
  {
    Arduboy2Core::SPITransfer(borderLineColor >> 4);
    Arduboy2Core::SPITransfer(((borderLineColor & 0xF) << 4) | (borderLineColor >> 8));
    Arduboy2Core::SPITransfer(borderLineColor);
  }

  setWriteRegion(TFT_WIDTH-marginX, marginY, 1, windowHeight);
  for (int i = 0; i < windowHeight/2; i++)
  {
    Arduboy2Core::SPITransfer(borderLineColor >> 4);
    Arduboy2Core::SPITransfer(((borderLineColor & 0xF) << 4) | (borderLineColor >> 8));
    Arduboy2Core::SPITransfer(borderLineColor);
  }

  // draw gap around display area
  setWriteRegion(marginX, marginY, windowWidth, innerGap);
  for (int i = 0; i < (windowWidth*innerGap)/2; i++)
  {
    Arduboy2Core::SPITransfer(bgColor >> 4);
    Arduboy2Core::SPITransfer(((bgColor & 0xF) << 4) | (bgColor >> 8));
    Arduboy2Core::SPITransfer(bgColor);
  }

  setWriteRegion(marginX, TFT_HEIGHT-marginY-innerGap, windowWidth, innerGap);
  for (int i = 0; i < (windowWidth*innerGap)/2; i++)
  {
    Arduboy2Core::SPITransfer(bgColor >> 4);
    Arduboy2Core::SPITransfer(((bgColor & 0xF) << 4) | (bgColor >> 8));
    Arduboy2Core::SPITransfer(bgColor);
  }

  setWriteRegion(marginX, marginY+innerGap, innerGap, HEIGHT);
  for (int i = 0; i < HEIGHT*innerGap/2; i++)
  {
    Arduboy2Core::SPITransfer(bgColor >> 4);
    Arduboy2Core::SPITransfer(((bgColor & 0xF) << 4) | (bgColor >> 8));
    Arduboy2Core::SPITransfer(bgColor);
  }

  setWriteRegion(TFT_WIDTH-marginX-innerGap, marginY+innerGap, innerGap, HEIGHT);
  for (int i = 0; i < HEIGHT*innerGap/2; i++)
  {
    Arduboy2Core::SPITransfer(bgColor >> 4);
    Arduboy2Core::SPITransfer(((bgColor & 0xF) << 4) | (bgColor >> 8));
    Arduboy2Core::SPITransfer(bgColor);
  }

  Arduboy2Core::endDisplaySPI();

  borderDrawn = true;
}

// invert the display or set to normal
void Arduboy2Core::invert(bool inverse)
{
  if (inverse == inverted)
    return;

  inverted = inverse;

  // keep LED bar color agnostic of inversion
  drawLEDs();

  beginDisplaySPI();
  sendDisplayCommand(inverse ? ST77XX_INVON : ST77XX_INVOFF);
  endDisplaySPI();
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
  const uint8_t red = inverted ? 0xFF - LEDs[RED_LED] : LEDs[RED_LED];
  const uint8_t green = inverted ? 0xFF - LEDs[GREEN_LED] : LEDs[GREEN_LED];
  const uint8_t blue = inverted ? 0xFF - LEDs[BLUE_LED] : LEDs[BLUE_LED];

  Arduboy2Core::beginDisplaySPI();
  int numBytes = (TFT_WIDTH*4)*12/8; // 12 bits/px, 8 bits/byte
  for (int i = 0; i < numBytes; i += 3)
  {
    const uint16_t color = COLOR((red*0xF)/0xFF, (green*0xF)/0xFF , (blue*0xF)/0xFF);

    // Reuse frameBuf since numBytes should be less than frameBufLen
    frameBuf[i] = color >> 4;
    frameBuf[i + 1] = ((color & 0xF) << 4) | (color >> 8);
    frameBuf[i + 2] = color;
  }

  setWriteRegion(0, (MADCTL & ST77XX_MADCTL_MX) ? 0 : TFT_HEIGHT-4, TFT_WIDTH, 4);
  startDMA(frameBuf, numBytes);
  // endDisplaySPI() called by IRQ handler after DMA completes
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
  while(true) {}
}

void Arduboy2Core::mainNoUSB()
{
  init();

  pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);

  // Delay to give time for the pin to be pulled high if it was floating
  delayShort(10);

  // If the down button is pressed
  if (!digitalRead(PIN_BUTTON_DOWN)) {
    exitToBootloader();
  }

  // The remainder is a copy of the Arduino main() function with the
  // USB code and other unneeded code removed.

  setup();

  for (;;) {
    loop();
  }
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
} dmaDescriptor ;

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
