/**
 * @file Arduboy2CoreDotMG.h
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#ifndef ARDUBOY2_CORE_DOTMG_H
#define ARDUBOY2_CORE_DOTMG_H

#include <Arduino.h>
#include "themes/themes.h"
#include "themes/colors.h"

// ----- Helpful values/macros -----

// Button values

#define A_BUTTON_BIT        0
#define A_BUTTON            bit(A_BUTTON_BIT)

#define B_BUTTON_BIT        1
#define B_BUTTON            bit(B_BUTTON_BIT)

#define UP_BUTTON_BIT       2
#define UP_BUTTON           bit(UP_BUTTON_BIT)

#define DOWN_BUTTON_BIT     3
#define DOWN_BUTTON         bit(DOWN_BUTTON_BIT)

#define LEFT_BUTTON_BIT     4
#define LEFT_BUTTON         bit(LEFT_BUTTON_BIT)

#define RIGHT_BUTTON_BIT    5
#define RIGHT_BUTTON        bit(RIGHT_BUTTON_BIT)

#define START_BUTTON_BIT    6
#define START_BUTTON        bit(START_BUTTON_BIT)

#define SELECT_BUTTON_BIT   7
#define SELECT_BUTTON       bit(SELECT_BUTTON_BIT)

// LED values

#define RED_LED    0
#define GREEN_LED  1
#define BLUE_LED   2

#define RGB_OFF    0
#define RGB_ON     1

// Display values

#define WIDTH       128
#define HEIGHT      64
#define DISP_WIDTH   160
#define DISP_HEIGHT  128


// ----- Pins -----

#define PORT_ST_SEL_UP_RT   (&(PORT->Group[PORTA]))
#define PORT_A_B_DN_LF      (&(PORT->Group[PORTB]))
#define PORT_DISP_DC_LED    (&(PORT->Group[PORTA]))

#define MASK_DISP_DC        digitalPinToBitMask(PIN_DISP_DC)
#define MASK_DISP_LED       digitalPinToBitMask(PIN_DISP_LED)
#define MASK_SPI_MOSI       digitalPinToBitMask(PIN_SPI_DISP_MOSI)
#define MASK_SPI_SCK        digitalPinToBitMask(PIN_SPI_DISP_SCK)

#define PIN_BUTTON_A        20
#define MASK_BUTTON_A       digitalPinToBitMask(PIN_BUTTON_A)

#define PIN_BUTTON_B        21
#define MASK_BUTTON_B       digitalPinToBitMask(PIN_BUTTON_B)

#define PIN_BUTTON_UP       18
#define MASK_BUTTON_UP      digitalPinToBitMask(PIN_BUTTON_UP)

#define PIN_BUTTON_DOWN     4
#define MASK_BUTTON_DOWN    digitalPinToBitMask(PIN_BUTTON_DOWN)

#define PIN_BUTTON_LEFT     5
#define MASK_BUTTON_LEFT    digitalPinToBitMask(PIN_BUTTON_LEFT)

#define PIN_BUTTON_RIGHT    19
#define MASK_BUTTON_RIGHT   digitalPinToBitMask(PIN_BUTTON_RIGHT)

#define PIN_BUTTON_START    0
#define MASK_BUTTON_START   digitalPinToBitMask(PIN_BUTTON_START)

#define PIN_BUTTON_SELECT   1
#define MASK_BUTTON_SELECT  digitalPinToBitMask(PIN_BUTTON_SELECT)

#define PIN_SPEAKER         PIN_DAC1
#define DAC_CH_SPEAKER      1
#define DAC_READY           DAC->STATUS.bit.READY1
#define DAC_DATA_BUSY       DAC->SYNCBUSY.bit.DATA1


// ----- For display configuration (ST7735R) -----

#define ST77XX_NOP        0x00
#define ST77XX_SWRESET    0x01
#define ST77XX_RDDID      0x04
#define ST77XX_RDDST      0x09

#define ST77XX_SLPIN      0x10
#define ST77XX_SLPOUT     0x11
#define ST77XX_PTLON      0x12
#define ST77XX_NORON      0x13

#define ST77XX_INVOFF     0x20
#define ST77XX_INVON      0x21
#define ST77XX_DISPOFF    0x28
#define ST77XX_DISPON     0x29
#define ST77XX_CASET      0x2A
#define ST77XX_RASET      0x2B
#define ST77XX_RAMWR      0x2C
#define ST77XX_RAMRD      0x2E

#define ST77XX_PTLAR      0x30
#define ST77XX_COLMOD     0x3A
#define ST77XX_MADCTL     0x36

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1      0xDA
#define ST77XX_RDID2      0xDB
#define ST77XX_RDID3      0xDC
#define ST77XX_RDID4      0xDD

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

#define ST7735_FRMCTR1    0xB1
#define ST7735_FRMCTR2    0xB2
#define ST7735_FRMCTR3    0xB3
#define ST7735_INVCTR     0xB4
#define ST7735_DISSET5    0xB6

#define ST7735_PWCTR1     0xC0
#define ST7735_PWCTR2     0xC1
#define ST7735_PWCTR3     0xC2
#define ST7735_PWCTR4     0xC3
#define ST7735_PWCTR5     0xC4
#define ST7735_VMCTR1     0xC5

#define ST7735_PWCTR6     0xFC

#define ST7735_GMCTRP1    0xE0
#define ST7735_GMCTRN1    0xE1

// --------------------


/** \brief
 * Lower level functions generally dealing directly with the hardware.
 *
 * \details
 * This class is inherited by Arduboy2Base and thus also Arduboy2, so wouldn't
 * normally be used directly by a sketch.
 *
 * \note
 * A friend class named _Arduboy2Ex_ is declared by this class. The intention
 * is to allow a sketch to create an _Arduboy2Ex_ class which would have access
 * to the private and protected members of the Arduboy2Core class. It is hoped
 * that this may eliminate the need to create an entire local copy of the
 * library, in order to extend the functionality, in most circumstances.
 */
class Arduboy2Core
{
  friend class Arduboy2Ex;

  public:
    Arduboy2Core();

    /** \brief
     * Originally intended to save power on the original Arduboy. It is not
     * necessary for dotMG, so this function does nothing.
     */
    void static idle() {}

    /** \brief
     * Put the display into data mode.
     *
     * \details
     * When placed in data mode, data that is sent to the display will be
     * considered as data to be displayed.
     *
     * \note
     * This is a low level function that is not intended for general use in a
     * sketch. It has been made public and documented for use by derived
     * classes.
     *
     * \see displayCommandMode() SPITransfer()
     */
    void static displayDataMode();
    inline void static LCDDataMode() { displayDataMode(); }  // For compatibility

    /** \brief
     * Put the display into command mode.
     *
     * \details
     * When placed in command mode, data that is sent to the display will be
     * treated as commands.
     *
     * \note
     * This is a low level function that is not intended for general use in a
     * sketch. It has been made public and documented for use by derived
     * classes.
     *
     * \see displayDataMode() sendDisplayCommand() SPITransfer()
     */
    void static displayCommandMode();
    inline void static LCDCommandMode() { displayCommandMode(); }  // For compatibility

    /** \brief
     * Initializes an SPI transfer for the display.
     *
     * \see SPITransfer()
     */
    void static beginDisplaySPI();

    /** \brief
     * Transfer a byte over SPI.
     *
     * \param data The byte to be sent over SPI.
     *
     * \details
     * Transfer one byte over the SPI bus and wait for the transfer to
     * complete.
     */
    void static SPITransfer(uint8_t data);
    inline void static SPItransfer(uint8_t data) { SPITransfer(data); }  // For compatibility

    /** \brief
     * Turn the display off.
     *
     * \details
     * The display will clear and be put into a low power mode. This can be
     * used to extend battery life when a game is paused or when a sketch
     * doesn't require anything to be displayed for a relatively long period
     * of time.
     *
     * \see displayOn()
     */
    void static displayOff();

    /** \brief
     * Turn the display on.
     *
     * \details
     * Used to power up and reinitialize the display after calling
     * `displayOff()`.
     *
     * \see displayOff()
     */
    void static displayOn();

    /** \brief
     * Get the width of the display in pixels.
     *
     * \return The width of the display in pixels.
     */
    constexpr uint8_t static width() { return WIDTH; }

    /** \brief
     * Get the height of the display in pixels.
     *
     * \return The height of the display in pixels.
     */
    constexpr uint8_t static height() { return HEIGHT; }

    /** \brief
     * Get the current state of all buttons as a bitmask.
     *
     * \return A bitmask of the state of all the buttons.
     *
     * \details
     * The returned mask contains a bit for each button. For any pressed button,
     * its bit will be 1. For released buttons their associated bits will be 0.
     *
     * The following defined mask values should be used for the buttons:
     *
     * A_BUTTON, B_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, START_BUTTON, SELECT_BUTTON
     */
    uint8_t static buttonsState();

    /** \brief
     * Get the current display pixel color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setPixelColor() getBorderLineColor() setBorderLineColor() getBorderFillColor() setBorderFillColor()
     * getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    uint16_t static getPixelColor();

    /** \brief
     * Set the display pixel color.
     *
     * \param color The color to set.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before `begin()`
     * or `boot()`. Value will take effect on next call to `paintScreen()`.
     *
     * \note
     * The file `colors.h` contains helpful utilities for creating 12-bit 444-formatted
     * color values.
     *
     * \see getPixelColor() getBorderLineColor() setBorderLineColor() getBorderFillColor()
     * setBorderFillColor() getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    void static setPixelColor(uint16_t color);

    /** \brief
     * Get the current display background color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBackgroundColor() getPixelColor() setPixelColor() getBorderLineColor()
     * setBorderLineColor() getBorderFillColor() setBorderFillColor() setColorTheme()
     *
     */
    uint16_t static getBackgroundColor();

    /** \brief
     * Set the display background color.
     *
     * \param color The color to set.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before `begin()`
     * or `boot()`. Value will take effect on next call to `paintScreen()`.
     *
     * \note
     * The file `colors.h` contains helpful utilities for creating 12-bit 444-formatted
     * color values.
     *
     * \see getBackgroundColor() getPixelColor() setPixelColor() getBorderLineColor()
     * setBorderLineColor() getBorderFillColor() setBorderFillColor() setColorTheme()
     */
    void static setBackgroundColor(uint16_t color);

    /** \brief
     * Get the current display border line color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBorderLineColor() getBorderFillColor() setBorderFillColor() getPixelColor()
     * setPixelColor() getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    uint16_t static getBorderLineColor();

    /** \brief
     * Set the display border line color.
     *
     * \param color The color to set.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before `begin()`
     * or `boot()`.
     *
     * \note
     * The file `colors.h` contains helpful utilities for creating 12-bit 444-formatted
     * color values.
     *
     * \see getBorderLineColor() getBorderFillColor() setBorderFillColor() getPixelColor()
     * setPixelColor() getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    void static setBorderLineColor(uint16_t color);

    /** \brief
     * Get the current display border fill color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBorderFillColor() getBorderLineColor() setBorderLineColor() getPixelColor()
     * setPixelColor() getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    uint16_t static getBorderFillColor();

    /** \brief
     * Set the display border fill color.
     *
     * \param color The color to set.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before `begin()`
     * or `boot()`.
     *
     * \note
     * The file `colors.h` contains helpful utilities for creating 12-bit 444-formatted
     * color values.
     *
     * \see getBorderFillColor() getBorderLineColor() setBorderLineColor() getPixelColor()
     * setPixelColor() getBackgroundColor() setBackgroundColor() setColorTheme()
     */
    void static setBorderFillColor(uint16_t color);


    /** \brief
     * Set the color theme.
     *
     * \param theme The theme to set.
     *
     * \details
     * Colors in `theme` must be 12-bit 444-formatted RGB color values. May be called before
     * `begin()` or `boot()`. Setting will take effect on next call to `paintScreen()`.
     *
     * \note
     * The file `colors.h` contains helpful utilities for creating 12-bit 444-formatted
     * color values.
     *
     * \see getPixelColor() setPixelColor() getBackgroundColor() setBackgroundColor()
     * getBorderLineColor() setBorderLineColor() getBorderFillColor() setBorderFillColor()
     */
    void static setColorTheme(Theme theme);

    /** \brief
     * Originally meant to paint 8 pixels vertically to the display. This is not
     * implemented for dotMG.
     *
     * \param pixels A byte whose bits specify a vertical column of 8 pixels.
     *
     * \see paintScreen()
     */
    void static paint8Pixels(uint8_t pixels) {}

    /** \brief
     * Paints an entire image directly to the display from program memory.
     *
     * \param image A byte array in program memory representing the entire
     * contents of the display.
     *
     * \details
     * The contents of the specified array in program memory are written to the
     * display. Each byte in the array represents a vertical column of 8 pixels
     * with the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of each
     * row, to the bottom right. The size of the array must exactly match the
     * number of pixels in the display area.
     */
    void static paintScreen(const uint8_t *image);

    /** \brief
     * Paints an entire image directly to the display from an array in RAM.
     *
     * \param image A byte array in RAM representing the entire contents of
     * the display.
     * \param clear If `true` the array in RAM will be cleared to zeros upon
     * return from this function. If `false` the RAM buffer will remain
     * unchanged. (optional; defaults to `false`)
     *
     * \details
     * The contents of the specified array in RAM are written to the display.
     * Each byte in the array represents a vertical column of 8 pixels with
     * the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of
     * each row, to the bottom right. The size of the array must exactly
     * match the number of pixels in the display area.
     *
     * If parameter `clear` is set to `true` the RAM array will be cleared to
     * zeros after its contents are written to the display.
     */
    void static paintScreen(uint8_t image[], bool clear = false);

    /** \brief
     * Blank the display screen by setting all pixels off.
     *
     * \details
     * All pixels on the screen will be written with a value of 0 to turn
     * them off.
     */
    void static blank();

    /** \brief
     * Swap pixel and background colors, or set back to normal.
     *
     * \param inverse `true` will invert the display area. `false` will set the
     * display area to normal.
     *
     * \details
     * Setting will take effect on next call to `paintScreen()`.
     *
     * Once in inverted mode, the display will remain this way
     * until it is set back to non-inverted mode by calling this function with
     * `false`.
     */
    void static invert(bool inverse);

    /** \brief
     * Turn all display pixels on or display the buffer contents.
     *
     * \param on `true` turns all pixels on. `false` displays the contents
     * of the hardware display buffer.
     *
     * \details
     * Calling this function with a value of `true` will override the contents
     * of the hardware display buffer and turn all pixels on. The contents of
     * the hardware buffer will remain unchanged.
     *
     * Calling this function with a value of `false` will set the normal state
     * of displaying the contents of the hardware display buffer.
     *
     * \note
     * All pixels will be lit even if the display is in inverted mode.
     *
     * \see invert()
     */
    void static allPixelsOn(bool on);

    /** \brief
     * Flip the display vertically or set it back to normal.
     *
     * \param flipped `true` will set vertical flip mode. `false` will set
     * normal vertical orientation.
     *
     * \details
     * Calling this function with a value of `true` will cause the Y coordinate
     * to start at the bottom edge of the display instead of the top,
     * effectively flipping the display vertically.
     *
     * Once in vertical flip mode, it will remain this way until normal
     * vertical mode is set by calling this function with a value of `false`.
     *
     * \see flipHorizontal()
     */
    void static flipVertical(bool flipped);

    /** \brief
     * Flip the display horizontally or set it back to normal.
     *
     * \param flipped `true` will set horizontal flip mode. `false` will set
     * normal horizontal orientation.
     *
     * \details
     * Calling this function with a value of `true` will cause the X coordinate
     * to start at the right edge of the display instead of the left,
     * effectively flipping the display horizontally.
     *
     * Once in horizontal flip mode, it will remain this way until normal
     * horizontal mode is set by calling this function with a value of `false`.
     *
     * \see flipVertical()
     */
    void static flipHorizontal(bool flipped);

    /** \brief
     * Send a single command byte to the display.
     *
     * \param command The command byte to send to the display.
     *
     * \details
     * The display will be set to command mode then the specified command
     * byte will be sent. The display will then be set to data mode.
     * Multi-byte commands can be sent by calling this function multiple times.
     *
     * \note
     * Sending improper commands to the display can place it into invalid or
     * unexpected states, possibly even causing physical damage.
     */
    void static sendDisplayCommand(uint8_t command);
    inline void static sendLCDCommand(uint8_t command) { sendDisplayCommand(command); }  // For compatibility

    /** \brief
     * Set the light output of the RGB LED.
     *
     * \param red,green,blue The brightness value for each LED.
     *
     * \details
     * The RGB LED is actually individual red, green and blue LEDs placed
     * very close together in a single package. By setting the brightness of
     * each LED, the RGB LED can show various colors and intensities.
     * The brightness of each LED can be set to a value from 0 (fully off)
     * to 255 (fully on).
     *
     * \see setRGBled(uint8_t, uint8_t) digitalWriteRGB() freeRGBled()
     */
    void static setRGBled(uint8_t red, uint8_t green, uint8_t blue);

    /** \brief
     * Set the brightness of one of the RGB LEDs without affecting the others.
     *
     * \param color The name of the LED to set. The value given should be one
     * of RED_LED, GREEN_LED or BLUE_LED.
     *
     * \param val The brightness value for the LED, from 0 to 255.
     *
     * \note
     * In order to use this function, the 3 parameter version must first be
     * called at least once, in order to initialize the hardware.
     *
     * \details
     * This 2 parameter version of the function will set the brightness of a
     * single LED within the RGB LED without affecting the current brightness
     * of the other two. See the description of the 3 parameter version of this
     * function for more details on the RGB LED.
     *
     * \see setRGBled(uint8_t, uint8_t, uint8_t) digitalWriteRGB() freeRGBled()
     */
    void static setRGBled(uint8_t color, uint8_t val);


    /** \brief
     * Originally designed to relinquish analog control of the RGB LED. This is not
     * needed for dotMG and has no implementation.
     *
     * \see digitalWriteRGB() setRGBled()
     */
    void static freeRGBled();

    /** \brief
     * Set the RGB LEDs digitally, to either fully on or fully off.
     *
     * \param red,green,blue Use value RGB_ON or RGB_OFF to set each LED.
     *
     * \details
     * This 3 parameter version of the function will set each LED either on or off,
     * to set the RGB LED to 7 different colors at their highest brightness or turn
     * it off.
     *
     * The colors are as follows:
     *
     *     RED LED   GREEN_LED   BLUE_LED   COLOR
     *     -------   ---------  --------    -----
     *     RGB_OFF    RGB_OFF    RGB_OFF    OFF
     *     RGB_OFF    RGB_OFF    RGB_ON     Blue
     *     RGB_OFF    RGB_ON     RGB_OFF    Green
     *     RGB_OFF    RGB_ON     RGB_ON     Cyan
     *     RGB_ON     RGB_OFF    RGB_OFF    Red
     *     RGB_ON     RGB_OFF    RGB_ON     Magenta
     *     RGB_ON     RGB_ON     RGB_OFF    Yellow
     *     RGB_ON     RGB_ON     RGB_ON     White
     *
     * \note
     * \parblock
     * Using the RGB LED in analog mode will prevent digital control of the
     * LED. To restore the ability to control the LED digitally, use the
     * `freeRGBled()` function.
     * \endparblock
     *
     * \see digitalWriteRGB(uint8_t, uint8_t) setRGBled() freeRGBled()
     */
    void static digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue);

    /** \brief
     * Set one of the RGB LEDs digitally, to either fully on or fully off.
     *
     * \param color The name of the LED to set. The value given should be one
     * of RED_LED, GREEN_LED or BLUE_LED.
     *
     * \param val Indicates whether to turn the specified LED on or off.
     * The value given should be RGB_ON or RGB_OFF.
     *
     * \details
     * This 2 parameter version of the function will set a single LED within
     * the RGB LED either fully on or fully off. See the description of the
     * 3 parameter version of this function for more details on the RGB LED.
     *
     * \see digitalWriteRGB(uint8_t, uint8_t, uint8_t) setRGBled() freeRGBled()
     */
    void static digitalWriteRGB(uint8_t color, uint8_t val);

    /** \brief
     * Initialize the Arduboy's hardware.
     *
     * \details
     * This function initializes the display, buttons, etc.
     *
     * This function is called by begin() so isn't normally called within a
     * sketch. However, in order to free up some code space, by eliminating
     * some of the start up features, it can be called in place of begin().
     * The functions that begin() would call after boot() can then be called
     * to add back in some of the start up features, if desired.
     * See the README file or documentation on the main page for more details.
     *
     * \see Arduboy2Base::begin()
     */
    void static boot();

    /** \brief
     * Disables current game and instead waits forever at a blank screen.
     * Similarly to `flashlight()`, useful if you don't want your game to
     * run while uploading.
     *
     * \details
     * If the UP button is held when this function is entered, the RGB LED
     * will be lit and the sketch will remain in a tight loop.
     *
     * It is intended to replace the `flashlight()` function when more
     * program space is required. If possible, it is more desirable to use
     * `flashlight()`, so that the actual flashlight feature isn't lost.
     *
     * \see Arduboy2Base::flashlight() boot()
     */
    void static safeMode();

    /** \brief
     * Delay for the number of milliseconds, specified as a 16 bit value.
     *
     * \param ms The delay in milliseconds.
     *
     * \details
     * This function works the same as the Arduino `delay()` function except
     * the provided value is 16 bits long, so the maximum delay allowed is
     * 65535 milliseconds (about 65.5 seconds). Using this function instead
     * of Arduino `delay()` will save a few bytes of code.
     */
    void static delayShort(uint16_t ms) __attribute__ ((noinline));

    /** \brief
     * This function was originally intented to allow USB uploads when USB code
     * was eliminated on the original Arduboy. Since this isn't necessary on
     * dotMG, it simply disables interrupts and waits forever.
     */
    void static exitToBootloader();

  protected:
    // internals
    void static bootDisplay();
    void static bootPins();
};

#endif
