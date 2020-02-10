/**
 * @file Arduboy2CoreDotMG.h
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#ifndef ARDUBOY2_CORE_DOTMG_H
#define ARDUBOY2_CORE_DOTMG_H

#include <Arduino.h>

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

// 12-bit ('444') color values

#define COLOR_BLACK      0x0000
#define COLOR_WHITE      0x0FFF
#define COLOR_GRAY       0x0AAA
#define COLOR_RED        0x0F00
#define COLOR_GREEN      0x00F0
#define COLOR_BLUE       0x000F
#define COLOR_CYAN       0x00FF
#define COLOR_MAGENTA    0x0F0F
#define COLOR_YELLOW     0x0FF0
#define	COLOR_ORANGE     0x0F40

// Builds a 12-bit color value from individual 4-bit RGB channel values
#define COLOR(r, g, b) ((((r) & 0xF) << 8) | (((g) & 0xF) << 4) | ((b) & 0xF))

// Computes the complement of a color
#define INVERT(color)  (COLOR_WHITE - (color))


// ----- SPI/DMA configuration -----

#define SPI_24MHZ_CLOCK     // Comment out to default to 12 MHz

#define SPI_SETTINGS        SPISettings(12000000, MSBFIRST, SPI_MODE0)
#define SPI_SERCOM          SERCOM1
#define DMA_TRIGGER_SRC     SERCOM1_DMAC_ID_TX
#define DMA_CHAN            0u


// ----- Pins -----

#define IO_PORT             (&(PORT->Group[PORTA]))

#define PIN_DISP_SS         16
#define MASK_DISP_SS        digitalPinToBitMask(PIN_DISP_SS)

#define PIN_DISP_DC         17
#define MASK_DISP_DC        digitalPinToBitMask(PIN_DISP_DC)

#define MASK_SPI_MOSI       digitalPinToBitMask(PIN_SPI_MOSI)
#define MASK_SPI_SCK        digitalPinToBitMask(PIN_SPI_SCK)

#define PIN_BUTTON_A        10
#define MASK_BUTTON_A       digitalPinToBitMask(PIN_BUTTON_A)

#define PIN_BUTTON_B        9
#define MASK_BUTTON_B       digitalPinToBitMask(PIN_BUTTON_B)

#define PIN_BUTTON_UP       5
#define MASK_BUTTON_UP      digitalPinToBitMask(PIN_BUTTON_UP)

#define PIN_BUTTON_DOWN     6
#define MASK_BUTTON_DOWN    digitalPinToBitMask(PIN_BUTTON_DOWN)

#define PIN_BUTTON_LEFT     7
#define MASK_BUTTON_LEFT    digitalPinToBitMask(PIN_BUTTON_LEFT)

#define PIN_BUTTON_RIGHT    8
#define MASK_BUTTON_RIGHT   digitalPinToBitMask(PIN_BUTTON_RIGHT)

#define PIN_BUTTON_START    14
#define MASK_BUTTON_START   digitalPinToBitMask(PIN_BUTTON_START)

#define PIN_BUTTON_SELECT   15
#define MASK_BUTTON_SELECT  digitalPinToBitMask(PIN_BUTTON_SELECT)

#define PIN_SPEAKER         PIN_DAC0


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
     * Initializes SPI transfers for the display.
     *
     * \details
     * Acquires the SPI bus and lowers the CS pin of the display, allowing
     * SPI data to be sent to it. Use before one or more calls to SPITransfer(). Other SPI devices
     * cannot receive data while the CS pin of the display is low.
     *
     * \see endDisplaySPI() SPITransfer()
     */
    void static beginDisplaySPI();

    /** \brief
     * Terminates SPI transfers for the display.
     *
     * \details
     * Raises the CS pin of the display, allowing other SPI devices to accept data.
     * Use after one or mor calls to SPITransfer().
     *
     * \see beginDisplaySPI() SPITransfer()
     */
    void static endDisplaySPI();

    /** \brief
     * Acquires usage of the SPI bus.
     *
     * \details
     * This library uses direct memory access (DMA) to write SPI data to the
     * display asynchronously. To avoid race conditions, any other usage of
     * the SPI bus must wait for the SPI bus to be free.
     *
     * To use the SPI bus for a device, use this method and the `freeSPI()`
     * method as shown below:
     *
     * \code{.cpp}
     * arduboy.acquireSPI();
     * // lower CS pin for SPI device here
     * SPI.beginTransaction(mySettings);
     * SPI.transfer(myData);  // or arduboy.SPITransfer(myData)
     * ...
     * SPI.endTransaction();
     * // raise CS pin for SPI device here
     * arduboy.freeSPI();
     *
     * \see freeSPI()
     */
    void static acquireSPI();

    /** \brief
     * Frees the SPI bus for use.
     *
     * \see acquireSPI();
     */
    void static freeSPI();

    /** \brief
     * Transfer a byte over SPI.
     *
     * \param data The byte to be sent over SPI.
     *
     * \details
     * Transfer one byte over the SPI bus and wait for the transfer to
     * complete. The SPI bus should first be acquired before sending.
     *
     * \see acquireSPI() freeSPI()
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
     * \note
     * The previous call to `displayOff()` will have caused the display's
     * buffer contents to be lost. The display will have to be re-painted,
     * which is usually done by calling `display()`.
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
     * Get the current display border line color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBorderLineColor() getBorderFillColor() setBorderFillColor() getPixelColor() setPixelColor()
     * getBackgroundColor() setBackgroundColor()
     */
    uint16_t static getBorderLineColor();

    /** \brief
     * Set the display border line color.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before begin()
     * or boot().
     *
     * \note
     * You can use the COLOR(r, g, b) macro to convert individual color channels
     * to a 12-bit 444-formatted color value.
     *
     * \see getBorderLineColor() getBorderFillColor() setBorderFillColor()  getPixelColor() setPixelColor()
     * getBackgroundColor() setBackgroundColor()
     */
    void static setBorderLineColor(uint16_t color);

    /** \brief
     * Get the current display border fill color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBorderFillColor() getBorderLineColor() setBorderLineColor() getPixelColor() setPixelColor()
     * getBackgroundColor() setBackgroundColor()
     */
    uint16_t static getBorderFillColor();

    /** \brief
     * Set the display border fill color.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before begin()
     * or boot().
     *
     * \note
     * You can use the COLOR(r, g, b) macro to convert individual color channels
     * to a 12-bit 444-formatted color value.
     *
     * \see getBorderFillColor() getBorderLineColor() setBorderLineColor() getPixelColor() setPixelColor()
     * getBackgroundColor() setBackgroundColor()
     */
    void static setBorderFillColor(uint16_t color);

    /** \brief
     * Get the current display pixel color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setPixelColor() getBorderLineColor() setBorderLineColor() getBorderFillColor() setBorderFillColor()
     * getBackgroundColor() setBackgroundColor()
     */
    uint16_t static getPixelColor();

    /** \brief
     * Set the display pixel color.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before begin()
     * or boot(). Value will take effect on next call to display().
     *
     * \note
     * You can use the COLOR(r, g, b) macro to convert individual color channels
     * to a 12-bit 444-formatted color value.
     *
     * \see getPixelColor() getBorderLineColor() setBorderLineColor() getBorderFillColor() setBorderFillColor()
     * getBackgroundColor() setBackgroundColor()
     */
    void static setPixelColor(uint16_t color);

    /** \brief
     * Get the current display background color.
     *
     * \details
     * Returns a 12-bit 444-formatted RGB color value.
     *
     * \see setBackgroundColor() getPixelColor() setPixelColor() getBorderLineColor() setBorderLineColor()
     * getBorderFillColor() setBorderFillColor()
     *
     */
    uint16_t static getBackgroundColor();

    /** \brief
     * Set the display background color.
     *
     * \details
     * Color must be a 12-bit 444-formatted RGB color value. May be called before begin()
     * or boot(). Value will take effect on next call to display().
     *
     * \note
     * You can use the COLOR(r, g, b) macro to convert individual color channels
     * to a 12-bit 444-formatted color value.
     *
     * \see getBackgroundColor() getPixelColor() setPixelColor() getBorderLineColor() setBorderLineColor()
     * getBorderFillColor() setBorderFillColor()
     */
    void static setBackgroundColor(uint16_t color);


    /** \brief
     * Paint 8 pixels vertically to the display.
     *
     * \param pixels A byte whose bits specify a vertical column of 8 pixels.
     *
     * \details
     * A byte representing a vertical column of 8 pixels is written to the
     * display at the current page and column address. The address is then
     * incremented. The page/column address will wrap to the start of the
     * display (the top left) when it increments past the end (lower right).
     *
     * The least significant bit represents the top pixel in the column.
     * A bit set to 1 is lit, 0 is unlit (background).
     *
     * Example:
     *
     *     X = lit pixels, . = unlit pixels
     *
     *     blank()                          paint8Pixels() 0xFF, 0, 0xF0, 0, 0x0F
     *     v TOP LEFT corner (8x9)          v TOP LEFT corner
     *     . . . . . . . . (page 1)         X . . . X . . . (page 1)
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . . (end of page 1)  X . X . . . . . (end of page 1)
     *     . . . . . . . . (page 2)         . . . . . . . . (page 2)
     */
    void static paint8Pixels(uint8_t pixels);

    /** \brief
     * Paints an entire image directly to the display from program memory.
     *
     * \param image A byte array in program memory representing the entire
     * contents of the display.
     *
     * \details
     * The contents of the specified array in program memory is written to the
     * display. Each byte in the array represents a vertical column of 8 pixels
     * with the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of each
     * row, to the bottom right. The size of the array must exactly match the
     * number of pixels in the entire display.
     *
     * \see paint8Pixels()
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
     * The contents of the specified array in RAM is written to the display.
     * Each byte in the array represents a vertical column of 8 pixels with
     * the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of
     * each row, to the bottom right. The size of the array must exactly
     * match the number of pixels in the entire display.
     *
     * If parameter `clear` is set to `true` the RAM array will be cleared to
     * zeros after its contents are written to the display.
     *
     * \see paint8Pixels()
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
     * Invert the entire display or set it back to normal.
     *
     * \param inverse `true` will invert the display. `false` will set the
     * display to no-inverted.
     *
     * \details
     * Calling this function with a value of `true` will set the display to
     * inverted mode. A pixel with a value of 0 will be on and a pixel set to 1
     * will be off.
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
     * to start at the left edge of the display instead of the right,
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
    void static bootSPI();
    void static bootDisplay();
    void static bootPins();
    void static bootPowerSaving() {}
};

#endif
