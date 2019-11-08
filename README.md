# Arduboy2DotMG Library

The **Arduboy2DotMG** library is a fork of the [Arduboy2 library](https://github.com/MLXXXp/Arduboy2), with modifications made to allow compatibility with the Modmatic dotMG DIY handheld game console.

Notable differences between this library and the original Arduboy2 library:

- (TBD)

## Library documentation

Comments in the library header files are formatted for the [Doxygen](http://www.doxygen.org) document generation system. The HTML files generated using the configuration file _extras/Doxyfile_ can be found at:

(TBD)

A generated PDF file can be found at:

(TBD)

## Installation

1. Download this repo as a ZIP file to a known location on your machine.

2. Install the library using the following guide:

[Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries#toc5)

## Start up features

The *begin()* function, used to initialize the library, includes features that are intended to be available to all sketches using the library (unless the sketch developer has chosen to disable one or more of them to free up some code space):

### The boot logo

At the start of the sketch, the **ARDUBOY** logo scrolls down from the top of the screen to the center.

The RGB LED lights red then green then blue while the logo is scrolling. For users who do not wish to have the RGB LED flash during the boot logo sequence, a flag can be set in system EEPROM to have it remain off. The included *SetSystemEEPROM* example sketch can be used to set this flag.

A user settable *unit name* of up to 6 characters can be saved in system EEPROM memory. If set, this name will be briefly displayed at the bottom of the boot logo screen, after the logo stops scrolling down. This feature is only available if the *Arduboy2* class is used, not the *Arduboy2Base* class. This is because it requires the text display functions, which are only available in the *Arduboy2* class. A flag in system EEPROM controls whether or not the *unit name* is displayed on the boot logo screen, regardless of whether the *unit name* itself has been set. The included *SetSystemEEPROM* example sketch can be used to set both the *unit name* and this flag.

Once the logo display sequence completes, the sketch continues.

For developers who wish to quickly begin testing, or impatient users who want to go strait to playing their game, the boot logo sequence can be bypassed by holding the *RIGHT* button while powering up, and then releasing it. Alternatively, the *RIGHT* button can be pressed while the logo is scrolling down.

For users who wish to always disable the displaying of the boot logo sequence on boot up, a flag in system EEPROM is available for this. The included *SetSystemEEPROM* example sketch can be used to set this flag.

### "Flashlight" mode

If the *UP* button is pressed and held when the Arduboy is powered on, it enters *flashlight* mode. This turns the RGB LED fully on, and all the pixels of the screen are lit, resulting in a bright white light suitable as a small flashlight. (For an incorrect RGB LED, only the screen will light). To exit *flashlight* mode the Arduboy must be restarted.

*Flashlight* mode is also sometimes useful to allow uploading of new sketches.

### Audio mute control

Pressing and holding the *B* button when powering on will enter *System Control* mode. The RGB LED will light blue (red for an incorrect LED) to indicate that you are in *system control* mode. You must continue to hold the *B* button to remain in this mode. The only *system control* function currently implemented is *audio mute control*.

Pressing the *UP* button (while still holding *B*) will set a flag in system EEPROM indicating *audio enabled*. The RGB LED will flash green once (off for an incorrect LED) to indicate this action.

Pressing the *DOWN* button (while still holding *B*) will set the flag to *audio disabled* (muted). The RGB LED will flash red once (blue for an incorrect LED) to indicate this action.

Releasing the *B* button will exit *system control* mode and the sketch will continue.

Note that the audio control feature only sets a flag in EEPROM. Whatever code actually produces the sound must use the *audio.enabled()* function to check and honor the mute state. Audio libraries written with the Arduboy system in mind, such as the available *ArduboyPlaytune* and *ArduboyTones*, should do this. However, be aware that for some sketches, which don't use the Arduboy2 or other compliant library and generate sounds in their own way, this method of muting sound may not work.

## Using the library in a sketch

As with most libraries, to use Arduboy2 in your sketch you must include its header file at the start:

```cpp
#include <Arduboy2DotMG.h>
```

You must then create an Arduboy2 class object:

```cpp
Arduboy2 arduboy;
```

Naming the object *arduboy* has become somewhat of a standard, but you can use a different name if you wish.

To initialize the library, you must call its *begin()* function. This is usually done at the start of the sketch's *setup()* function:

```cpp
void setup()
{
  arduboy.begin();
  // more setup code follows, if required
}
```

The rest of the Arduboy2 functions will now be available for use.

If you wish to use the Sprites class functions you must create a Sprites object:

```cpp
Sprites sprites;
```

Sample sketches have been included with the library as examples of how to use it. To load an example, for examination and uploading to the Arduboy, using the Arduino IDE menus select:

`File > Examples > Arduboy2`

More information on writing sketches for the Arduboy can be found in the [Arduboy Community Forum](http://community.arduboy.com/).

### Using EEPROM in a sketch

The Arduboy2 library reserves an area at the start of EEPROM for storing system information, such as the current audio mute state and the Unit Name and Unit ID. A sketch **must not** use this reserved area for its own purposes. A sketch may use any EEPROM past this reserved area. The first EEPROM address available for sketch use is given as the defined value *EEPROM_STORAGE_SPACE_START*

### Audio control functions

The library includes an Arduboy2Audio class. This class provides functions to enable and disable (mute) sound and also save the current mute state so that it remains in effect over power cycles and after loading a different sketch. It doesn't contain anything to actually produce sound.

The Arduboy2Base class, and thus the Arduboy2 class, creates an Arduboy2Audio class object named *audio*, so a sketch doesn't need to create its own Arduboy2Audio object.

Example:

```cpp
#include <Arduboy2DotMG.h>

Arduboy2 arduboy;

// Arduboy2Audio functions can be called as follows:
  arduboy.audio.on();
  arduboy.audio.off();
```

### Simple tone generation

The *BeepPin1* and *BeepPin2* classes are available to generate simple square wave tones. These classes are documented in file *Arduboy2BeepDotMG.h*. Also, *BeepDemo* is included as one of the example sketches, which demonstrates basic use.

### Ways to make more code space available to sketches

#### Sound effects and music

If all you want is to play single tones, using the built in *BeepPin1* or *BeepPin2* classes will be very efficient.

If you want to be able to play sequences of tones or background music, using the [*ArduboyTones*](https://github.com/MLXXXp/ArduboyTones) library will be more code efficient than using [*ArduboyPlaytune*](https://github.com/Arduboy/ArduboyPlayTune) or most other sound libraries compatible with the Arduboy. *ArduboyTones* even produces less code than the [Arduino built in *tone()* function](https://www.arduino.cc/en/Reference/Tone). You'll have to decide on the appropriate library or functions you use to generate sound, based on the features required and how much memory you want it to use.

#### Remove the text functions

If your sketch doesn't use any of the functions for displaying text, such as *setCursor()* and *print()*, you can remove them. You could do this if your sketch generates whatever text it requires by some other means. Removing the text functions frees up code by not including the font table and some code that is always pulled in by inheriting the [Arduino *Print* class](http://playground.arduino.cc/Code/Printclass).

To eliminate text capability in your sketch, when creating the library object simply use the *Arduboy2Base* class instead of *Arduboy2*:

For example, if the object will be named *arduboy*:

Replace

```cpp
Arduboy2 arduboy;
```

with

```cpp
Arduboy2Base arduboy;
```

#### Remove boot up features

As previously described, the *begin()* function includes features that are intended to be available to all sketches during boot up. However, if you're looking to gain some code space, you can call *boot()* instead of *begin()*. This will initialize the system but not include any of the extra boot up features. If desired, you can then add back in any of these features by calling the functions that perform them. You will have to trade off between the desirability of having a feature and how much memory you can recover by not including it.

A good way to use *boot()* instead of *begin()* is to copy the code from the body of the *begin()* function, in file *Arduboy2.cpp*, into your sketch and then edit it to retain the *boot()* call and any feature calls desired. Remember to add the class object name in front of each function call, since they're now being called from outside the class itself. If your sketch uses sound, it's a good idea to keep the call to *audio.begin()*.

There are a few functions provided that are roughly equivalent to the standard functions used by *begin()* but which use less code space.

- *bootLogoCompressed()*, *bootLogoSpritesSelfMasked()*, *bootLogoSpritesOverwrite()*, *bootLogoSpritesBSelfMasked()* and *bootLogoSpritesBOverwrite()* will do the same as *bootLogo()* but will use *drawCompressed()*, or *Sprites* / *SpritesB* class *drawSelfMasked()* or *drawOverwrite()* functions respectively, instead of *drawBitmask()*, to render the logo. If the sketch uses one of these functions, then using the boot logo function that also uses it may reduce code size. It's best to try each of them to see which one produces the smallest size.
- *bootLogoText()* can be used in place *bootLogo()* in the case where the sketch uses text functions. It renders the logo as text instead of as a bitmap (so doesn't look as good).
- *safeMode()* can be used in place of *flashlight()* for cases where it's needed to allow uploading a new sketch when the bootloader "magic key" problem is an issue. It only lights the red RGB LED, so you don't get the bright light that is the primary purpose of *flashlight()*.

#### Use the SpritesB class instead of Sprites

The *SpritesB* class has functions identical to the *Sprites* class. The difference is that *SpritesB* is optimized for small code size rather than execution speed. If you want to use the sprites functions, and the slower speed of *SpritesB* doesn't affect your sketch, you may be able to use it to gain some code space.

Even if the speed is acceptable when using *SpritesB*, you should still try using *Sprites*. In some cases *Sprites* will produce less code than *SpritesB*, notably when only one of the functions is used.

You can easily switch between using *Sprites* or *SpritesB* by using one or the other to create an object instance:

```cpp
Sprites sprites;  // Use this to optimize for execution speed
SpritesB sprites; // Use this to (likely) optimize for code size
```

----------

## What's different from Arduboy library V1.1

A main goal of Arduboy2 is to provide ways in which more code space can be freed for use by large sketches. Another goal is to allow methods other than the *tunes* functions to be used to produce sounds. Arduboy2 remains substantially compatible with [Arduboy library V1.1](https://github.com/Arduboy/Arduboy/releases/tag/v1.1), which was the latest stable release at the time of the fork. Arduboy2 is based on the code targeted for Arduboy library V1.2, which was still in development and unreleased at the time it was forked.

Main differences between Arduboy2 and Arduboy V1.1 are:

- The *ArduboyTunes* subclass, which provided the *tunes.xxx()* functions, has been removed. It's functionality is available in a separate [*ArduboyPlaytune* library](https://github.com/Arduboy/ArduboyPlayTune). By removing these functions, more code space may become available because interrupt routines and other support code was being compiled in even if a sketch didn't make use them. Another benefit is that without the automatic installation of timer interrupt service routines, other audio generating functions and libraries, that need access to the same interrupts, can now be used. Removal of the *tunes* functions is the main API incompatibility with Arduboy V1.1. Sketches written to use *tunes* functions will need some minor modifications in order to make them work with Arduboy2 plus ArduboyPlaytune, [ArduboyTones](https://github.com/MLXXXp/ArduboyTones), or some other audio library.
- Arduboy library V1.1 uses timer 1 for the *tunes* functions. This causes problems when attempting to control the Arduboy's RGB LED using PWM, such as with *setRGBled()*, because it also requires timer 1. Since the *tunes* functionality has been removed from Arduboy2, there are no problems with using the RGB LED (except those caused by the RGB LED being incorrectly installed). Of course, using an external library that uses timer 1, such as *ArduboyPlaytune*, may reintroduce the problems. However, using a library that doesn't use timer 1, such as *ArduboyTones*, is now an option.
- The code to generate text output, using *setCursor()*, *print()*, etc., can be removed to free up code space, if a sketch doesn't use any text functions. The *Arduboy2* class includes the text functions but using the *Arduboy2Base* class instead will eliminate them. With text functions included, the font table and some support functions are always compiled in even if not used. The API for using text functions is the same as Arduboy V1.1 with some additional functions added:
 - *setTextColor()* and *setTextBackground()* allow for printing black text on a white background.
 - *getCursorX()* and *getCursorY()* allow for determining the current text cursor position.
 - The *clear()* function will now reset the text cursor to home position 0, 0.
- A new feature has been added which allows the *audio on/off* flag in system EEPROM to be configured by the user when the sketch starts. The flag is used by the Arduboy and Arduboy2 *audio* subclass, along with external sound functions and libraries, to provide a standardized sound mute capability. See the information above, under the heading *Audio mute control*, for more details.
- The *color* parameter, which is the last parameter for most of the drawing functions, has been made optional and will default to WHITE if not included in the call. This doesn't save any code but has been added as a convenience, since most drawing functions are called with WHITE specified.
- A new function *digitalWriteRGB()* has been added to control the RGB LED digitally instead of using PWM. This uses less code if just turning the RGB LEDs fully on or off is all that's required.
- The *beginNoLogo()* function is not included. This function could be used in Arduboy V1.1 in place of *begin()* to suppress the displaying of the ARDUBOY logo and thus free up the code that it required. Instead, Arduboy2 allows a sketch to call *boot()* and then add in any extra features that *begin()* provides by calling their functions directly after *boot()*, if desired.
- The *ArduboyCore* and *ArduboyAudio* base classes, previously only available to, and used to derive, the *Arduboy* class, have been made publicly available for the benefit of developers who may wish to use them as the base of an entirely new library. This change doesn't affect the existing API.

As of version 2.1.0 functionality from the [Team A.R.G.](http://www.team-arg.org/) *Arglib* library has been added:

- The sprite drawing functions, collision detection functions, and button handling functions that Team A.R.G. incorporated from the [ArduboyExtra](https://github.com/yyyc514/ArduboyExtra) project. The *poll()* function was renamed *pollButtons()* for clarity. The *Sprites* class doesn't require a parameter for the constructor, whereas in *Arglib* a pointer to an Arduboy class object is required.
- The *drawCompressed()* function, which allows compressed bitmaps to be drawn. Saving bitmaps in compressed form may reduce overall sketch size.

Team A.R.G. has now migrated all of their games and demos to use the Arduboy2 library.

----------

