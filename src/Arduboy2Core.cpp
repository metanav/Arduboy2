/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2Core.h"
#include <SPI.h>

TFT_eSPI screen = TFT_eSPI();

static uint16_t borderLineColor =  TFT_LIGHTGREY;
static uint16_t borderFillColor =  TFT_BLACK; 
static uint16_t pixelColor = TFT_WHITE;
static uint16_t bgColor = TFT_BLACK;
static uint8_t LEDs[] = {0, 0, 0};
static bool inverted = false;
static bool borderDrawn = false;
const uint8_t borderInnerGap = 1;
const uint8_t borderWindowWidth = WIDTH+borderInnerGap*2;
const uint8_t borderWindowHeight = HEIGHT+borderInnerGap*2;

#define BYTES_FOR_REGION(width, height) ((width)*(height)*12/8)  // 12 bits/px, 8 bits/byte
static const int frameBufLen = BYTES_FOR_REGION(WIDTH, HEIGHT);
static uint8_t frameBuf[frameBufLen];

// Forward declarations

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
  screen.begin();
  delay(200);
  screen.setRotation(3);
  screen.fillScreen(TFT_BLACK);
  delayShort(100);
  //drawBorder();
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
    screen.writecommand(ILI9341_DISPOFF); 
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
    screen.writecommand(ILI9341_DISPON);
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

  if (borderDrawn) {
    drawBorderGap();
  }
}

uint16_t Arduboy2Core::getBorderLineColor()
{
  return borderLineColor;
}

void Arduboy2Core::setBorderLineColor(uint16_t color)
{
  borderLineColor = color;

  if (borderDrawn) {
    drawBorderLines();
  }
}

uint16_t Arduboy2Core::getBorderFillColor()
{
  return borderFillColor;
}

void Arduboy2Core::setBorderFillColor(uint16_t color)
{
  borderFillColor = color;

  if (borderDrawn) {
    drawBorderFill();
  }
}

void Arduboy2Core::setColorTheme(Theme theme)
{
  setPixelColor(theme.pixelColor);
  setBackgroundColor(theme.backgroundColor);
  setBorderLineColor(theme.borderLineColor);
  setBorderFillColor(theme.borderFillColor);
}

void Arduboy2Core::scale(const uint8_t *image, uint16_t w1, uint16_t h1, uint8_t *scaledImage, uint16_t w2, uint16_t h2) 
{
    uint16_t x_ratio = (uint16_t)((w1<<16)/w2) +1;
    uint16_t  y_ratio = (uint16_t)((h1<<16)/h2) +1;
    //SerialUSB.print("x_ratio: ");
    //SerialUSB.println(x_ratio);
    //SerialUSB.print("y_ratio: ");
    //SerialUSB.println(x_ratio);

    uint16_t x2, y2 ;

    for (uint16_t i=0; i < h2; i++) {
        for (uint16_t j=0; j<w2; j++) {
            x2 = ((j * x_ratio) >> 16) ;
            y2 = ((i * y_ratio) >> 16) ;

            SerialUSB.print(x2);
            SerialUSB.print(",");
            SerialUSB.print(y2);
            SerialUSB.print(",");
            SerialUSB.print((i * w2) + j);
            SerialUSB.print(",");
            SerialUSB.println((y2 * w1) + x2);
            scaledImage[(i * w2) + j] = image[(y2 * w1) + x2] ;
        }                
    }                
}

void Arduboy2Core::paintScreen(const uint8_t *image)
{
  paintScreen((uint8_t *)image, false);
}

void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{

  static uint16_t frameBuf_1[WIDTH * HEIGHT];
  static uint16_t xPos, yPos, addr;
  static uint8_t pixel;

  for (xPos = 0; xPos < WIDTH; xPos++) {
    for (yPos = 0; yPos < HEIGHT; yPos++) {
      if (!(yPos % 8)) {
        pixel = image[xPos + (yPos >> 3) * WIDTH];
      }
      addr = yPos * WIDTH  + xPos;
      
      if (pixel & 0x01) {
        frameBuf_1[addr] = pixelColor;
      } else {
        frameBuf_1[addr] = bgColor;
      }
      
      pixel = pixel >> 1;
    }
  }

  static uint16_t scaledImage[S_WIDTH * S_HEIGHT] = {0};
  uint16_t x2, y2;
  uint16_t x_ratio = (uint16_t)((WIDTH<<16)/S_WIDTH) + 1;
  uint16_t y_ratio = (uint16_t)((HEIGHT<<16)/S_HEIGHT) + 1;

  for (uint16_t i = 0; i < S_HEIGHT; i++) {
      for (uint16_t j = 0; j < S_WIDTH; j++) {
          x2 = ((j * x_ratio) >> 16) ;
          y2 = ((i * y_ratio) >> 16) ;
          scaledImage[(i * S_WIDTH) + j] = frameBuf_1[(y2 * WIDTH) + x2] ;
      }                
  }                
  
  //screen.pushImage((DISP_WIDTH-WIDTH)/2,(DISP_HEIGHT-HEIGHT)/2, WIDTH, HEIGHT, frameBuf_1);
  screen.pushImage((DISP_WIDTH - S_WIDTH)/2,(DISP_HEIGHT - S_HEIGHT)/2, S_WIDTH, S_HEIGHT, scaledImage);

  if (clear) {
    memset(image, 0, WIDTH*HEIGHT/8);
  }
}

/*void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
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
      //frameBuf[row * WIDTH + x] = p0 >> 4;
      //frameBuf[row * WIDTH + x + 1] = ((p0 & 0xF) << 4) | (p1 >> 8);
      //frameBuf[row * WIDTH + x + 2] = p1;
    }
  }

  screen.pushImage((DISP_WIDTH-HEIGHT)/2, (DISP_HEIGHT-WIDTH)/2, HEIGHT, WIDTH, frameBuf);

  if (clear) {
    memset(image, 0, WIDTH*HEIGHT/8);
  }
}*/

void Arduboy2Core::blank()
{
  drawRegion(bgColor);
}

static void drawRegion(uint16_t color, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
  int numBytes = BYTES_FOR_REGION(width, height);

  for (int i = 0; i < numBytes; i += 3)
  {
    frameBuf[i] = color >> 4;
    frameBuf[i+1] = ((color & 0xF) << 4) | (color >> 8);
    frameBuf[i+2] = color;
  }

  screen.pushImage(x, y,  width, height, frameBuf);
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
 if (on) {
    displayOn();
 } else {
    displayOff();
 }
 
 delayShort(100);
}

// flip the display vertically or set to normal
void Arduboy2Core::flipVertical(bool flipped)
{
  if (flipped)
  {
    //MADCTL |= ST77XX_MADCTL_MX;
  }
  else
  {
    //MADCTL &= ~ST77XX_MADCTL_MX;
  }
  //beginDisplaySPI();
  //sendDisplayCommand(ST77XX_MADCTL);
  //SPITransfer(MADCTL);
}

// flip the display horizontally or set to normal
void Arduboy2Core::flipHorizontal(bool flipped)
{
  if (flipped)
  {
    //MADCTL &= ~ST77XX_MADCTL_MY;
  }
  else
  {
    //MADCTL |= ST77XX_MADCTL_MY;
  }
  //beginDisplaySPI();
  //sendDisplayCommand(ST77XX_MADCTL);
  //SPITransfer(MADCTL);
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
  //Arduboy2Core::beginDisplaySPI();

  int numBytes = BYTES_FOR_REGION(DISP_WIDTH, 4);
  //setWriteRegion(0, (MADCTL & ST77XX_MADCTL_MX) ? 0 : DISP_HEIGHT-4, DISP_WIDTH, 4);

  for (int i = 0; i < numBytes; i += 3)
  {
    const uint16_t color = color444::from8BitRGB(LEDs[RED_LED], LEDs[GREEN_LED], LEDs[BLUE_LED]);

    // Reuse frameBuf since numBytes should be less than frameBufLen
    frameBuf[i] = color >> 4;
    frameBuf[i + 1] = ((color & 0xF) << 4) | (color >> 8);
    frameBuf[i + 2] = color;
  }

  //dispSPI.transfer(frameBuf, NULL, numBytes, false);
}


/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  //uint32_t st_sel_up_rt = ~(*portInputRegister(PORT_ST_SEL_UP_RT));
  //uint32_t a_b_dn_lf = ~(*portInputRegister(PORT_A_B_DN_LF));
  uint32_t st_a_b = ~(*portInputRegister(PORT_ST_A_B));
  uint32_t up_lf_dn_rt_ps = ~(*portInputRegister(PORT_UP_LF_DN_RT_PS));

  return (
    (((st_a_b & MASK_BUTTON_A) != 0) << A_BUTTON_BIT) |
    (((st_a_b & MASK_BUTTON_B) != 0) << B_BUTTON_BIT) |
    (((up_lf_dn_rt_ps & MASK_BUTTON_DOWN) != 0) << DOWN_BUTTON_BIT) |
    (((up_lf_dn_rt_ps & MASK_BUTTON_LEFT) != 0) << LEFT_BUTTON_BIT) |
    (((up_lf_dn_rt_ps & MASK_BUTTON_UP) != 0) << UP_BUTTON_BIT) |
    (((up_lf_dn_rt_ps & MASK_BUTTON_RIGHT) != 0) << RIGHT_BUTTON_BIT) |
    (((st_a_b & MASK_BUTTON_START) != 0) << START_BUTTON_BIT) |
    (((up_lf_dn_rt_ps & MASK_BUTTON_SELECT) != 0) << SELECT_BUTTON_BIT)
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
